#include "stdafx.h"
#include "font.h"


#ifndef _WINDOWS
#include "GenericTypeDefs.h"
#pragma romdata overlay FONT_SECTION =0x00AC00
static rom 
#else
const
#endif
    WORD Arial_Data_L[] = {
      //0x0003,0x002,0x005,0x002,                                            // ������
    	0x2003,0x0000,0x000,0x0000,                       		 // ������    
    	0x2101,0x02F,                                                    // !
	0x2203,0x003,0x000,0x003,                                        // "
        0x2305,0x014,0x03E,0x014,0x03E,0x014,                            // #
        0x2405,0x024,0x02A,0x07F,0x02A,0x012,                            // $
	0x2508,0x002,0x025,0x012,0x008,0x004,0x012,0x029,0x010,          // %
	0x2607,0x010,0x02A,0x025,0x02D,0x012,0x038,0x024,                // &
	0x2701,0x003,                                                    // '
        0x2802,0x03E,0x041,                                              // (
        0x2902,0x041,0x03E,                                              // )
        0x2A05,0x014,0x008,0x03E,0x008,0x014,                            // *
	0x2B03,0x008,0x01C,0x008,                                        // +
        0x2C01,0x060,                                                    // ,
	0x2D03,0x008,0x008,0x008,                                        // -
	0x2E01,0x020,                                                    // .
	0x2F03,0x030,0x00C,0x003,                                        // /
	0x3004,0x01E,0x021,0x021,0x01E,                                  // 0
	0x3102,0x002,0x03F,                                              // 1
	0x3204,0x022,0x031,0x029,0x026,                                  // 2
	0x3304,0x012,0x021,0x025,0x01A,                                  // 3
	0x3405,0x018,0x014,0x012,0x03F,0x010,                            // 4
	0x3504,0x017,0x025,0x025,0x019,                                  // 5
	0x3604,0x01E,0x025,0x025,0x019,                                  // 6
	0x3704,0x001,0x039,0x005,0x003,                                  // 7
	0x3804,0x01A,0x025,0x025,0x01A,                                  // 8
	0x3904,0x026,0x029,0x029,0x01E,                                  // 9
	0x3A01,0x024,                                                    // :
        0x3B01,0x064,                                                    // ;
	0x3C03,0x008,0x014,0x022,                                        // <
	0x3D04,0x014,0x014,0x014,0x014,                                  // =
	0x3E03,0x022,0x014,0x008,                                        // >
	0x3F03,0x002,0x029,0x006,                                        // ?
        0x4008,0x01C,0x022,0x049,0x055,0x05D,0x051,0x04E,0x020,          // @
	0x4104,0x03E,0x009,0x009,0x03E,                                  // A
	0x4204,0x03F,0x025,0x025,0x01A,                                  // B
	0x4304,0x01E,0x021,0x021,0x012,                                  // C
	0x4404,0x03F,0x021,0x021,0x01E,                                  // D
	0x4504,0x03F,0x025,0x025,0x021,                                  // E
	0x4604,0x03F,0x005,0x005,0x001,                                  // F
	0x4704,0x01E,0x021,0x029,0x03A,                                  // G
	0x4804,0x03F,0x004,0x004,0x03F,                                  // H
	0x4903,0x021,0x03F,0x021,                                        // I
	0x4A04,0x010,0x020,0x020,0x01F,                                  // J
	0x4B04,0x03F,0x004,0x00A,0x031,                                  // K
	0x4C04,0x03F,0x020,0x020,0x020,                                  // L
	0x4D05,0x03F,0x002,0x004,0x002,0x03F,                            // M
        0x4E04,0x03F,0x004,0x008,0x03F,                                  // N
	0x4F06,0x00C,0x012,0x021,0x021,0x012,0x00C,                      // O
	0x5004,0x03F,0x009,0x009,0x006,                                  // P
	0x5106,0x00C,0x012,0x021,0x029,0x012,0x02C,                      // Q
	0x5204,0x03F,0x009,0x009,0x036,                                  // R
	0x5304,0x012,0x025,0x029,0x012,                                  // S
	0x5405,0x001,0x001,0x03F,0x001,0x001,                            // T
	0x5504,0x01F,0x020,0x020,0x01F,                                  // U
	0x5605,0x003,0x00C,0x030,0x00C,0x003,                            // V
        0x5707,0x003,0x00C,0x030,0x00E,0x030,0x00C,0x003,                // W
	0x5805,0x021,0x012,0x00C,0x012,0x021,                            // X
	0x5905,0x003,0x004,0x038,0x004,0x003,                            // Y
        0x5A04,0x031,0x029,0x025,0x023,                                  // Z
        0x5B02,0x07F,0x041,                                              // [
        0x5C03,0x003,0x00C,0x030,                                       /* \ */
        0x5D02,0x041,0x07F,                                              // ]
        0x5E03,0x002,0x001,0x002,                                        // ^
	0x5F05,0x080,0x080,0x080,0x080,0x080,                            // _
	0x6003,0x002,0x005,0x002,                                        // `	   
	0x6105,0x012,0x02A,0x02A,0x02A,0x03C,                            // a	   
	0x6204,0x03F,0x024,0x024,0x018,                                  // b
	0x6304,0x01C,0x022,0x022,0x022,                                  // c 
	0x6404,0x018,0x024,0x024,0x03F,                                  // d
	0x6505,0x01C,0x02A,0x02A,0x02A,0x004,                            // e
	0x6603,0x004,0x03F,0x005,                                        // f
	0x6704,0x09C,0x0A2,0x0A2,0x07E,                                  // g
	0x6804,0x03F,0x004,0x004,0x038,                                  // h
	0x6901,0x03D,                                                    // i
	0x6A02,0x080,0x27D,                                              // j
	0x6B04,0x03F,0x008,0x014,0x022,                                  // k
	0x6C01,0x03F,                                                    // l
	0x6D05,0x03E,0x002,0x03E,0x002,0x03C,                            // m
	0x6E04,0x03E,0x002,0x002,0x03C,                                  // n
	0x6F04,0x01C,0x022,0x022,0x01C,                                  // o
	0x7004,0x0FE,0x022,0x022,0x01C,                                  // p
	0x7104,0x01C,0x022,0x022,0x0FE,                                  // q
	0x7204,0x03E,0x004,0x002,0x002,                                  // r
	0x7304,0x024,0x02A,0x02A,0x012,                                  // s 
	0x7403,0x002,0x01F,0x022,                                        // t
	0x7504,0x01E,0x020,0x020,0x03E,                                  // u
	0x7605,0x006,0x018,0x020,0x018,0x006,                            // v
	0x7705,0x01E,0x020,0x01E,0x020,0x01E,                            // w
	0x7804,0x032,0x00C,0x00C,0x032,                                  // x
	0x7904,0x09E,0x0A0,0x0A0,0x07E,                                  // y
	0x7A04,0x032,0x02A,0x026,0x022,                                  // z
        0x7B03,0x008,0x036,0x041,                                        // {
	0x7C01,0x0FF,                                                    // |
        0x7D03,0x041,0x036,0x008,                                        // }
	0x7E05,0x008,0x004,0x004,0x008,0x004,                            // ~
	0xA005,0x03E,0x02B,0x02A,0x023,0x022,                            // �
	0xB005,0x01C,0x02B,0x02A,0x02B,0x004,                            // �
	0xC004,0x03E,0x009,0x009,0x03E,                                  // �
	0xC105,0x03F,0x025,0x025,0x025,0x019,                            // �
	0xC205,0x03F,0x025,0x025,0x025,0x01A,                            // �
	0xC305,0x03F,0x001,0x001,0x001,0x001,                            // �
	0xC406,0x060,0x03E,0x021,0x021,0x03E,0x060,                      // �
	0xC505,0x03F,0x025,0x025,0x021,0x021,                            // �
	0xC607,0x031,0x00A,0x004,0x03F,0x004,0x00A,0x031,                // �
	0xC704,0x021,0x025,0x025,0x01A,                                  // �
	0xC805,0x03F,0x010,0x008,0x004,0x03F,                            // �
	0xC905,0x03F,0x010,0x009,0x004,0x03F,                            // �
	0xCA04,0x03F,0x004,0x00A,0x031,                                  // �
        0xCB05,0x020,0x03E,0x001,0x001,0x03F,                            // �
	0xCC05,0x03F,0x002,0x004,0x002,0x03F,                            // �
	0xCD04,0x03F,0x004,0x004,0x03F,                                  // � 
	0xCE06,0x00C,0x012,0x021,0x021,0x012,0x00C,                      // �
	0xCF04,0x03F,0x001,0x001,0x03F,                                  // �
	0xD004,0x03F,0x009,0x009,0x006,                                  // �
	0xD104,0x01E,0x021,0x021,0x021,                                  // �
	0xD205,0x001,0x001,0x03F,0x001,0x001,                            // �
	0xD305,0x007,0x028,0x028,0x028,0x01F,                            // �
	0xD405,0x00E,0x011,0x03F,0x011,0x00E,                            // �
	0xD505,0x031,0x00A,0x004,0x00A,0x031,                            // �
	0xD605,0x03F,0x020,0x020,0x03F,0x060,                            // �
	0xD704,0x007,0x008,0x008,0x03F,                                  // �
	0xD805,0x03F,0x020,0x03F,0x020,0x03F,                            // �
	0xD906,0x03F,0x020,0x03F,0x020,0x03F,0x060,                      // �
	0xDA05,0x001,0x03F,0x024,0x024,0x018,                            // �
	0xDB05,0x03F,0x024,0x024,0x018,0x03F,                            // �
	0xDC04,0x03F,0x024,0x024,0x018,                                  // �
	0xDD05,0x012,0x021,0x025,0x025,0x01E,                            // �
	0xDE06,0x03F,0x008,0x01E,0x021,0x021,0x01E,                      // �
	0xDF05,0x036,0x009,0x009,0x009,0x03F,                            // �
	0xE005,0x012,0x02A,0x02A,0x02A,0x03C,                            // �
        0xE105,0x01E,0x025,0x025,0x025,0x019,                            // �
	0xE205,0x03E,0x02A,0x02A,0x02A,0x014,                            // �
	0xE304,0x03E,0x002,0x002,0x002,                                  // �
	0xE405,0x060,0x03C,0x022,0x03E,0x060,                            // �
	0xE505,0x01C,0x02A,0x02A,0x02A,0x024,                            // �
	0xE605,0x036,0x008,0x03E,0x008,0x036,                            // �
	0xE704,0x022,0x02A,0x02A,0x014,                                  // �
	0xE804,0x03E,0x010,0x008,0x03E,                                  // �
	0xE904,0x03C,0x011,0x009,0x03C,                                  // �
	0xEA04,0x03E,0x008,0x008,0x036,                                  // �
        0xEB05,0x020,0x03C,0x002,0x002,0x03E,                            // �
	0xEC05,0x03E,0x004,0x008,0x004,0x03E,                            // �
	0xED04,0x03E,0x008,0x008,0x03E,                                  // �
	0xEE04,0x01C,0x022,0x022,0x01C,                                  // �
	0xEF04,0x03E,0x002,0x002,0x03E,                                  // �
        0xF004,0x0FE,0x022,0x022,0x01C,                                  // �
	0xF104,0x01C,0x022,0x022,0x022,                                  // �
        0xF205,0x006,0x002,0x03E,0x002,0x006,                            // �
	0xF304,0x09E,0x0A0,0x0A0,0x07E,                                  // �
	0xF405,0x01C,0x022,0x07F,0x022,0x01C,                            // �
	0xF504,0x032,0x00C,0x00C,0x032,                                  // �
	0xF605,0x03E,0x020,0x020,0x03E,0x060,                            // �
	0xF704,0x00E,0x010,0x010,0x03E,                                  // �
	0xF805,0x03E,0x020,0x03E,0x020,0x03E,                            // �
	0xF906,0x03E,0x020,0x03E,0x020,0x03E,0x060,                      // �
	0xFA05,0x002,0x03E,0x024,0x024,0x018,                            // �
	0xFB05,0x03E,0x024,0x024,0x018,0x03E,                            // �
	0xFC04,0x03E,0x024,0x024,0x018,                                  // �
	0xFD05,0x014,0x022,0x02A,0x02A,0x01C,                            // �
	0xFE06,0x03E,0x008,0x01C,0x022,0x022,0x01C,                      // �
	0xFF05,0x02C,0x012,0x012,0x012,0x03E,                            // �
};                                                                       
 #ifndef _WINDOWS                                                         
 static rom                                                               
 #else                                                                    
 const
 #endif                                                                   
     WORD Arial_Data_B[] = {                                                      
     0x2002,0x000,0x000,		                                        // ������
     0x2104,0x00E,0x0DF,0x0DF,0x00E,                                             // !     
     0x2205,0x00B,0x007,0x000,0x00B,0x007,                                       // "     
     0x2306,0x0E4,0x03C,0x027,0x0E4,0x03C,0x027,                                 // #     
     0x2405,0x09C,0x132,0x3FF,0x132,0x0E4,                                       // $     
     0x2508,0x00F,0x009,0x0CF,0x030,0x00C,0x0F3,0x090,0x0F0,                     // %     
     0x2607,0x060,0x0F6,0x09F,0x099,0x0FF,0x0E6,0x0B0,                           // &     
     0x2702,0x00B,0x007,                                                         // '     
     0x2803,0x078,0x1FE,0x303,                                                   // (     
     0x2903,0x303,0x1FE,0x078,                                                   // )     
     0x2A05,0x014,0x008,0x03E,0x008,0x014,                                       // *     
     0x2B05,0x010,0x010,0x07C,0x010,0x010,                                       // +     
     0x2C02,0x2C0,0x1C0,                                                         // ,     
     0x2D03,0x010,0x010,0x010,                                                   // -     
     0x2E02,0x0C0,0x0C0,                                                         // .     
     0x2F03,0x0C0,0x03C,0x003,                                                   // /     
     0x3005,0x07E,0x0FF,0x081,0x0FF,0x07E,                                       // 0     
     0x3104,0x00C,0x006,0x0FF,0x0FF,                                             // 1     
     0x3205,0x0C2,0x0E3,0x0B1,0x09F,0x08E,                                       // 2     
     0x3305,0x042,0x0C3,0x089,0x0FF,0x076,                                       // 3     
     0x3406,0x030,0x02C,0x022,0x0FF,0x0FF,0x020,                                 // 4     
     0x3505,0x05F,0x0CF,0x089,0x0F9,0x071,                                       // 5     
     0x3605,0x07E,0x0FF,0x089,0x0FB,0x072,                                       // 6     
     0x3705,0x001,0x0F1,0x0FD,0x00F,0x003,                                       // 7     
     0x3805,0x076,0x0FF,0x089,0x0FF,0x076,                                       // 8     
     0x3905,0x04E,0x0DF,0x091,0x0FF,0x07E,                                       // 9     
     0x3A02,0x0CC,0x0CC,                                                         // :     
     0x3B02,0x2CC,0x1CC,                                                         // ;     
     0x3C05,0x018,0x018,0x024,0x024,0x042,                                       // <     
     0x3D05,0x028,0x028,0x028,0x028,0x028,                                       // =     
     0x3E05,0x042,0x024,0x024,0x018,0x018,                                       // >     
     0x3F05,0x002,0x0D3,0x0D9,0x00F,0x006,                                       // ?     
     0x400A,0x078,0x186,0x132,0x24D,0x285,0x2C5,0x2FD,0x28D,0x142,0x13C,         // @     
     0x4107,0x0E0,0x0FC,0x03F,0x023,0x03F,0x0FC,0x0E0,                           // A     
     0x4206,0x0FF,0x0FF,0x089,0x089,0x0FF,0x076,                                 // B     
     0x4307,0x03C,0x07E,0x0C3,0x081,0x081,0x0C3,0x042,                           // C     
     0x4406,0x0FF,0x0FF,0x081,0x081,0x0FF,0x07E,                                 // D     
     0x4506,0x0FF,0x0FF,0x089,0x089,0x089,0x081,                                 // E     
     0x4606,0x0FF,0x0FF,0x009,0x009,0x009,0x001,                                 // F     
     0x4707,0x03C,0x07E,0x0C3,0x081,0x091,0x0F3,0x072,                           // G     
     0x4806,0x0FF,0x0FF,0x008,0x008,0x0FF,0x0FF,                                 // H     
     0x4902,0x0FF,0x0FF,                                                         // I     
     0x4A05,0x040,0x0C0,0x080,0x0FF,0x07F,                                       // J     
     0x4B06,0x0FF,0x0FF,0x018,0x03E,0x0F7,0x0C1,                                 // K     
     0x4C06,0x0FF,0x0FF,0x080,0x080,0x080,0x080,                                 // L     
     0x4D09,0x0FF,0x0FF,0x00F,0x07C,0x0E0,0x07C,0x00F,0x0FF,0x0FF,               // M     
     0x4E06,0x0FF,0x0FF,0x00C,0x030,0x0FF,0x0FF,                                 // N     
     0x4F07,0x03C,0x07E,0x0C3,0x081,0x0C3,0x07E,0x03C,                           // O     
     0x5006,0x0FF,0x0FF,0x011,0x011,0x01F,0x00E,                                 // P     
     0x5108,0x03C,0x07E,0x0C3,0x0A1,0x0C3,0x0FE,0x13C,0x100,                     // Q     
     0x5207,0x0FF,0x0FF,0x011,0x031,0x07F,0x0CE,0x080,                           // R     
     0x5306,0x04E,0x0CF,0x099,0x099,0x0F3,0x072,                                 // S     
     0x5406,0x001,0x001,0x0FF,0x0FF,0x001,0x001,                                 // T     
     0x5506,0x07F,0x0FF,0x080,0x080,0x0FF,0x07F,                                 // U     
     0x5607,0x003,0x03F,0x0FC,0x0C0,0x0FC,0x03F,0x003,                           // V     
     0x570B,0x003,0x03F,0x0FC,0x0E0,0x03E,0x007,0x03E,0x0E0,0x0FC,0x03F,0x003,   // W     
     0x5806,0x0C3,0x0E7,0x03C,0x03C,0x0E7,0x0C3,                                 // X     
     0x5906,0x003,0x00F,0x0F8,0x0F8,0x00F,0x003,                                 // Y     
     0x5A07,0x0C1,0x0E1,0x0B1,0x099,0x08D,0x087,0x083,                           // Z     
     0x5B03,0x3FF,0x3FF,0x201,                                                   // [     
     0x5C03,0x003,0x03C,0x0C0,                                                  /* \ */   
     0x5D03,0x201,0x3FF,0x3FF,                                                   // ]     
     0x5E05,0x008,0x00E,0x003,0x00E,0x008,                                       // ^     
     0x5F06,0x200,0x200,0x200,0x200,0x200,0x200,                                 // _     
     0x6005,0x006,0x00F,0x009,0x00F,0x006,                                       // `	  
     0x6105,0x068,0x0F4,0x094,0x0FC,0x0F8,                                       // a	  
     0x6206,0x0FF,0x0FF,0x084,0x084,0x0FC,0x078,                                 // b     
     0x6305,0x078,0x0FC,0x084,0x0CC,0x048,                                       // c     
     0x6406,0x078,0x0FC,0x084,0x084,0x0FF,0x0FF,                                 // d     
     0x6506,0x078,0x0FC,0x094,0x094,0x0DC,0x058,                                 // e     
     0x6605,0x004,0x0FE,0x0FF,0x005,0x001,                                       // f     
     0x6706,0x178,0x2FC,0x284,0x284,0x3FC,0x1FC,                                 // g     
     0x6806,0x0FF,0x0FF,0x004,0x004,0x0FC,0x0F8,                                 // h     
     0x6902,0x0FD,0x0FD,                                                         // i     
     0x6A03,0x200,0x3FD,0x1FD,                                                   // j     
     0x6B06,0x0FF,0x0FF,0x030,0x078,0x0EC,0x084,                                 // k     
     0x6C02,0x0FE,0x0FF,                                                         // l     
     0x6D0A,0x0FC,0x0FC,0x004,0x004,0x0FC,0x0FC,0x004,0x004,0x0FC,0x0F8,         // m     
     0x6E06,0x0FC,0x0FC,0x004,0x004,0x0FC,0x0F8,                                 // n     
     0x6F06,0x078,0x0FC,0x084,0x084,0x0FC,0x078,                                 // o     
     0x7006,0x3FC,0x3FC,0x084,0x084,0x0FC,0x078,                                 // p     
     0x7106,0x078,0x0FC,0x084,0x084,0x3FC,0x3FC,                                 // q     
     0x7206,0x0FC,0x0FC,0x008,0x004,0x004,0x004,                                 // r     
     0x7306,0x058,0x0DC,0x0B4,0x0B4,0x0EC,0x068,                                 // s     
     0x7404,0x004,0x07E,0x0FF,0x084,                                             // t     
     0x7506,0x07C,0x0FC,0x080,0x080,0x0FC,0x0FC,                                 // u     
     0x7605,0x03C,0x0FC,0x0C0,0x0FC,0x03C,                                       // v     
     0x7709,0x01C,0x0FC,0x0E0,0x078,0x00C,0x078,0x0E0,0x0FC,0x01C,               // w     
     0x7805,0x0CC,0x0FC,0x030,0x0FC,0x0CC,                                       // x     
     0x7907,0x204,0x23C,0x3F8,0x1C0,0x0F8,0x03C,0x004,                           // y     
     0x7A05,0x0C4,0x0E4,0x0B4,0x09C,0x08C,                                       // z     
     0x7B04,0x020,0x1FE,0x3DF,0x201,                                             // {     
     0x7C01,0x3FF,                                                               // |     
     0x7D04,0x201,0x3DF,0x1FE,0x020,                                             // }     
     0x7E05,0x010,0x008,0x008,0x010,0x008,                                       // ~     
     0xA006,0x0FE,0x0FF,0x092,0x092,0x083,0x082,                                 // �     
     0xB006,0x078,0x0FD,0x094,0x094,0x0DD,0x058,                                 // �     
     0xC007,0x0E0,0x0FC,0x03F,0x023,0x03F,0x0FC,0x0E0,                           // �     
     0xC107,0x0FF,0x0FF,0x089,0x089,0x089,0x0F9,0x070,                           // �     
     0xC206,0x0FF,0x0FF,0x089,0x089,0x0FF,0x076,                                 // �     
     0xC305,0x0FF,0x0FF,0x001,0x001,0x001,                                       // �     
     0xC407,0x380,0x0FF,0x0FF,0x081,0x0FF,0x0FF,0x380,                           // �     
     0xC505,0x0FF,0x0FF,0x089,0x089,0x081,                                       // �     
     0xC60A,0x081,0x0F3,0x07E,0x008,0x0FF,0x0FF,0x008,0x07E,0x0F3,0x081,         // �     
     0xC706,0x042,0x0C3,0x089,0x089,0x0FF,0x076,                                 // �     
     0xC807,0x0FF,0x0FF,0x060,0x030,0x018,0x0FF,0x0FF,                           // �     
     0xC907,0x0FF,0x0FF,0x060,0x033,0x018,0x0FF,0x0FF,                           // �     
     0xCA06,0x0FF,0x0FF,0x008,0x07E,0x0F3,0x081,                                 // �     
     0xCB07,0x080,0x0FF,0x07F,0x001,0x001,0x0FF,0x0FF,                           // �     
     0xCC09,0x0FF,0x0FF,0x00F,0x07C,0x0E0,0x07C,0x00F,0x0FF,0x0FF,               // �     
     0xCD06,0x0FF,0x0FF,0x008,0x008,0x0FF,0x0FF,                                 // �     
     0xCE07,0x03C,0x07E,0x0C3,0x081,0x0C3,0x07E,0x03C,                           // �     
     0xCF07,0x0FF,0x0FF,0x001,0x001,0x001,0x0FF,0x0FF,                           // �     
     0xD006,0x0FF,0x0FF,0x011,0x011,0x01F,0x00E,                                 // �     
     0xD107,0x03C,0x07E,0x0C3,0x081,0x081,0x0C3,0x042,                           // �     
     0xD206,0x001,0x001,0x0FF,0x0FF,0x001,0x001,                                 // �     
     0xD306,0x003,0x08F,0x0FC,0x078,0x01F,0x007,                                 // �     
     0xD408,0x03C,0x07E,0x042,0x0FF,0x0FF,0x042,0x07E,0x03C,                     // �     
     0xD506,0x0C3,0x0E7,0x03C,0x03C,0x0E7,0x0C3,                                 // �     
     0xD607,0x0FF,0x0FF,0x080,0x080,0x0FF,0x0FF,0x380,                           // �     
     0xD707,0x00F,0x01F,0x010,0x010,0x010,0x0FF,0x0FF,                           // �     
     0xD80A,0x0FF,0x0FF,0x080,0x080,0x0FF,0x0FF,0x080,0x080,0x0FF,0x0FF,         // �     
     0xD90B,0x0FF,0x0FF,0x080,0x080,0x0FF,0x0FF,0x080,0x080,0x0FF,0x0FF,0x380,   // �     
     0xDA09,0x001,0x001,0x0FF,0x0FF,0x088,0x088,0x088,0x0F8,0x070,               // �     
     0xDB09,0x0FF,0x0FF,0x088,0x088,0x088,0x0F8,0x070,0x0FF,0x0FF,               // �     
     0xDC07,0x0FF,0x0FF,0x088,0x088,0x088,0x0F8,0x070,                           // �     
     0xDD07,0x024,0x066,0x0C3,0x089,0x0CB,0x07E,0x03C,                           // �     
     0xDE0A,0x0FF,0x0FF,0x018,0x03C,0x07E,0x0C3,0x081,0x0C3,0x07E,0x03C,         // �     
     0xDF07,0x080,0x0CE,0x07F,0x031,0x011,0x0FF,0x0FF,                           // �     
     0xE005,0x068,0x0F4,0x094,0x0FC,0x0F8,                                       // �     
     0xE106,0x07E,0x0FD,0x085,0x085,0x0FD,0x079,                                 // �     
     0xE206,0x0FC,0x0FC,0x094,0x094,0x0FC,0x068,                                 // �     
     0xE304,0x0FC,0x0FC,0x004,0x004,                                             // �     
     0xE407,0x380,0x0C0,0x0FC,0x084,0x0FC,0x0FC,0x380,                           // �     
     0xE506,0x078,0x0FC,0x094,0x094,0x0DC,0x058,                                 // �     
     0xE608,0x0C4,0x0EC,0x010,0x0FC,0x0FC,0x010,0x0EC,0x0C4,                     // �     
     0xE705,0x048,0x084,0x094,0x0FC,0x068,                                       // �     
     0xE806,0x0FC,0x0FC,0x060,0x018,0x0FC,0x0FC,                                 // �     
     0xE906,0x0FC,0x0FD,0x061,0x019,0x0FD,0x0FC,                                 // �     
     0xEA06,0x0FC,0x0FC,0x010,0x078,0x0EC,0x084,                                 // �     
     0xEB06,0x080,0x0FC,0x07C,0x004,0x0FC,0x0FC,                                 // �     
     0xEC07,0x0FC,0x0FC,0x030,0x0C0,0x030,0x0FC,0x0FC,                           // �     
     0xED06,0x0FC,0x0FC,0x010,0x010,0x0FC,0x0FC,                                 // �     
     0xEE06,0x078,0x0FC,0x084,0x084,0x0FC,0x078,                                 // �     
     0xEF06,0x0FC,0x0FC,0x004,0x004,0x0FC,0x0FC,                                 // �     
     0xF006,0x3FC,0x3FC,0x084,0x084,0x0FC,0x078,                                 // �     
     0xF105,0x078,0x0FC,0x084,0x0CC,0x048,                                       // �     
     0xF204,0x004,0x0FC,0x0FC,0x004,                                             // �     
     0xF307,0x204,0x23C,0x3F8,0x1C0,0x0F8,0x03C,0x004,                           // �     
     0xF408,0x078,0x0FC,0x084,0x3FF,0x3FF,0x084,0x0FC,0x078,                     // �     
     0xF505,0x0CC,0x0FC,0x030,0x0FC,0x0CC,                                       // �     
     0xF607,0x0FC,0x0FC,0x080,0x080,0x0FC,0x0FC,0x380,                           // �     
     0xF705,0x01C,0x03C,0x020,0x0FC,0x0FC,                                       // �     
     0xF808,0x0FC,0x0FC,0x080,0x0FC,0x0FC,0x080,0x0FC,0x0FC,                     // �     
     0xF909,0x0FC,0x0FC,0x080,0x0FC,0x0FC,0x080,0x0FC,0x0FC,0x380,               // �     
     0xFA06,0x004,0x0FC,0x0FC,0x090,0x0F0,0x060,                                 // �     
     0xFB07,0x0FC,0x0FC,0x090,0x0F0,0x060,0x0FC,0x0FC,                           // �     
     0xFC05,0x0FC,0x0FC,0x090,0x0F0,0x060,                                       // �     
     0xFD05,0x048,0x0CC,0x094,0x0FC,0x078,                                       // �     
     0xFE08,0x0FC,0x0FC,0x010,0x078,0x0FC,0x084,0x0FC,0x078,                     // �     
     0xFF07,0x080,0x0D8,0x07C,0x024,0x0FC,0x0FC                                  // �     
 };
