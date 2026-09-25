/*
 * Copyright 1998-2003 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2003 S3 Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * VIA, S3 GRAPHICS, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef XF86DRI

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Priv.h"

#include "xaalocal.h"
#include "xaarop.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86fbman.h"

#if 0
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb16.h"
#include "cfb32.h"
#endif

#include "miline.h"


/*#include "savage_vbe.h"*/
#include "savage_regs.h"
#include "savage_driver.h"
#include "savage_bci.h"

#define _XF86DRI_SERVER_
#include "GL/glxtokens.h"
#include "sarea.h"
#include "savage_dri.h"
#include "savage_sarea.h"

static char SAVAGEKernelDriverName[] = "savage";
static char SAVAGEClientDriverName[] = "savage";

static Bool SAVAGEDRIOpenFullScreen(ScreenPtr pScreen);
static Bool SAVAGEDRICloseFullScreen(ScreenPtr pScreen);
/* DRI buffer management
 */
void SAVAGEDRIInitBuffers( WindowPtr pWin, RegionPtr prgn,
				CARD32 index );
void SAVAGEDRIMoveBuffers( WindowPtr pParent, DDXPointRec ptOldOrg,
				RegionPtr prgnSrc, CARD32 index );

/*        almost the same besides set src/desc to */
/*        Primary Bitmap Description              */

static void 
SAVAGEDRISetupForScreenToScreenCopy(
    ScrnInfoPtr pScrn, int xdir, int ydir,
    int rop, unsigned planemask, int transparency_color);


static void 
SAVAGEDRISubsequentScreenToScreenCopy(
    ScrnInfoPtr pScrn, int x1, int y1, int x2, int y2,
    int w, int h);


static void 
SAVAGEDRISetupForSolidFill(
    ScrnInfoPtr pScrn, int color, int rop, unsigned planemask);


static void 
SAVAGEDRISubsequentSolidFillRect(
    ScrnInfoPtr pScrn, int x, int y, int w, int h);

#if 0
extern int
SavageHelpSolidROP(ScrnInfoPtr pScrn, int *fg,int pm,int *rop);
#endif
unsigned long SAVAGEDRIGetPhyAddress(ScrnInfoPtr pScrn,void * pointer);

/* Initialize the visual configs that are supported by the hardware.
 * These are combined with the visual configs that the indirect
 * rendering core supports, and the intersection is exported to the
 * client.
 */
static Bool SAVAGEInitVisualConfigs( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   int numConfigs = 0;
   __GLXvisualConfig *pConfigs = 0;
   SAVAGEConfigPrivPtr pSAVAGEConfigs = 0;
   SAVAGEConfigPrivPtr *pSAVAGEConfigPtrs = 0;
   int i, db, depth, stencil, accum;

   switch ( pScrn->bitsPerPixel ) {
   case 8:
   case 24:
      break;

   case 16:
      numConfigs = 8;

      pConfigs = (__GLXvisualConfig*)xcalloc( sizeof(__GLXvisualConfig),
						numConfigs );
      if ( !pConfigs ) {
	 return FALSE;
      }

      pSAVAGEConfigs = (SAVAGEConfigPrivPtr)xcalloc( sizeof(SAVAGEConfigPrivRec),
						 numConfigs );
      if ( !pSAVAGEConfigs ) {
	 xfree( pConfigs );
	 return FALSE;
      }

      pSAVAGEConfigPtrs = (SAVAGEConfigPrivPtr*)xcalloc( sizeof(SAVAGEConfigPrivPtr),
						     numConfigs );
      if ( !pSAVAGEConfigPtrs ) {
	 xfree( pConfigs );
	 xfree( pSAVAGEConfigs );
	 return FALSE;
      }

      for ( i = 0 ; i < numConfigs ; i++ ) {
	 pSAVAGEConfigPtrs[i] = &pSAVAGEConfigs[i];
      }

      i = 0;
      depth = 1;
      for ( accum = 0 ; accum <= 1 ; accum++ ) {
         for ( stencil = 0 ; stencil <= 1 ; stencil++ ) {
            for ( db = 1 ; db >= 0 ; db-- ) {
               pConfigs[i].vid			= -1;
               pConfigs[i].class		= -1;
               pConfigs[i].rgba			= TRUE;
               pConfigs[i].redSize		= 5;
               pConfigs[i].greenSize		= 6;
               pConfigs[i].blueSize		= 5;
               pConfigs[i].alphaSize		= 0;
               pConfigs[i].redMask		= 0x0000F800;
               pConfigs[i].greenMask		= 0x000007E0;
               pConfigs[i].blueMask		= 0x0000001F;
               pConfigs[i].alphaMask		= 0;
               if ( accum ) {
                  pConfigs[i].accumRedSize	= 16;
                  pConfigs[i].accumGreenSize	= 16;
                  pConfigs[i].accumBlueSize	= 16;
                  pConfigs[i].accumAlphaSize	= 0;
               } else {
                  pConfigs[i].accumRedSize	= 0;
                  pConfigs[i].accumGreenSize	= 0;
                  pConfigs[i].accumBlueSize	= 0;
                  pConfigs[i].accumAlphaSize	= 0;
               }
               if ( db ) {
                  pConfigs[i].doubleBuffer	= TRUE;
               } else {
                  pConfigs[i].doubleBuffer	= FALSE;
	       }
               pConfigs[i].stereo		= FALSE;
               pConfigs[i].bufferSize		= 16;
               if ( depth ) {
                  pConfigs[i].depthSize		= 16;
               } else {
                  pConfigs[i].depthSize		= 0;
	       }
               if ( stencil ) {
                  pConfigs[i].stencilSize	= 8;
               } else {
                  pConfigs[i].stencilSize	= 0;
	       }
               pConfigs[i].auxBuffers		= 0;
               pConfigs[i].level		= 0;
               if ( accum || stencil ) {
                  pConfigs[i].visualRating	= GLX_SLOW_VISUAL_EXT;
               } else {
                  pConfigs[i].visualRating	= GLX_NONE_EXT;
	       }
               pConfigs[i].transparentPixel	= 0;
               pConfigs[i].transparentRed	= 0;
               pConfigs[i].transparentGreen	= 0;
               pConfigs[i].transparentBlue	= 0;
               pConfigs[i].transparentAlpha	= 0;
               pConfigs[i].transparentIndex	= 0;
               i++;
            }
         }
      }
      if ( i != numConfigs ) {
         xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		     "[drm] Incorrect initialization of visuals\n" );
         return FALSE;
      }
      break;

   case 32:
      numConfigs = 8;

      pConfigs = (__GLXvisualConfig*)xcalloc( sizeof(__GLXvisualConfig),
						numConfigs );
      if ( !pConfigs ) {
	 return FALSE;
      }

      pSAVAGEConfigs = (SAVAGEConfigPrivPtr)xcalloc( sizeof(SAVAGEConfigPrivRec),
						 numConfigs );
      if ( !pSAVAGEConfigs ) {
	 xfree( pConfigs );
	 return FALSE;
      }

      pSAVAGEConfigPtrs = (SAVAGEConfigPrivPtr*)xcalloc( sizeof(SAVAGEConfigPrivPtr),
						     numConfigs );
      if ( !pSAVAGEConfigPtrs ) {
	 xfree( pConfigs );
	 xfree( pSAVAGEConfigs );
	 return FALSE;
      }

      for ( i = 0 ; i < numConfigs ; i++ ) {
	 pSAVAGEConfigPtrs[i] = &pSAVAGEConfigs[i];
      }

      i = 0;
      for ( accum = 0 ; accum <= 1 ; accum++ ) {
         for ( depth = 0 ; depth <= 1 ; depth++ ) { /* and stencil */
            for ( db = 1 ; db >= 0 ; db-- ) {
               pConfigs[i].vid			= -1;
               pConfigs[i].class		= -1;
               pConfigs[i].rgba			= TRUE;
               pConfigs[i].redSize		= 8;
               pConfigs[i].greenSize		= 8;
               pConfigs[i].blueSize		= 8;
               pConfigs[i].alphaSize		= 8;
               pConfigs[i].redMask		= 0x00FF0000;
               pConfigs[i].greenMask		= 0x0000FF00;
               pConfigs[i].blueMask		= 0x000000FF;
               pConfigs[i].alphaMask		= 0;
               if ( accum ) {
                  pConfigs[i].accumRedSize	= 16;
                  pConfigs[i].accumGreenSize	= 16;
                  pConfigs[i].accumBlueSize	= 16;
                  pConfigs[i].accumAlphaSize	= 0;
               } else {
                  pConfigs[i].accumRedSize	= 0;
                  pConfigs[i].accumGreenSize	= 0;
                  pConfigs[i].accumBlueSize	= 0;
                  pConfigs[i].accumAlphaSize	= 0;
               }
               if ( db ) {
                  pConfigs[i].doubleBuffer	= TRUE;
               } else {
                  pConfigs[i].doubleBuffer	= FALSE;
	       }
               pConfigs[i].stereo		= FALSE;
               pConfigs[i].bufferSize		= 32;
               if ( depth ) {
		     pConfigs[i].depthSize	= 24;
                     pConfigs[i].stencilSize	= 8;
               }
               else {
                     pConfigs[i].depthSize	= 0;
                     pConfigs[i].stencilSize	= 0;
               }
               pConfigs[i].auxBuffers		= 0;
               pConfigs[i].level		= 0;
               if ( accum ) {
                  pConfigs[i].visualRating	= GLX_SLOW_VISUAL_EXT;
               } else {
                  pConfigs[i].visualRating	= GLX_NONE_EXT;
	       }
               pConfigs[i].transparentPixel	= 0;
               pConfigs[i].transparentRed	= 0;
               pConfigs[i].transparentGreen	= 0;
               pConfigs[i].transparentBlue	= 0;
               pConfigs[i].transparentAlpha	= 0;
               pConfigs[i].transparentIndex	= 0;
               i++;
            }
         }
      }
      if ( i != numConfigs ) {
         xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		     "[drm] Incorrect initialization of visuals\n" );
         return FALSE;
      }
      break;

   default:
      /* Unexpected bits/pixels */
      break;
   }

   psav->numVisualConfigs = numConfigs;
   psav->pVisualConfigs = pConfigs;
   psav->pVisualConfigsPriv = pSAVAGEConfigs;

   GlxSetVisualConfigs( numConfigs, pConfigs, (void **)pSAVAGEConfigPtrs );

   return TRUE;
}

