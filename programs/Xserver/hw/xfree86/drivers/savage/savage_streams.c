#include "Xv.h"
#include "dix.h"
#include "dixstruct.h"

#include "savage_driver.h"
#include "savage_streams.h"

#define STREAMS_TRACE	4

static void SavageInitStreamsOld(ScrnInfoPtr pScrn);
static void SavageInitStreamsNew(ScrnInfoPtr pScrn);

static void OverlayParamInit(ScrnInfoPtr pScrn);
static void InitStreamsForExpansion(ScrnInfoPtr pScrn);

static void
SavageInitSecondaryStreamOld(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
    vgaHWPtr hwp;
    unsigned short vgaIOBase, vgaCRIndex, vgaCRReg;
    int offset = (psav->FBStart2nd - psav->FBStart);
    int colorkey = pScrn->colorKey;
    int pitch = pScrn->displayWidth * DEPTH_BPP(DEPTH_2ND(pScrn))/8;
    CARD8 cr92;

    hwp = VGAHWPTR(pScrn);
    vgaHWGetIOBase(hwp);
    vgaIOBase = hwp->IOBase;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

    OUTREG(COL_CHROMA_KEY_CONTROL_REG, 0x37000000 | (colorkey & 0xFF));
    OUTREG(CHROMA_KEY_UPPER_BOUND_REG, 0x00000000 | (colorkey & 0xFF));
    OUTREG(BLEND_CONTROL_REG, 0x05000000 );
    OUTREG(SSTREAM_CONTROL_REG, SSTREAMS_MODE(DEPTH_BPP(DEPTH_2ND(pScrn)))
	   << 24  | pScrn->displayWidth );
    OUTREG(SSTREAM_STRETCH_REG, 1 << 15);
    OUTREG(SSTREAM_VSCALE_REG, 1 << 15);
    OUTREG(SSTREAM_LINES_REG, pScrn->virtualY );
    OUTREG(SSTREAM_VINITIAL_REG, 0 );
    OUTREG(SSTREAM_FBADDR0_REG, offset & 0x1ffffff & ~BASE_PAD);
    OUTREG(SSTREAM_FBADDR1_REG, 0 );
    
    OUTREG(SSTREAM_STRIDE_REG, pitch);
    OUTREG(SSTREAM_WINDOW_START_REG, OS_XY(0,0));
    OUTREG(SSTREAM_WINDOW_SIZE_REG, 
	   OS_WH(pScrn->displayWidth, pScrn->virtualY));

    pitch = (pitch + 7) / 8;
    VGAOUT8(vgaCRIndex, 0x92);
    cr92 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, (cr92 & 0x40) | (pitch >> 8) | 0x80);
    VGAOUT8(vgaCRIndex, 0x93);
    VGAOUT8(vgaCRReg, pitch);
    OUTREG(STREAMS_FIFO_REG, 2 | 25 << 5 | 32 << 11);
}

static void
SavageInitSecondaryStreamNew(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
    vgaHWPtr hwp;
    unsigned short vgaIOBase, vgaCRIndex, vgaCRReg;
    int offset = (psav->FBStart2nd - psav->FBStart);
    int colorkey = pScrn->colorKey;
    int pitch = pScrn->displayWidth * DEPTH_BPP(DEPTH_2ND(pScrn))/8;
    CARD8 cr92;

    hwp = VGAHWPTR(pScrn);

    vgaHWGetIOBase(hwp);
    vgaIOBase = hwp->IOBase;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

    OUTREG( SEC_STREAM_CKEY_LOW, 0x47000000 | (colorkey & 0xFF));
    OUTREG( SEC_STREAM_CKEY_UPPER, 0x47000000 | (colorkey & 0xFF));
    OUTREG( BLEND_CONTROL, SSTREAMS_MODE(DEPTH_BPP(DEPTH_2ND(pScrn))) << 9
	    | 0x08 );
    if( psav->Chipset == S3_SAVAGE2000 )  {
	OUTREG(SEC_STREAM_HSCALING,  1 << 15);
	OUTREG(SEC_STREAM_HSCALE_NORMALIZE, 2048 << 16 );
	OUTREG(SEC_STREAM_VSCALING, 1 << 15);
    }  else  {
	OUTREG(SEC_STREAM_HSCALING,((pScrn->displayWidth &0xfff)<<20) | 1<<15);
	/* BUGBUG need to add 00040000 if src stride > 2048 */
	OUTREG(SEC_STREAM_VSCALING,((pScrn->virtualY &0xfff)<<20) | 1<<15);
    }

    OUTREG(SEC_STREAM_FBUF_ADDR0, offset & (0x7ffffff & ~BASE_PAD));
    OUTREG(SEC_STREAM_STRIDE, pitch);
    OUTREG(SEC_STREAM_WINDOW_START, OS_XY(0,0));
    /* ? width may need to be increased by 1 */
    OUTREG(SEC_STREAM_WINDOW_SZ, OS_WH(pScrn->displayWidth, pScrn->virtualY));
    
    pitch = (pitch + 7) / 8;
    VGAOUT8(vgaCRIndex, 0x92);
    cr92 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, (cr92 & 0x40) | (pitch >> 8) | 0x80);
    VGAOUT8(vgaCRIndex, 0x93);
    VGAOUT8(vgaCRReg, pitch);
}

