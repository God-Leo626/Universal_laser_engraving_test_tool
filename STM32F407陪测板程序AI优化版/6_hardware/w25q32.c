/* 两片W25Q32共用SPI1，select=1使用PA4，select=2使用PC1。
 * 启动阶段和串口2业务任务是唯一调用者，禁止从ISR或另一个任务并发调用。
 * 保留原void接口；读/写后查询w25q32_last_operation_ok，禁止忽略存储失败。
 * 容量4MB、页256B、扇区4096B；带擦除写保留扇区内其余字节。 */
#include "w25q32.h"
#include "systick_delay_freertos.h"
#include <string.h>
#include <stdint.h>

#define FLASH_SIZE       (4UL * 1024UL * 1024UL)
#define PAGE_SIZE        256U
#define SECTOR_SIZE      4096U
#define READY_TIMEOUT_MS 3000UL
#define CHIP_TIMEOUT_MS  120000UL

static bool operation_ok = true;

bool w25q32_last_operation_ok(void) { return operation_ok; }

static void deselect(void)
{
    W25Q32_CS_PA4 = 1;
    W25Q32_CS_PC1 = 1;
}

static bool begin_operation(u8 select)
{
    operation_ok = (select == 1 || select == 2);
    deselect();
    return operation_ok;
}

static bool select_chip(u8 select)
{
    deselect();
    if (!operation_ok) return false;
    if (select == 1) W25Q32_CS_PA4 = 0;
    else W25Q32_CS_PC1 = 0;
    return true;
}

static u8 transfer(u8 value)
{
    u8 result;
    if (!operation_ok) return 0xff;
    result = spi1_read_write_byte(value);
    if (!spi1_last_transfer_ok()) operation_ok = false;
    return result;
}

static void send_address(u32 address)
{
    transfer((u8)(address >> 16));
    transfer((u8)(address >> 8));
    transfer((u8)address);
}

static bool valid_range(const u8 *buffer, u32 address, u32 count)
{
    if (buffer == NULL || address >= FLASH_SIZE || count > FLASH_SIZE - address)
        operation_ok = false;
    return operation_ok;
}

static u8 read_status(u8 select)
{
    u8 status;
    if (!select_chip(select)) return 0xff;
    transfer(W25X_ReadStatusReg1);
    status = transfer(DUMMY_DATA);
    deselect();
    return status;
}

static bool wait_ready(u8 select, u32 timeout_ms)
{
    u32 elapsed = 0;
    while (operation_ok)
    {
        if ((read_status(select) & 1U) == 0) return operation_ok;
        if (elapsed++ >= timeout_ms) break;
        /* 明确上限，断线返回0xFF时不再永久卡住开机或配置。
         * 使用普通延时，启动阶段也可用；中断始终可响应。 */
        systick_delay_ms(1);
    }
    operation_ok = false;
    deselect();
    return false;
}

static void command(u8 select, u8 value)
{
    if (select_chip(select)) transfer(value);
    deselect();
}

static bool write_enable(u8 select)
{
    if (!wait_ready(select, READY_TIMEOUT_MS)) return false;
    command(select, W25X_WriteEnable);
    /* WEL未置位则终止，避免假装已经写入。 */
    if ((read_status(select) & 2U) == 0) operation_ok = false;
    return operation_ok;
}

u8 w25q32_flash_readSR(u8 select)
{
    if (!begin_operation(select)) return 0xff;
    return read_status(select);
}

void w25q32_spi_write_disable(u8 select)
{
    if (begin_operation(select)) command(select, W25X_WriteDisable);
}

u8 w25q32_flash_read_device_id(u8 select)
{
    u8 result;
    if (!begin_operation(select)) return 0xff;
    select_chip(select);
    transfer(W25X_DeviceID);
    transfer(DUMMY_DATA); transfer(DUMMY_DATA); transfer(DUMMY_DATA);
    result = transfer(DUMMY_DATA);
    deselect();
    return result;
}

u32 w25q32_flash_read_jedec_id(u8 select)
{
    u32 result;
    if (!begin_operation(select)) return 0xffffffUL;
    select_chip(select);
    transfer(W25X_JedecDeviceID);
    result = (u32)transfer(DUMMY_DATA) << 16;
    result |= (u32)transfer(DUMMY_DATA) << 8;
    result |= transfer(DUMMY_DATA);
    deselect();
    return result;
}

void wait_for_read_ready(u8 select)
{
    if (begin_operation(select)) wait_ready(select, READY_TIMEOUT_MS);
}

void w25q32_flash_waitfor_write_end(u8 select) { wait_for_read_ready(select); }