static Bool SAVAGECreateContext( ScreenPtr pScreen, VisualPtr visual,
			      drm_context_t hwContext, void *pVisualConfigPriv,
			      DRIContextType contextStore )
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SavagePtr psav = SAVPTR(pScrn);
    SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;
    savageAgpBufferPtr pAgp;
    
    int ret,size,i;
    unsigned long  handle,map_handle;
    drm_savage_sarea_t * pShare;
    unsigned long offset;
   
    if(psav->xvmcContext)
        return FALSE;
    else
    {
        psav->DRIrunning++;
    }
    /* alloc agp memory to dma */
    pShare = (drm_savage_sarea_ptr)DRIGetSAREAPrivate(pScreen);
    pShare->agp_offset=0;
    /* find the available buffer*/
    for (i=0,pAgp = pSAVAGEDRIServer->agpBuffer;i<pSAVAGEDRIServer->numBuffer;i++,pAgp++)
    {
        if (pAgp->flags==0)
            break;
    }
    if (i >= pSAVAGEDRIServer->numBuffer)
    {
        return TRUE;
    }
          
    offset=pAgp->agp_offset;
    size = DRM_SAVAGE_DMA_AGP_SIZE;
    if (size <=0)
        return TRUE;
    ret=drmAgpAlloc( psav->drmFD,size,0,NULL,&handle);
    if ( ret<0)
    {
        return TRUE;
    }
    ret=drmAgpBind( psav->drmFD,handle,offset);
    if (ret<0)
    {
        return TRUE;
    }
    if (drmAddMap(psav->drmFD,
                  offset,
                  DRM_SAVAGE_DMA_AGP_SIZE,
                  DRM_AGP,
                  0,
                  &map_handle)<0)
    {
        return TRUE;
    }
    pAgp->ctxOwner = hwContext;
    pAgp->flags = 1;
    pAgp->agp_handle = handle;
    pAgp->map_handle = map_handle;
    pShare->agp_offset=map_handle;
    
    return TRUE;
    
}

static void SAVAGEDestroyContext( ScreenPtr pScreen, drm_context_t hwContext,
			       DRIContextType contextStore )
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SavagePtr psav = SAVPTR(pScrn);

    SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;
    savageAgpBufferPtr pAgp;
    drm_savage_sarea_t * pShare;
    int i;
    unsigned long handle,map_handle;
    
    psav->DRIrunning--;

    pShare = (drm_savage_sarea_ptr)DRIGetSAREAPrivate(pScreen);
    
    for (i=0,pAgp = pSAVAGEDRIServer->agpBuffer;i<pSAVAGEDRIServer->numBuffer;i++,pAgp++)
    {
        if (pAgp->ctxOwner == hwContext)
            break;
    }
    if (i >= pSAVAGEDRIServer->numBuffer || !pAgp->flags || !pAgp->agp_handle)
    {
        return;
    }
    handle = pAgp->agp_handle;
    map_handle = pAgp->map_handle;
    drmRmMap(psav->drmFD,map_handle);
    drmAgpUnbind(psav->drmFD,handle);
    drmAgpFree(psav->drmFD,handle);
    pAgp->flags=0;
    pAgp->ctxOwner=0;
}

#if 0
/* Quiescence, locking
 */
#define SAVAGE_TIMEOUT		2048

static void SAVAGEWaitForIdleDMA( ScrnInfoPtr pScrn )
{
   SavagePtr psav = SAVPTR(pScrn);
   int ret;
   int i = 0;

   for (;;) {
      do {
	 ret = drmSAVAGEFlushDMA( psav->drmFD,
			       DRM_LOCK_QUIESCENT | DRM_LOCK_FLUSH );
      } while ( ( ret == -EBUSY ) && ( i++ < SAVAGE_TIMEOUT ) );

      if ( ret == 0 )
	 return;

      xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		  "[dri] Idle timed out, resetting engine...\n" );

      drmSAVAGEEngineReset( psav->drmFD );
   }
}


void SAVAGEGetQuiescence( ScrnInfoPtr pScrn )
{
   SavagePtr psav = SAVPTR(pScrn);

   DRILock( screenInfo.screens[pScrn->scrnIndex], 0 );
   psav->haveQuiescense = 1;

   if ( psav->directRenderingEnabled ) {
      SAVAGEFBLayout *pLayout = &psav->CurrentLayout;

      SAVAGEWaitForIdleDMA( pScrn );

      WAITFIFO( 11 );
      OUTREG( SAVAGEREG_MACCESS, psav->MAccess );
      OUTREG( SAVAGEREG_PITCH, pLayout->displayWidth );

      psav->PlaneMask = ~0;
      OUTREG( SAVAGEREG_PLNWT, psav->PlaneMask );

      psav->BgColor = 0;
      psav->FgColor = 0;
      OUTREG( SAVAGEREG_BCOL, psav->BgColor );
      OUTREG( SAVAGEREG_FCOL, psav->FgColor );
      OUTREG( SAVAGEREG_SRCORG, psav->realSrcOrg );

      psav->SrcOrg = 0;
      OUTREG( SAVAGEREG_DSTORG, psav->DstOrg );
      OUTREG( SAVAGEREG_OPMODE, SAVAGEOPM_DMA_BLIT );
      OUTREG( SAVAGEREG_CXBNDRY, 0xFFFF0000 ); /* (maxX << 16) | minX */
      OUTREG( SAVAGEREG_YTOP, 0x00000000 );    /* minPixelPointer */
      OUTREG( SAVAGEREG_YBOT, 0x007FFFFF );    /* maxPixelPointer */

      psav->AccelFlags &= ~CLIPPER_ON;
   }
}

void SAVAGEGetQuiescenceShared( ScrnInfoPtr pScrn )
{
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEEntPtr pSAVAGEEnt = psav->entityPrivate;
   SavagePtr pSAVAGE2 = SAVPTR(pSAVAGEEnt->pScrn_2);

   DRILock( screenInfo.screens[pSAVAGEEnt->pScrn_1->scrnIndex], 0 );

   psav = SAVPTR(pSAVAGEEnt->pScrn_1);
   psav->haveQuiescense = 1;
   pSAVAGE2->haveQuiescense = 1;

   if ( pSAVAGEEnt->directRenderingEnabled ) {
      SAVAGEWaitForIdleDMA( pSAVAGEEnt->pScrn_1 );
      psav->RestoreAccelState( pScrn );
      xf86SetLastScrnFlag( pScrn->entityList[0], pScrn->scrnIndex );
   }
}

#endif 

/* move in SAVAGEDRISwapContext() */
#if 0
static void SAVAGESwapContext( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);

   /* Arrange for dma_quiescence and xaa sync to be called as
    * appropriate.
    */
   psav->LockHeld = 1;   /* port as pMGA->haveQuiescense*/
   psav->AccelInfoRec->NeedToSync = TRUE;
}
#endif

/* no double-head */
#if 0
static void SAVAGESwapContextShared( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEEntPtr pSAVAGEEnt = psav->entityPrivate;
   SavagePtr pSAVAGE2 = SAVPTR(pSAVAGEEnt->pScrn_2);

   psav = SAVPTR(pSAVAGEEnt->pScrn_1);

   psav->haveQuiescense = 0;
   psav->AccelInfoRec->NeedToSync = TRUE;

   pSAVAGE2->haveQuiescense = 0;
   pSAVAGE2->AccelInfoRec->NeedToSync = TRUE;
}
#endif 

/* This is really only called from validate/postvalidate as we
 * override the dri lock/unlock.  Want to remove validate/postvalidate
 * processing, but need to remove all client-side use of drawable lock
 * first (otherwise there is noone recover when a client dies holding
 * the drawable lock).
 *
 * What does this mean?
 *
 *   - The above code gets executed every time a
 *     window changes shape or the focus changes, which isn't really
 *     optimal.
 *   - The X server therefore believes it needs to do an XAA sync
 *     *and* a dma quiescense ioctl each time that happens.
 *
 * We don't wrap wakeuphandler any longer, so at least we can say that
 * this doesn't happen *every time the mouse moves*...
 */
static void
SAVAGEDRISwapContext( ScreenPtr pScreen, DRISyncType syncType,
		   DRIContextType oldContextType, void *oldContext,
		   DRIContextType newContextType, void *newContext )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);

#if 1
   /* this case call from DRIDoWakeupandler,  */
   /* swap context in                         */
   if ( syncType == DRI_3D_SYNC &&
	oldContextType == DRI_2D_CONTEXT &&
	newContextType == DRI_2D_CONTEXT )
   {
      psav->LockHeld = 1;   /* port as pMGA->haveQuiescense*/
      psav->AccelInfoRec->NeedToSync = TRUE;
   }
   /* this case call from DRIDoBlockHandler,   */
   /* swap context out                         */
   else if (syncType == DRI_2D_SYNC &&
	    oldContextType == DRI_NO_CONTEXT &&
	    newContextType == DRI_2D_CONTEXT)
   {
      psav->LockHeld = 0;
      psav->AccelInfoRec->NeedToSync = TRUE;
   }
#endif
}

/* no Double-Head */
#if 0
static void
SAVAGEDRISwapContextShared( ScreenPtr pScreen, DRISyncType syncType,
			  DRIContextType oldContextType, void *oldContext,
			  DRIContextType newContextType, void *newContext )
{
#if 0
   if ( syncType == DRI_3D_SYNC &&
	oldContextType == DRI_2D_CONTEXT &&
	newContextType == DRI_2D_CONTEXT )
   {
      SAVAGESwapContextShared( pScreen );
   }
#endif
}
#endif

/* following I810 and R128 swaping  method      */
/*       MGA swaping method is for their double head  */
#if 0
static void SAVAGEWakeupHandler( int screenNum, pointer wakeupData,
			      unsigned long result, pointer pReadmask )
{
    ScreenPtr pScreen = screenInfo.screens[screenNum];
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

    if ( xf86IsEntityShared( pScrn->entityList[0] ) ) {
        SAVAGESwapContextShared( pScreen );
    } else {
        SAVAGESwapContext( pScreen );
    }
}

static void SAVAGEBlockHandler( int screenNum, pointer blockData,
			     pointer pTimeout, pointer pReadmask )

