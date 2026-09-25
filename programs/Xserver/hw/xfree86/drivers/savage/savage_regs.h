/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/savage/savage_regs.h,v 1.11 2002/05/14 20:19:52 alanh Exp $ */

#ifndef _SAVAGE_REGS_H
#define _SAVAGE_REGS_H

/* These are here until xf86PciInfo.h is updated. */

#ifndef PCI_CHIP_S3TWISTER_P
#define PCI_CHIP_S3TWISTER_P	0x8d01
#endif
#ifndef PCI_CHIP_S3TWISTER_K
#define PCI_CHIP_S3TWISTER_K	0x8d02
#endif
#ifndef PCI_CHIP_SUPSAV_MX128
#define PCI_CHIP_SUPSAV_MX128		0x8c22
#define PCI_CHIP_SUPSAV_MX64		0x8c24
#define PCI_CHIP_SUPSAV_MX64C		0x8c26
#define PCI_CHIP_SUPSAV_IX128SDR	0x8c2a
#define PCI_CHIP_SUPSAV_IX128DDR	0x8c2b
#define PCI_CHIP_SUPSAV_IX64SDR		0x8c2c
#define PCI_CHIP_SUPSAV_IX64DDR		0x8c2d
#define PCI_CHIP_SUPSAV_IXCSDR		0x8c2e
#define PCI_CHIP_SUPSAV_IXCDDR		0x8c2f
#endif
#ifndef PCI_CHIP_PROSAVAGE_DDR
#define PCI_CHIP_PROSAVAGE_DDR	0x8d03
#define PCI_CHIP_PROSAVAGE_DDRK	0x8d04
#endif

#define S3_SAVAGE3D_SERIES(chip)  ((chip>=S3_SAVAGE3D) && (chip<=S3_SAVAGE_MX))

#define S3_SAVAGE4_SERIES(chip)   ((chip==S3_SAVAGE4) || (chip==S3_PROSAVAGE))

#define	S3_SAVAGE_MOBILE_SERIES(chip)	((chip==S3_SAVAGE_MX) || (chip==S3_SUPERSAVAGE))

#define S3_SAVAGE_SERIES(chip)    ((chip>=S3_SAVAGE3D) && (chip<=S3_SAVAGE2000))


/* Chip tags.  These are used to group the adapters into 
 * related families.
 */

enum S3CHIPTAGS {
    S3_UNKNOWN = 0,
    S3_SAVAGE3D,
    S3_SAVAGE_MX,
    S3_SAVAGE4,
    S3_PROSAVAGE,
    S3_SUPERSAVAGE,
    S3_SAVAGE2000,
    S3_LAST
};

#define BIOS_BSIZE			1024
#define BIOS_BASE			0xc0000

#define SAVAGE_NEWMMIO_REGBASE_S3	0x1000000  /* 16MB */
#define SAVAGE_NEWMMIO_REGBASE_S4	0x0000000 
#define SAVAGE_NEWMMIO_REGSIZE		0x0080000	/* 512kb */
#define SAVAGE_NEWMMIO_VGABASE		0x8000

#define BASE_FREQ			14.31818	

#define FIFO_CONTROL_REG		0x8200
#define MIU_CONTROL_REG			0x8204
#define STREAMS_TIMEOUT_REG		0x8208
#define MISC_TIMEOUT_REG		0x820c


#define SUBSYS_STAT_REG			0x8504

#define SRC_BASE			0xa4d4
#define DEST_BASE			0xa4d8
#define CLIP_L_R			0xa4dc
#define CLIP_T_B			0xa4e0
#define DEST_SRC_STR			0xa4e4
#define MONO_PAT_0			0xa4e8
#define MONO_PAT_1			0xa4ec

/* Constants for CR69. */

#define CRT_ACTIVE	0x01
#define LCD_ACTIVE	0x02
#define TV_ACTIVE	0x04
#define CRT_ATTACHED	0x10
#define LCD_ATTACHED	0x20
#define TV_ATTACHED	0x40


/*
 * reads from SUBSYS_STAT
 */
#define STATUS_WORD0            (INREG(0x48C00))
#define ALT_STATUS_WORD0        (INREG(0x48C60))
#define MAXLOOP			0xffffff
#define IN_SUBSYS_STAT()	(INREG(SUBSYS_STAT_REG))

#define MAXFIFO		0x7f00

/*
 * NOTE: don't remove 'VGAIN8(vgaCRIndex);'.
 * If not present it will cause lockups on Savage4.
 * Ask S3, why.
 */
#define VerticalRetraceWait(psav) \
{ \
        VGAIN8(psav->vgaIOBase+4); \
	VGAOUT8(psav->vgaIOBase+4, 0x17); \
	if (VGAIN8(psav->vgaIOBase+5) & 0x80) { \
		while ((VGAIN8(psav->vgaIOBase + 0x0a) & 0x08) == 0x08) {}; \
		while ((VGAIN8(psav->vgaIOBase + 0x0a) & 0x08) == 0x00) {}; \
	} \
}

#define	I2C_REG		0xa0
#define InI2CREG(psav,a)	\
{ \
    VGAOUT8(psav->vgaIOBase + 4, I2C_REG);	\
    a = VGAIN8(psav->vgaIOBase + 5);		\
}

#define OutI2CREG(psav,a)	\
{ \
    VGAOUT8(psav->vgaIOBase + 4, I2C_REG);	\
    VGAOUT8(psav->vgaIOBase + 5, a);		\
}
 
#define HZEXP_COMP_1		0x54
#define HZEXP_BORDER		0x58
#define HZEXP_FACTOR_IGA1	0x59

#define VTEXP_COMP_1		0x56
#define VTEXP_BORDER		0x5a
#define VTEXP_FACTOR_IGA1	0x5b

#define EC1_CENTER_ON	0x10
#define EC1_EXPAND_ON	0x0c

#endif /* _SAVAGE_REGS_H */
