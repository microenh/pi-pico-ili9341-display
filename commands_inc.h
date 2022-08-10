// if macro DEFINITION is defined:
// - create definition (.c file)
// - else create declaration (.h file)

#ifdef DEFINITION
#define C(a,b) const uint8_t a = b
#else
#define C(a,b) extern const uint8_t a
#endif
#undef DEFINITION

#include <stdio.h>

C(NOP, 0x00);
C(SWRESET, 0x01);
C(RDDID, 0x04);
C(RDDST, 0x09);
C(SLPIN, 0x10);
C(SLPOUT, 0x11);
C(PTLON, 0x12);
C(NORON, 0x13);
C(RDMODE, 0x0a);
C(RDMADCTL, 0x0b);
C(RDPIXFMT, 0x0c);
C(RDIMGFMT, 0x0d);
C(RDSELFDIAG, 0x0f);
C(INVOFF, 0x20);
C(INVON, 0x21);
C(GAMMASET, 0x26);
C(DISPLAY_OFF, 0x28);
C(DISPLAY_ON, 0x29);
C(SET_COLUMN, 0x2a);
C(SET_PAGE, 0x2b);
C(WRITE_RAM, 0x2c);
C(READ_RAM, 0x2e);
C(PTLAR, 0x30);
C(VSCRDEF, 0x33);
C(PIXFMT, 0x3a);
C(WRITE_DISPLAY_BRIGHTNESS, 0x51);
C(READ_DISPLAY_BRIGHTNESS, 0x52);
C(WRITE_CTRL_DISPLAY, 0x53);
C(READ_CTRL_DISPLAY, 0x54);
C(WRITE_CABC, 0x55);
C(READ_CABC, 0x56);
C(WRITE_CABC_MINIMUM, 0x5e);
C(READ_CABC_MINIMUM, 0x5f);
C(FRMCTR1, 0x81);
C(FRMCTR2, 0x82);
C(FRMCTR3, 0x83);
C(INVCTR, 0x84);
C(DFUNCTR, 0xb6);
C(PWCTR1, 0xc0);
C(PWCTR2, 0xc1);
C(PWCTRA, 0xcb);
C(PWCTRB, 0xcf);
C(VMCTR1, 0xc5);
C(VMCTR2, 0xc7);
C(RDID1, 0xda);
C(RDID2, 0xdb);
C(RDID3, 0xdc);
C(RDID4, 0xdd);
C(GMCTRP1, 0xe0);
C(GMCTRN1, 0xe1);
C(DTCA, 0xe8);
C(DTCB, 0xea);
C(POSC, 0xed);
C(ENABLE3G, 0xf2);
C(PUMPRC, 0xf7); 

#undef C