{
   ScreenPtr pScreen = screenInfo.screens[screenNum];
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEEntPtr pSAVAGEEnt;

   if ( psav->haveQuiescense ) {
      if ( xf86IsEntityShared( pScrn->entityList[0] ) ) {
	 /* Restore to first screen */
	 psav->RestoreAccelState( pScrn );
	 xf86SetLastScrnFlag( pScrn->entityList[0], pScrn->scrnIndex );
	 pSAVAGEEnt = psav->entityPrivate;

	 if ( pSAVAGEEnt->directRenderingEnabled ) {
	    DRIUnlock( screenInfo.screens[pSAVAGEEnt->pScrn_1->scrnIndex] );
	 }
      } else {
	 if ( psav->directRenderingEnabled ) {
	    DRIUnlock( pScreen );
	 }
      }
      psav->haveQuiescense = 0;
   }
}
#endif

void SAVAGESelectBuffer( ScrnInfoPtr pScrn, int which )
{
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEDRIPtr pSAVAGEDRI = (SAVAGEDRIPtr)psav->pDRIInfo->devPrivate;

   psav->WaitIdleEmpty(psav);

   OUTREG(0x48C18,INREG(0x48C18)&(~0x00000008));
   
   switch ( which ) {
   case SAVAGE_BACK:
      OUTREG( 0x8170, pSAVAGEDRI->backOffset );
      OUTREG( 0x8174, pSAVAGEDRI->backBitmapDesc );
      break;
   case SAVAGE_DEPTH:
      OUTREG( 0x8170, pSAVAGEDRI->depthOffset );
      OUTREG( 0x8174, pSAVAGEDRI->depthBitmapDesc );
      break;
   default:
   case SAVAGE_FRONT:
      OUTREG( 0x8170, pSAVAGEDRI->frontOffset );
      OUTREG( 0x8174, pSAVAGEDRI->frontBitmapDesc );
      break;
   }
   OUTREG(0x48C18,INREG(0x48C18)|(0x00000008));
   psav->WaitIdleEmpty(psav);

}


static unsigned int mylog2( unsigned int n )
{
   unsigned int log2 = 1;

   n--;
   while ( n > 1 ) n >>= 1, log2++;

   return log2;
}

static Bool SAVAGEDRIAgpInit(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;
   unsigned long mode;
   unsigned int vendor, device;
   int ret;
   int size,numbuffer,i;
   savageAgpBufferPtr agpbuffer;
   
   /* FIXME: Make these configurable...
    */
   /*pSAVAGEDRIServer->agp.size = 16 * 1024 * 1024;*/
   pSAVAGEDRIServer->agp.size = psav->agpSize * 1024 * 1024;
   pSAVAGEDRIServer->agp.offset = pSAVAGEDRIServer->agp.size;
   
   pSAVAGEDRIServer->agpTextures.offset = 0;
   /*pSAVAGEDRIServer->agpTextures.size  = 16*1024*1024;*/
   pSAVAGEDRIServer->agpTextures.size  = psav->agpSize * 1024 * 1024;
   pSAVAGEDRIServer->logAgpTextureGranularity = 10;

   if ( drmAgpAcquire( psav->drmFD ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR, "[agp] AGP not available\n" );
      return FALSE;
   }

   mode   = drmAgpGetMode( psav->drmFD );        /* Default mode */
   vendor = drmAgpVendorId( psav->drmFD );
   device = drmAgpDeviceId( psav->drmFD );

   mode &= ~SAVAGE_AGP_MODE_MASK;

   switch ( psav->agpMode ) {
   case 4:
      mode |= SAVAGE_AGP_4X_MODE;
   case 2:
      mode |= SAVAGE_AGP_2X_MODE;
   case 1:
   default:
      mode |= SAVAGE_AGP_1X_MODE;
   }

   /*   mode |= SAVAGE_AGP_1X_MODE;*/

   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[agp] Mode 0x%08lx [AGP 0x%04x/0x%04x; Card 0x%04x/0x%04x]\n",
	       mode, vendor, device,
	       psav->PciInfo->vendor,
	       psav->PciInfo->chipType );

   if ( drmAgpEnable( psav->drmFD, mode ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR, "[agp] AGP not enabled\n" );
      drmAgpRelease( psav->drmFD );
      return FALSE;
   }

   ret = drmAgpAlloc( psav->drmFD, pSAVAGEDRIServer->agp.size,
		      0, NULL, &pSAVAGEDRIServer->agp.handle );
   if ( ret < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR, "[agp] Out of memory (%d)\n", ret );
      drmAgpRelease( psav->drmFD );
      return FALSE;
   }
   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[agp] %d kB allocated with handle 0x%08x\n",
	       pSAVAGEDRIServer->agp.size/1024, pSAVAGEDRIServer->agp.handle );

   if ( drmAgpBind( psav->drmFD, pSAVAGEDRIServer->agp.handle, 0 ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR, "[agp] Could not bind memory\n" );
      drmAgpFree( psav->drmFD, pSAVAGEDRIServer->agp.handle );
      drmAgpRelease( psav->drmFD );
      return FALSE;
   }

   if ( drmAddMap( psav->drmFD,
		   pSAVAGEDRIServer->agpTextures.offset,
		   pSAVAGEDRIServer->agpTextures.size,
		   DRM_AGP, 0,
		   &pSAVAGEDRIServer->agpTextures.handle ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[agp] Could not add agpTextures \n" );
      return FALSE;
   }
   /*   pSAVAGEDRIServer->agp_offset=pSAVAGEDRIServer->agpTexture.size;*/
   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[agp] agpTextures microcode handle = 0x%08lx\n",
	       pSAVAGEDRIServer->agpTextures.handle );

   /*if ( drmMap( psav->drmFD,
		pSAVAGEDRIServer->agpTextures.handle,
		pSAVAGEDRIServer->agpTextures.size,
		&pSAVAGEDRIServer->agpTextures.map ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[agp] Could not map agpTextures \n" );
      return FALSE;
   }*/
   
   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[agp] agpTextures mapped at 0x%08lx\n",
	       (unsigned long)pSAVAGEDRIServer->agpTextures.map );

   /* for agp dma buffer*/
   size = drmAgpSize(psav->drmFD);
   size -=    pSAVAGEDRIServer->agpTextures.size;/*sub texture size*/
   numbuffer = size /  DRM_SAVAGE_DMA_AGP_SIZE;
   agpbuffer = (savageAgpBufferPtr)xcalloc(sizeof(savageAgpBuffer),numbuffer);
   for (i=0;i<numbuffer;i++)
   {
       agpbuffer[i].ctxOwner = 0;
       agpbuffer[i].agp_offset =
           pSAVAGEDRIServer->agpTextures.size + i* DRM_SAVAGE_DMA_AGP_SIZE;
       agpbuffer[i].flags = 0;
       agpbuffer[i].agp_handle = 0;
   }
   pSAVAGEDRIServer->numBuffer = numbuffer;
   pSAVAGEDRIServer->agpBuffer = agpbuffer;
        
   return TRUE;
}

static Bool SAVAGEDRIMapInit( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;

   pSAVAGEDRIServer->registers.size = SAVAGEIOMAPSIZE;

   if ( drmAddMap( psav->drmFD,
		   (drm_handle_t)psav->MmioBase,
		   pSAVAGEDRIServer->registers.size,
		   DRM_REGISTERS,0,
		   &pSAVAGEDRIServer->registers.handle ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[drm] Could not add MMIO registers mapping\n" );
      return FALSE;
   }
   
   pSAVAGEDRIServer->aperture.size = 5 * 0x01000000;
   
   if ( drmAddMap( psav->drmFD,
		   (drm_handle_t)(psav->ApertureBase),
		   pSAVAGEDRIServer->aperture.size,
		   DRM_FRAME_BUFFER,0,
		   &pSAVAGEDRIServer->aperture.handle ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[drm] Could not add aperture mapping\n" );
      return FALSE;
   }
   
   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[drm] aperture handle = 0x%08lx\n",
	       pSAVAGEDRIServer->aperture.handle );

   /*if(drmMap(psav->drmFD,
          pSAVAGEDRIServer->registers.handle,
          pSAVAGEDRIServer->registers.size,
          &pSAVAGEDRIServer->registers.map)<0)
   {
         xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[drm] Could not map MMIO registers region to virtual\n" );
      return FALSE;
   
   }*/

   pSAVAGEDRIServer->status.size = SAREA_MAX;

   if ( drmAddMap( psav->drmFD, 0, pSAVAGEDRIServer->status.size,
		   DRM_SHM, DRM_READ_ONLY | DRM_LOCKED | DRM_KERNEL,
		   &pSAVAGEDRIServer->status.handle ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[drm] Could not add status page mapping\n" );
      return FALSE;
   }
   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[drm] Status handle = 0x%08lx\n",
	       pSAVAGEDRIServer->status.handle );

   if ( drmMap( psav->drmFD,
		pSAVAGEDRIServer->status.handle,
		pSAVAGEDRIServer->status.size,
		&pSAVAGEDRIServer->status.map ) < 0 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[agp] Could not map status page\n" );
      return FALSE;
   }
   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[agp] Status page mapped at 0x%08lx\n",
	       (unsigned long)pSAVAGEDRIServer->status.map );

   return TRUE;
}

/* no need DMA */
#if 0 
static Bool SAVAGEDRIKernelInit( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;
   drmSAVAGEInit init;
   int ret;

   memset( &init, 0, sizeof(drmSAVAGEInit) );

   init.sarea_priv_offset = sizeof(XF86DRISAREARec);

   switch ( psav->Chipset ) {
   case PCI_CHIP_SAVAGEG400:
      init.chipset = SAVAGE_CARD_TYPE_G400;
      break;
   case PCI_CHIP_SAVAGEG200:
   case PCI_CHIP_SAVAGEG200_PCI:
      init.chipset = SAVAGE_CARD_TYPE_G200;
      break;
   default:
      return FALSE;
   }
   init.sgram = !psav->HasSDRAM;

   init.maccess = psav->MAccess;

   init.fb_cpp		= pScrn->bitsPerPixel / 8;
   init.front_offset	= pSAVAGEDRIServer->frontOffset;
   init.front_pitch	= pSAVAGEDRIServer->frontPitch / init.fb_cpp;
   init.back_offset	= pSAVAGEDRIServer->backOffset;
   init.back_pitch	= pSAVAGEDRIServer->backPitch / init.fb_cpp;

   init.depth_cpp	= pScrn->bitsPerPixel / 8;
   init.depth_offset	= pSAVAGEDRIServer->depthOffset;
   init.depth_pitch	= pSAVAGEDRIServer->depthPitch / init.depth_cpp;

   init.texture_offset[0] = pSAVAGEDRIServer->textureOffset;
   init.texture_size[0] = pSAVAGEDRIServer->textureSize;

   init.fb_offset = psav->FbAddress;
   init.mmio_offset = pSAVAGEDRIServer->registers.handle;
   init.status_offset = pSAVAGEDRIServer->status.handle;

   init.warp_offset = pSAVAGEDRIServer->warp.handle;
   init.primary_offset = pSAVAGEDRIServer->primary.handle;
   init.buffers_offset = pSAVAGEDRIServer->buffers.handle;

   ret = drmSAVAGEInitDMA( psav->drmFD, &init );
   if ( ret < 0 ) {
      xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		  "[drm] Failed to initialize DMA! (%d)\n", ret );
      return FALSE;
   }

