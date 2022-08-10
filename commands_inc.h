// if macro DEFINITION is defined:
// - create definition (.c file)
// - else create declaration (.h file)

#ifdef DEFINITION
#define C(a, b) const uint8_t a = b
#else
#define C(a, b) extern const uint8_t a
#endif
#undef DEFINITION

#include <stdio.h>

C(NOP, 0x00);
C(SWRESET, 0x01);
C(RDDIDIF, 0x04);
C(RDDST, 0x09);
C(RDDPM, 0x0a);
C(RDMADCTL, 0x0b);
C(RDDCOLMOD, 0x0c);
C(RDDIM, 0x0d);
C(RDDSM, 0x0e);
C(RDDSDR, 0x0f);
C(SLPIN, 0x10);
C(SLPOUT, 0x11);
C(PTLON, 0x12);
C(NORON, 0x13);
C(DINVOFF, 0x20);
C(DINVON, 0x21);
C(GAMSET, 0x26);
C(DISPOFF, 0x28);
C(DISPON, 0x29);
C(CASET, 0x2a);
C(PASET, 0x2b);
C(RAMWR, 0x2c);
C(RGBSET, 0x2d);
C(RAMRD, 0x2e);
C(PTLAR, 0x30);
C(VSCRDEF, 0x33);
C(TEOFF, 0x34);
C(TEON, 0x35);
C(MADCTL, 0x36);
C(VSCRSADD, 0x37);
C(IDMOFF, 0x38);
C(IDMON, 0x39);
C(PIXSET, 0x3a);
C(WRITE_MEMORY_CONTINUE, 0x3c);
C(READ_MEMORY_CONTINUE, 0x3e);
C(SET_TEAR_SCANLINE, 0x44);
C(GET_SCANLINE, 0x45);
C(WRDISBV, 0x51);
C(RDDISBV, 0x52);
C(WRCTRLD, 0x53);
C(RDCTRLD, 0x54);
C(WRCABC, 0x55);
C(RDCABC, 0x56);
C(WRCABC_MIN, 0x5e);
C(RDCABC_MIN, 0x5f);
C(RDID1, 0xda);
C(RDID2, 0xdb);
C(RDID3, 0xdc);

// Level 2 Commands
C(IFMODE, 0xb0);
C(FRMCTR1, 0xb1);
C(FRMCTR2, 0xb2);
C(FRMCTR3, 0xb3);
C(INVTR, 0xb4);
C(PRCTR, 0xb5);
C(DISCTRL, 0xb6);
C(ETMOD, 0xb7);
C(BL_CTRL1, 0xb8);
C(BL_CTRL2, 0xb9);
C(BL_CTRL3, 0xba);
C(BL_CTRL4, 0xbb);
C(BL_CTRL5, 0xbc);
C(BL_CTRL7, 0xbe);
C(BL_CTRL8, 0xbf);
C(PWCTRL1, 0xc0);
C(PWCTRL2, 0xc1);
C(VMCTRL1, 0xc5);
C(VMCTRL2, 0xc7);
C(NVMWR, 0xd0);
C(NVMPKEY, 0xd1);
C(RDNVM, 0xd2);
C(RDID4, 0xd3);
C(PGAMCTRL, 0xe0);
C(NGAMCTRL, 0xe1);
C(DGAMCTRL1, 0xe2);
C(DGAMCTRL2, 0xe3);
C(IFCTL, 0xf6);

// Extended register commands
C(PWCTRLA, 0xcb);
C(PWCTRLB, 0xcf);
C(DTIMCTRLA, 0xe8);
C(DTIMCTRLB, 0xea);
C(POSCTRL, 0xed);
C(ENABLE_3G, 0xf2);
C(PUMPRATIOCTRL, 0xf7);

#undef C