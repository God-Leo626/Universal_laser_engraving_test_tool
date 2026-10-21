#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f4xx.h"
#include "io_bit.h"
#include "stdbool.h"
#include "coolkit_logo.h"
//===========================================================================
/* PD4  PD5  PD7 PD11   PD12   PD13
 PD4: RD   PD5:WR  PD7:CS FSMC_NE1  PD11:RS FSMC A16   PD12: BL(L:亮    H:灭) PD13:RESET
 PD11 命令，数据选择使用的是地址 FSMC_A16, 片选使用的是 FSMC_NE1
 所以， 使用的是bank1 的 第一区
 FSMC 总共管理 1GB 空间，拥有 4 个存储块（Bank），每个bank是256M 每个区是64M
 BANK1是属于NOR/PSRAM，
 BANK2、
 BANK3属于NAND，
 BANK4属于PC卡。
第一区	FSMC_NE1	0X6000,0000~63FF,FFFF
第二区	FSMC_NE2	0X6400,0000~67FF,FFFF
第三区	FSMC_NE3	0X6800,0000~6BFF,FFFF
第四区	FSMC_NE4    0X6C00,0000~6FFF,FFFF
因为PD11 命令，数据选择使用的是地址 FSMC_A16，则对应的HADDR中的低17为应给是1 或者0
（16bit 宽度时，fsmc的地址向左移动一位）

FSMC 的 NOR FLASH 控制器支持同步和异步突发两种访问方式。选用同步突发访问方式时， 
FSMC 将 HCLK(系统时钟)分频后，发送给外部存储器作为同步时钟信号 FSMC_CLK。

对于异步突发访问方式， FSMC 主要设置 3 个时间参数：地址建立时间(ADDSET)、
数据建立时间(DATAST)和地址保持时间(ADDHLD)。 FSMC 综合了 SRAM／ROM、 
PSRAM 和 NOR Flash 产品的信号特点，定义了 4 种不同的异步时序模型。选用不同的时序模型时，需要设置不同的时序参数

时序模型	简单描述	时间参数
Mode1	SRAM/CRAM     时序	            DATAST、 ADDSET
ModeA	SRAM/CRAM OE 选通型时序	      DATAST、 ADDSET
Mode2/B	NOR FLASH 时序	              DATAST、 ADDSET
ModeC	NOR FLASH OE 选通型时序	      DATAST、 ADDSET
ModeD	延长地址保持时间的异步时序	      DATAST、 ADDSET、 ADDHLK


*/
//===========================================================================
#define LCD_BL             PDout(12)                   // 背光控制 L:亮    H:灭
#define LCD_RESET          PDout(13)
#define BANK1_LCD_DATA    ((uint32_t *)0x60020000)     //  写数据对应地址
#define BANK1_LCD_CMD     ((uint32_t *)0x60000000)	   //  写命令对应地址
//===========================================================================
#define PASS_FLAG      1     //测试pass的标志
#define FAIL_FLAG      2     //测试fail的标志
#define STDBY_FLAG     3     //测试未开始的标志
#define TESTING_FLAG   4     //测试进行中的标志
//===========================================================================
#define LCD_DIRECTION   3  // 5

#if  ((LCD_DIRECTION == 1)||(LCD_DIRECTION==3)||(LCD_DIRECTION==5)||(LCD_DIRECTION==7))

	 #define LCD_DEFAULT_WIDTH		         320  // X轴长度
	 #define LCD_DEFAULT_HEIGTH         	 480  // Y轴长度
 
#else 

	 #define LCD_DEFAULT_WIDTH		         480  // X轴长度
	 #define LCD_DEFAULT_HEIGTH         	 320  // Y轴长度
 
#endif