#if 0
   /* FIXME: This is just here to clean up after the engine reset test
    * in the kernel module.  Please remove it later...
    */
   psav->GetQuiescence( pScrn );
#endif

   return TRUE;
}

static Bool SAVAGEDRIBuffersInit( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;


   pSAVAGEDRIServer->drmBuffers = drmMapBufs( psav->drmFD );
   if ( !pSAVAGEDRIServer->drmBuffers ) {
	xf86DrvMsg( pScreen->myNum, X_ERROR,
		    "[drm] Failed to map DMA buffers list\n" );
	return FALSE;
    }
    xf86DrvMsg( pScreen->myNum, X_INFO,
		"[drm] Mapped %d DMA buffers\n",
		pSAVAGEDRIServer->drmBuffers->count );

    return TRUE;
}
#endif

Bool SAVAGEDRIScreenInit( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   DRIInfoPtr pDRIInfo;
   SAVAGEDRIPtr pSAVAGEDRI;
   SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer;

/*  disable first....*/
#if 0
   switch ( psav->Chipset ) {
   case PCI_CHIP_SAVAGEG400:
   case PCI_CHIP_SAVAGEG200:
#if 0
   case PCI_CHIP_SAVAGEG200_PCI:
#endif
      break;
   default:
      xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		  "[drm] Direct rendering only supported with AGP G200/G400 cards!\n" );
      return FALSE;
   }
#endif

   /* Check that the GLX, DRI, and DRM modules have been loaded by testing
    * for canonical symbols in each module.
    */
   if ( !xf86LoaderCheckSymbol( "GlxSetVisualConfigs" ) )	return FALSE;
   if ( !xf86LoaderCheckSymbol( "DRIScreenInit" ) )		return FALSE;
   if ( !xf86LoaderCheckSymbol( "drmAvailable" ) )		return FALSE;
   if ( !xf86LoaderCheckSymbol( "DRIQueryVersion" ) ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[dri] SAVAGEDRIScreenInit failed (libdri.a too old)\n" );
      return FALSE;
   }

/* disable first */
#if 1
   /* Check the DRI version */
   {
      int major, minor, patch;
      DRIQueryVersion( &major, &minor, &patch );
      if ( major != 4 || minor < 0 ) {
         xf86DrvMsg( pScreen->myNum, X_ERROR,
		     "[dri] SAVAGEDRIScreenInit failed because of a version mismatch.\n"
		     "[dri] libDRI version = %d.%d.%d but version 4.0.x is needed.\n"
		     "[dri] Disabling the DRI.\n",
		     major, minor, patch );
         return FALSE;
      }
   }
#endif

   xf86DrvMsg( pScreen->myNum, X_INFO,
	       "[drm] bpp: %d depth: %d\n",
	       pScrn->bitsPerPixel, pScrn->depth );

   if ( (pScrn->bitsPerPixel / 8) != 2 &&
	(pScrn->bitsPerPixel / 8) != 4 ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[dri] Direct rendering only supported in 16 and 32 bpp modes\n" );
      return FALSE;
   }

   pDRIInfo = DRICreateInfoRec();
   if ( !pDRIInfo ) {
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[dri] DRICreateInfoRec() failed\n" );
      return FALSE;
   }
   psav->pDRIInfo = pDRIInfo;

   pDRIInfo->drmDriverName = SAVAGEKernelDriverName;
   pDRIInfo->clientDriverName = SAVAGEClientDriverName;
   if (xf86LoaderCheckSymbol("DRICreatePCIBusID")) {
      pDRIInfo->busIdString = DRICreatePCIBusID(psav->PciInfo);
   } else {
      pDRIInfo->busIdString            = xalloc(64);
      sprintf(pDRIInfo->busIdString,
              "PCI:%d:%d:%d",
              psav->PciInfo->bus,
              psav->PciInfo->device,
              psav->PciInfo->func);
   }
   pDRIInfo->ddxDriverMajorVersion = 1/*SAVAGE_MAJOR_VERSION*/;
   pDRIInfo->ddxDriverMinorVersion = 1/*SAVAGE_MINOR_VERSION*/;
   pDRIInfo->ddxDriverPatchVersion = 16/*SAVAGE_PATCHLEVEL*/;
   pDRIInfo->frameBufferPhysicalAddress = psav->FrameBufferBase;
   pDRIInfo->frameBufferSize = psav->videoRambytes;
   pDRIInfo->frameBufferStride = pScrn->displayWidth*(pScrn->bitsPerPixel/8);
   pDRIInfo->ddxDrawableTableEntry = SAVAGE_MAX_DRAWABLES;

/* mark off these... we use default */
#if 0
   pDRIInfo->wrap.BlockHandler = SAVAGEBlockHandler;
   pDRIInfo->wrap.WakeupHandler = SAVAGEWakeupHandler;
#endif
   
   pDRIInfo->wrap.ValidateTree = NULL;
   pDRIInfo->wrap.PostValidateTree = NULL;

   pDRIInfo->createDummyCtx = TRUE;
   pDRIInfo->createDummyCtxPriv = FALSE;

   if ( SAREA_MAX_DRAWABLES < SAVAGE_MAX_DRAWABLES ) {
      pDRIInfo->maxDrawableTableEntry = SAREA_MAX_DRAWABLES;
   } else {
      pDRIInfo->maxDrawableTableEntry = SAVAGE_MAX_DRAWABLES;
   }

   /* For now the mapping works by using a fixed size defined
    * in the SAREA header.
    */
   if ( sizeof(XF86DRISAREARec) + sizeof(SAVAGESAREAPrivRec) > SAREA_MAX ) {
      xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		  "[drm] Data does not fit in SAREA\n" );
      return FALSE;
   }

   xf86DrvMsg( pScrn->scrnIndex, X_INFO,
	       "[drm] Sarea %d+%d: %d\n",
	       sizeof(XF86DRISAREARec), sizeof(SAVAGESAREAPrivRec),
	       sizeof(XF86DRISAREARec) + sizeof(SAVAGESAREAPrivRec) );

   pDRIInfo->SAREASize = SAREA_MAX;

   pSAVAGEDRI = (SAVAGEDRIPtr)xcalloc( sizeof(SAVAGEDRIRec), 1 );
   if ( !pSAVAGEDRI ) {
      DRIDestroyInfoRec( psav->pDRIInfo );
      psav->pDRIInfo = 0;
      xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		  "[drm] Failed to allocate memory for private record\n" );
      return FALSE;
   }

   pSAVAGEDRIServer = (SAVAGEDRIServerPrivatePtr)
      xcalloc( sizeof(SAVAGEDRIServerPrivateRec), 1 );
   if ( !pSAVAGEDRIServer ) {
      xfree( pSAVAGEDRI );
      DRIDestroyInfoRec( psav->pDRIInfo );
      psav->pDRIInfo = 0;
      xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		  "[drm] Failed to allocate memory for private record\n" );
      return FALSE;
   }
   psav->DRIServerInfo = pSAVAGEDRIServer;

   pDRIInfo->devPrivate = pSAVAGEDRI;
   pDRIInfo->devPrivateSize = sizeof(SAVAGEDRIRec);
   pDRIInfo->contextSize = sizeof(SAVAGEDRIContextRec);

   pDRIInfo->CreateContext = SAVAGECreateContext;
   pDRIInfo->DestroyContext = SAVAGEDestroyContext;

/* we don't have double head */
#if 0   
   if ( xf86IsEntityShared( pScrn->entityList[0] ) ) {
      pDRIInfo->SwapContext = SAVAGEDRISwapContextShared;
   } else
#endif
 
   {
      pDRIInfo->SwapContext = SAVAGEDRISwapContext;
   }

#if 0
   switch( pScrn->bitsPerPixel ) {
   case 8:
       pDRIInfo->InitBuffers = Savage8DRIInitBuffers;
       pDRIInfo->MoveBuffers = Savage8DRIMoveBuffers;
   case 16:
       pDRIInfo->InitBuffers = Savage16DRIInitBuffers;
       pDRIInfo->MoveBuffers = Savage16DRIMoveBuffers;
   case 24:
       pDRIInfo->InitBuffers = Savage24DRIInitBuffers;
       pDRIInfo->MoveBuffers = Savage24DRIMoveBuffers;
   case 32:
       pDRIInfo->InitBuffers = Savage32DRIInitBuffers;
       pDRIInfo->MoveBuffers = Savage32DRIMoveBuffers;
   }
#endif
   
   pDRIInfo->InitBuffers = SAVAGEDRIInitBuffers;
   pDRIInfo->MoveBuffers = SAVAGEDRIMoveBuffers;
   pDRIInfo->OpenFullScreen = SAVAGEDRIOpenFullScreen;
   pDRIInfo->CloseFullScreen = SAVAGEDRICloseFullScreen;
   pDRIInfo->bufferRequests = DRI_ALL_WINDOWS;

   if ( !DRIScreenInit( pScreen, pDRIInfo, &psav->drmFD ) ) {
      xfree( pSAVAGEDRIServer );
      psav->DRIServerInfo = 0;
      xfree( pDRIInfo->devPrivate );
      pDRIInfo->devPrivate = 0;
      DRIDestroyInfoRec( psav->pDRIInfo );
      psav->pDRIInfo = 0;
      xf86DrvMsg( pScreen->myNum, X_ERROR,
		  "[drm] DRIScreenInit failed.  Disabling DRI.\n" );
      return FALSE;
   }

/* Disable these */
#if 1
   /* Check the SAVAGE DRM version */
   {
      drmVersionPtr version = drmGetVersion(psav->drmFD);
      if ( version ) {
         if ( version->version_major != 1 ||
	      version->version_minor < 0 ) {
            /* incompatible drm version */
            xf86DrvMsg( pScreen->myNum, X_ERROR,
			"[dri] SAVAGEDRIScreenInit failed because of a version mismatch.\n"
			"[dri] savage.o kernel module version is %d.%d.%d but version 3.0.x is needed.\n"
			"[dri] Disabling DRI.\n",
			version->version_major,
			version->version_minor,
			version->version_patchlevel );
            drmFreeVersion( version );
	    SAVAGEDRICloseScreen( pScreen );		/* FIXME: ??? */
            return FALSE;
         }
         drmFreeVersion( version );
      }
   }