void
SavageInitSecondaryStream(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);

    if( S3_SAVAGE_MOBILE_SERIES(psav->Chipset) ||
	(psav->Chipset == S3_SUPERSAVAGE) ||
	(psav->Chipset == S3_SAVAGE2000) )
	SavageInitSecondaryStreamNew(pScrn);
    else 
	SavageInitSecondaryStreamOld(pScrn);
}

void SavageInitStreamsOld(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
    unsigned long jDelta;
    unsigned long format = 0;

    /*
     * For the OLD streams engine, several of these registers
     * cannot be touched unless streams are on.  Seems backwards to me;
     * I'd want to set 'em up, then cut 'em loose.
     */

    xf86ErrorFVerb(STREAMS_TRACE, "SavageInitStreams\n" );

    /* Primary stream reflects the frame buffer. */

    if (psav->FBStart2nd) {
	jDelta = pScrn->displayWidth;
	format = 0 << 24;
    } else {
	jDelta = pScrn->displayWidth * (pScrn->bitsPerPixel + 7) / 8;
	switch( pScrn->depth ) {
	    case  8: format = 0 << 24; break;
	    case 15: format = 3 << 24; break;
	    case 16: format = 5 << 24; break;
	    case 24: format = 7 << 24; break;
	}
    }
    
    OUTREG( PSTREAM_WINDOW_START_REG, OS_XY(0,0) );
    OUTREG( PSTREAM_WINDOW_SIZE_REG, OS_WH(pScrn->displayWidth, pScrn->virtualY) );
    OUTREG( PSTREAM_FBADDR0_REG, pScrn->fbOffset );
    OUTREG( PSTREAM_FBADDR1_REG, 0 );
    OUTREG( PSTREAM_STRIDE_REG, jDelta );
    OUTREG( PSTREAM_CONTROL_REG, format );
    OUTREG( PSTREAM_FBSIZE_REG, jDelta * pScrn->virtualY >> 3 );

    OUTREG( COL_CHROMA_KEY_CONTROL_REG, 0 );
    OUTREG( SSTREAM_CONTROL_REG, 0 );
    OUTREG( CHROMA_KEY_UPPER_BOUND_REG, 0 );
    OUTREG( SSTREAM_STRETCH_REG, 0 );
    OUTREG( COLOR_ADJUSTMENT_REG, 0 );
    OUTREG( BLEND_CONTROL_REG, 1 << 24 );
    OUTREG( DOUBLE_BUFFER_REG, 0 );
    OUTREG( SSTREAM_FBADDR0_REG, 0 );
    OUTREG( SSTREAM_FBADDR1_REG, 0 );
    OUTREG( SSTREAM_FBADDR2_REG, 0 );
/*    OUTREG( SSTREAM_FBSIZE_REG, 0 ); */
    OUTREG( SSTREAM_STRIDE_REG, 0 );
    OUTREG( SSTREAM_VSCALE_REG, 0 );
    OUTREG( SSTREAM_LINES_REG, 0 );
    OUTREG( SSTREAM_VINITIAL_REG, 0 );
    OUTREG( SSTREAM_WINDOW_START_REG, OS_XY(0xfffe, 0xfffe) );
    OUTREG( SSTREAM_WINDOW_SIZE_REG, OS_WH(10,2) );
    OUTREG(STREAMS_FIFO_REG, 2 | 25 << 5 | 32 << 11);

    {
	vgaHWPtr hwp;
	unsigned short vgaIOBase, vgaCRIndex, vgaCRReg;
	unsigned char cr90;

	hwp = VGAHWPTR(pScrn);
	
	vgaHWGetIOBase(hwp);
	vgaIOBase = hwp->IOBase;
	vgaCRIndex = vgaIOBase + 4;
	vgaCRReg = vgaIOBase + 5;

	VGAOUT8(vgaCRIndex, 0x90);
	cr90 = VGAIN8(vgaCRReg);
	VGAOUT8(vgaCRReg, (cr90 & 0x7F));
    }
}