//===========================================================================
#define      Blue                     0x001F     //24位0x0000FF        纯蓝
#define      Cyan                     0x07FF     //24位0x00FFFF        青色
#define      Purple                   0x8010     //24位0x800080        紫色
#define      Green                    0x0400     //24位0x008000        纯绿
#define      Yellow                   0xFFE0     //24位0xFFFF00        纯黄
#define      Gold                     0xFEA0     //24位0xFFD700        金
#define      Tan                      0xD5B1     //24位0xD2B48C        晒黑
#define      White                    0xFFFF     //24位0xFFFFFF        纯白
#define      Gray                     0x8410     //24位0x808080        灰色
#define      Black                    0x0000     //24位0x000000        纯黑
#define      Red                      0xF800     //24位0xFF0000        纯红
#define      Brown                    0xA145     //24位0xA52A2A        棕色
#define      MossGreen                0X6BA4     //                    苔藓绿
#define      SlateBlue                0X6AD9     //岩蓝




    
#define      LightPink                0xFDB8     //24位0xFFB6C1        浅粉红
#define      Pink                     0xFE19     //24位0xFFC0CB        粉红
#define      Crimson                  0xD8A7     //24位0xDC143C        猩红
#define      LavenderBlush            0xFF9E     //24位0xFFF0F5        脸红的淡紫色
#define      PaleVioletRed            0xDB92     //24位0xDB7093        苍白的紫罗兰红色
#define      HotPink                  0xFB56     //24位0xFF69B4        热情的粉红
#define      DeepPink                 0xF8B2     //24位0xFF1493        深粉色
#define      MediumVioletRed          0xC0B0     //24位0xC71585        适中的紫罗兰红色
#define      Orchid                   0xDB9A     //24位0xDA70D6        兰花的紫色
#define      Thistle                  0xDDFB     //24位0xD8BFD8        蓟
#define      plum                     0xDD1B     //24位0xDDA0DD        李子
#define      Violet                   0xEC1D     //24位0xEE82EE        紫罗兰
#define      Magenta                  0xF81F     //24位0xFF00FF        洋红
#define      Fuchsia                  0xF81F     //24位0xFF00FF        灯笼海棠(紫红色)
#define      DarkMagenta              0x8811     //24位0x8B008B        深洋红色

#define      MediumOrchid             0xBABA     //24位0xBA55D3        适中的兰花紫
#define      DarkVoilet               0xD81C     //24位0x9400D3        深紫罗兰色
#define      DarkOrchid               0x9999     //24位0x9932CC        深兰花紫
#define      Indigo                   0x4810     //24位0x4B0082        靛青
#define      BlueViolet               0x895C     //24位0x8A2BE2        深紫罗兰的蓝色
#define      MediumPurple             0x939B     //24位0x9370DB        适中的紫色
#define      MediumSlateBlue          0x7B5D     //24位0x7B68EE        适中的板岩暗蓝灰色

#define      DarkSlateBlue            0x49F1     //24位0x483D8B        深岩暗蓝灰色
#define      Lavender                 0xE73F     //24位0xE6E6FA        熏衣草花的淡紫色
#define      GhostWhite               0xFFDF     //24位0xF8F8FF        幽灵的白色

#define      MediumBlue               0x0019     //24位0x0000CD        适中的蓝色
#define      MidnightBlue             0x18CE     //24位0x191970        午夜的蓝色
#define      DarkBlue                 0x0011     //24位0x00008B        深蓝色
#define      Navy                     0x0010     //24位0x000080        海军蓝
#define      RoyalBlue                0x435C     //24位0x4169E1        皇军蓝
#define      CornflowerBlue           0x64BD     //24位0x6495ED        矢车菊的蓝色
#define      LightSteelBlue           0xB63B     //24位0xB0C4DE        淡钢蓝
#define      LightSlateGray           0x7453     //24位0x778899        浅石板灰
#define      SlateGray                0x7412     //24位0x708090        石板灰
#define      DoderBlue                0xD700     //24位0x1E90FF        道奇蓝 （黄色）
#define      AliceBlue                0xF7DF     //24位0xF0F8FF        爱丽丝蓝
#define      SteelBlue                0x4416     //24位0x4682B4        钢蓝
#define      LightSkyBlue             0x867F     //24位0x87CEFA        淡蓝色
#define      SkyBlue                  0x867D     //24位0x87CEEB        天蓝色
#define      DeepSkyBlue              0x05FF     //24位0x00BFFF        深天蓝
#define      LightBLue                0xAEDC     //24位0xADD8E6        淡蓝
#define      PowDerBlue               0xB71C     //24位0xB0E0E6        火药蓝
#define      CadetBlue                0x5CF4     //24位0x5F9EA0        军校蓝
#define      Azure                    0xF7FF     //24位0xF0FFFF        蔚蓝色
#define      LightCyan                0xE7FF     //24位0xE1FFFF        淡青色
#define      PaleTurquoise            0xAF7D     //24位0xAFEEEE        苍白的绿宝石