#endif

#if 0
   /* Calculate texture constants for AGP texture space.
    * FIXME: move!
    */
   {
      CARD32 agpTextureOffset = SAVAGE_DMA_BUF_SZ * SAVAGE_DMA_BUF_NR;
      CARD32 agpTextureSize = pSAVAGEDRI->agp.size - agpTextureOffset;

      i = mylog2(agpTextureSize / SAVAGE_NR_TEX_REGIONS);
      if (i < SAVAGE_LOG_MIN_TEX_REGION_SIZE)
         i = SAVAGE_LOG_MIN_TEX_REGION_SIZE;

      pSAVAGEDRI->logAgpTextureGranularity = i;
      pSAVAGEDRI->agpTextureSize = (agpTextureSize >> i) << i;
      pSAVAGEDRI->agpTextureOffset = agpTextureOffset;
   }
#endif

   if ( !SAVAGEDRIAgpInit( pScreen ) ) {
      DRICloseScreen( pScreen );
      return FALSE;
   }

   if ( !SAVAGEDRIMapInit( pScreen ) ) {
      DRICloseScreen( pScreen );
      return FALSE;
   }

   if ( !SAVAGEInitVisualConfigs( pScreen ) ) {
      DRICloseScreen( pScreen );
      return FALSE;
   }
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[dri] visual configs initialized\n" );

   return TRUE;
}