static void read_bytes(u8 *buffer, u32 address, u32 count, u8 select)
{
    u32 i;
    if (!wait_ready(select, READY_TIMEOUT_MS)) return;
    select_chip(select);
    transfer(W25X_ReadData);
    send_address(address);
    for (i = 0; i < count && operation_ok; ++i) buffer[i] = transfer(DUMMY_DATA);
    deselect();
}

void w25q32_flash_read(u8 *buffer, u32 address, u16 count, u8 select)
{
    if (!begin_operation(select) || !valid_range(buffer, address, count)) return;
    if (count != 0) read_bytes(buffer, address, count, select);
    if (!operation_ok) memset(buffer, 0xff, count);
}

static void write_page(u8 *buffer, u32 address, u16 count, u8 select)
{
    u16 i;
    if (!write_enable(select)) return;
    select_chip(select);
    transfer(W25X_PageProgram);
    send_address(address);
    for (i = 0; i < count && operation_ok; ++i) transfer(buffer[i]);
    deselect();
    wait_ready(select, READY_TIMEOUT_MS);
}

void w25q32_flash_page256_write_no_erase(u8 *buffer, u32 address, u16 count, u8 select)
{
    if (!begin_operation(select) || !valid_range(buffer, address, count)) return;
    if (count > PAGE_SIZE - address % PAGE_SIZE) { operation_ok = false; return; }
    if (count != 0) write_page(buffer, address, count, select);
}

static void write_pages(u8 *buffer, u32 address, u32 count, u8 select)
{
    while (count != 0 && operation_ok)
    {
        u16 chunk = (u16)(PAGE_SIZE - address % PAGE_SIZE);
        if (chunk > count) chunk = (u16)count;
        write_page(buffer, address, chunk, select);
        buffer += chunk; address += chunk; count -= chunk;
    }
}

void w25q32_flash_autopage_write_no_erase(u8 *buffer, u32 address, u32 count, u8 select)
{
    if (begin_operation(select) && valid_range(buffer, address, count))
        write_pages(buffer, address, count, select);
}

static void erase_sector(u32 sector, u8 select)
{
    if (!write_enable(select)) return;
    select_chip(select);
    transfer(W25X_SectorErase);
    send_address(sector * SECTOR_SIZE);
    deselect();
    wait_ready(select, READY_TIMEOUT_MS);
}

void w25q32_flash_sector4k_erase(u32 sector, u8 select)
{
    if (!begin_operation(select)) return;
    if (sector >= FLASH_SIZE / SECTOR_SIZE) { operation_ok = false; return; }
    erase_sector(sector, select);
}

void w25q32_flash_autosector_write_with_erase(u8 *buffer, u32 address, u32 count, u8 select)
{
    if (!begin_operation(select) || !valid_range(buffer, address, count)) return;
    /* SPI1_FLASH_BUF专用于本函数的扇区暂存，不得同时作为源数据。 */
    if ((uintptr_t)buffer >= (uintptr_t)SPI1_FLASH_BUF &&
        (uintptr_t)buffer < (uintptr_t)SPI1_FLASH_BUF + SECTOR_SIZE)
    { operation_ok = false; return; }
    while (count != 0 && operation_ok)
    {
        u32 sector = address / SECTOR_SIZE;
        u16 offset = (u16)(address % SECTOR_SIZE);
        u16 chunk = SECTOR_SIZE - offset;
        u16 i;
        bool erase_needed = false;
        if (chunk > count) chunk = (u16)count;
        read_bytes(SPI1_FLASH_BUF, sector * SECTOR_SIZE, SECTOR_SIZE, select);
        if (!operation_ok) break; /* 读取失败时绝不能擦除原配置。 */
        for (i = 0; i < chunk; ++i)
            if (SPI1_FLASH_BUF[offset + i] != 0xff) { erase_needed = true; break; }
        if (erase_needed)
        {
            memcpy(SPI1_FLASH_BUF + offset, buffer, chunk);
            erase_sector(sector, select);
            if (operation_ok) write_pages(SPI1_FLASH_BUF, sector * SECTOR_SIZE, SECTOR_SIZE, select);
        }
        else write_pages(buffer, address, chunk, select);
        buffer += chunk; address += chunk; count -= chunk;
    }
}

void w25q32_flash_erase_chip(u8 select)
{
    if (!begin_operation(select) || !write_enable(select)) return;
    command(select, W25X_ChipErase);
    wait_ready(select, CHIP_TIMEOUT_MS);
}

void w25q32_flash_powerdown(u8 select)
{
    if (begin_operation(select) && wait_ready(select, READY_TIMEOUT_MS))
        command(select, W25X_PowerDown);
    systick_delay_us(5);
}

void w25q32_flash_wakeup(u8 select)
{
    if (begin_operation(select)) command(select, W25X_ReleasePowerDown);
    systick_delay_us(5);
}