#define      Aqua                     0x07FF     //24位0x00FFFF        水绿色
#define      DarkTurquoise            0x067A     //24位0x00CED1        深绿宝石
#define      DarkSlateGray            0x2A69     //24位0x2F4F4F        深石板灰
#define      DarkCyan                 0x0451     //24位000x8B8B        深青色
#define      Teal                     0x0410     //24位0x008080        水鸭色
#define      MediumTurquoise          0x4E99     //24位0x48D1CC        适中的绿宝石
#define      LightSeaGreen            0x2595     //24位0x20B2AA        浅海洋绿
#define      Turquoise                0x471A     //24位0x40E0D0        绿宝石
#define      Auqamarin                0xA500     //24位0x7FFFAA        绿玉\碧绿色
#define      MediumAquamarine         0x6675     //24位0x00FA9A        适中的碧绿色
#define      MediumSpringGreen        0x07D3     //24位0xF5FFFA        适中的春天的绿色
#define      MintCream                0xF7FF     //24位0x00FF7F        薄荷奶油
#define      SpringGreen              0x07EF     //24位0x3CB371        春天的绿色
#define      SeaGreen                 0x2C4A     //24位0x2E8B57        海洋绿
#define      Honeydew                 0xF7FE     //24位0xF0FFF0        蜂蜜
#define      LightGreen               0x9772     //24位0x90EE90        淡绿色
#define      PaleGreen                0x9FD3     //24位0x98FB98        苍白的绿色
#define      DarkSeaGreen             0x8DF1     //24位0x8FBC8F        深海洋绿
#define      LimeGreen                0x3666     //24位0x32CD32        酸橙绿
#define      Lime                     0x07E0     //24位0x00FF00        酸橙色
#define      ForestGreen              0x2444     //24位0x228B22        森林绿

#define      DarkGreen                0x0320     //24位0x006400        深绿色
#define      Chartreuse               0x7FE0     //24位0x7FFF00        查特酒绿
#define      LawnGreen                0x7FE0     //24位0x7CFC00        草坪绿
#define      GreenYellow              0xAFE5     //24位0xADFF2F        绿黄色
#define      OliveDrab                0x6C64     //24位0x556B2F        橄榄土褐色
#define      Beige                    0xF7BB     //24位0x6B8E23        米色(浅褐色)
#define      LightGoldenrodYellow     0xFFDA     //24位0xFAFAD2        浅秋麒麟黄
#define      Ivory                    0xFFFE     //24位0xFFFFF0        象牙
#define      LightYellow              0xFFFC     //24位0xFFFFE0        浅黄色

#define      Olive                    0x8400     //24位0x808000        橄榄
#define      DarkKhaki                0xBDAD     //24位0xBDB76B        深卡其布
#define      LemonChiffon             0xFFD9     //24位0xFFFACD        柠檬薄纱
#define      PaleGodenrod             0xA060     //24位0xEEE8AA        灰秋麒麟
#define      Khaki                    0xF731     //24位0xF0E68C        卡其布

#define      Cornislk                 0xC000     //24位0xFFF8DC        玉米色
#define      GoldEnrod                0xDD24     //24位0xDAA520        秋麒麟
#define      FloralWhite              0xFFDE     //24位0xFFFAF0        花的白色
#define      OldLace                  0xFFBC     //24位0xFDF5E6        老饰带
#define      Wheat                    0xF6F6     //24位0xF5DEB3        小麦色
#define      Moccasin                 0xFF36     //24位0xFFE4B5        鹿皮鞋
#define      Orange                   0xFD20     //24位0xFFA500        橙色
#define      PapayaWhip               0xFF7A     //24位0xFFEFD5        番木瓜
#define      BlanchedAlmond           0xFF59     //24位0xFFEBCD        漂白的杏仁
#define      NavajoWhite              0xFEF5     //24位0xFFDEAD        Navajo白
#define      AntiqueWhite             0xFF5A     //24位0xFAEBD7        古代的白色