Bool SAVAGEDRIFinishScreenInit( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;
   SAVAGEDRIPtr pSAVAGEDRI = (SAVAGEDRIPtr)psav->pDRIInfo->devPrivate;
   int i;
   unsigned int TileStride;
   
   
   if ( !psav->pDRIInfo )
      return FALSE;

   psav->pDRIInfo->driverSwapMethod = DRI_HIDE_X_CONTEXT;

   /* NOTE: DRIFinishScreenInit must be called before *DRIKernelInit
    * because *DRIKernelInit requires that the hardware lock is held by
    * the X server, and the first time the hardware lock is grabbed is
    * in DRIFinishScreenInit.
    */
   if ( !DRIFinishScreenInit( pScreen ) ) {
      SAVAGEDRICloseScreen( pScreen );
      return FALSE;
   }

/* no dma...... */
#if 0
   if ( !SAVAGEDRIKernelInit( pScreen ) ) {
      SAVAGEDRICloseScreen( pScreen );
      return FALSE;
   }

   if ( !SAVAGEDRIBuffersInit( pScreen ) ) {
      SAVAGEDRICloseScreen( pScreen );
      return FALSE;
   }

   switch ( psav->Chipset ) {
   case PCI_CHIP_SAVAGEG400:
      pSAVAGEDRI->chipset = SAVAGE_CARD_TYPE_G400;
      break;
   case PCI_CHIP_SAVAGEG200:
   case PCI_CHIP_SAVAGEG200_PCI:
      pSAVAGEDRI->chipset = SAVAGE_CARD_TYPE_G200;
      break;
   default:
      return FALSE;
   }
#endif

   pSAVAGEDRI->chipset          = psav->Chipset;
   pSAVAGEDRI->width		= pScrn->virtualX;
   pSAVAGEDRI->height		= pScrn->virtualY;
   pSAVAGEDRI->mem			= pScrn->videoRam * 1024;
   pSAVAGEDRI->cpp			= pScrn->bitsPerPixel / 8;
   pSAVAGEDRI->zpp			=pSAVAGEDRI->cpp;  
   pSAVAGEDRI->agpMode		= psav->agpMode;

   pSAVAGEDRI->frontOffset		= pSAVAGEDRIServer->frontOffset;
   pSAVAGEDRI->frontbufferSize		= pSAVAGEDRIServer->frontbufferSize;
   pSAVAGEDRI->frontbuffer		= psav->FrameBufferBase + 
                                          pSAVAGEDRI->frontOffset;
   pSAVAGEDRI->frontPitch		= pSAVAGEDRIServer->frontPitch;
   pSAVAGEDRI->IsfrontTiled             = psav->bTiled; /* AGD: was 0 */
   
   if(pSAVAGEDRI->IsfrontTiled)
   {
      if(pSAVAGEDRI->cpp==2)
        TileStride = (pSAVAGEDRI->width+63)&(~63);
      else
        TileStride = (pSAVAGEDRI->width+31)&(~31);

      if ((psav->Chipset == S3_TWISTER)
         || (psav->Chipset == S3_PROSAVAGE)
         || (psav->Chipset == S3_PROSAVAGEDDR)
         || (psav->Chipset == S3_SUPERSAVAGE))
      { 
      		pSAVAGEDRI->frontBitmapDesc   	= 0x10000000 | /* block write disabled */
                	                          (1<<24) | /* destination tile format */
                        	                  (pScrn->bitsPerPixel<<16) | /* bpp */
                        	                  TileStride; /* stride */
      } else {        
      		pSAVAGEDRI->frontBitmapDesc   	= 0x10000000 | /* block write disabled */
                	                          ((pSAVAGEDRI->cpp==2)?
                        	                     BCI_BD_TILE_16:BCI_BD_TILE_32) | /*16/32 bpp tile format */
                        	                  (pScrn->bitsPerPixel<<16) | /* bpp */
                        	                  TileStride; /* stride */
      		pSAVAGEDRI->frontPitch 		= TileStride;
      }
   }
   else
   {
      pSAVAGEDRI->frontBitmapDesc	= 0x10000000 | /* AGD: block write should be disabled:  was 0x00000000 */
      					  pScrn->bitsPerPixel<<16 |
      					  pSAVAGEDRI->width;
   }
   
   pSAVAGEDRI->backOffset		= pSAVAGEDRIServer->backOffset;
   pSAVAGEDRI->backbufferSize		= pSAVAGEDRIServer->backbufferSize;
   pSAVAGEDRI->backbuffer		= psav->FrameBufferBase +
   					  pSAVAGEDRI->backOffset;
   pSAVAGEDRI->backPitch		= pSAVAGEDRIServer->backPitch;

   {
      if(pSAVAGEDRI->cpp==2)
        TileStride = (pSAVAGEDRI->width+63)&(~63);
      else
        TileStride = (pSAVAGEDRI->width+31)&(~31);

      if ((psav->Chipset == S3_TWISTER)
         || (psav->Chipset == S3_PROSAVAGE)
         || (psav->Chipset == S3_PROSAVAGEDDR)
         || (psav->Chipset == S3_SUPERSAVAGE)) /* AGD: supersavage may work like savage4/MX/IX, I just don't know. */
      {        				       /* It's here since the 2D driver sets it up like prosavage */
      		pSAVAGEDRI->backBitmapDesc   	= 0x10000000 |
                	                          (1<<24) |
                	                          (pScrn->bitsPerPixel<<16) |
                	                          TileStride;
      } else {
      		pSAVAGEDRI->backBitmapDesc   	= 0x10000000 |
                	                          ((pSAVAGEDRI->cpp==2)?
                	                             BCI_BD_TILE_16:BCI_BD_TILE_32) |
                	                          (pScrn->bitsPerPixel<<16) |
                	                          TileStride;
      }
   }

   pSAVAGEDRI->depthOffset		= pSAVAGEDRIServer->depthOffset;
   pSAVAGEDRI->depthbufferSize		= pSAVAGEDRIServer->depthbufferSize;
   pSAVAGEDRI->depthbuffer		= psav->FrameBufferBase +
   					  pSAVAGEDRI->depthOffset;
   pSAVAGEDRI->depthPitch		= pSAVAGEDRIServer->depthPitch;

   {
      if(pSAVAGEDRI->zpp==2)
        TileStride = (pSAVAGEDRI->width+63)&(~63);
      else
        TileStride = (pSAVAGEDRI->width+31)&(~31);

      if ((psav->Chipset == S3_TWISTER)
         || (psav->Chipset == S3_PROSAVAGE)
         || (psav->Chipset == S3_PROSAVAGEDDR)
         || (psav->Chipset == S3_SUPERSAVAGE))
      {  
      		pSAVAGEDRI->depthBitmapDesc   	= 0x10000000 |
 	     					  (1<<24) |
	                                          (pScrn->bitsPerPixel<<16) |
	                                          TileStride;
      } else {
	        pSAVAGEDRI->depthBitmapDesc   	= 0x10000000 |
                	                          ((pSAVAGEDRI->zpp==2)?
                	                             BCI_BD_TILE_16:BCI_BD_TILE_32) |
                	                          (pScrn->bitsPerPixel<<16) |
                	                          TileStride;
      }
   }

   pSAVAGEDRI->textureOffset	= pSAVAGEDRIServer->textureOffset;
   pSAVAGEDRI->textures 		= psav->FrameBufferBase +
   					  pSAVAGEDRI->textureOffset;
   pSAVAGEDRI->textureSize		= pSAVAGEDRIServer->textureSize;

   i = mylog2( pSAVAGEDRI->textureSize / SAVAGE_NR_TEX_REGIONS );
   if ( i < SAVAGE_LOG_MIN_TEX_REGION_SIZE )
      i = SAVAGE_LOG_MIN_TEX_REGION_SIZE;

   pSAVAGEDRI->logTextureGranularity = i;
   pSAVAGEDRI->textureSize = (pSAVAGEDRI->textureSize >> i) << i; /* truncate */

   pSAVAGEDRI->agpTextures.handle = pSAVAGEDRIServer->agpTextures.handle;
   pSAVAGEDRI->agpTextures.offset = pSAVAGEDRIServer->agpTextures.offset;   
   pSAVAGEDRI->agpTextures.size = pSAVAGEDRIServer->agpTextures.size;
   pSAVAGEDRI->agpTextures.map = pSAVAGEDRIServer->agpTextures.map;

   i = mylog2( pSAVAGEDRI->agpTextures.size / SAVAGE_NR_TEX_REGIONS );
   if ( i < SAVAGE_LOG_MIN_TEX_REGION_SIZE )
      i = SAVAGE_LOG_MIN_TEX_REGION_SIZE;

   pSAVAGEDRI->logAgpTextureGranularity = i;
   pSAVAGEDRI->agpTextures.size = (pSAVAGEDRI->agpTextures.size >> i) << i; /* truncate */

   pSAVAGEDRI->aperture.handle	= pSAVAGEDRIServer->aperture.handle;
   pSAVAGEDRI->aperture.size	= pSAVAGEDRIServer->aperture.size;
   pSAVAGEDRI->aperture.map    = pSAVAGEDRIServer->aperture.map;
   {
      unsigned int shift = 0;
      
      if(pSAVAGEDRI->width > 1024)
        shift = 1; 

      /* pSAVAGEDRI->aperturePitch = psav->ulAperturePitch; */
      pSAVAGEDRI->aperturePitch = psav->ul3DAperturePitch;
   }

   {
      unsigned int value = 0;
      
      OUTREG(0x850C,(INREG(0x850C) | 0x00008000)); /* AGD: I don't think this does anything on 3D/MX/IX */
						   /* maybe savage4 too... */
      /* we don't use Y range flag,so comment it */
      /*
        if(pSAVAGEDRI->width <= 1024)
            value |= (1<<29);
      */
      if ((psav->Chipset == S3_SAVAGE_MX) /* 3D/MX/IX seem to set up the tile stride differently */
	|| (psav->Chipset == S3_SAVAGE3D)) {
      	    if(pSAVAGEDRI->cpp == 2)
      	    {
         	value |=  ((psav->l3DDelta / 4) >> 5) << 24; /* I assume psav->l3DDelta for 3D */
         	value |= 2<<30;
      	    } else {
         	value |=  ((psav->l3DDelta / 4) >> 5) << 24; /* I assume psav->l3DDelta for 3D */
         	value |= 3<<30;
      	    }

	    OUTREG(0x48C40, value|(pSAVAGEDRI->frontOffset) ); /* front */ 
	    OUTREG(0x48C44, value|(pSAVAGEDRI->backOffset) ); /* back  */
	    OUTREG(0x48C48, value|(pSAVAGEDRI->depthOffset) ); /* depth */
      } else {
      	    if(pSAVAGEDRI->cpp == 2)
      	    {
         	value |=  (((pSAVAGEDRI->width + 0x3F) & 0xFFC0) >> 6) << 20;
         	value |= 2<<30;
      	    } else {
         	value |=  (((pSAVAGEDRI->width + 0x1F) & 0xFFE0) >> 5) << 20;
         	value |= 3<<30;
      	    }

	    OUTREG(0x48C40, value|(pSAVAGEDRI->frontOffset >> 5) ); /* front */
	    OUTREG(0x48C44, value|(pSAVAGEDRI->backOffset >> 5) ); /* back  */
	    OUTREG(0x48C48, value|(pSAVAGEDRI->depthOffset >> 5) ); /* depth */
      }
   }
   
   pSAVAGEDRI->registers.handle	= pSAVAGEDRIServer->registers.handle;
   pSAVAGEDRI->registers.size	= pSAVAGEDRIServer->registers.size;
   pSAVAGEDRI->registers.map    = pSAVAGEDRIServer->registers.map;
   
   pSAVAGEDRI->BCIcmdBuf.handle	= pSAVAGEDRIServer->registers.handle+0x00010000;
   pSAVAGEDRI->BCIcmdBuf.size	= 0x00020000;
   pSAVAGEDRI->BCIcmdBuf.map    = (drmAddress)( (unsigned int)
                                     pSAVAGEDRIServer->registers.map +
                                     0x00010000);
   
   pSAVAGEDRI->status.handle	= pSAVAGEDRIServer->status.handle;
   pSAVAGEDRI->status.size		= pSAVAGEDRIServer->status.size;
   pSAVAGEDRI->shadowStatus = psav->ShadowStatus;

/* no DMA now ....*/
#if 0
   pSAVAGEDRI->primary.handle	= pSAVAGEDRIServer->primary.handle;
   pSAVAGEDRI->primary.size	= pSAVAGEDRIServer->primary.size;
   pSAVAGEDRI->buffers.handle	= pSAVAGEDRIServer->buffers.handle;
   pSAVAGEDRI->buffers.size	= pSAVAGEDRIServer->buffers.size;
#endif
   
   pSAVAGEDRI->sarea_priv_offset = sizeof(XF86DRISAREARec);
   
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]pSAVAGEDRIServer:\n" );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	reserved_map_agpstart:0x%08x\n",pSAVAGEDRIServer->reserved_map_agpstart); 
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	reserved_map_idx:0x%08x\n",pSAVAGEDRIServer->reserved_map_idx);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	sarea_priv_offset:0x%08lx\n",pSAVAGEDRIServer->sarea_priv_offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	chipset:0x%08x\n",pSAVAGEDRIServer->chipset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	sgram:0x%08x\n",pSAVAGEDRIServer->sgram);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontbufferSize:0x%08x\n",pSAVAGEDRIServer->frontbufferSize);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontOffset:0x%08x\n",pSAVAGEDRIServer->frontOffset);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontPitch:0x%08x\n",pSAVAGEDRIServer->frontPitch);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backbufferSize:0x%08x\n",pSAVAGEDRIServer->backbufferSize);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backOffset:0x%08x\n",pSAVAGEDRIServer->backOffset);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backPitch:0x%08x\n",pSAVAGEDRIServer->backPitch);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthbufferSize:0x%08x\n",pSAVAGEDRIServer->depthbufferSize);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthOffset:0x%08x\n",pSAVAGEDRIServer->depthOffset);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthPitch:0x%08x\n",pSAVAGEDRIServer->depthPitch);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	textureOffset:0x%08x\n",pSAVAGEDRIServer->textureOffset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	textureSize:0x%08x\n",pSAVAGEDRIServer->textureSize);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	textureSize:0x%08x\n",pSAVAGEDRIServer->textureSize);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	logTextureGranularity:0x%08x\n",pSAVAGEDRIServer->logTextureGranularity);

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agp:handle:0x%08lx\n",pSAVAGEDRIServer->agp.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agp:offset:0x%08lx\n",pSAVAGEDRIServer->agp.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agp:size:0x%08lx\n",pSAVAGEDRIServer->agp.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agp:map:0x%08lx\n",(unsigned long)pSAVAGEDRIServer->agp.map);
   
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:handle:0x%08lx\n",pSAVAGEDRIServer->registers.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:offset:0x%08lx\n",pSAVAGEDRIServer->registers.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:size:0x%08lx\n",pSAVAGEDRIServer->registers.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:map:0x%08lx\n",(unsigned long)pSAVAGEDRIServer->registers.map);
   
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:handle:0x%08lx\n",pSAVAGEDRIServer->status.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:offset:0x%08lx\n",pSAVAGEDRIServer->status.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:size:0x%08lx\n",pSAVAGEDRIServer->status.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:map:0x%08lx\n",(unsigned long)pSAVAGEDRIServer->status.map);
   
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextures:handle:0x%08lx\n",pSAVAGEDRIServer->agpTextures.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextures:offset:0x%08lx\n",pSAVAGEDRIServer->agpTextures.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextures:size:0x%08lx\n",pSAVAGEDRIServer->agpTextures.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	apgTextures:map:0x%08lx\n",(unsigned long)pSAVAGEDRIServer->agpTextures.map);

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	logAgpTextureGranularity:0x%08x\n",pSAVAGEDRIServer->logAgpTextureGranularity); 

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]pSAVAGEDRI:\n" );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	chipset:0x%08x\n",pSAVAGEDRI->chipset );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	width:0x%08x\n",pSAVAGEDRI->width );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	height:0x%08x\n",pSAVAGEDRI->height );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	mem:0x%08lx\n",pSAVAGEDRI->mem );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	cpp:%d\n",pSAVAGEDRI->cpp );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	zpp:%d\n",pSAVAGEDRI->zpp );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpMode:%d\n",pSAVAGEDRI->agpMode );
   
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontbuffer:0x%08lx\n",pSAVAGEDRI->frontbuffer);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontbufferSize:0x%08x\n",pSAVAGEDRI->frontbufferSize);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontOffset:0x%08x\n",pSAVAGEDRI->frontOffset );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontPitch:0x%08x\n",pSAVAGEDRI->frontPitch );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	frontBitmapDesc:0x%08lx\n",pSAVAGEDRI->frontBitmapDesc );

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backbuffer:0x%08lx\n",pSAVAGEDRI->backbuffer);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backbufferSize:0x%08x\n",pSAVAGEDRI->backbufferSize);     
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backOffset:0x%08x\n",pSAVAGEDRI->backOffset );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backPitch:0x%08x\n",pSAVAGEDRI->backPitch );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	backBitmapDesc:0x%08lx\n",pSAVAGEDRI->backBitmapDesc );
   
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthbuffer:0x%08lx\n",pSAVAGEDRI->depthbuffer);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthbufferSize:0x%08x\n",pSAVAGEDRI->depthbufferSize);  
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthOffset:0x%08x\n",pSAVAGEDRI->depthOffset );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthPitch:0x%08x\n",pSAVAGEDRI->depthPitch );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	depthBitmapDesc:0x%08lx\n",pSAVAGEDRI->depthBitmapDesc );

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	textures:0x%08lx\n",pSAVAGEDRI->textures );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	textureOffset:0x%08x\n",pSAVAGEDRI->textureOffset );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	textureSize:0x%08x\n",pSAVAGEDRI->textureSize );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	logTextureGranularity:0x%08x\n",pSAVAGEDRI->logTextureGranularity );
   
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpBufferOffset:0x%08x\n",pSAVAGEDRI->agpBufferOffset );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextureOffset:0x%08x\n",pSAVAGEDRI->agpTextureOffset );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextureSize:0x%08x\n",pSAVAGEDRI->agpTextureSize );
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	logAgpTextureGranularity:0x%08x\n",pSAVAGEDRI->logAgpTextureGranularity );

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextures:handle:0x%08lx\n",pSAVAGEDRI->agpTextures.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextures:offset:0x%08lx\n",pSAVAGEDRI->agpTextures.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	agpTextures:size:0x%08lx\n",pSAVAGEDRI->agpTextures.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	apgTextures:map:0x%08lx\n",(unsigned long)pSAVAGEDRI->agpTextures.map);

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:handle:0x%08lx\n",pSAVAGEDRI->registers.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:offset:0x%08lx\n",pSAVAGEDRI->registers.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:size:0x%08lx\n",pSAVAGEDRI->registers.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	registers:map:0x%08lx\n",(unsigned long)pSAVAGEDRI->registers.map);

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	aperture:handle:0x%08lx\n",pSAVAGEDRI->aperture.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	aperture:offset:0x%08lx\n",pSAVAGEDRI->aperture.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	aperture:size:0x%08lx\n",pSAVAGEDRI->aperture.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	aperture:map:0x%08lx\n",(unsigned long)pSAVAGEDRI->aperture.map);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	aperturePitch:0x%08x\n",pSAVAGEDRI->aperturePitch);

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	BCIcmdBuf:handle:0x%08lx\n",pSAVAGEDRI->BCIcmdBuf.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	BCIcmdBuf:offset:0x%08lx\n",pSAVAGEDRI->BCIcmdBuf.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	BCIcmdBuf:size:0x%08lx\n",pSAVAGEDRI->BCIcmdBuf.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	BCIcmdBuf:map:0x%08lx\n",(unsigned long)pSAVAGEDRI->BCIcmdBuf.map);

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:handle:0x%08lx\n",pSAVAGEDRI->status.handle);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:offset:0x%08lx\n",pSAVAGEDRI->status.offset);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:size:0x%08lx\n",pSAVAGEDRI->status.size);
   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	status:map:0x%08lx\n",(unsigned long)pSAVAGEDRI->status.map);

   xf86DrvMsg( pScrn->scrnIndex, X_INFO, "[junkers]	sarea_priv_offset:0x%08lx\n",pSAVAGEDRI->sarea_priv_offset);
    
   return TRUE;
}