static void
SavageInitStreamsNew(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
    unsigned long jDelta;

    xf86ErrorFVerb(STREAMS_TRACE, "SavageInitStreams\n" );

    if( 
	S3_SAVAGE_MOBILE_SERIES(psav->Chipset) && 
	!psav->CrtOnly && 
	!psav->TvOn 
    ) {
	OverlayParamInit( pScrn );
    }

    /* Primary stream reflects the frame buffer. */

    if (psav->FBStart2nd)
	jDelta = pScrn->displayWidth;
    else
	jDelta = pScrn->displayWidth * (pScrn->bitsPerPixel + 7) / 8;
    
    OUTREG( PRI_STREAM_BUFFERSIZE, jDelta * pScrn->virtualY >> 3 );
    OUTREG( PRI_STREAM_FBUF_ADDR0, pScrn->fbOffset );
    OUTREG( PRI_STREAM_STRIDE, jDelta );

    OUTREG( SEC_STREAM_CKEY_LOW, 0 );
    OUTREG( SEC_STREAM_CKEY_UPPER, 0 );
    OUTREG( SEC_STREAM_HSCALING, 0 );
    OUTREG( SEC_STREAM_VSCALING, 0 );
    OUTREG( BLEND_CONTROL, 0 );
    OUTREG( SEC_STREAM_FBUF_ADDR0, 0 );
    OUTREG( SEC_STREAM_FBUF_ADDR1, 0 );
    OUTREG( SEC_STREAM_FBUF_ADDR2, 0 );
    OUTREG( SEC_STREAM_WINDOW_START, 0 );
    OUTREG( SEC_STREAM_WINDOW_SZ, 0 );
/*    OUTREG( SEC_STREAM_BUFFERSIZE, 0 ); */
    OUTREG( SEC_STREAM_TILE_OFF, 0 );
    OUTREG( SEC_STREAM_OPAQUE_OVERLAY, 0 );
    OUTREG( SEC_STREAM_STRIDE, 0 );

    /* These values specify brightness, contrast, saturation and hue. */
    OUTREG( SEC_STREAM_COLOR_CONVERT1, 0x0000C892 );
    OUTREG( SEC_STREAM_COLOR_CONVERT2, 0x00039F9A );
    OUTREG( SEC_STREAM_COLOR_CONVERT3, 0x01F1547E );
#if 0
    {
	vgaHWPtr hwp;
	unsigned short vgaIOBase, vgaCRIndex, vgaCRReg;
	hwp = VGAHWPTR(pScrn);
	unsigned char cr90;

	vgaHWGetIOBase(hwp);
	vgaIOBase = hwp->IOBase;
	vgaCRIndex = vgaIOBase + 4;
	vgaCRReg = vgaIOBase + 5;

	VGAOUT8(vgaCRIndex, 0x90);
	cr90 = VGAIN8(vgaCRReg);
	VGAOUT8(vgaCRReg, (cr90 & 0x7F));
    }
#endif
}


/* Function to get lcd factor, display offset for overlay use
 * Input: pScrn; Output: x,yfactor, displayoffset in pScrn
 */
static void OverlayParamInit(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);

    psav = SAVPTR(pScrn);
    psav->cxScreen = pScrn->currentMode->HDisplay;
    InitStreamsForExpansion(pScrn);
}

/* Function to calculate lcd expansion x,y factor and offset for overlay
 */
static void InitStreamsForExpansion(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
    int		PanelSizeX,PanelSizeY;
    int		ViewPortWidth,ViewPortHeight;
    int		XExpansion, YExpansion;
    int		XFactor, YFactor;
    int		Hstate, Vstate;

    static CARD32 Xfactors[] = {
	0x00010001,
	0x00010001, /* 1 */
	0,
	0x00090008, /* 3 */
	0x00050004, /* 4 */
	0,
	0x00030002, /* 6 */
	0x00020001  /* 7 */
    };

    static CARD32 Yfactors[] = {
	0x00010001,	0x00010001,
	0,		0x00060005,
	0x00050004,	0x00040003,
	0,		0x00030002,
	0x00020001,	0x00050002,
	0x000C0005,	0x00080003,
	0x00090004,	0,
	0x00030001,	0x00040001,
    };



    PanelSizeX = psav->PanelX;
    PanelSizeY = psav->PanelY;
    ViewPortWidth = pScrn->currentMode->HDisplay;
    ViewPortHeight = pScrn->currentMode->VDisplay;

    if( PanelSizeX == 1408 )
	PanelSizeX = 1400;

    XExpansion = 0x00010001;
    YExpansion = 0x00010001;

    psav->displayXoffset = 0;
    psav->displayYoffset = 0;

    VGAOUT8(0x3C4, HZEXP_COMP_1);
    Hstate = VGAIN8(0x3C5);
    VGAOUT8(0x3C4, VTEXP_COMP_1);
    Vstate = VGAIN8(0x3C5);
    VGAOUT8(0x3C4, HZEXP_FACTOR_IGA1);
    XFactor = VGAIN8(0x3C5);
    VGAOUT8(0x3C4, VTEXP_FACTOR_IGA1);
    YFactor = VGAIN8(0x3C5);

    if( Hstate & EC1_EXPAND_ON )
    {
	XExpansion = Xfactors[XFactor>>4];
    }

    if( Vstate & EC1_EXPAND_ON )
    {
	YExpansion = Yfactors[YFactor>>4];
    }

    psav->XExp1 = XExpansion >> 16;
    psav->XExp2 = XExpansion & 0xFFFF;

    psav->YExp1 = YExpansion >> 16;
    psav->YExp2 = YExpansion & 0xFFFF;

    psav->displayXoffset = 
       ((PanelSizeX - (psav->XExp1 * ViewPortWidth) / psav->XExp2) / 2 + 7) & 0xfff8;
    psav->displayYoffset = 
       ((PanelSizeY - (psav->YExp1 * ViewPortHeight) / psav->YExp2) / 2);

}  /* InitStreamsForExpansionPM */