#define      BrulyWood                0xB000     //24位0xDEB887        结实的树
#define      Bisque                   0xFF38     //24位0xFFE4C4        (浓汤)乳脂,番茄等
#define      DarkOrange               0xFC60     //24位0xFF8C00        深橙色
#define      Linen                    0xFF9C     //24位0xFAF0E6        亚麻布
#define      Peru                     0xCC27     //24位0xCD853F        秘鲁
#define      PeachPuff                0xFED7     //24位0xFFDAB9        桃色
#define      SandyBrown               0xF52C     //24位0xF4A460        沙棕色
#define      Chocolate                0xD343     //24位0xD2691E        巧克力
#define      SaddleBrown              0x8A22     //24位0x8B4513        马鞍棕色
#define      SeaShell                 0xFFBD     //24位0xFFF5EE        海贝壳
#define      Sienna                   0xA285     //24位0xA0522D        黄土赭色
#define      LightSalmon              0xFD0F     //24位0xFFA07A        浅鲜肉(鲑鱼)色
#define      Coral                    0xFBEA     //24位0xFF7F50        珊瑚
#define      OrangeRed                0xFA20     //24位0xFF4500        橙红色
#define      DarkSalmon               0xECAF     //24位0xE9967A        深鲜肉(鲑鱼)色
#define      Tomato                   0xFB08     //24位0xFF6347        番茄
#define      MistyRose                0xFF3C     //24位0xFFE4E1        薄雾玫瑰
#define      Salmon                   0xFC0E     //24位0xFA8072        鲜肉(鲑鱼)色
#define      Snow                     0xFFDF     //24位0xFFFAFA        雪
#define      LightCoral               0xF410     //24位0xF08080        淡珊瑚色
#define      RosyBrown                0xBC71     //24位0xBC8F8F        玫瑰棕色
#define      IndianRed                0xCAEB     //24位0xCD5C5C        印度红

#define      FireBrick                0xB104     //24位0xB22222        耐火砖
#define      DarkRed                  0x8800     //24位0x8B0000        深红色
#define      Maroon                   0x8000     //24位0x800000        栗色

#define      WhiteSmoke               0xF7BE     //24位0xF5F5F5        白烟
#define      Gainsboro                0xDEFB     //24位0xDCDCDC        Gainsboro
#define      LightGrey                0xD69A     //24位0xD3D3D3        浅灰色
#define      Silver                   0xC618     //24位0xC0C0C0        银白色
#define      DarkGray                 0xAD55     //24位0xA9A9A9        深灰色

#define      DimGray                  0x6B4D     //24位0x696969        暗淡的灰色


//===========================================================================
void lcd__write_register(u16 cmd,u16 value);
void lcd_bl_ctrl(bool on_off);
void lcd_set_direction( uint8_t mode_select );
void lcd_ili9488_set_windows(u16 x_start,u16 y_start,u16 x_end,u16 y_end);
void lcd_init(void);
void lcd_clear(unsigned int color);
void lcd_set_cursor(u16 x,u16 y);
void lcd_fill_color(u16 xstart,u16 ystart,u16 xend,u16 yend,unsigned int color);
void lcd_drawpoint(u16 x,u16 y,u16 color);
void lcd_draw_line(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);
void lcd_draw_rectangle(u16 x1,u16 y1,u16 x2,u16 y2,u16 color);
void lcd_draw_circle(u16 x0,u16 y0,u8 r,u16 color);
void lcd_show_image(u16 x ,u16 y,const unsigned char pic[]);
void display_ascii_string(u16 x,u16 y,u16 zk_size,u8 text[],u32 fc,u32 bc);
void lcd_display_test_result(u16 x_start,u16 y_start,u16 x_end,u16 y_end,u8 txt_size,u8 result);

#endif