void SAVAGEDRICloseScreen( ScreenPtr pScreen )
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SavagePtr psav = SAVPTR(pScrn);
   SAVAGEDRIServerPrivatePtr pSAVAGEDRIServer = psav->DRIServerInfo;

/* no DMA now..... */
#if 0
   if ( pSAVAGEDRIServer->drmBuffers ) {
      drmUnmapBufs( pSAVAGEDRIServer->drmBuffers );
      pSAVAGEDRIServer->drmBuffers = NULL;
   }

   drmSAVAGECleanupDMA( psav->drmFD );
#endif

   if ( pSAVAGEDRIServer->status.map ) {
      drmUnmap( pSAVAGEDRIServer->status.map, pSAVAGEDRIServer->status.size );
      pSAVAGEDRIServer->status.map = NULL;
   }

   if ( pSAVAGEDRIServer->registers.map ) {
      drmUnmap( pSAVAGEDRIServer->registers.map, pSAVAGEDRIServer->registers.size );
      pSAVAGEDRIServer->registers.map = NULL;
   }
   

   if ( pSAVAGEDRIServer->agpTextures.map ) {
      drmUnmap( pSAVAGEDRIServer->agpTextures.map, 
                pSAVAGEDRIServer->agpTextures.size );
      pSAVAGEDRIServer->agpTextures.map = NULL;
   }

   if (pSAVAGEDRIServer->status.handle)
       drmRmMap(psav->drmFD,pSAVAGEDRIServer->status.handle);

   if (pSAVAGEDRIServer->registers.handle)
       drmRmMap(psav->drmFD,pSAVAGEDRIServer->registers.handle);

   if (pSAVAGEDRIServer->agpTextures.handle)
       drmRmMap(psav->drmFD,pSAVAGEDRIServer->agpTextures.handle);

/* no DMA now.... */
#if 0
   if ( pSAVAGEDRIServer->buffers.map ) {
      drmUnmap( pSAVAGEDRIServer->buffers.map, pSAVAGEDRIServer->buffers.size );
      pSAVAGEDRIServer->buffers.map = NULL;
   }
   if ( pSAVAGEDRIServer->primary.map ) {
      drmUnmap( pSAVAGEDRIServer->primary.map, pSAVAGEDRIServer->primary.size );
      pSAVAGEDRIServer->primary.map = NULL;
   }
   if ( pSAVAGEDRIServer->warp.map ) {
      drmUnmap( pSAVAGEDRIServer->warp.map, pSAVAGEDRIServer->warp.size );
      pSAVAGEDRIServer->warp.map = NULL;
   }
#endif

   if ( pSAVAGEDRIServer->agp.handle ) {
      drmAgpUnbind( psav->drmFD, pSAVAGEDRIServer->agp.handle );
      drmAgpFree( psav->drmFD, pSAVAGEDRIServer->agp.handle );
      pSAVAGEDRIServer->agp.handle = 0;
      drmAgpRelease( psav->drmFD );
   }

   DRICloseScreen( pScreen );
   
   /*Don't use shadow status any more*/
   psav->ShadowVirtual = NULL;
   psav->ShadowPhysical = 0;

   if(psav->reserved)
      xf86FreeOffscreenLinear(psav->reserved);

   if ( psav->pDRIInfo ) {
      if ( psav->pDRIInfo->devPrivate ) {
	 xfree( psav->pDRIInfo->devPrivate );
	 psav->pDRIInfo->devPrivate = 0;
      }
      DRIDestroyInfoRec( psav->pDRIInfo );
      psav->pDRIInfo = 0;
   }
   if ( psav->DRIServerInfo ) {
      xfree( psav->DRIServerInfo );
      psav->DRIServerInfo = 0;
   }
   if ( psav->pVisualConfigs ) {
      xfree( psav->pVisualConfigs );
   }
   if ( psav->pVisualConfigsPriv ) {
      xfree( psav->pVisualConfigsPriv );
   }
}

void
SAVAGEDRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SavagePtr psav = SAVPTR(pScrn);
    BoxPtr pbox = REGION_RECTS(prgn);
    int nbox  = REGION_NUM_RECTS(prgn);


#if 0
    CHECK_DMA_QUIESCENT(SAVPTR(pScrn), pScrn);
#endif
#if 0
    if(!psav->LockHeld)
       psav->LockHeld = 1;
    SAVAGEDRISetupForSolidFill(pScrn, 0, GXcopy, -1);
    while (nbox--) {
        SAVAGESelectBuffer(pScrn, SAVAGE_BACK);
        SAVAGEDRISubsequentSolidFillRect(pScrn, pbox->x1, pbox->y1,
					 pbox->x2-pbox->x1, pbox->y2-pbox->y1);
        SAVAGESelectBuffer(pScrn, SAVAGE_DEPTH);
        SAVAGEDRISubsequentSolidFillRect(pScrn, pbox->x1, pbox->y1,
					 pbox->x2-pbox->x1, pbox->y2-pbox->y1);
        pbox++;
    }
#endif
#if 0
    {
       SAVAGEDRIPtr pSAVAGEDRI = (SAVAGEDRIPtr)psav->pDRIInfo->devPrivate;
       unsigned int *bcicmd;
       BCI_GET_PTR;
       /*bcicmd = (unsigned int *) psav->BciMem;
       *(bcicmd) = 0x4BCC8000;
       *(bcicmd) = 0x00F80000;
       *(bcicmd) = 0x02000200;
       *(bcicmd) = 0x01000100;*/
       BCI_SEND(0x4BCC8000);
       BCI_SEND(0x00F80000);
       BCI_SEND(0x02000200);
       BCI_SEND(0x01000100);
       sleep(5);
    }
#endif
#if 0
    SAVAGESelectBuffer(pScrn, SAVAGE_FRONT);
    psav->AccelInfoRec->NeedToSync = TRUE;
#endif
}

/*
  This routine is a modified form of XAADoBitBlt with the calls to
  ScreenToScreenBitBlt built in. My routine has the prgnSrc as source
  instead of destination. My origin is upside down so the ydir cases
  are reversed.
*/

void
SAVAGEDRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg,
		   RegionPtr prgnSrc, CARD32 index)
{
    ScreenPtr pScreen = pParent->drawable.pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SavagePtr psav = SAVPTR(pScrn);
    int nbox;
    BoxPtr pbox, pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
    DDXPointPtr pptTmp, pptNew1, pptNew2;
    int xdir, ydir;
    int dx, dy;
    DDXPointPtr pptSrc;
    int screenwidth = pScrn->virtualX;
    int screenheight = pScrn->virtualY;

#if 1
#if 0
    CHECK_DMA_QUIESCENT(SAVPTR(pScrn), pScrn);
#endif
    if(!psav->LockHeld)
       psav->LockHeld = 1;

    pbox = REGION_RECTS(prgnSrc);
    nbox = REGION_NUM_RECTS(prgnSrc);
    pboxNew1 = 0;
    pptNew1 = 0;
    pboxNew2 = 0;
    pboxNew2 = 0;
    pptSrc = &ptOldOrg;

    dx = pParent->drawable.x - ptOldOrg.x;
    dy = pParent->drawable.y - ptOldOrg.y;

    /* If the copy will overlap in Y, reverse the order */
    if (dy>0) {
        ydir = -1;

        if (nbox>1) {
	    /* Keep ordering in each band, reverse order of bands */
	    pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec)*nbox);
	    if (!pboxNew1) return;
	    pptNew1 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec)*nbox);
	    if (!pptNew1) {
	        DEALLOCATE_LOCAL(pboxNew1);
	        return;
	    }
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox) {
	        while ((pboxNext >= pbox) && (pboxBase->y1 == pboxNext->y1))
		  pboxNext--;
	        pboxTmp = pboxNext+1;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp <= pboxBase) {
		    *pboxNew1++ = *pboxTmp++;
		    *pptNew1++ = *pptTmp++;
		}
	        pboxBase = pboxNext;
	    }
	    pboxNew1 -= nbox;
	    pbox = pboxNew1;
	    pptNew1 -= nbox;
	    pptSrc = pptNew1;
	}
    } else {
        /* No changes required */
        ydir = 1;
    }

    /* If the regions will overlap in X, reverse the order */
    if (dx>0) {
        xdir = -1;

        if (nbox > 1) {
	    /*reverse orderof rects in each band */
	    pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec)*nbox);
	    pptNew2 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec)*nbox);
	    if (!pboxNew2 || !pptNew2) {
	        if (pptNew2) DEALLOCATE_LOCAL(pptNew2);
	        if (pboxNew2) DEALLOCATE_LOCAL(pboxNew2);
	        if (pboxNew1) {
		    DEALLOCATE_LOCAL(pptNew1);
		    DEALLOCATE_LOCAL(pboxNew1);
		}
	       return;
	    }
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox) {
	        while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		  pboxNext++;
	        pboxTmp = pboxNext;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp != pboxBase) {
		    *pboxNew2++ = *--pboxTmp;
		    *pptNew2++ = *--pptTmp;
		}
	        pboxBase = pboxNext;
	    }
	    pboxNew2 -= nbox;
	    pbox = pboxNew2;
	    pptNew2 -= nbox;
	    pptSrc = pptNew2;
	}
    } else {
        /* No changes are needed */
        xdir = 1;
    }