void 
SavageStreamsOn(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
     unsigned char jStreamsControl;
     unsigned short vgaCRIndex = psav->vgaIOBase + 4;
     unsigned short vgaCRReg = psav->vgaIOBase + 5;

    xf86ErrorFVerb(STREAMS_TRACE, "SavageStreamsOn\n" );

    /* Sequence stolen from streams.c in M7 NT driver */


    xf86EnableIO();

    /* Unlock extended registers. */

    VGAOUT16(vgaCRIndex, 0x4838);
    VGAOUT16(vgaCRIndex, 0xa039);
    VGAOUT16(0x3c4, 0x0608);

    VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );

    if( S3_SAVAGE_MOBILE_SERIES(psav->Chipset) ||
#if 0 /* I don't think commenting this out is correct (EE) */
	(psav->Chipset == S3_SUPERSAVAGE) ||
#endif
	(psav->Chipset == S3_SAVAGE2000) )
    {
	SavageInitStreamsNew( pScrn );

	jStreamsControl = VGAIN8( vgaCRReg ) | ENABLE_STREAM1;

	/* Wait for VBLANK. */
	
	VerticalRetraceWait(psav);

	/* Fire up streams! */

	VGAOUT16( vgaCRIndex, (jStreamsControl << 8) | EXT_MISC_CTRL2 );

	/* These values specify brightness, contrast, saturation and hue. */
	OUTREG( SEC_STREAM_COLOR_CONVERT1, 0x0000C892 );
	OUTREG( SEC_STREAM_COLOR_CONVERT2, 0x00039F9A );
	OUTREG( SEC_STREAM_COLOR_CONVERT3, 0x01F1547E );
    }
    else
    {
	jStreamsControl = VGAIN8( vgaCRReg ) | ENABLE_STREAMS_OLD;

	/* Wait for VBLANK. */
	
	VerticalRetraceWait(psav);

	/* Fire up streams! */

	VGAOUT16( vgaCRIndex, (jStreamsControl << 8) | EXT_MISC_CTRL2 );

	SavageInitStreamsOld( pScrn );
    }

    /* Wait for VBLANK. */
    
    VerticalRetraceWait(psav);

    /* Turn on secondary stream TV flicker filter, once we support TV. */

    /* SR70 |= 0x10 */

}

void
SavageStreamsOff(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
    unsigned char jStreamsControl;
    unsigned short vgaCRIndex = psav->vgaIOBase + 4;
    unsigned short vgaCRReg = psav->vgaIOBase + 5;

    xf86ErrorFVerb(STREAMS_TRACE, "SavageStreamsOff\n" );
    
    xf86EnableIO();

    /* Unlock extended registers. */

    VGAOUT16(vgaCRIndex, 0x4838);
    VGAOUT16(vgaCRIndex, 0xa039);
    VGAOUT16(0x3c4, 0x0608);

    VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );
    if( S3_SAVAGE_MOBILE_SERIES(psav->Chipset) ||
        (psav->Chipset == S3_SUPERSAVAGE) ||
        (psav->Chipset == S3_SAVAGE2000) )
	jStreamsControl = VGAIN8( vgaCRReg ) & NO_STREAMS;
    else
	jStreamsControl = VGAIN8( vgaCRReg ) & NO_STREAMS_OLD;

    /* Wait for VBLANK. */

    VerticalRetraceWait(psav);

    /* Kill streams. */

    VGAOUT16( vgaCRIndex, (jStreamsControl << 8) | EXT_MISC_CTRL2 );

    VGAOUT16( vgaCRIndex, 0x0093 );
    VGAOUT8( vgaCRIndex, 0x92 );
    VGAOUT8( vgaCRReg, VGAIN8(vgaCRReg) & 0x40 );

}