#ifndef _WINDOWS
#pragma romdata
#endif
#define Arial_Data_L_Size 834
#define Arial_Data_B_Size 1104

int GetSymbolImage(BYTE symbol, WORD* Image, WORD* ImageSize, FONT CFont)
{
    WORD i;        
    WORD Begin =  0;
    WORD Size =  0;
    WORD_VAL c;
    c.Val = 0;
    memset(Image,0,(*ImageSize));
    switch(CFont){
        case ARIAL_L:        
            for(i = 0;i < Arial_Data_L_Size;){
                c.Val = Arial_Data_L[i];
                if(c.byte.HB == symbol) {
                    Begin = i + 1;
                    Size = c.byte.LB;
                    break;
                }
                i += c.byte.LB + 1;
            } 
            if((Size>(*ImageSize))||(symbol!=c.byte.HB)){
                (*ImageSize) = Size;
                return Size;
            } 
            for(i = 0; i<Size;i++){
                Image[i] = Arial_Data_L[Begin+i];
            }
            //memcpy(Image,&Arial_Data_L[Begin],Size*sizeof(WORD));
        break;
         case ARIAL_B:
             for(i = 0;i < Arial_Data_B_Size;){
                 c.Val = Arial_Data_B[i];
                 if(c.byte.HB == symbol) {
                     Begin = i + 1;
                     Size = c.byte.LB;
                     break;
                 }
                 i += c.byte.LB + 1;
             } 
             if((Size>(*ImageSize))||(symbol!=c.byte.HB)){
                 (*ImageSize) = Size;
                 return Size;
             } 
             for(i = 0; i<Size;i++){
                 Image[i] = Arial_Data_B[Begin+i];
             }   
             //memcpy(Image,&Arial_Data_B[Begin],Size*2);
        break;
        default:
            Begin = 0;
            Size = 0;
    }       
    (*ImageSize) = Size;
    return (*ImageSize);
}