#if 0
    SAVAGEDRISetupForScreenToScreenCopy(pScrn, xdir, ydir, GXcopy, -1, -1);
    for ( ; nbox-- ; pbox++)
     {
	 int x1 = pbox->x1;
	 int y1 = pbox->y1;
	 int destx = x1 + dx;
	 int desty = y1 + dy;
	 int w = pbox->x2 - x1 + 1;
	 int h = pbox->y2 - y1 + 1;

	 if ( destx < 0 ) x1 -= destx, w += destx, destx = 0;
	 if ( desty < 0 ) y1 -= desty, h += desty, desty = 0;
	 if ( destx + w > screenwidth ) w = screenwidth - destx;
	 if ( desty + h > screenheight ) h = screenheight - desty;
	 if ( w <= 0 ) continue;
	 if ( h <= 0 ) continue;

	 SAVAGESelectBuffer(pScrn, SAVAGE_BACK);
	 SAVAGEDRISubsequentScreenToScreenCopy(pScrn, x1, y1,
					       destx,desty, w, h);
	 SAVAGESelectBuffer(pScrn, SAVAGE_DEPTH);
	 SAVAGEDRISubsequentScreenToScreenCopy(pScrn, x1,y1,
					       destx,desty, w, h);
     }
    SAVAGESelectBuffer(pScrn, SAVAGE_FRONT);

    if (pboxNew2) {
        DEALLOCATE_LOCAL(pptNew2);
        DEALLOCATE_LOCAL(pboxNew2);
    }
    if (pboxNew1) {
        DEALLOCATE_LOCAL(pptNew1);
        DEALLOCATE_LOCAL(pboxNew1);
    }

    psav->AccelInfoRec->NeedToSync = TRUE;
#endif
#endif
}

static void 
SAVAGEDRISetupForScreenToScreenCopy(
    ScrnInfoPtr pScrn,
    int xdir, 
    int ydir,
    int rop,
    unsigned planemask,
    int transparency_color)
{
    SavagePtr psav = SAVPTR(pScrn);
    int cmd =0;

    cmd = BCI_CMD_RECT | BCI_CMD_DEST_PBD | BCI_CMD_SRC_PBD_COLOR;
    BCI_CMD_SET_ROP( cmd, XAAGetCopyROP(rop) );
    if (transparency_color != -1)
        cmd |= BCI_CMD_SEND_COLOR | BCI_CMD_SRC_TRANSPARENT;

    if (xdir == 1 ) cmd |= BCI_CMD_RECT_XP;
    if (ydir == 1 ) cmd |= BCI_CMD_RECT_YP;

    psav->SavedBciCmd = cmd;
    psav->SavedBgColor = transparency_color;

}

static void 
SAVAGEDRISubsequentScreenToScreenCopy(
    ScrnInfoPtr pScrn,
    int x1,
    int y1,
    int x2,
    int y2,
    int w,
    int h)
{
    SavagePtr psav = SAVPTR(pScrn);
    BCI_GET_PTR;

    if (!w || !h) return;
    if (!(psav->SavedBciCmd & BCI_CMD_RECT_XP)) {
        w --;
        x1 += w;
        x2 += w;
        w ++;
    }
    if (!(psav->SavedBciCmd & BCI_CMD_RECT_YP)) {
        h --;
        y1 += h;
        y2 += h;
        h ++;
    }

    psav->WaitQueue(psav,6);
    BCI_SEND(psav->SavedBciCmd);
    if (psav->SavedBgColor != -1) 
	BCI_SEND(psav->SavedBgColor);
    BCI_SEND(BCI_X_Y(x1, y1));
    BCI_SEND(BCI_X_Y(x2, y2));
    BCI_SEND(BCI_W_H(w, h));

}


/*
 * SetupForSolidFill is also called to set up for lines.
 */ 
#if 0
static void 
SAVAGEDRISetupForSolidFill(
    ScrnInfoPtr pScrn,
    int color, 
    int rop,
    unsigned planemask)
{
    SavagePtr psav = SAVPTR(pScrn);
    XAAInfoRecPtr xaaptr = GET_XAAINFORECPTR_FROM_SCRNINFOPTR( pScrn );
    int cmd;
    int mix;

    cmd = BCI_CMD_RECT
        | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_DEST_PBD | BCI_CMD_SRC_SOLID;

    /* Don't send a color if we don't have to. */

    if( rop == GXcopy )
    {
	if( color == 0 )
	    rop = GXclear;
	else if( color == xaaptr->FullPlanemask )
	    rop = GXset;
    }

#if 1
    mix = SavageHelpSolidROP( pScrn, &color, planemask, &rop );
#endif
    if( mix & ROP_PAT )
	cmd |= BCI_CMD_SEND_COLOR;

    BCI_CMD_SET_ROP( cmd, rop );

    psav->SavedBciCmd = cmd;
    psav->SavedFgColor = color;
}
#endif    
    
static void 
SAVAGEDRISubsequentSolidFillRect(
    ScrnInfoPtr pScrn,
    int x,
    int y,
    int w,
    int h)
{
    SavagePtr psav = SAVPTR(pScrn);
    BCI_GET_PTR;
    
    if( !w || !h )
	return;

    psav->WaitQueue(psav,5);

    BCI_SEND(psav->SavedBciCmd);
    if( psav->SavedBciCmd & BCI_CMD_SEND_COLOR )
	BCI_SEND(psav->SavedFgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
}


static Bool
SAVAGEDRIOpenFullScreen(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  vgaHWPtr hwp = VGAHWPTR(pScrn);
  SavagePtr psav = SAVPTR(pScrn);
  unsigned int vgaCRIndex = hwp->IOBase + 4;
  unsigned int vgaCRReg = hwp->IOBase + 5;
  SAVAGEDRIPtr pSAVAGEDRI = (SAVAGEDRIPtr)psav->pDRIInfo->devPrivate;
  unsigned int TileStride;
  unsigned int WidthinTiles;
  unsigned int depth;
  
  OUTREG(0x48C18, INREG(0x48C18) & 0xFFFFFFF7);
  /*VGAOUT8(vgaCRIndex,0x66);
  VGAOUT8(vgaCRReg, VGAIN8(vgaCRReg)|0x10);*/
  VGAOUT8(vgaCRIndex,0x69);
  VGAOUT8(vgaCRReg, 0x80);
  
  depth = pScrn->bitsPerPixel;
  
  if(depth == 16)
  {
      WidthinTiles = (pSAVAGEDRI->width+63)>>6;
      TileStride = (pSAVAGEDRI->width+63)&(~63);
  
  }
  else
  {
      WidthinTiles = (pSAVAGEDRI->width+31)>>5;
      TileStride = (pSAVAGEDRI->width+31)&(~31);
  
  }  


  /* set primary stream stride */
  {
      unsigned int value;
      
      /*value = 0x80000000|(WidthinTiles<<24)|(TileStride*depth/8);*/
      value = 0x80000000|(WidthinTiles<<24);
      if(depth == 32)
          value |= 0x40000000;
      
      OUTREG(PRI_STREAM_STRIDE, value);
  
  }
  
  /* set global bitmap descriptor */
  {
      unsigned int value;
      value = 0x10000000|
              0x00000009|
              0x01000000|
              (depth<<16)  | TileStride;
  
      OUTREG(0x816C,value);
  
  }
 
   OUTREG(0x48C18, INREG(0x48C18) | 0x8);
   
  return TRUE;
}

static Bool
SAVAGEDRICloseFullScreen(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  SavagePtr psav = SAVPTR(pScrn);
  SAVAGEDRIPtr pSAVAGEDRI = (SAVAGEDRIPtr)psav->pDRIInfo->devPrivate;
  unsigned int depth = pScrn->bitsPerPixel;
  BCI_GET_PTR;

  BCI_SEND(0xC0FF0000);
  psav->WaitIdleEmpty(psav);
  OUTREG(0x48C18, INREG(0x48C18) & 0xFFFFFFF7);
  /* set primary stream stride */
  {
      /* MM81C0 and 81C4 are used to control primary stream. */
      OUTREG32(PRI_STREAM_FBUF_ADDR0,0x00000000);
      OUTREG32(PRI_STREAM_FBUF_ADDR1,0x00000000);
      
      /* FIFO control */
      OUTREG32(0X81EC,0Xffffffff);
      
      if (!psav->bTiled) {
          OUTREG32(PRI_STREAM_STRIDE,
                   (((psav->lDelta * 2) << 16) & 0x3FFFE000) |
                   (psav->lDelta & 0x00001fff));
      }
      else if (pScrn->bitsPerPixel == 16) {
          /* Scanline-length-in-bytes/128-bytes-per-tile * 256 Qwords/tile */
          OUTREG32(PRI_STREAM_STRIDE,
                   (((psav->lDelta * 2) << 16) & 0x3FFFE000)
                   | 0x80000000 | (psav->lDelta & 0x00001fff));
      }
      else if (pScrn->bitsPerPixel == 32) {
          OUTREG32(PRI_STREAM_STRIDE,
                   (((psav->lDelta * 2) << 16) & 0x3FFFE000)
                   | 0xC0000000 | (psav->lDelta & 0x00001fff));
      }
      
      
  }
  
  /* set global bitmap descriptor */
      {
          OUTREG32(S3_GLB_BD_LOW,psav->GlobalBD.bd2.LoPart );
          OUTREG32(S3_GLB_BD_HIGH,psav->GlobalBD.bd2.HiPart | BCI_ENABLE | S3_LITTLE_ENDIAN | S3_BD64);
          
      }
  
  OUTREG(0x48C18, INREG(0x48C18) | 0x8);
  return TRUE;
}
/* get the physics address (for shadow status)*/
unsigned long SAVAGEDRIGetPhyAddress(ScrnInfoPtr pScrn,void * pointer)
{

  SavagePtr psav = SAVPTR(pScrn);

  drm_savage_get_physcis_address_t req;
  int ret;

  req.v_address = (unsigned long )pointer;
  ret = ioctl(psav->drmFD, DRM_IOCTL_SAVAGE_GET_PHYSICS_ADDRESS,&req);
  if (ret != 1)
    fprintf(stderr,"[GetPhyAddress],Wrong virtual address.\n");
  return req.p_address;
}

#endif
