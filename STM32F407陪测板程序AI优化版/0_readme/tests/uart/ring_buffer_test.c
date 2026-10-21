#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usart_ring_buff.h"

uint32_t test_primask;
static stRingBuff_usart1 *interrupt_ring;
static int inject_byte;

void test_irq_restore(uint32_t value)
{
    test_primask = value;
    if (value == 0U && inject_byte)
    {
        inject_byte = 0;
        assert(write_onebyte_to_ringbuffer_for_usart1(interrupt_ring, 'B'));
    }
}

int main(void)
{
    struct { unsigned long before; stRingBuff_usart1 ring; unsigned char after[2048]; } guarded;
    stRingBuff generic;
    char data[USART1_RING_BUFFER_LEN + 2U];
    unsigned int i, round;
    memset(&guarded, 0xA5, sizeof(guarded));
    ring_buffer_init_for_usart1(&guarded.ring);
    assert(guarded.before == 0xA5A5A5A5UL);
    for (i = 0; i < sizeof(guarded.after); ++i) assert(guarded.after[i] == 0xA5);
    for (i = 0; i < USART1_RING_BUFFER_LEN; ++i) assert(guarded.ring.buffer[i] == 0);
    assert(get_ringbuffer_length_for_usart1(&guarded.ring) == 0);

    /* Full capacity, overflow accounting and no writes beyond returned length. */
    for (i = 0; i < USART1_RING_BUFFER_LEN; ++i)
        assert(write_onebyte_to_ringbuffer_for_usart1(&guarded.ring, (char)i));
    assert(!write_onebyte_to_ringbuffer_for_usart1(&guarded.ring, '!'));
    assert(get_ringbuffer_dropped_for_usart1(&guarded.ring) == 1);
    memset(data, 0x6A, sizeof(data));
    assert(read_ringbuffer_count_for_usart1(&guarded.ring, data, sizeof(data)) == USART1_RING_BUFFER_LEN);
    for (i = 0; i < USART1_RING_BUFFER_LEN; ++i) assert((unsigned char)data[i] == (unsigned char)i);
    assert(data[USART1_RING_BUFFER_LEN] == 0x6A);

    /* Repeated partial chunks cross both index wrap boundaries. */
    for (round = 0; round < 100; ++round)
    {
        for (i = 0; i < 37; ++i) assert(write_onebyte_to_ringbuffer_for_usart1(&guarded.ring, (char)(round+i)));
        assert(read_ringbuffer_count_for_usart1(&guarded.ring, data, 13) == 13);
        for (i = 0; i < 13; ++i) assert((unsigned char)data[i] == (unsigned char)(round+i));
        assert(read_ringbuffer_count_for_usart1(&guarded.ring, data, sizeof(data)) == 24);
        for (i = 0; i < 24; ++i) assert((unsigned char)data[i] == (unsigned char)(round+i+13));
    }

    /* An ISR publishes another byte immediately after a task releases its lock. */
    interrupt_ring = &guarded.ring;
    assert(write_onebyte_to_ringbuffer_for_usart1(interrupt_ring, 'A'));
    inject_byte = 1;
    assert(read_ringbuffer_count_for_usart1(interrupt_ring, data, sizeof(data)) == 2);
    assert(data[0] == 'A' && data[1] == 'B');
    assert(get_ringbuffer_length_for_usart1(interrupt_ring) == 0);

    /* Nested callers must retain the original interrupt mask. */
    test_primask = 1;
    assert(write_onebyte_to_ringbuffer_for_usart1(interrupt_ring, 'X'));
    assert(test_primask == 1);
    assert(read_one_byte_from_ringbuffer_for_usart1(interrupt_ring, data));
    assert(test_primask == 1);
    test_primask = 0;
    assert(read_ringbuffer_count_for_usart1(NULL, data, 1) == 0);
    assert(read_ringbuffer_count_for_usart1(interrupt_ring, NULL, 1) == 0);
    assert(read_ringbuffer_count_for_usart1(interrupt_ring, data, 0) == 0);

    ring_buffer_init(&generic);
    write_ring_bufer(&generic, "abc", 3);
    assert(read_ringbuffer_count(&generic, data, sizeof(data)) == 3);
    assert(memcmp(data, "abc", 3) == 0);
    puts("PASS: initialization bounds, FIFO/wrap, overflow, short reads, ISR handoff, PRIMASK restore");
    return 0;
}
