/*****************************************************************************
 *  Module for Microchip Graphics Library
 *  Custom display controller driver template
 *****************************************************************************
 * FileName:            MicrochipGraphicsModule.c
 * Dependencies:    Graphics.h
 * Processor:           PIC24
 * Compiler:       	MPLAB C30
 * Linker:                MPLAB LINK30
 * Company:            Microchip Technology Incorporated
 *
 * Software License Agreement
 *
 * Copyright � 2008 Microchip Technology Inc.  All rights reserved.
 * Microchip licenses to you the right to use, modify, copy and distribute
 * Software only when embedded on a Microchip microcontroller or digital
 * signal controller, which is integrated into your product or third party
 * product (pursuant to the sublicense terms in the accompanying license
 * agreement).  
 *
 * You should refer to the license agreement accompanying this Software
 * for additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 * BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY
 * CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
 * OR OTHER SIMILAR COSTS.
 *
 * Date				Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 8/20/09          Initial Version
 * 12/03/09         Added Double Buffering Support
 * 03/29/10         Fixed EPMP base address programming
 * 04/04/10      	Added block scrolling 
 * 07/12/10         Added EDS type images
 * 09/27/10         Added CHRGPU & IPU support
 *****************************************************************************/
#include "Graphics/Graphics.h"

#if (DISPLAY_PANEL == TFT_G240320LTSW_118W_E)

    #include "TCON_SSD1289.c"

#elif (DISPLAY_PANEL == TFT_G320240DTSW_69W_TP_E)

    #include "TCON_HX8238.c"

#elif (DISPLAY_PANEL == PH480272T_005_I06Q)

    #include "TCON_HX8257.c"

#else

    #include "TCON_Custom.c"

#endif

#ifdef USE_PALETTE

    #include "Graphics/Palette.h"

    extern void *_palette;
    extern BYTE PaletteBpp;
    extern BYTE blPaletteChangeError;
    extern void *pPendingPalette;
    extern WORD PendingStartEntry;
    extern WORD PendingLength;

#endif

#ifdef USE_COMP_IPU
    
    #if (GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE > 1024)
        #error "GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE must be <= 1024"
    #endif
    
    #ifndef GFX_DECOMPRESSED_BUFFER_SIZE
        #error "GFX_DECOMPRESSED_BUFFER_SIZE is not defined. This must be defined in Hardware Profile."
    #endif
    
#endif // #ifdef USE_COMP_IPU

// Color
WORD_VAL    _color;
WORD_VAL    _chrcolor;

// Clipping region control
SHORT       _clipRgn;

// Clipping region borders
SHORT       _clipLeft;
SHORT       _clipTop;
SHORT       _clipRight;
SHORT       _clipBottom;

// work areas base addresses
volatile DWORD 		_workArea1BaseAddr;
volatile DWORD 		_workArea2BaseAddr;

// display buffer base address
volatile DWORD 		_displayAreaBaseAddr;

#ifdef USE_DOUBLE_BUFFERING

    volatile DWORD  _drawbuffer;
    volatile BYTE   blDisplayUpdatePending;

#endif //USE_DOUBLE_BUFFERING

#define GFX_FLIP(a,b) { SHORT t=a; a=b; b=t; }

/////////////////////// LOCAL FUNCTIONS PROTOTYPES ////////////////////////////
SHORT 	DrvGetImageColorDepth(void *pImage);
SHORT   DrvGetImageHeight(void *pImage);
SHORT   DrvGetImageWidth(void *pImage);
WORD 	DrvOutChar(XCHAR ch);
SHORT   DrvGetTextWidth(XCHAR *textString, void *pFont);
SHORT   DrvGetTextHeight(void *pFont);
void    SetDrvFont(void *font, FONT_HEADER *pHeader);
#define DrvGetX() (G1CHRX)
#define DrvGetY() (G1CHRY)

#if defined (GFX_EPMP_CS1_BASE_ADDRESS) || defined (GFX_EPMP_CS2_BASE_ADDRESS)

void EPMP_Init(void)
{
	/* Note: When using the EPMP to access external RAM or Flash, PMA0-PMA16 will only access a range of 
	         256K RAM. To increase this range enable higher Address lines.
	*/
	ANSDbits.ANSD7 = 0;   // PMD15
	ANSDbits.ANSD6 = 0;   // PMD14
	ANSFbits.ANSF0 = 0;   // PMD11

	ANSBbits.ANSB15 = 0;  // PMA0
	ANSBbits.ANSB14 = 0;  // PMA1
	ANSGbits.ANSG9  = 0;  // PMA2
	ANSBbits.ANSB13 = 0;  // PMA10
	ANSBbits.ANSB12 = 0;  // PMA11
	ANSBbits.ANSB11 = 0;  // PMA12
	ANSBbits.ANSB10 = 0;  // PMA13
	ANSAbits.ANSA7 = 0;   // PMA17
	ANSGbits.ANSG6 = 0;   // PMA18

	PMCON1bits.ADRMUX = 0;	    								// address is not multiplexed
	PMCON1bits.MODE = 3;        								// master mode
	PMCON1bits.CSF = 0;         								// PMCS1 pin used for chip select 1, PMCS2 pin used for chip select 2
	PMCON1bits.ALP = 1;         								// set address latch strobes to high active level (for sn74lvc16373)
	PMCON1bits.ALMODE = 1;      								// "smart" address strobes are not used
	PMCON1bits.BUSKEEP = 0;     								// bus keeper is not used

  	#if defined (GFX_EPMP_CS1_BASE_ADDRESS)
		PMCS1BS = (GFX_EPMP_CS1_BASE_ADDRESS>>8);			// CS1 start address
		
		// check if how many address lines to use
		// check for 17 address lines (PMA[16:0])
		// note that GFX_EPMP_CS1_MEMORY_SIZE is in bytes
		#if ((GFX_EPMP_CS1_MEMORY_SIZE) < 0x80000)
			PMCON3 |= 0x0003;     							// PMA17, PMA16 address lines are enabled 
		#elif ((GFX_EPMP_CS1_MEMORY_SIZE) < 0x100000)
			PMCON3 |= 0x0007;     							// PMA18, PMA17, PMA16 address lines are enabled 
		#else
			PMCON3 |= 0x0001;           					// PMA16 address line is enabled 
		#endif	     					

		PMCS1CFbits.CSDIS = 0;      							// enable CS
		PMCS1CFbits.CSP = EPMPCS1_CS_POLARITY;        			// CS1 polarity 
		PMCS1CFbits.BEP = EPMPCS1_BE_POLARITY;       	 		// byte enable polarity
		PMCS1CFbits.WRSP = EPMPCS1_WR_POLARITY;       			// write strobe polarity
		PMCS1CFbits.RDSP = EPMPCS1_RD_POLARITY;       			// read strobe polarity
		PMCS1CFbits.CSPTEN = 1;     							// enable CS port
		PMCS1CFbits.SM = 0;         							// read and write strobes on separate lines 
		PMCS1CFbits.PTSZ = 2;       							// data bus width is 16-bit 
	
		PMCS1MDbits.ACKM = 0;        							// PMACK is not used

		// The device timing parameters. Set the proper timing
		// according to the device used (the timing macros are defined in the hardware profile)
		PMCS1MDbits.DWAITB = EPMPCS1_DWAITB;      				// access time 1 Tcy
		PMCS1MDbits.DWAITM = EPMPCS1_DWAITM;
		PMCS1MDbits.DWAITE = EPMPCS1_DWAITE;
		PMCS1MDbits.AMWAIT = EPMPCS1_AMWAIT;					// Note: adjust this delay for slower devices 

	#else
			PMCS1CFbits.CSDIS = 1;       						// disable CS1 functionality  
	#endif //#if defined (GFX_EPMP_CS1_BASE_ADDRESS)

	#if defined (GFX_EPMP_CS2_BASE_ADDRESS)

		PMCS2BS = (GFX_EPMP_CS2_BASE_ADDRESS>>8);				// CS2 start address

		#if ((GFX_EPMP_CS2_MEMORY_SIZE) < 0x80000)
			PMCON3 |= 0x0003;     							// PMA17, PMA16 address lines are enabled 
		#elif ((GFX_EPMP_CS2_MEMORY_SIZE) < 0x100000)
			PMCON3 |= 0x0007;     							// PMA18, PMA17, PMA16 address lines are enabled 
		#else
			PMCON3 |= 0x0001;           					// PMA16 address line is enabled 
		#endif	     					

		PMCS2CFbits.CSDIS = 0;      							// enable CS

		PMCS2CFbits.CSP = EPMPCS2_CS_POLARITY;        			// CS2 polarity 
		PMCS2CFbits.BEP = EPMPCS2_BE_POLARITY;       	 		// byte enable polarity
		PMCS2CFbits.WRSP = EPMPCS2_WR_POLARITY;       			// write strobe polarity
		PMCS2CFbits.RDSP = EPMPCS2_RD_POLARITY;       			// read strobe polarity
		PMCS2CFbits.CSPTEN = 1;     							// enable CS port
		PMCS2CFbits.SM = 0;         							// read and write strobes on separate lines 
		PMCS2CFbits.PTSZ = 2;       							// data bus width is 16-bit 
	
		PMCS2MDbits.ACKM = 0;        							// PMACK is not used
		
		// The device timing parameters. Set the proper timing
		// according to the device used (the timing macros are defined in the hardware profile)
		PMCS2MDbits.DWAITB = EPMPCS2_DWAITB;      				// access time 1 Tcy
		PMCS2MDbits.DWAITM = EPMPCS2_DWAITM;
		PMCS2MDbits.DWAITE = EPMPCS2_DWAITE;
		PMCS2MDbits.AMWAIT = EPMPCS2_AMWAIT;					// Note: adjust this delay for slower devices 


	#else	
		PMCS2CFbits.CSDIS = 1;       							// disable CS2 functionality  
		PMCS2BS = 0x0000;
	#endif //#if defined (GFX_EPMP_CS2_BASE_ADDRESS)
	
	PMCON2bits.RADDR = 0xFF;									// set CS2 end address
	PMCON4 = 0xFFFF;            								// PMA0 - PMA15 address lines are enabled
	

	PMCON3bits.PTWREN = 1;      								// enable write strobe port
	PMCON3bits.PTRDEN = 1;      								// enable read strobe port
	PMCON3bits.PTBE0EN = 1;     								// enable byte enable port
	PMCON3bits.PTBE1EN = 1;     								// enable byte enable port
	PMCON3bits.AWAITM = 0;      								// set address latch pulses width to 1/2 Tcy
	PMCON3bits.AWAITE = 0;      								// set address hold time to 1/4 Tcy
	
	DelayMs(100);

	PMCON2bits.MSTSEL = 3;										// select EPMP bypass mode (for Graphics operation)
	PMCON1bits.PMPEN = 1;										// enable the module

	DelayMs(100);

}

#endif //#if defined (GFX_EPMP_CS1_BASE_ADDRESS) || defined (GFX_EPMP_CS2_BASE_ADDRESS)

/*********************************************************************
* Function:  void ResetDevice()
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: Resets LCD, initializes PMP. FRM Section 43. Graphics 
*			Controller Module (GFX) (Document #:DS39731) for details.
*
* Note: none
*
********************************************************************/
void ResetDevice(void)
{
#ifdef USE_DOUBLE_BUFFERING

    blInvalidateAll = 1;
    blDisplayUpdatePending = 0;
    NoOfInvalidatedRectangleAreas = 0;
    _drawbuffer = GFX_BUFFER1;
    SwitchOnDoubleBuffering();

#endif //USE_DOUBLE_BUFFERING


	/* Switch On display if IO controlled; if controlled 
	   by the Graphics Module, hook up the power-on signal of the display to
	   GPWR pin of the Graphics Module and this code should not compile */
	   
    DisplayPowerOn();
    DisplayPowerConfig();


#if defined (GFX_EPMP_CS1_BASE_ADDRESS) || defined (GFX_EPMP_CS2_BASE_ADDRESS)

    EPMP_Init();
	
#endif

/************ End - Project Specific Code - Relocate ************/


    G1CON1   = 0; /* Switch off the module */
    G1CON2   = 0;
    G1CON3   = 0;
    G1IE     = 0;
    G1IR     = 0;
    G1CLUT   = 0;
    G1MRGN   = 0;
    G1CLUTWR = 0;
    G1CHRX   = 0;
    G1CHRY   = 0;
    G1CMDL   = 0;
    G1CMDH   = 0;
    
    // set the  processing unit bit per pixel
    _PUBPP   = GFX_BITS_PER_PIXEL;
    // set the  display controller bit per pixel
    _DPBPP   = GFX_BITS_PER_PIXEL;
    // set the LCD type used (TFT, MSTN or CSTN)
    _DPMODE  = GFX_LCD_TYPE;

    #if (GFX_LCD_TYPE == GFX_LCD_MSTN) || (GFX_LCD_TYPE == GFX_LCD_CSTN)
		// set the display width
        _DPGWDTH = STN_DISPLAY_WIDTH;

    #endif

    /* Port configurations */
    #ifdef GFX_DISPLAYENABLE_ENABLE
    	// set the display enable polarity
        _DPENPOL = GFX_DISPLAYENABLE_POLARITY;
        _DPENOE = 1;

    #endif
    
    #ifdef GFX_HSYNC_ENABLE
		// set the HSYNC signal polarity
        _DPHSPOL = GFX_HSYNC_POLARITY;
        _DPHSOE = 1;

    #endif
    
    #ifdef GFX_VSYNC_ENABLE
		// set the VSYNC signal polarity
        _DPVSPOL = GFX_VSYNC_POLARITY;
        _DPVSOE = 1;
    
    #endif
    
    #ifdef GFX_DISPLAYPOWER_ENABLE
    	// set the display power (GPWR) signal polarity 
        _DPPOWER = GFX_DISPLAYPOWER_POLARITY;
        _DPPWROE = 1;
    
    #endif
    
    /* Display timing signal configurations */
    // set the clock polarity	
    _DPCLKPOL = GFX_CLOCK_POLARITY;
    
    // set the display buffer dimension
    G1DPW = DISP_HOR_RESOLUTION;
    G1DPH = DISP_VER_RESOLUTION;
    
    // set the work area dimension
    G1PUW = DISP_HOR_RESOLUTION;
    G1PUH = DISP_VER_RESOLUTION;
    
    /* Note:
    	In some display panel the definition of porches (front and back porches) varies.
    	Example TFT display definitions (for horizontal timing):
    		1. Horizontal Cycle = horizontal front porch + horizontal back porch + horizontal display period
    		2. Horizontal Cycle = horizontal front porch + horizontal back porch + horizontal display period + horizontal sync pulse width
		In example (1) the horizontal sync pulse width must not exceed the horizontal back porch.
		For the vertical timing, the equations are the same (replace horizontal with vertical).
		
		For the Microchip graphics controller: the definition follows example (2). To accomodate displays like
		example (1), adjust the back porches and pulse widths accordingly. Refer to 
		FRM Section 43. Graphics Controller Module (GFX) (Document #:DS39731).
    */
    #define HT  (DISP_HOR_PULSE_WIDTH + DISP_HOR_BACK_PORCH + DISP_HOR_FRONT_PORCH + DISP_HOR_RESOLUTION)
    #define VT  (DISP_VER_PULSE_WIDTH + DISP_VER_BACK_PORCH + DISP_VER_FRONT_PORCH + DISP_VER_RESOLUTION)

    G1DPWT = HT;
    G1DPHT = VT;

	// set the horizontal pulse width
    _HSLEN = DISP_HOR_PULSE_WIDTH;
    _HSST  = 0; 
    
	// set the verrizontal pulse width
    _VSLEN = DISP_VER_PULSE_WIDTH;
    _VSST  = 0; 

	// set the horizontal & vertical start position
    _HENST = _HSST + DISP_HOR_PULSE_WIDTH + DISP_HOR_BACK_PORCH;
    _VENST = _VSST + DISP_VER_PULSE_WIDTH + DISP_VER_BACK_PORCH;

	// set the active pixel and active line start position
	_ACTPIX  = _HENST;
	_ACTLINE = _VENST;
	
	// initialize the work areas and display buffer base addresses global variables
    _workArea1BaseAddr   = GFX_DISPLAY_BUFFER_START_ADDRESS;
	_workArea2BaseAddr   = GFX_DISPLAY_BUFFER_START_ADDRESS;
	_displayAreaBaseAddr = GFX_DISPLAY_BUFFER_START_ADDRESS;

    // Set the display buffer base address (SFR) (or the start addresses in RAM) 
    GFX_SetDisplayArea(_displayAreaBaseAddr);	

    /* Switch On the Graphics Module */
    _GDBEN = 0xFFFF;
    _DPPINOE = 1;

    #ifdef USE_PALETTE
    	// initialize the color look up table (CLUT) if enabled
        PaletteInit();
    
    #endif
        
    _G1CLKSEL = 1;  				// 96MHz Enable
    _GCLKDIV = GFX_GCLK_DIVIDER;	// value used is dependent on the display panel specification
    _DPSTGER = 0;					// display data stagger (set to none)
    _GFX1IF = 0;					// clear graphics interrupt flag 
    _GFX1IE = 0;					// graphics interrupt are not enabled
    _G1EN = 1;						// turn on the graphics module

    DelayMs(100);
    
    _DPPWROE = 1;					// enable the display power sequencer port to function as GPWR
    _DPPOWER = 1;					// turn on the display power sequencer	

	// clear the screen to all black first to remove the noise screen
	SetColor(0);
	Bar(0,0,GetMaxX(),GetMaxY());

    TCON_Init();    				// Panel Timing Controller (TCON) Programming

    DelayMs(100);

	// disable clipping (default setting)    
    SetClip(0);
}

/*********************************************************************
* Function: SetClip(control)
*
* Overview: Enables/disables clipping.
*
* PreCondition: none
*
* Input: control - Enables or disables the clipping.
*			- 0: Disable clipping
*			- 1: Enable clipping
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void SetClip(BYTE control)
{
    _clipRgn=control;
    
    if(_clipRgn)
    {
        GFX_WaitForCommandQueue(2);
        GFX_CHR_SetTextAreaStart(_clipLeft, _clipTop);
        GFX_CHR_SetTextAreaEnd(_clipRight, _clipBottom) ;
    }
    else
    {
        GFX_WaitForCommandQueue(2);
        GFX_CHR_SetTextAreaStart(0, 0);
        GFX_CHR_SetTextAreaEnd(GetMaxX(), GetMaxY());
    }
}

/*********************************************************************
* Function: SetClipRgn(left, top, right, bottom)
*
* Overview: Sets clipping region.
*
* PreCondition: none
*
* Input: left - Defines the left clipping region border.
*		 top - Defines the top clipping region border.
*		 right - Defines the right clipping region border.
*	     bottom - Defines the bottom clipping region border.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void SetClipRgn(SHORT left, SHORT top, SHORT right, SHORT bottom)
{
    _clipLeft=left;
    _clipTop=top;
    _clipRight=right;
    _clipBottom=bottom;

    if(_clipRgn)
    {
        SetClip(_clipRgn);
    }
}

/*********************************************************************
* Function: WORD ROPBlock (DWORD srcAddr,   DWORD dstAddr, 
*						   DWORD srcOffset, DWORD dstOffset, 
*			               WORD  srcType,   WORD  dstType,  
*                          WORD  copyOp,    WORD rop, 
*			               WORD color,      WORD width,  WORD height)
*
* PreCondition: none
*
* Input: srcAddr - the base address of the data to be moved
*        dstAddr - the base address of the new location of the moved data 
*        srcOffset - offset of the data to be moved with respect to the 
*					 source base address.
*        dstOffset - offset of the new location of the moved data respect 
*					 to the source base address.
*        srcType - sets the source type (continuous or discontinuous)
*        dstType - sets the source type (continuous or discontinuous) 
*        copyOp - sets the type of copy operation
*			- RCC_SOLID_FILL: Solid fill of the set color
*			- RCC_COPY: direct copy of source to destination
*			- RCC_TRANSPARENT_COPY: copy with transparency. Transparency color is set by color
*        rop - sets the raster operation equation
*			- RCC_ROP_0: Solid black color fill 
*			- RCC_ROP_1: not (Source or Destination)
*			- RCC_ROP_2: (not Source) and Destination
*			- RCC_ROP_3: not Source 
*			- RCC_ROP_4: Source and (not Destination)
*			- RCC_ROP_5: not Destination
*			- RCC_ROP_6: Source xor Destination
*			- RCC_ROP_7: not (Source and Destination) 
*			- RCC_ROP_8: Source and Destination
*			- RCC_ROP_9: not (Source xor Destination) 
*			- RCC_ROP_A: Destination
*			- RCC_ROP_B: (not Source) or Destination 
*			- RCC_ROP_C: Source 
*			- RCC_ROP_D: Source or (not Destination)
*			- RCC_ROP_E: Source or Destination
*			- RCC_ROP_F: Solid white color fill 
*        color - color value used when transparency operation is set or using solid color fill
*        width - width of the block of data to be moved
*        height - height of the block of data to be moved
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and operation is not completely performed.
*         - Returns 1 when the operation is completely performed.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: Performs the rectangle copy operation with the indicated 
*           type of copy and raster operation. 
*
* Note: none
*
********************************************************************/
WORD ROPBlock(DWORD srcAddr, DWORD dstAddr, DWORD srcOffset, DWORD dstOffset, 
			   WORD srcType, WORD dstType, WORD copyOp, WORD rop, WORD color, WORD width, WORD height)
{
	DWORD_VAL workArea1Temp, workArea2Temp;
	
	GFX_WaitForCommandQueue(16);
    GFX_WaitForGpu();
    
    // store the work area settings temporarily
    workArea1Temp.w[0] = G1W1ADRL;
    workArea1Temp.w[1] = G1W1ADRH;
    workArea2Temp.w[0] = G1W2ADRL;
    workArea2Temp.w[1] = G1W2ADRH;

	GFX_SetWorkArea1(srcAddr);
	GFX_SetWorkArea2(dstAddr);
	
	GFX_RCC_SetSrcOffset(srcOffset);
	GFX_RCC_SetDestOffset(dstOffset);

	GFX_RCC_SetSize(width, height);
	
	if ((copyOp == RCC_TRANSPARENT_COPY) || (copyOp == RCC_SOLID_FILL))
		GFX_RCC_SetColor(color);

	GFX_RCC_StartCopy(copyOp, rop, srcType, dstType);

    GFX_WaitForCommandQueue(16);
    GFX_WaitForGpu();

    // restore the work area settings 
    G1W1ADRL = workArea1Temp.w[0];
    G1W1ADRH = workArea1Temp.w[1];
    G1W2ADRL = workArea2Temp.w[0];
    G1W2ADRH = workArea2Temp.w[1];

	return (1);
	
}	

/*********************************************************************
* Function: WORD Scroll(SHORT left, SHORT top, SHORT right, SHORT bottom, 
*                       SHORT delta, WORD dir)
*
* PreCondition: none
*
* Input: left - left position of the scrolled rectangle
*        top - top position of the scrolled rectangle
*        right - right position of the scrolled rectangle
*        bottom - bottom position of the scrolled rectangle
*        delta - defines the scroll delta
*        dir - defines the direction of the scroll. 
*		 - 0 : scroll to the left
*		 - 1 : scroll to the right
*
* Output: none
*
* Side Effects: none
*
* Overview: Scrolls the rectangular area defined by left, top, right, bottom by 
*           delta pixels with the dir defining the direction (left or right).
*
* Note: none
*
********************************************************************/
WORD Scroll(SHORT left, SHORT top, SHORT right, SHORT bottom, SHORT delta, WORD dir)
{
	DWORD address;
    SHORT width, height;

	// make sure there are no pending RCC GPU commands
    if(GFX_GetFreeCommandSpace() < 16)
    {
        #ifndef USE_NONBLOCKING_CONFIG
            GFX_WaitForCommandQueue(16);
        #else
            return (0);
        #endif
    }

    #if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180)

        width   =   right - left + 1;
        height  =   bottom - top + 1;

    #elif (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)

        height  =   right - left + 1;
        width   =   bottom - top + 1;

    #endif

    #if (DISP_ORIENTATION == 90)

        bottom  =   DISP_VER_RESOLUTION - left;
        left    =   top;
        right   =   left + width;
        top     =   bottom - height;

    #elif (DISP_ORIENTATION == 180)

        right   =   DISP_HOR_RESOLUTION - left;
        bottom  =   DISP_VER_RESOLUTION - top;
        left    =   right - width;
        top     =   bottom - height;

    #elif (DISP_ORIENTATION == 270)

        right   =   DISP_HOR_RESOLUTION - top;
        top     =   left;
        bottom  =   top + height;
        left    =   right - width;

    #endif
	
	
	GFX_SetWorkArea1(_workArea1BaseAddr);
	GFX_SetWorkArea2(_workArea2BaseAddr); 
		
	// get source address
	address = (top * (DWORD)_PUW) + left;
	GFX_RCC_SetSrcOffset(address);

	if (dir == 0) // go left
	{
		// get destination address
	    #if (DISP_ORIENTATION == 90)
			address = (bottom * (DWORD)_PUW) + left;
		#else	
			address = (top * (DWORD)_PUW) + left - delta;
		#endif	
	}
	else // go right
	{
		// get destination address
	    #if (DISP_ORIENTATION == 90)
			address = ((top-delta) * (DWORD)_PUW) + left;
		#else	
			address = (top * (DWORD)_PUW) + left + delta;
		#endif			
	}
	GFX_RCC_SetDestOffset(address);		
	
	GFX_RCC_SetSize(width, height);
	GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_DEST_ADDR_DISCONTINUOUS,RCC_DEST_ADDR_DISCONTINUOUS);
	
	return (1);
}	


/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates
*
* Output: none
*
* Side Effects: none
*
* Overview: puts pixel
*
* Note: none
*
********************************************************************/
void PutPixel(SHORT x, SHORT y)
{

    if(_clipRgn)
    {
        if(x < _clipLeft)
            return;
        if(x > _clipRight)
            return;
        if(y < _clipTop)
            return;
        if(y > _clipBottom)
            return;
    }

    #if (DISP_ORIENTATION == 90)
    {
        SHORT t = x;
        x = y;
        y = GetMaxX() - t;
    }
    #elif (DISP_ORIENTATION == 180)
    {
        x = GetMaxX() - x;
        y = GetMaxY() - y;
    }
    #elif (DISP_ORIENTATION == 270)
    {
        SHORT t = x;
        x = GetMaxY() - y;
        y = t;
    }
    #endif

    #ifndef USE_DOUBLE_BUFFERING
        GFX_SetWorkArea2(_workArea2BaseAddr);
    #else
        GFX_SetWorkArea2(_drawbuffer);
    #endif

    GFX_WaitForCommandQueue(4);

    GFX_RCC_SetColor(_color.Val);
    GFX_RCC_SetDestOffset((y * (DWORD)DISP_HOR_RESOLUTION) + x);
    GFX_RCC_SetSize(1, 1);
    GFX_RCC_StartCopy(RCC_SOLID_FILL, RCC_ROP_C, RCC_SRC_ADDR_DISCONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);

    /* Note: No need to wait for  complete execution of the command even for Blocking Mode. The next commands will be in the queue & hence will execute only after the completion of this command. */
}

/*********************************************************************
* Function: WORD GetPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates 
*
* Output: pixel color
*
* Side Effects: none
*
* Overview: returns pixel color at x,y position
*
* Note: none
*
********************************************************************/
WORD GetPixel(SHORT x, SHORT y)
{
    volatile WORD getcolor = 0;
    
    if(x < 0 || y < 0 || x > GetMaxX() || y > GetMaxY())
    {
        return 1; /* Don't draw but return 1 */
    }

    #if (DISP_ORIENTATION == 90)
    {
        SHORT t = x;
        x = y;
        y = GetMaxX() - t;
    }
    #elif (DISP_ORIENTATION == 180)
    {
        x = GetMaxX() - x;
        y = GetMaxY() - y;
    }
    #elif (DISP_ORIENTATION == 270)
    {
        SHORT t = x;
        x = GetMaxY() - y;
        y = t;
    }
    #endif

    /* Wait till previous commands are fully executed */
    GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
    GFX_WaitForGpu();

    #ifndef USE_DOUBLE_BUFFERING
        GFX_SetWorkArea1(_workArea1BaseAddr);
    #else
        GFX_SetWorkArea1(_drawbuffer);
    #endif    

    GFX_SetWorkArea2((WORD)&getcolor);

    GFX_WaitForCommandQueue(4);
    
    GFX_RCC_SetSrcOffset((y * (DWORD)DISP_HOR_RESOLUTION) + x);
    GFX_RCC_SetDestOffset(0);
    GFX_RCC_SetSize(1, 1);
    GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_DISCONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);

    /* Wait till the command is fully executed */
    GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
    GFX_WaitForGpu();

    return (getcolor);
}

#ifdef USE_DRV_BAR
/*********************************************************************
* Function: WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
*
* PreCondition: none
*
* Input: left,top - top left corner coordinates,
*        right,bottom - bottom right corner coordinates
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the shape is not yet completely drawn.
*         - Returns 1 when the shape is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: draws rectangle filled with current color
*
* Note: none
*
********************************************************************/
WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
{
    DWORD address;
    SHORT width, height;

    if(left > right)
    {
        return (1); /* Don't draw but return 1 */
    }

    if(top > bottom)
    {
        return (1); /* Don't draw but return 1 */
    }
    
    if(_clipRgn)
    {
        if(left<_clipLeft)
           left = _clipLeft;
        if(right>_clipRight)
           right= _clipRight;
        if(top<_clipTop)
           top = _clipTop;
        if(bottom>_clipBottom)
           bottom = _clipBottom;
    }
    else
    {
            left    = (left < 0)    ? 0: (left > GetMaxX())  ? GetMaxX(): left;
            top     = (top < 0)     ? 0: (top > GetMaxY())   ? GetMaxY(): top;
            right   = (right < 0)   ? 0: (right > GetMaxX()) ? GetMaxX(): right;
            bottom  = (bottom < 0)  ? 0: (bottom > GetMaxY())? GetMaxY(): bottom;
    }

    #if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180)

        width   =   right - left + 1;
        height  =   bottom - top + 1;

    #elif (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)

        height  =   right - left + 1;
        width   =   bottom - top + 1;

    #endif

    #if (DISP_ORIENTATION == 90)

        bottom  =   DISP_VER_RESOLUTION - left;
        left    =   top;
        right   =   left + width;
        top     =   bottom - height;

    #elif (DISP_ORIENTATION == 180)

        right   =   DISP_HOR_RESOLUTION - left;
        bottom  =   DISP_VER_RESOLUTION - top;
        left    =   right - width;
        top     =   bottom - height;

    #elif (DISP_ORIENTATION == 270)

        right   =   DISP_HOR_RESOLUTION - top;
        top     =   left;
        bottom  =   top + height;
        left    =   right - width;

    #endif

    if(GFX_GetFreeCommandSpace() < 4)
    {
        #ifndef USE_NONBLOCKING_CONFIG
            GFX_WaitForCommandQueue(4);
        #else
            return (0);
        #endif
    }

    address = (top * (DWORD)DISP_HOR_RESOLUTION) + left;

    #ifndef USE_DOUBLE_BUFFERING
        GFX_SetWorkArea2(_workArea2BaseAddr);
    #else
        GFX_SetWorkArea2(_drawbuffer);
    #endif

    GFX_RCC_SetColor(_color.Val);
    GFX_RCC_SetDestOffset(address);
    GFX_RCC_SetSize(width, height);
    GFX_RCC_StartCopy(RCC_SOLID_FILL, RCC_ROP_C, RCC_SRC_ADDR_DISCONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
    
    /* Note: No need to wait for  complete execution of the command even for Blocking Mode. The next commands will be in the queue & hence will execute only after the completion of this command. */
    return (1);
}
#endif

#ifdef USE_DRV_CLEARDEVICE
/*********************************************************************
* Function: void ClearDevice(void)
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: clears screen with current color 
*
* Note: none
*
********************************************************************/
void ClearDevice(void)
{
    GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
    GFX_WaitForGpu();

    #ifndef USE_DOUBLE_BUFFERING
        GFX_SetWorkArea2(_workArea2BaseAddr);
    #else
        GFX_SetWorkArea2(_drawbuffer);
    #endif

    GFX_RCC_SetColor(_color.Val);
    GFX_RCC_SetDestOffset(0);
    GFX_RCC_SetSize(DISP_HOR_RESOLUTION, DISP_VER_RESOLUTION);
    GFX_RCC_StartCopy(RCC_SOLID_FILL, RCC_ROP_C, RCC_SRC_ADDR_DISCONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);

    GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
    GFX_WaitForGpu();
}
#endif

#ifdef USE_DRV_LINE
/*********************************************************************
* Function: WORD Line(SHORT x1, SHORT y1, SHORT x2, SHORT y2)
*
* PreCondition: none
*
* Input: x1,y1 - starting coordinates, x2,y2 - ending coordinates
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the shape is not yet completely drawn.
*         - Returns 1 when the shape is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: draws line
*
* Note: none
*
********************************************************************/

WORD Line(SHORT x1, SHORT y1, SHORT x2, SHORT y2)
{
    SHORT   deltaX, deltaY;
    SHORT   error, stepErrorLT, stepErrorGE;
    SHORT   stepX, stepY;
    SHORT   steep;
    SHORT   temp;
    SHORT   style, type;

    if((_lineType == 0) && ((x1 == x2) || (y1 == y2)))
    {
        if(x1 > x2)
        {
            temp = x1;
            x1 = x2;
            x2 = temp;
        }
        else if(y1 > y2)
        {
            temp = y1;
            y1 = y2;
            y2 = temp;
        }

        if(_lineThickness)
        {
            if(x1 == x2)
            {
                if(!Bar(x1 - 1, y1, x2 + 1, y2)) return (0);
            }
            else
            {
                if(!Bar(x1, y1 - 1, x2, y2 + 1)) return (0);
            }
        }
        else
        {
            if(!Bar(x1, y1, x2, y2)) return (0);
        }

        return (1);
    }

    // Move cursor
    MoveTo(x2, y2);

    if(x1 == x2)
    {
        if(y1 > y2)
        {
            temp = y1;
            y1 = y2;
            y2 = temp;
        }

        style = 0;
        type = 1;
        for(temp = y1; temp < y2 + 1; temp++)
        {
            if((++style) == _lineType)
            {
                type ^= 1;
                style = 0;
            }

            if(type)
            {
                PutPixel(x1, temp);
                if(_lineThickness)
                {
                    PutPixel(x1 + 1, temp);
                    PutPixel(x1 - 1, temp);
                }
            }
        }

        return (1);
    }

    if(y1 == y2)
    {
        if(x1 > x2)
        {
            temp = x1;
            x1 = x2;
            x2 = temp;
        }

        style = 0;
        type = 1;
        for(temp = x1; temp < x2 + 1; temp++)
        {
            if((++style) == _lineType)
            {
                type ^= 1;
                style = 0;
            }

            if(type)
            {
                PutPixel(temp, y1);
                if(_lineThickness)
                {
                    PutPixel(temp, y1 + 1);
                    PutPixel(temp, y1 - 1);
                }
            }
        }

        return (1);
    }

    stepX = 0;
    deltaX = x2 - x1;
    if(deltaX < 0)
    {
        deltaX = -deltaX;
        --stepX;
    }
    else
    {
        ++stepX;
    }

    stepY = 0;
    deltaY = y2 - y1;
    if(deltaY < 0)
    {
        deltaY = -deltaY;
        --stepY;
    }
    else
    {
        ++stepY;
    }

    steep = 0;
    if(deltaX < deltaY)
    {
        ++steep;
        temp = deltaX;
        deltaX = deltaY;
        deltaY = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
        temp = stepX;
        stepX = stepY;
        stepY = temp;
        PutPixel(y1, x1);
    }
    else
    {
        PutPixel(x1, y1);
    }

    // If the current error greater or equal zero
    stepErrorGE = deltaX << 1;

    // If the current error less than zero
    stepErrorLT = deltaY << 1;

    // Error for the first pixel
    error = stepErrorLT - deltaX;

    style = 0;
    type = 1;

    while(--deltaX >= 0)
    {
        if(error >= 0)
        {
            y1 += stepY;
            error -= stepErrorGE;
        }

        x1 += stepX;
        error += stepErrorLT;

        if((++style) == _lineType)
        {
            type ^= 1;
            style = 0;
        }

        if(type)
        {
            if(steep)
            {
                PutPixel(y1, x1);
                if(_lineThickness)
                {
                    PutPixel(y1 + 1, x1);
                    PutPixel(y1 - 1, x1);
                }
            }
            else
            {
                PutPixel(x1, y1);
                if(_lineThickness)
                {
                    PutPixel(x1, y1 + 1);
                    PutPixel(x1, y1 - 1);
                }
            }
        }
    }   // end of while

    return (1);
}

#endif

#ifdef USE_DRV_SETFONT
/*********************************************************************
* Function: void SetFont(void* font)
*
* PreCondition: none
*
* Input: pointer to the font image
*
* Output: none
*
* Side Effects: none
*
* Overview: defines current font
*
* Note: none
*
********************************************************************/
void SetFont(void *font)
{
    FONT_HEADER *pHeader;

        #ifdef USE_FONT_EXTERNAL
    FONT_HEADER header;
        #endif
    _font = font;
    switch(*((SHORT *)font))
    {
                #ifdef USE_FONT_FLASH

        case FLASH:
            pHeader = (FONT_HEADER *) ((FONT_FLASH *)font)->address;
            break;

				#endif // USE_FONT_FLASH
                #ifdef USE_FONT_EXTERNAL

                #ifdef USE_DRV_FONT

		case EDS_EPMP:
        	pHeader = &header;
			SetDrvFont(font, pHeader);
			break;

                #endif // USE_DRV_FONT
				
        case EXTERNAL:
            pHeader = &header;
            ExternalMemoryCallback(font, 0, sizeof(FONT_HEADER), pHeader);
            break;

				#endif // USE_FONT_EXTERNAL

        default:
            return;
    }

    _fontFirstChar = pHeader->firstChar;
    _fontLastChar = pHeader->lastChar;
    _fontHeight = pHeader->height;
}
#endif //#ifdef USE_DRV_SETFONT

/*********************************************************************
* Function: void SetDrvFont(void* font, FONT_HEADER *pHeader)
*
* PreCondition: none
*
* Input: font - Pointer to the font image in memory.
*        pHeader - Pointer to the font header. SerDrvFont() populates the 
*                  header with the selected font information.
*
* Output: none
*
* Side Effects: none
*
* Overview: Sets the location of the current font to be used.
*
* Note: none
*
********************************************************************/
void SetDrvFont(void *font, FONT_HEADER *pHeader)
{
//    WORD  fontHeaderSize = sizeof(FONT_HEADER);
	WORD  TempGPUBpp;
	
    // make sure there are no GPU commands pending and GPUs are all idle
	GFX_WaitForCommandQueue(16);		
	while(GFX_IsPuGpuBusy() == 1);

    // make GPU operate on 8 bpp so the font table is easier to operate on
    TempGPUBpp = _PUBPP;
    _PUBPP = GFX_8_BPP;

    // grab the font header from memory
	while(!MoveWindow(((FONT_EXTERNAL*) font)->address, (DWORD)(WORD)pHeader, 0, 0, 								\
	                   RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, sizeof(FONT_HEADER), 1));

    // make sure the data copy terminates
	GFX_WaitForCommandQueue(16);		
	while(GFX_IsPuGpuBusy() == 1);
	
	// reset the bpp back
    _PUBPP = TempGPUBpp;

	// set the font address in the CHRGPU	
    GFX_CHR_SetFont(((FONT_EXTERNAL*) font)->address);
}

#ifdef USE_DRV_GETTEXTWIDTH
/*********************************************************************
* Function: SHORT GetTextWidth(XCHAR* textString, void* font)
*
* PreCondition: none
*
* Input: textString - pointer to the text string,
*        font - pointer to the font
*
* Output: text width in pixels
*
* Side Effects: none
*
* Overview: returns text width for the font
*
* Note: none
*
********************************************************************/
SHORT GetTextWidth(XCHAR *textString, void *font)
{
		#if defined (USE_FONT_RAM) || defined (USE_FONT_FLASH) 
    GLYPH_ENTRY *pChTable;
    FONT_HEADER *pHeader;
    	#endif
        #ifdef USE_FONT_EXTERNAL
    GLYPH_ENTRY chTable;
    FONT_HEADER header;
        #endif

    	#if defined (USE_FONT_RAM) || defined (USE_FONT_FLASH) || defined (USE_FONT_EXTERNAL)
    SHORT       textWidth;
    XCHAR       ch;
    XCHAR       fontFirstChar;
    XCHAR       fontLastChar;
    	#endif

    switch(*((SHORT *)font))
    {
                #ifdef USE_FONT_RAM

        case RAM:
            pHeader = (FONT_HEADER *) ((FONT_RAM *)font)->address;
            fontFirstChar = pHeader->firstChar;
            fontLastChar = pHeader->lastChar;
            pChTable = (GLYPH_ENTRY *) (pHeader + 1);
            textWidth = 0;
            while((unsigned XCHAR)15 < (unsigned XCHAR)(ch = *textString++))
            {
                if((unsigned XCHAR)ch < (unsigned XCHAR)fontFirstChar)
                    continue;
                if((unsigned XCHAR)ch > (unsigned XCHAR)fontLastChar)
                    continue;
                textWidth += (pChTable + ((unsigned XCHAR)ch - (unsigned XCHAR)fontFirstChar))->width;
            }

            return (textWidth);
                #endif
                	    
                #ifdef USE_FONT_FLASH

        case FLASH:
            pHeader = (FONT_HEADER *) ((FONT_FLASH *)font)->address;
            fontFirstChar = pHeader->firstChar;
            fontLastChar = pHeader->lastChar;
            pChTable = (GLYPH_ENTRY *) (pHeader + 1);
            textWidth = 0;
            while((XCHAR)15 < (XCHAR)(ch = *textString++))
            {
                if((XCHAR)ch < (XCHAR)fontFirstChar)
                    continue;
                if((XCHAR)ch > (XCHAR)fontLastChar)
                    continue;
                textWidth += (pChTable + ((XCHAR)ch - (XCHAR)fontFirstChar))->width;
            }

            return (textWidth);
                #endif
                #ifdef USE_FONT_EXTERNAL

				#ifdef USE_DRV_FONT
		case EDS_EPMP:	
   			textWidth = DrvGetTextWidth(textString, font);
            return (textWidth);
				#endif

        case EXTERNAL:
            ExternalMemoryCallback(font, 0, sizeof(FONT_HEADER), &header);
            fontFirstChar = header.firstChar;
            fontLastChar = header.lastChar;
            textWidth = 0;
            while((XCHAR)15 < (XCHAR)(ch = *textString++))
            {
                if((XCHAR)ch < (XCHAR)fontFirstChar)
                    continue;
                if((XCHAR)ch > (XCHAR)fontLastChar)
                    continue;
                ExternalMemoryCallback
                (
                    font,
                    sizeof(FONT_HEADER) + sizeof(GLYPH_ENTRY) * ((XCHAR)ch - (XCHAR)fontFirstChar),
                    sizeof(GLYPH_ENTRY),
                    &chTable
                );
                textWidth += chTable.width;
            }

            return (textWidth);
                #endif

        default:
            return (0);
    }
}
#endif //#ifdef USE_DRV_GETTEXTWIDTH

#ifdef USE_DRV_GETTEXTHEIGHT
/*********************************************************************
* Function: SHORT GetTextHeight(void* font)
*
* PreCondition: none
*
* Input: pointer to the font
*
* Output: character height in pixels
*
* Side Effects: none
*
* Overview: returns characters height for the font
*
* Note: none
*
********************************************************************/
SHORT GetTextHeight(void *font)
{
        #ifdef USE_FONT_EXTERNAL

    char    height;
        #endif
    switch(*((SHORT *)font))
    {
                #ifdef USE_FONT_RAM
        case RAM:
            return ((FONT_HEADER *) ((FONT_RAM *)font)->address)->height;
                #endif
                
                #ifdef USE_FONT_FLASH
        case FLASH:
            return ((FONT_HEADER *) ((FONT_FLASH *)font)->address)->height;
                #endif
                
                #ifdef USE_FONT_EXTERNAL

       			#ifdef USE_DRV_FONT
        case EDS_EPMP:        
            return (DrvGetTextHeight(font));
            	#endif
            	
        case EXTERNAL:
            ExternalMemoryCallback(font, sizeof(FONT_HEADER) - 2, 1, &height);
            return (height);
                #endif

        default:
            return (0);
    }
}
#endif //#ifdef USE_DRV_GETTEXTHEIGHT

#ifdef USE_DRV_OUTCHAR
/*********************************************************************
* Function: WORD OutChar(XCHAR ch)
*
* PreCondition: none
*
* Input: character code
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the character is not yet completely drawn.
*         - Returns 1 when the character is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: outputs a character
*
* Note: none
*
********************************************************************/
WORD OutChar(XCHAR ch)
{
   		#ifdef USE_FONT_FLASH	
    GLYPH_ENTRY *pChTable = NULL;
    	#endif
    BYTE        *pChImage = NULL;

        #ifdef USE_FONT_EXTERNAL
    GLYPH_ENTRY chTable;
    BYTE        chImage[EXTERNAL_FONT_BUFFER_SIZE];
    WORD        imageSize;
    DWORD_VAL   glyphOffset;
        #endif
    SHORT       chWidth = 0;
    SHORT       xCnt, yCnt, x = 0, y;
    BYTE        temp = 0, mask;

        #ifndef USE_NONBLOCKING_CONFIG
    while(IsDeviceBusy() != 0) Nop();

    /* Ready */
        #else
    if(IsDeviceBusy() != 0)
        return (0);
        #endif
    if((XCHAR)ch < (XCHAR)_fontFirstChar)
        return (-1);
    if((XCHAR)ch > (XCHAR)_fontLastChar)
        return (-1);

    switch(*((SHORT *)_font))
    {
                #ifdef USE_FONT_FLASH

        case FLASH:
            pChTable = (GLYPH_ENTRY *) (((FONT_FLASH *)_font)->address + sizeof(FONT_HEADER)) + ((XCHAR)ch - (XCHAR)_fontFirstChar);

            pChImage = (BYTE *) (((FONT_FLASH *)_font)->address + ((DWORD)(pChTable->offsetMSB) << 8) + pChTable->offsetLSB);

            chWidth = pChTable->width;

            break;
                #endif
                #ifdef USE_FONT_EXTERNAL

	    case EDS_EPMP:
			if (DrvOutChar(ch) == 0)
				return 0;
			else
			{			
				MoveTo(DrvGetX(), DrvGetY());
				return 1;
			}
			
        case EXTERNAL:

            // get glyph entry
            ExternalMemoryCallback
            (
                _font,
                sizeof(FONT_HEADER) + ((XCHAR)ch - (XCHAR)_fontFirstChar) * sizeof(GLYPH_ENTRY),
                sizeof(GLYPH_ENTRY),
                &chTable
            );

            chWidth = chTable.width;

            // width of glyph in bytes
            imageSize = 0;
            if(chWidth & 0x0007)
                imageSize = 1;
            imageSize += (chWidth >> 3);

            // glyph image size
            imageSize *= _fontHeight;

            // get glyph image
            glyphOffset.w[1] = (chTable.offsetMSB >> 8);
            glyphOffset.w[0] = (chTable.offsetMSB << 8) + (chTable.offsetLSB);

            ExternalMemoryCallback(_font, glyphOffset.Val, imageSize, &chImage);
            pChImage = (BYTE *) &chImage;

            break;
                #endif

        default:
            break;
    }

    if(_fontOrientation == ORIENT_HOR)
    {
        y = GetY();
        for(yCnt = 0; yCnt < _fontHeight; yCnt++)
        {
            x = GetX();
            mask = 0;
            for(xCnt = 0; xCnt < chWidth; xCnt++)
            {
                if(mask == 0)
                {
                    temp = *pChImage++;
                    mask = 0x01;
                }

                if(temp & mask)
                {
                    PutPixel(x, y);
                }

                x++;
                mask <<= 1;
            }

            y++;
        }

        // move cursor
        _cursorX = x;
    }
    else
    {
        y = GetX();
        for(yCnt = 0; yCnt < _fontHeight; yCnt++)
        {
            x = GetY();
            mask = 0; 
            for(xCnt = 0; xCnt < chWidth; xCnt++)
            {
                if(mask == 0)
                {
                    temp = *pChImage++;
                    mask = 0x01; 
                }

                if(temp & mask)
                {
                    PutPixel(y, x);
                }

                x--;
                mask <<= 1;
            }

            y++;
        }

        // move cursor
        _cursorY = x;
    }

    return (1);
}
#endif // #ifdef USE_DRV_OUTCHAR

/*********************************************************************
* Function: WORD DrvOutChar(XCHAR ch)
*
* PreCondition: none
*
* Input: character code
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the character is not yet completely drawn.
*         - Returns 1 when the character is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: outputs a character
*
* Note: none
*
********************************************************************/
WORD DrvOutChar(XCHAR ch)
{
	typedef enum
    {
        OUTCHAR_IDLE,
        OUTCHAR_WAITTILLDONE,
    } DRV_OUTCHAR_STATES;

    static DRV_OUTCHAR_STATES state = OUTCHAR_IDLE;

	// it is assumed that this code exexutes only when there is at least 
	// 4 empty spaces in the command buffer
	
	if (state == OUTCHAR_IDLE)
	{
		// set the color if the color is being changed
		if(_chrcolor.Val != _color.Val)
		{
			//GFX_WaitForCommandQueue(3);  /* Already 2 commands space will be available */
			GFX_CHR_SetFgColor(_color.Val);
		}
	
		// if using double buffering, work area is managed	
	#ifndef USE_DOUBLE_BUFFERING
		GFX_SetWorkArea1(_workArea1BaseAddr);
	#else
		GFX_SetWorkArea1(_drawbuffer);
	#endif

		// set the position and render the character
		GFX_CHR_SetPrintPos(GetX(), GetY());
		GFX_CHR_PrintChar(ch, CHR_TRANSPARENT);

		// if font orientation is vertical force the x and y position to go to the next line	
		if(_fontOrientation == ORIENT_VER)
			GFX_CHR_PrintChar(0x0a, CHR_TRANSPARENT);
		state = OUTCHAR_WAITTILLDONE;	
	}
	if (state == OUTCHAR_WAITTILLDONE)
	{
		// check if the character was rendered
    #ifdef USE_NONBLOCKING_CONFIG
		if (GFX_GetFreeCommandSpace() != 16) 
			return 0;
		if (GFX_IsChrGpuBusy())
			return 0;
	#else
		GFX_WaitForCommandQueue(16);
		GFX_WaitForGpu();
	#endif	
		
		state = OUTCHAR_IDLE;
	}
	return (1);
}

/*********************************************************************
* Function:  SHORT DrvGetTextWidth(XCHAR *textString, void *pFont)
*
* Overview: This function returns the width of the given string for a given font in EDS.
*
* PreCondition: A valid font table must exist in the location specified 
*               by the pointer pFont.
*
* Input: textString - Pointer to the string.
*        pFont - Pointer to the font.
*
* Output: Returns the string width in pixels.
*
* Side Effects: none
*
********************************************************************/
SHORT   DrvGetTextWidth(XCHAR *textString, void *pFont)
{
	FONT_HEADER header;
  	WORD        fontFirstChar;
    WORD        fontLastChar;
    WORD 		textWidth, TempGPUBpp;
    BYTE        temp;
    static XCHAR       ch;	
    DWORD 		offset;
	

	GFX_WaitForCommandQueue(16);		
	while(GFX_IsPuGpuBusy() == 1);
	
    // make GPU operate on 8 bpp so the font table is easier to operate on
    TempGPUBpp = _PUBPP;
    _PUBPP = GFX_8_BPP;
	
	while(!MoveWindow(((FONT_EXTERNAL *)pFont)->address, (DWORD)(WORD)&header, 0, 0, 
					RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, sizeof(FONT_HEADER), 1));
				
	GFX_WaitForCommandQueue(16);		
	while(GFX_IsPuGpuBusy() == 1);

    fontFirstChar = header.firstChar;
    fontLastChar = header.lastChar;
    textWidth = 0;
	ch = *textString;
	
    while((XCHAR)15 < (XCHAR)(ch = *textString++))
    {
		if((XCHAR)ch < (XCHAR)fontFirstChar)
        	continue;
		if((XCHAR)ch > (XCHAR)fontLastChar)
        	continue;

		// grab the width of the character
		offset = sizeof(FONT_HEADER) + (((XCHAR)ch - (XCHAR)fontFirstChar)*sizeof(GLYPH_ENTRY));
		while(!MoveWindow(((FONT_EXTERNAL*) pFont)->address, (DWORD)(WORD)&temp, offset, 0, 
				RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, 1, 1));

		GFX_WaitForCommandQueue(16);		
		while(GFX_IsPuGpuBusy() == 1);

		textWidth += temp;
	}

    _PUBPP = TempGPUBpp;
    return (textWidth);
}

/*********************************************************************
* Function:  SHORT DrvGetTextHeight(void *pFont)
*
* Overview: This function returns the height of the given font in EDS.
*
* PreCondition: A valid font table must exist in the location specified 
*               by the pointer pFont.
*
* Input: pFont - Pointer to the font.
*
* Output: Returns the height in pixels.
*
* Side Effects: none
*
********************************************************************/
SHORT   DrvGetTextHeight(void *pFont)
{
	FONT_HEADER header;
    WORD 		TempGPUBpp;

	
	GFX_WaitForCommandQueue(16);		
	while(GFX_IsPuGpuBusy() == 1);

    // make GPU operate on 8 bpp so the font table is easier to operate on
    TempGPUBpp = _PUBPP;
    _PUBPP = GFX_8_BPP;
	
	while(!MoveWindow(((FONT_EXTERNAL *)pFont)->address, (DWORD)(WORD)&header, 0, 0, 
					RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, sizeof(FONT_HEADER), 1));
				
	GFX_WaitForCommandQueue(16);		
	while(GFX_IsPuGpuBusy() == 1);

    _PUBPP = TempGPUBpp;
    return (header.height);
}

#ifdef USE_DRV_GETIMAGEWIDTH
/*********************************************************************
* Function: SHORT GetImageWidth(void* bitmap)
*
* PreCondition: none
*
* Input: bitmap - image pointer
*
* Output: none
*
* Side Effects: none
*
* Overview: returns image width
*
* Note: none
*
********************************************************************/
SHORT GetImageWidth(void *bitmap)
{

	#ifdef USE_COMP_IPU
    
    if((*((SHORT *)bitmap) & GFX_COMP_MASK) == COMP_IPU)
    {
	    return ((GFX_IMAGE_HEADER*)bitmap)->width;
	}
	
	#endif
	
    #ifdef USE_BITMAP_EXTERNAL

    SHORT   width;
    #endif
    switch(*((SHORT *)bitmap) & GFX_MEM_MASK)
    {
            #ifdef USE_BITMAP_FLASH

        case FLASH:
            return (*((FLASH_WORD *) ((IMAGE_FLASH *)bitmap)->address + 2));
            #endif
            #ifdef USE_BITMAP_EXTERNAL

		case EDS_EPMP:
        	return DrvGetImageWidth(bitmap);
        	
        case EXTERNAL:
            ExternalMemoryCallback(bitmap, 4, 2, &width);
            return (width);
            #endif

        default:
            return (0);
    }
}
#endif //#ifdef USE_DRV_GETIMAGEWIDTH

#ifdef USE_DRV_GETIMAGEHEIGHT
/*********************************************************************
* Function: SHORT GetImageHeight(void* bitmap)
*
* PreCondition: none
*
* Input: bitmap - image pointer
*
* Output: none
*
* Side Effects: none
*
* Overview: returns image height
*
* Note: none
*
********************************************************************/
SHORT GetImageHeight(void *bitmap)
{

	#ifdef USE_COMP_IPU
    
    if((*((SHORT *)bitmap) & GFX_COMP_MASK) == COMP_IPU)
    {
	    return ((GFX_IMAGE_HEADER*)bitmap)->height;
	}
	
	#endif
	
    #ifdef USE_BITMAP_EXTERNAL

    SHORT   height;
    #endif
    switch(*((SHORT *)bitmap) & GFX_MEM_MASK)
    {
            #ifdef USE_BITMAP_FLASH

        case FLASH:
            return (*((FLASH_WORD *) ((IMAGE_FLASH *)bitmap)->address + 1));
            #endif
            #ifdef USE_BITMAP_EXTERNAL
            
		case EDS_EPMP:
        	return DrvGetImageHeight(bitmap);

        case EXTERNAL:
            ExternalMemoryCallback(bitmap, 2, 2, &height);
            return (height);
            #endif


        default:
            return (0);
    }
}
#endif //#ifdef USE_DRV_GETIMAGEHEIGHT

#ifdef USE_DRV_PUTIMAGE

    #ifdef USE_BITMAP_FLASH

/*********************************************************************
* Function: void PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner,
*        image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE *image, BYTE stretch)
{
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    BYTE                temp = 0;
    WORD                sizeX, sizeY;
    WORD                x, y, yc;
    BYTE                stretchY;
    WORD                palette[2], color;
    BYTE                mask;
    WORD                outputSize;

    WORD            	lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            	*pData; 

    // Move pointer to size information
    flashAddress = image + 2;

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    palette[0] = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    palette[1] = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}
	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 		
		
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

	GFX_SetWorkArea1((WORD)lineBuffer);
   	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    yc = top;
    // store current line data address 
    tempFlashAddress = flashAddress;

	for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
	        // get flash address location of current line being processed
            //flashAddress = tempFlashAddress;

            flashAddress = tempFlashAddress;
            mask = 0;

			// get the location of the line buffer 
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData = lineBuffer;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData = &lineBuffer[outputSize-1];
	        #endif

			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {            

                // Read 8 pixels from flash
                if(mask == 0)
                {
                    temp = *flashAddress++;
                    mask = 0x80;
                }

                // Set color
                if(mask & temp)
                {
                	// Set color
	                #ifdef USE_PALETTE
                    	if (IsPaletteEnabled())
	                    	color = 1;
	                   	else 
	                    	color = palette[1];
	                #else
                    	color = palette[1];
	                #endif
                }
                else
                {
	                // Set color
	                #ifdef USE_PALETTE
                    	if(IsPaletteEnabled())
	                    	color = 0;
	                   	else 
	                    	color = palette[0];
	                #else
                    	color = palette[0];
	                #endif
                }

   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	                *pData++ = color;
	            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	                *pData-- = color;
           	    #endif			

           	    // Shift to the next pixel (note that if image is stretched, shifting is delayed)
           	    if (x%(stretch==IMAGE_X2?2:1) == 0) 
                	mask >>= 1;		

            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

			GFX_WaitForCommandQueue(3);
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);

            yc++;
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)

   	    // adjust for the next line data address 
   	    if (stretch == IMAGE_X2)
        	tempFlashAddress += ((sizeX>>3)+(sizeX%2));
        else	
			tempFlashAddress = flashAddress;
					
    } // for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();
}

/*********************************************************************
* Function: void PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs 16 color image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
        #if (COLOR_DEPTH >= 4)

void PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE *image, BYTE stretch)
{
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    WORD                sizeX, sizeY;
    register WORD       x, y, yc;

    BYTE                temp = 0;
    BYTE       			stretchY;
    WORD                palette[16], color;
    WORD                counter;
    WORD                outputSize;

    WORD            	lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            	*pData; 

    #ifdef USE_PALETTE
    BYTE            	*pByteData; 
    #endif

    // Move pointer to size information
    flashAddress = image + 2;

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}
	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 		
		
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

   	GFX_SetWorkArea1((WORD)lineBuffer);
	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    // Read palette
    for(counter = 0; counter < 16; counter++)
    {
        palette[counter] = *((FLASH_WORD *)flashAddress);
        flashAddress += 2;
    }

    yc = top;

    // store current line data address 
    tempFlashAddress = flashAddress;

	for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
	        // get flash address location of current line being processed
            flashAddress = tempFlashAddress;

			// get the location of the line buffer 
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData = lineBuffer;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData = &lineBuffer[outputSize-1];

                #ifdef USE_PALETTE
                if(IsPaletteEnabled())
                {
                    pData -= outputSize/2;
                }
				#endif

	        #endif

		    #ifdef USE_PALETTE
	          	pByteData = (BYTE*)pData;
		    #endif

			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {            
	            // take one pixel data when needed 
	            if (x%(stretch*2) == 0)
                	temp = *flashAddress++; 

	            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		                color = temp >> (((x>>(stretch-1))&0x0001)*4) & 0x000F;
                   	else 
		                color = palette[(temp >> (((x>>(stretch-1))&0x0001)*4)) & 0x000F];
	            #else
	                color = palette[(temp >> (((x>>(stretch-1))&0x0001)*4)) & 0x000F];
	            #endif	            

   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
                    #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData++ = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData++ = color;
        	          	}    
                    #else
	                    *pData++ = color;
                    #endif
	            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
                    #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData-- = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData-- = color;
        	          	}    
                    #else
	                    *pData-- = color;
                    #endif
           	    #endif			
            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++) 

			GFX_WaitForCommandQueue(3);
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
								
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
            yc++;
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)

   	    // adjust for the next line data address 
   	    if (stretch == IMAGE_X2)
        	tempFlashAddress += ((sizeX>>1)+(sizeX%2));
        else	
			tempFlashAddress = flashAddress;
					
    } // for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}
        #endif //#if (COLOR_DEPTH >= 4)

/*********************************************************************
* Function: void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs 256 color image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
        #if (COLOR_DEPTH >= 8)

void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE *image, BYTE stretch)
{
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    WORD                sizeX, sizeY;
    WORD                x, y, yc;
    BYTE                temp = 0;
    BYTE                stretchY;
    WORD                palette[256], color;
    WORD                counter;
    WORD                outputSize;

    WORD            	lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            	*pData; 

    #ifdef USE_PALETTE
    BYTE            	*pByteData; 
    #endif

    // Move pointer to size information
    flashAddress = image + 2;

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}

	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 

	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();
		 	
	GFX_SetWorkArea1((WORD)lineBuffer);
	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    // Read palette
    for(counter = 0; counter < 256; counter++)
    {
        palette[counter] = *((FLASH_WORD *)flashAddress);
        flashAddress += 2;
    }

    yc = top;

    // store current line data address 
    tempFlashAddress = flashAddress;

    for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
	{

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
	        // get flash address location of current line being processed
            flashAddress = tempFlashAddress;

			// get the location of the line buffer 
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData = lineBuffer;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData = &lineBuffer[outputSize-1];

                #ifdef USE_PALETTE
                if(IsPaletteEnabled())
                {
                    pData -= outputSize/2;
                }
                #endif
	        #endif

            #ifdef USE_PALETTE
                pByteData = (BYTE*)pData;
            #endif

			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {
	            // take one pixel data when needed
	            if (x%stretch == 0)
                	temp = *flashAddress++; 

	            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		                color = temp;
                   	else 
		                color = palette[temp];
	            #else
	                color = palette[temp];
	            #endif

   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
                    #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData++ = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData++ = color;
        	          	}    
                    #else
	                    *pData++ = color;
                    #endif
                #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
                   #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData-- = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData-- = color;
        	          	}    
                    #else
	                    *pData-- = color;
                    #endif
           	    #endif			
            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

			GFX_WaitForCommandQueue(3);
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
            
            yc++;
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)

   	    // adjust for the next line data address 
        tempFlashAddress += (sizeX);

    } // for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}

        #endif //#if (COLOR_DEPTH >= 8)

/*********************************************************************
* Function: void PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs hicolor image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
 		#if (COLOR_DEPTH == 16)
void PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE *image, BYTE stretch)
{
    register FLASH_WORD *flashAddress;
    register FLASH_WORD *tempFlashAddress;
    WORD                sizeX, sizeY;
    register WORD       x, y;
    WORD                yc, temp = 0;
    BYTE       			stretchY;
	WORD 				outputSize;

    WORD            	lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            	*pData; 

	// Move pointer to size information
    flashAddress = (FLASH_WORD *)image + 1;

    // Read image size
    sizeY = *flashAddress;
    flashAddress++;
    sizeX = *flashAddress;
    flashAddress++;

	if (stretch == IMAGE_X2) 
	    outputSize = sizeX << 1;
	else 
	    outputSize = sizeX;

	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 

	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();
			    
   	GFX_SetWorkArea1((WORD)lineBuffer);
	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    yc = top; 

    // store current line data address 
    tempFlashAddress = flashAddress;

    for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
	        // get flash address location of current line being processed
            flashAddress = tempFlashAddress;

			// get the location of the line buffer 
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData = lineBuffer;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData = &lineBuffer[outputSize-1];
	        #endif
	            
			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {
	            // take one pixel data when needed
	            if (x%stretch == 0)
                	temp = *flashAddress++;

   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	                *pData++ = temp;
	            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	                *pData-- = temp;
           	    #endif			           	    
           	    
            } // for(x = 0; x < outputSize; x++)

			GFX_WaitForCommandQueue(3);
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);

            yc++;
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)

   	    // adjust for the next line data address 
        tempFlashAddress += (sizeX);
    } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}	
	#endif // #if (COLOR_DEPTH == 16)
    #endif //USE_BITMAP_FLASH
	
    #ifdef USE_BITMAP_EXTERNAL

/*********************************************************************
* Function: void PutImage1BPPExt(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage1BPPExt(SHORT left, SHORT top, void *image, BYTE stretch)
{
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;
    BYTE            lineBuffer[((GetMaxX() + (DWORD) 1) / 8) + 1];
    WORD            palette[2], color;
    BYTE            *pData; 
    WORD 			outputSize;
    WORD			*pBufAddr, *pData2;
    WORD            lineBuffer2[(GetMaxX() + (DWORD) 1)];

    SHORT           byteWidth;
    BYTE            mask, temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    WORD            yc;
    BYTE            stretchY;

    // Get image header
    ExternalMemoryCallback(image, 0, sizeof(BITMAP_HEADER), &bmp);
    // Get palette (2 entries)
    ExternalMemoryCallback(image, sizeof(BITMAP_HEADER), 2 * sizeof(WORD), palette);
    // Set offset to the image data
	memOffset = sizeof(BITMAP_HEADER) + 2 * sizeof(WORD);
	
    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

	// Line width in bytes
    byteWidth = sizeX >> 3;
    if(sizeX & 0x0007)
        byteWidth++;	
    
	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}		
	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 		

	pBufAddr = lineBuffer2;

	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

   	GFX_SetWorkArea1((WORD)pBufAddr);
	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    yc = top;

    for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {

		// Get line
	    ExternalMemoryCallback(image, memOffset, byteWidth, lineBuffer);
    	memOffset += byteWidth;
    	
    	// render the pixels only in the viewable area (display buffer area)
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
          	pData = lineBuffer;
			// get the location of the line buffer 
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData2 = lineBuffer2;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData2 = &lineBuffer2[outputSize-1];
	        #endif
	            
			mask = 0;

			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {

				if (mask == 0) 
				{
					temp = *pData++;
					mask = 0x80;
				}	
				if (mask & temp)
				{
		            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		            	color = 1;
                   	else 
		            	color = palette[1];
		            #else	
		            	color = palette[1];
		            #endif
				} 
				else
				{
		            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		            	color = 0;
                   	else 
		            	color = palette[0];
		            #else	
		            	color = palette[0];
		            #endif
				}		

   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	                *pData2++ = color;
	            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	                *pData2-- = color;
           	    #endif			           	    
           	    
           	    // Shift to the next pixel (note that if image is stretched, shifting is delayed)
           	    if (x%(stretch==IMAGE_X2?2:1) == 0) 
                	mask >>= 1;		
           	    
            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

			GFX_WaitForCommandQueue(2);
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);

			// update the vertical position counter
            yc++;
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)
    }  // for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    
	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();
    
}


/*********************************************************************
* Function: void PutImage4BPPExt(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
        #if (COLOR_DEPTH >= 4)

void PutImage4BPPExt(SHORT left, SHORT top, void *image, BYTE stretch)
{
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;
    BYTE            lineBuffer[((GetMaxX() + (DWORD) 1) / 2) + 1];
    WORD            palette[16], color;
    BYTE            *pData; 
    WORD 			outputSize;
	WORD			*pBufAddr, *pData2;
    WORD            lineBuffer2[(GetMaxX() + (DWORD) 1)];
    SHORT           byteWidth;
    BYTE            temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    WORD            yc;
    BYTE            stretchY;

    #ifdef USE_PALETTE
    BYTE            	*pByteData; 
    #endif

    // Get image header
    ExternalMemoryCallback(image, 0, sizeof(BITMAP_HEADER), &bmp);
    // Get palette (16 entries)
    ExternalMemoryCallback(image, sizeof(BITMAP_HEADER), 16 * sizeof(WORD), palette);
    // Set offset to the image data
	memOffset = sizeof(BITMAP_HEADER) + 16 * sizeof(WORD);
	
    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

	// Line width in bytes
    byteWidth = sizeX >> 1;
    if(sizeX & 0x0001)
        byteWidth++;	
    
	yc = top;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}
	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 		

	pBufAddr = lineBuffer2;

	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

   	GFX_SetWorkArea1((WORD)pBufAddr);
	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    yc = top;

    for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {

		// Get line
	    ExternalMemoryCallback(image, memOffset, byteWidth, lineBuffer);
    	memOffset += byteWidth;
    	
    	// render the pixels only in the viewable area (display buffer area)
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
          	pData = lineBuffer;

            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData2 = lineBuffer2;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData2 = &lineBuffer2[outputSize-1];

                #ifdef USE_PALETTE
                if(IsPaletteEnabled())
                {
                    pData2 -= outputSize/2;
                }
                #endif
	        #endif

            #ifdef USE_PALETTE
                pByteData = (BYTE*)pData2;
            #endif          	

			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {

	            // take one pixel data when needed 
	            if (x%(stretch*2) == 0)
                	temp = *pData++; 

	            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		                color = temp >> (((x>>(stretch-1))&0x0001)*4) & 0x000F;
                   	else 
		                color = palette[(temp >> (((x>>(stretch-1))&0x0001)*4)) & 0x000F];
	            #else
	                color = palette[(temp >> (((x>>(stretch-1))&0x0001)*4)) & 0x000F];
	            #endif	            
	            
   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
                    #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData++ = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData2++ = color;
        	          	}    
                    #else
	                	*pData2++ = color;
                    #endif
	            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
                   #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData-- = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData2-- = color;
        	          	}    
                    #else
	                	*pData2-- = color;
                    #endif
           	    #endif			           	    
           	    		
            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);

			// update the vertical position counter
            yc++;
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)
    }  // for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}

        #endif //#if (COLOR_DEPTH >= 4)

/*********************************************************************
* Function: void PutImage8BPPExt(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
        #if (COLOR_DEPTH >= 8)

void PutImage8BPPExt(SHORT left, SHORT top, void *image, BYTE stretch)
{
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;
    BYTE            lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            palette[256], color;    
    BYTE            *pData; 
    WORD 			outputSize;
    WORD			*pBufAddr, *pData2;
    WORD            lineBuffer2[(GetMaxX() + (DWORD) 1)];

    BYTE            temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    WORD            yc;
    BYTE            stretchY;

    #ifdef USE_PALETTE
    BYTE            	*pByteData; 
    #endif

    // Get image header
    ExternalMemoryCallback(image, 0, sizeof(BITMAP_HEADER), &bmp);
    // Get palette (256 entries)
    ExternalMemoryCallback(image, sizeof(BITMAP_HEADER), 256 * sizeof(WORD), palette);
    // Set offset to the image data
	memOffset = sizeof(BITMAP_HEADER) + 256 * sizeof(WORD);
	
    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}

	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 		
		
	pBufAddr = lineBuffer2;

	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

	GFX_SetWorkArea1((WORD)pBufAddr);
   	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    yc = top;

    for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {

		// Get line
	    ExternalMemoryCallback(image, memOffset, sizeX, lineBuffer);
    	memOffset += sizeX;
    	
    	// render the pixels only in the viewable area (display buffer area)
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
          	pData = lineBuffer;
			// get the location of the line buffer 
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData2 = lineBuffer2;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData2 = &lineBuffer2[outputSize-1];
                
                #ifdef USE_PALETTE
                if(IsPaletteEnabled())
                {
                    pData2 -= outputSize/2;
                }
                #endif
	        #endif

            #ifdef USE_PALETTE
                pByteData = (BYTE*)pData2;
            #endif
	            
			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {
	            // take one pixel data when needed
	            if (x%stretch == 0)
                	temp = *pData++;

	            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		                color = temp;
                   	else 
		                color = palette[temp];
	            #else
		                color = palette[temp];
	            #endif
	            
   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
                    #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData++ = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData2++ = color;
        	          	}    
                    #else
                        *pData2++ = color;
                    #endif
	            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
                   #ifdef USE_PALETTE
        	          	if(IsPaletteEnabled())
        	          	{
        	          	    *pByteData-- = (BYTE)color;
        	          	}
        	          	else
        	          	{
                            *pData2-- = color;
        	          	}    
                    #else
	                	*pData2-- = color;
					#endif
           	    #endif			
            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)


            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 
    		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);

			// update the vertical position counter
            yc++;
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)
    }  // for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}

        #endif //#if (COLOR_DEPTH >= 8)

/*********************************************************************
* Function: void PutImage16BPPExt(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
        #if (COLOR_DEPTH == 16)

void PutImage16BPPExt(SHORT left, SHORT top, void *image, BYTE stretch)
{
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;
    WORD            lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            *pData; 
    WORD 			outputSize;
    WORD			*pBufAddr, *pData2;
    WORD            lineBuffer2[(GetMaxX() + (DWORD) 1)];
    WORD            byteWidth;

    WORD            temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    WORD			yc;
    BYTE            stretchY;

    // Get image header
    ExternalMemoryCallback(image, 0, sizeof(BITMAP_HEADER), &bmp);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER);

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

	// calculate how many bytes to fetch
    byteWidth = sizeX << 1;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}

	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > GetMaxX()+1) || (left + outputSize > GetMaxX()+1))
		outputSize = GetMaxX() + 1 - left; 
		
	pBufAddr = lineBuffer2;

	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

   	GFX_SetWorkArea1((WORD)pBufAddr);
	GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
	GFX_RCC_SetSrcOffset(0);

    yc = top; 

    for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {

		// Get line
	    ExternalMemoryCallback(image, memOffset, byteWidth, lineBuffer);
    	memOffset += byteWidth;
    	
    	// render the pixels only in the viewable area (display buffer area)
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
          	pData = lineBuffer;
			// get the location of the line buffer 
            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	          	pData2 = lineBuffer2;
            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	           	pData2 = &lineBuffer2[outputSize-1];
	        #endif

			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {
	            // take one pixel data when needed
	            if (x%stretch == 0)
                	temp = *pData++;

   	            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270))
	                *pData2++ = temp;
	            #elif ((DISP_ORIENTATION == 180) || (DISP_ORIENTATION == 90))   
	                *pData2-- = temp;
           	    #endif			
            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)


            #if ((DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180))
				GFX_RCC_SetSize(x, 1);
			#else	
				GFX_RCC_SetSize(1, x);
			#endif
            #if (DISP_ORIENTATION == 0) 
    			GFX_RCC_SetDestOffset((yc * (DWORD)DISP_HOR_RESOLUTION) + left);
            #elif (DISP_ORIENTATION == 90)
    			GFX_RCC_SetDestOffset(((GetMaxX() + 1 - (left + outputSize)) * (DWORD)DISP_HOR_RESOLUTION) + yc);
            #elif (DISP_ORIENTATION == 180)
    			GFX_RCC_SetDestOffset(((GetMaxY() + 1 - yc) * (DWORD)DISP_HOR_RESOLUTION) + GetMaxX() + 1 - (left+outputSize));
            #elif (DISP_ORIENTATION == 270)
    			GFX_RCC_SetDestOffset((left * (DWORD)DISP_HOR_RESOLUTION) + (GetMaxY() + 1 - yc));
			#endif	 

			GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
			// update the vertical position counter
            yc++;
            
        }  // for(stretchY = 0; stretchY < stretch; stretchY++)
    }  // for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}
        #endif //#if (COLOR_DEPTH == 16)
    #endif //#ifdef USE_BITMAP_EXTERNAL

/*********************************************************************
* Function: void PutImage1BPPEDS(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: Image must be located in EDS memory that is accessible by the GPUs.
*
********************************************************************/
void PutImage1BPPEDS(SHORT left, SHORT top, void *image, BYTE stretch)
{
    BYTE			TempGPUBpp;
    BYTE            lineBuffer[((GetMaxX() + (DWORD) 1) / 8) + 1];
    WORD            lineBuffer2[(GetMaxX() + (DWORD) 1)];
    WORD            palette[2], color;
    BYTE            *pData; 
    WORD			*pData2;
    WORD 			outputSize, offsetCount;

    BYTE            mask, temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    WORD            yc, xp, yp;
    BYTE            stretchY;

    // First get palette (256 entries)

	// make sure there are no pending RCC GPU commands
    GFX_WaitForCommandQueue(16);
    GFX_WaitForGpu();

	GFX_SetWorkArea1(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress);
	GFX_SetWorkArea2((WORD)palette);
	GFX_RCC_SetSrcOffset((WORD)(sizeof(BITMAP_HEADER)) >> 1);
	GFX_RCC_SetDestOffset(0);
	GFX_RCC_SetSize(2, 1);
	GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);
	
    // Get the image size
    sizeY = DrvGetImageHeight(image);
    sizeX = DrvGetImageWidth(image);

    yc = top;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}

	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > (GetMaxX()+1)) || (left + outputSize > (GetMaxX()+1)))
		outputSize = GetMaxX() + 1 - left; 		


	// 32 is used here since the palette size is 2 words or 32 bits
	offsetCount = 32 + (sizeof(BITMAP_HEADER)*8);

    for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {
	    	    
		// copy one line from the source
		// make sure there are no pending RCC GPU commands & 
		// make sure the GPUs are not operating since changing the base addresses
		// will break the currently executing GPU command.
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	    
	    // the image data is 1 nibble per pixel and normally
	    // if the data is outside the PIC (in extended EDS) we cannot do
	    // GPU operation with less than 8bpp color depth.
	    // But since this will be read and the destination is in internal RAM we can use
	    // 4bpp on the GPU operation.
	    TempGPUBpp = _PUBPP;
	    _PUBPP = GFX_1_BPP;

		// make sure there are no pending RCC GPU commands
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	
		GFX_SetWorkArea1(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress);
		GFX_SetWorkArea2((DWORD)(WORD)lineBuffer);
				
		GFX_RCC_SetSrcOffset(offsetCount);
		GFX_RCC_SetDestOffset(0);
		// check if there are padded bits
		if (sizeX%8)
		{
			GFX_RCC_SetSize(outputSize+(8-(outputSize%8)), 1);
		}
		else
		{
			GFX_RCC_SetSize(outputSize, 1);
		}
		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);

		// wait until copying is done
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	    
	    // reset the _PUBPP to what it was
	    _PUBPP = TempGPUBpp;

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
           	pData = lineBuffer;

			#if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270) 
            	pData2 = lineBuffer2;
		    #elif (DISP_ORIENTATION == 180) 
            	pData2 = &lineBuffer2[outputSize-1];
		    #elif (DISP_ORIENTATION == 90) 
            	pData2 = &lineBuffer2[outputSize-1];
            #endif	           	
	
            xp = 0;
            mask = 0;
            
            // process the pixels on the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {
				if (mask == 0) 
				{
					temp = *pData++;
					mask = 0x80;
				}	

				if (mask & temp)
				{
		            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		            	color = 1;
                   	else 
		            	color = palette[1];
		            #else	
		            	color = palette[1];
		            #endif
				} 
				else
				{
		            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		            	color = 0;
                   	else 
		            	color = palette[0];
		            #else	
		            	color = palette[0];
		            #endif
				}
	            
				#if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270) 
	                *pData2++ = color;
                #elif (DISP_ORIENTATION == 180) 
   	                *pData2-- = color;
                #elif (DISP_ORIENTATION == 90)
   	                *pData2-- = color;
           	    #endif

           	    // Shift to the next pixel (note that if image is stretched, shifting is delayed)
           	    if (x%(stretch==IMAGE_X2?2:1) == 0) 
                	mask >>= 1;		

            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

			// adjust the x and y coordinate positions based on the screen rotation
			#if (DISP_ORIENTATION == 0)
		    {
		        xp = left;
        		yp = yc;
			} 
		    #elif (DISP_ORIENTATION == 90)
		    {
	        	xp = yc;
	        	yp = GetMaxX() + 1 - (left + outputSize);
		    }
		    #elif (DISP_ORIENTATION == 180)
		    {
		        xp = GetMaxX() + 1 - (left + outputSize);
		        yp = GetMaxY() + 1 - yc;
		    }
		    #elif (DISP_ORIENTATION == 270)
		    {
	        	xp = GetMaxY() + 1 - yc;
		        yp = left;
		    }
			#endif				

			// make sure there are no pending RCC GPU commands
		    GFX_WaitForCommandQueue(16);
		    GFX_WaitForGpu();
			GFX_SetWorkArea1((DWORD)(WORD)lineBuffer2);
			GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
			GFX_RCC_SetSrcOffset(0);
			GFX_RCC_SetDestOffset((yp * (DWORD)DISP_HOR_RESOLUTION) + xp);
		
    		#if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)
				GFX_RCC_SetSize(1, outputSize);
			#else
				GFX_RCC_SetSize(outputSize, 1);
			#endif	
		
			GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
            yc++;
        } // for(stretchY = 0; stretchY < stretch; stretchY++)
                    
        // adjust the offset counter (adjust the offset if there are padded bits)
        if (sizeX%8)
			offsetCount += (sizeX + (8-(sizeX%8)));
		else
			offsetCount += (sizeX);

    } //for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}

/*********************************************************************
* Function: void PutImage4BPPEDS(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: Image must be located in EDS memory that is accessible by the GPUs.
*
********************************************************************/
        #if (COLOR_DEPTH >= 4)

void PutImage4BPPEDS(SHORT left, SHORT top, void *image, BYTE stretch)
{
    BYTE			TempGPUBpp;
    BYTE            lineBuffer[((GetMaxX() + (DWORD) 1) / 2) + 1];
    WORD            lineBuffer2[(GetMaxX() + (DWORD) 1)];
    WORD            palette[16], color;
    BYTE            *pData; 
    WORD			*pData2;
    WORD 			outputSize, offsetCount;

    BYTE            temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    WORD            yc, xp, yp;
    BYTE            stretchY;

    // First get palette (256 entries)

	// make sure there are no pending RCC GPU commands
    GFX_WaitForCommandQueue(16);
    GFX_WaitForGpu();

	GFX_SetWorkArea1(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress);
	GFX_SetWorkArea2((WORD)palette);
	GFX_RCC_SetSrcOffset((WORD)(sizeof(BITMAP_HEADER)) >> 1);
	GFX_RCC_SetDestOffset(0);
	GFX_RCC_SetSize(16, 1);
	GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);
	
    // Get the image size
    sizeY = DrvGetImageHeight(image);
    sizeX = DrvGetImageWidth(image);

    yc = top;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}
		
	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > (GetMaxX()+1)) || (left + outputSize > (GetMaxX()+1)))
		outputSize = GetMaxX() + 1 - left; 		
		

	// 64 is used here since the palette size is 16 words or 64 nibbles
	offsetCount = 64 + (sizeof(BITMAP_HEADER)<<1);

	for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {
		// copy one line from the source
		// make sure there are no pending RCC GPU commands & 
		// make sure the GPUs are not operating since changing the base addresses
		// will break the currently executing GPU command.
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	    
	    // the image data is 1 nibble per pixel and normally
	    // if the data is outside the PIC (in extended EDS) we cannot do
	    // GPU operation with less than 8bpp color depth.
	    // But since this will be read and the destination is in internal RAM we can use
	    // 4bpp on the GPU operation.
	    TempGPUBpp = _PUBPP;
	    _PUBPP = GFX_4_BPP;

		// make sure there are no pending RCC GPU commands
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	
		GFX_SetWorkArea1(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress);
		GFX_SetWorkArea2((DWORD)(WORD)lineBuffer);
		GFX_RCC_SetSrcOffset(offsetCount);
		GFX_RCC_SetDestOffset(0);
		GFX_RCC_SetSize(sizeX, 1);
		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);

		// wait until copying is done
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	    
	    // reset the _PUBPP to what it was
	    _PUBPP = TempGPUBpp;

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
           	pData = lineBuffer;

			#if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270) 
            	pData2 = lineBuffer2;
		    #elif (DISP_ORIENTATION == 180) 
            	pData2 = &lineBuffer2[outputSize-1];
		    #elif (DISP_ORIENTATION == 90) 
            	pData2 = &lineBuffer2[outputSize-1];
            #endif	           	
	
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {

	            // take one pixel data when needed 
	            if (x%(stretch*2) == 0)
                	temp = *pData++; 

	            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		                color = temp >> (((x>>(stretch-1))&0x0001)*4) & 0x000F;
                   	else 
		                color = palette[(temp >> (((x>>(stretch-1))&0x0001)*4)) & 0x000F];
	            #else
	                color = palette[(temp >> (((x>>(stretch-1))&0x0001)*4)) & 0x000F];
	            #endif	            
	            
				#if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270) 
					// set the color to use based on the pixel value when using PutPixel()
					SetColor(color);
				#endif	

                // Write pixel to buffer
				#if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270) 
	                *pData2++ = color;
   	            #elif (DISP_ORIENTATION == 180) 
	                *pData2-- = color;
   	            #elif (DISP_ORIENTATION == 90)
	                *pData2-- = color;
           	    #endif

            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

			// adjust the x and y coordinate positions based on the screen rotation
			// adjust the x and y coordinate positions based on the screen rotation
		    #if (DISP_ORIENTATION == 0)
		    {
		        xp = left;
        		yp = yc;
			} 
		    #elif (DISP_ORIENTATION == 90)
		    {
	        	xp = yc;
	        	yp = GetMaxX() + 1 - (left + outputSize);
		    }
		    #elif (DISP_ORIENTATION == 180)
		    {
		        xp = GetMaxX() + 1 - (left + outputSize);
		        yp = GetMaxY() + 1 - yc;
		    }
		    #elif (DISP_ORIENTATION == 270)
		    {
	        	xp = GetMaxY() + 1 - yc;
		        yp = left;
		    }
			#endif			

			// make sure there are no pending RCC GPU commands
		    GFX_WaitForCommandQueue(16);
		    GFX_WaitForGpu();
			GFX_SetWorkArea1((DWORD)(WORD)lineBuffer2);
			GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
			GFX_RCC_SetSrcOffset(0);
			GFX_RCC_SetDestOffset((yp * (DWORD)DISP_HOR_RESOLUTION) + xp);
		
    		#if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)
				GFX_RCC_SetSize(1, outputSize);
			#else
				GFX_RCC_SetSize(outputSize, 1);
			#endif	
		
			GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
            yc++;
        } // for(stretchY = 0; stretchY < stretch; stretchY++)
        
        // adjust the offset counter (adjust the offset if there are padded bits)
        if (sizeX%4)
			offsetCount += (sizeX + 1);
		else
			offsetCount += (sizeX);

    } //for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}

        #endif //#if (COLOR_DEPTH >= 4)
    
/*********************************************************************
* Function: void PutImage8BPPEDS(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: Image must be located in EDS memory that is accessible by the GPUs.
*
********************************************************************/
        #if (COLOR_DEPTH >= 8)

void PutImage8BPPEDS(SHORT left, SHORT top, void *image, BYTE stretch)
{
    BYTE			TempGPUBpp;
    BYTE            lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            lineBuffer2[(GetMaxX() + (DWORD) 1)];
    
    WORD            palette[256], color;
    BYTE            *pData; 
    WORD			*pData2;
    WORD 			outputSize;

    BYTE            temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    WORD            yc, xp, yp;
    BYTE            stretchY;

    // First get palette (256 entries)

	// make sure there are no pending RCC GPU commands
    GFX_WaitForCommandQueue(16);
    GFX_WaitForGpu();

	GFX_SetWorkArea1(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress);
	GFX_SetWorkArea2((WORD)palette);
	GFX_RCC_SetSrcOffset((WORD)(sizeof(BITMAP_HEADER)) >> 1);
	GFX_RCC_SetDestOffset(0);
	GFX_RCC_SetSize(256, 1);
	GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);
	
    // Get the image size
    sizeY = DrvGetImageHeight(image);
    sizeX = DrvGetImageWidth(image);

    yc = top;

	if (stretch == IMAGE_X2) {
		outputSize = sizeX << 1;
	}   	
	else //if (stretch == IMAGE_NORMAL)
	{
		outputSize = sizeX;
	}
	
	// area of the buffer to use is only the area that
	// spans the viewable area
	if ((outputSize > (GetMaxX()+1)) || (left + outputSize > (GetMaxX()+1)))
		outputSize = GetMaxX() + 1 - left; 		
		
	
	for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)
    {
		// copy one line from the source
		// make sure there are no pending RCC GPU commands & 
		// make sure the GPUs are not operating since changing the base addresses
		// will break the currently executing GPU command.
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	    
	    // since the image data is 1 byte per pixel, set the GPU bpp to 8 bpp.
	    TempGPUBpp = _PUBPP;
	    _PUBPP = GFX_8_BPP;

		// make sure there are no pending RCC GPU commands
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	
		GFX_SetWorkArea1(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress);
		GFX_SetWorkArea2((DWORD)(WORD)lineBuffer);
		// 512 is used here since the palette size is 256 words or 512 bytes
		GFX_RCC_SetSrcOffset(512 + sizeof(BITMAP_HEADER) + (sizeX*(DWORD)y));
		GFX_RCC_SetDestOffset(0);
		GFX_RCC_SetSize(sizeX, 1);
		GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);

		// wait until copying is done
	    GFX_WaitForCommandQueue(16);
	    GFX_WaitForGpu();
	    
	    // reset the _PUBPP to what it was
	    _PUBPP = TempGPUBpp;

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
           	pData = lineBuffer;

			#if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270) 
            	pData2 = lineBuffer2;
		    #elif (DISP_ORIENTATION == 180) 
            	pData2 = &lineBuffer2[outputSize-1];
		    #elif (DISP_ORIENTATION == 90) 
            	pData2 = &lineBuffer2[outputSize-1];
            #endif	           	
	
			// process the pixels of the current line
            for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)
            {
	            // take one pixel data when needed
	            if (x%stretch == 0)
                	temp = *pData++;
                
	            #ifdef USE_PALETTE
                   	if(IsPaletteEnabled())
		                color = temp;
                   	else 
		                color = palette[temp];
	            #else
		                color = palette[temp];
	            #endif

                // Write pixel to buffer
				#if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 270) 
	                *pData2++ = color;
   	            #elif (DISP_ORIENTATION == 180) 
   	                *pData2-- = color;
                #elif (DISP_ORIENTATION == 90)
   	                *pData2-- = color;
           	    #endif
            } // for(x = 0; (x < outputSize) && (x + left < GetMaxX()); x++)

			// adjust the x and y coordinate positions based on the screen rotation
		    #if (DISP_ORIENTATION == 0)
		    {
		        xp = left;
        		yp = yc;
			} 
		    #elif (DISP_ORIENTATION == 90)
		    {
	        	xp = yc;
	        	yp = GetMaxX() + 1 - (left + outputSize);
		    }
		    #elif (DISP_ORIENTATION == 180)
		    {
		        xp = GetMaxX() + 1 - (left + outputSize);
		        yp = GetMaxY() + 1 - yc;
		    }
		    #elif (DISP_ORIENTATION == 270)
		    {
	        	xp = GetMaxY() + 1 - yc;
		        yp = left;
		    }
			#endif

			// make sure there are no pending RCC GPU commands
		    GFX_WaitForCommandQueue(16);
		    GFX_WaitForGpu();
			GFX_SetWorkArea1((DWORD)(WORD)lineBuffer2);
			GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
			GFX_RCC_SetSrcOffset(0);
			GFX_RCC_SetDestOffset((yp * (DWORD)DISP_HOR_RESOLUTION) + xp);
		
    		#if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)
				GFX_RCC_SetSize(1, outputSize);
			#else
				GFX_RCC_SetSize(outputSize, 1);
			#endif	
		
			GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
            yc++;
        } // for(stretchY = 0; stretchY < stretch; stretchY++)
    } //for(y = 0; (y < sizeY) && (yc < GetMaxY()); y++)

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();

}

        #endif //#if (COLOR_DEPTH >= 8)
       
/*********************************************************************
* Function: void PutImage16BPPEDS(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, image - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: Image must be located in EDS memory that is accessible by the GPUs.
*
********************************************************************/
        #if (COLOR_DEPTH == 16)
void PutImage16BPPEDS(SHORT left, SHORT top, void *image, BYTE stretch)
{
    register WORD       x, y; 
    register BYTE       stretchY;

    WORD            	lineBuffer[(GetMaxX() + (DWORD) 1)];
    WORD            	lineBuffer2[(GetMaxX() + (DWORD) 1)];
	WORD 				outputSize, sizeX, sizeY, actualY;
#if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 180)
    WORD                temp;
#endif
    WORD				*pSrc;
    SHORT               xc, yc;

    // Read image size
    sizeY = DrvGetImageHeight(image);
    sizeX = DrvGetImageWidth(image);
    
	// this initialization of outputSize is not performed only 
	// when DISP_ORIENTATION == 0 and image is not stretched
#if (DISP_ORIENTATION == 0) 
	if (stretch == IMAGE_X2) 
#endif   		
	{
		if (stretch == IMAGE_X2) {
			outputSize = sizeX << 1;
		}   	
		else //if (stretch == IMAGE_NORMAL)
		{
			outputSize = sizeX;
		}
			
		// area of the buffer to use is only the area that
		// spans the viewable area
		if ((outputSize > (GetMaxX()+1)) || (left + outputSize > (GetMaxX()+1)))
			outputSize = GetMaxX() + 1 - left; 		
	}	

	// When the DISP_ORIENTATION = 0, the image can be transferred to the display buffer
	// using one transfer. If stretched or using other orientation, the transfers
	// are performed line by line.
#if (DISP_ORIENTATION == 0) 
	if (stretch != IMAGE_X2) 
	{
    	// Since the data in memory is oriented in the same way as the display we c
    	// an move the whole block. The source is continuous and destination is  
    	// discontinupus since it is the display buffer. The magic number 3 is the 
    	// offset of the actual image data in the image structure.
		MoveWindow(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress, GFX_DISPLAY_BUFFER_START_ADDRESS, 
				  3, (top * (DWORD)DISP_HOR_RESOLUTION) + left, 
				  RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS, sizeX, sizeY);
	}
	else
#endif   
	{
		// copy the data line by line and block copy it into the destination
		actualY = top;

	    for(y = 0; (y < sizeY) && (actualY < GetMaxY()); y++)
		{
			for(stretchY = 0; stretchY < stretch; stretchY++)
        	{
				// make sure there are no pending RCC GPU commands & 
				// make sure the GPUs are not operating since changing the base addresses
				// will break the currently executing GPU command.
	            GFX_WaitForCommandQueue(16);
		        GFX_WaitForGpu();
		        
				// copy one line from the source
				GFX_SetWorkArea1(((GFX_IMAGE_HEADER*)image)->LOCATION.extAddress); // image location is the source
				GFX_SetWorkArea2((WORD)lineBuffer);					  // buffer is the destination
				GFX_RCC_SetSize(outputSize, 1);						  // copy one line of the image
					
				GFX_RCC_SetSrcOffset(3 + (sizeX*(DWORD)y));			  // set the offset of the source
				GFX_RCC_SetDestOffset(0);							  // no offset for the destination
	
				// initiate the copy, the source is continuous and the 
				// destination is also continuous
				GFX_RCC_StartCopy(	RCC_COPY, RCC_ROP_C, 			\
								RCC_SRC_ADDR_CONTINUOUS, 			\
								RCC_DEST_ADDR_CONTINUOUS);		
	
				// make sure the copy was executed fully
	            GFX_WaitForCommandQueue(16);
		        GFX_WaitForGpu();
				
				if (stretch == IMAGE_X2)
		       	{
			       	// stretch the data first
					for (x = 0; x < (outputSize>>1); x += 1)
					{
						lineBuffer2[x*2] = lineBuffer[x];
						lineBuffer2[x*2+1] = lineBuffer[x];
					}	
					pSrc = lineBuffer2;		
			    }   
			    else
			    {
			    	pSrc = lineBuffer;
			    }	
					// rearrange the copied data if necessary
			    #if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 180)
					// swap the data 
					for (x = 0; x < (outputSize>>1); x++)
					{
						temp = *(pSrc + outputSize-x-1);
						*(pSrc + outputSize-x-1) = *(pSrc+x);
						*(pSrc+x) = temp;
					}			
			    #endif
								
				// adjust the x and y coordinate positions based on the screen rotation
				#if (DISP_ORIENTATION == 0)
			    {
			        xc = left;
	        		yc = actualY;
				} 
			    #elif (DISP_ORIENTATION == 90)
			    {
		        	xc = actualY;
		        	yc = GetMaxX() + 1 - (left + outputSize);		        	
			    }
			    #elif (DISP_ORIENTATION == 180)
			    {
			        xc = GetMaxX() + 1 - (left + outputSize);
			        yc = GetMaxY() + 1 - actualY;
			    }
			    #elif (DISP_ORIENTATION == 270)
			    {
		        	xc = GetMaxY() + 1 - actualY;
			        yc = left;
			    }
				#endif		
			
				GFX_SetWorkArea1((WORD)pSrc);
				GFX_SetWorkArea2(GFX_DISPLAY_BUFFER_START_ADDRESS);
				
				GFX_RCC_SetSrcOffset(0);
				GFX_RCC_SetDestOffset((yc*(DWORD)DISP_HOR_RESOLUTION)+xc);
			
	    		#if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)
					GFX_RCC_SetSize(1, outputSize);
				#else
					GFX_RCC_SetSize(outputSize, 1);
				#endif	
			
				GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
								
				actualY++;		
			} //for(stretchY = 0; stretchY < stretch; stretchY++)
		} // for(y = 0; (y < sizeY) && (y < GetMaxY()); y++)
	}	

	// make sure the GPUs are done before exiting
	GFX_WaitForCommandQueue(16);
	GFX_WaitForGpu();
	
}
        #endif //#if (COLOR_DEPTH == 16)
    
/*********************************************************************
* Function: WORD PutImage(SHORT left, SHORT top, void* image, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner,
*        image - image pointer,
*        stretch - image stretch factor
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the image is not yet completely drawn.
*         - Returns 1 when the image is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: outputs image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
WORD PutImage(SHORT left, SHORT top, void *image, BYTE stretch)
{
#if defined (USE_BITMAP_FLASH)
    FLASH_BYTE  *flashAddress;
#endif    
    BYTE        colorDepth;
    WORD        colorTemp;
#if defined(USE_COMP_IPU)
    SHORT       resType;
#endif

    if(GFX_GetFreeCommandSpace() < 4)
    {
        #ifndef USE_NONBLOCKING_CONFIG
            GFX_WaitForCommandQueue(4);
        #else
            return (0);
        #endif
    }

    // Save current color
    colorTemp = GetColor();

#ifdef USE_COMP_IPU

    resType = *((SHORT *)image);

    if((resType & GFX_COMP_MASK) == COMP_IPU)
    {
        GFX_IMAGE_HEADER *piHdr;
        GFX_IMAGE_HEADER NewiHdr;

        piHdr = (GFX_IMAGE_HEADER*)image;
        memcpy(&NewiHdr, image, sizeof(GFX_IMAGE_HEADER));
        NewiHdr.type = (EDS_EPMP | IMAGE_MBITMAP | COMP_IPU);
        NewiHdr.ID = 1;
        NewiHdr.LOCATION.extAddress = GFX_DECOMPRESSED_DATA_RAM_ADDRESS;

        if(((resType & GFX_MEM_MASK) == EDS_EPMP) || ((resType & GFX_MEM_MASK) == RAM))
        {
            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();
            
            GFX_SetWorkArea1(piHdr->LOCATION.progByteAddress);
            GFX_SetWorkArea2(GFX_DECOMPRESSED_DATA_RAM_ADDRESS);
            GFX_IPU_SetSrcOffset(0);
            GFX_IPU_SetDestOffset(0);
            GFX_IPU_Inflate(piHdr->param2);

            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();

            if(GFX_IPU_GetDecompressionError())
            {
                return -1;
            }

            image = &NewiHdr;
        }    
            #ifdef USE_BITMAP_FLASH
        else if((resType & GFX_MEM_MASK) == FLASH)
        {
            BYTE aTemp[GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE];
            WORD  quotient, remainder, i;
            DWORD srcaddress, offset;
            FLASH_BYTE *pFlashData = piHdr->LOCATION.progByteAddress;
            BYTE  savedpubpp = _PUBPP;
            
            quotient = piHdr->param1 / GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE;
            remainder = piHdr->param1 - (quotient * GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE);
            
            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();

            _PUBPP = GFX_8_BPP;

            srcaddress = (DWORD)(WORD)aTemp;
            offset = 0;
            
            while(quotient)
            {
                for(i=0; i < GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE; i++)
                {
                    aTemp[i] = pFlashData[i + offset];                
                }
                
                GFX_WaitForCommandQueue(4);
                
                MoveWindow(srcaddress, GFX_COMPRESSED_DATA_RAM_ADDRESS, 0, offset, 			\
                			RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, 				\
                			GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE, 1);						
                offset += GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE;
                
                quotient--;
            }

            for(i=0; i < remainder; i++)
            {
                aTemp[i] = pFlashData[i + offset];                
            }
            
            GFX_WaitForCommandQueue(4);
            MoveWindow(srcaddress, GFX_COMPRESSED_DATA_RAM_ADDRESS, 0, offset, 				\
            			RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, remainder, 1);

            _PUBPP = savedpubpp;

            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();

            GFX_SetWorkArea1(GFX_COMPRESSED_DATA_RAM_ADDRESS);
            GFX_SetWorkArea2(GFX_DECOMPRESSED_DATA_RAM_ADDRESS);
            GFX_IPU_SetSrcOffset(0);
            GFX_IPU_SetDestOffset(0);
            GFX_IPU_Inflate(piHdr->param2);

            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();

            if(GFX_IPU_GetDecompressionError())
            {
                return -1;
            }
            
            image = &NewiHdr;
        }
            #endif // #ifdef USE_BITMAP_FLASH
            #ifdef USE_BITMAP_EXTERNAL 
        else if((resType & GFX_MEM_MASK) == EXTERNAL)
        {
            BYTE aTemp[GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE];
            WORD  quotient, remainder;
            DWORD srcaddress, offset;
            BYTE  savedpubpp = _PUBPP;
            
            quotient = piHdr->param1 / GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE;
            remainder = piHdr->param1 - (quotient * GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE);
            
            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();

            _PUBPP = GFX_8_BPP;

            srcaddress = (DWORD)(WORD)aTemp;
            offset = 0;
            
            while(quotient)
            {
                ExternalMemoryCallback(image, offset, GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE, (void*)aTemp);        
                
                GFX_WaitForCommandQueue(4);
                MoveWindow(srcaddress, GFX_COMPRESSED_DATA_RAM_ADDRESS, 0, offset, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE, 1);

                offset += GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE;
                quotient--;
            }

            ExternalMemoryCallback(image, offset, remainder, (void*)aTemp);        
            
            GFX_WaitForCommandQueue(4);
            MoveWindow(srcaddress, GFX_COMPRESSED_DATA_RAM_ADDRESS, 0, offset, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS, remainder, 1);

            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();

            _PUBPP = savedpubpp;

            GFX_SetWorkArea1(GFX_COMPRESSED_DATA_RAM_ADDRESS);
            GFX_SetWorkArea2(GFX_DECOMPRESSED_DATA_RAM_ADDRESS);
            GFX_IPU_SetSrcOffset(0);
            GFX_IPU_SetDestOffset(0);
            GFX_IPU_Inflate(piHdr->param2);

            GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
            GFX_WaitForGpu();

            if(GFX_IPU_GetDecompressionError())
            {
                return -1;
            }

            image = &NewiHdr;
        }
            #endif // #ifdef USE_BITMAP_EXTERNAL
        else
        {
            return -1;
        }
    }


#endif // #ifdef USE_COMP_IPU

    switch(*((SHORT *)image) & GFX_MEM_MASK)
    {
                #ifdef USE_BITMAP_FLASH

        case FLASH:

            // Image address
            flashAddress = ((IMAGE_FLASH *)image)->address;

            // Read color depth
            colorDepth = *(flashAddress + 1);

            // Draw picture
            switch(colorDepth)
            {
                case 1:     PutImage1BPP(left, top, flashAddress, stretch); break;
                    
                    #if (COLOR_DEPTH >= 4)
                case 4:     PutImage4BPP(left, top, flashAddress, stretch); break;
                    #endif
                    
                    #if (COLOR_DEPTH >= 8)
                case 8:     PutImage8BPP(left, top, flashAddress, stretch); break;
                    #endif
                    
                    #if (COLOR_DEPTH == 16)
                case 16:    PutImage16BPP(left, top, flashAddress, stretch); break;
                    #endif
                    
                default:    break;
            }

            break;
                #endif //USE_BITMAP_FLASH

		case EDS_EPMP:

            // Read color depth
            colorDepth = DrvGetImageColorDepth(image);
            
            // Draw picture
            switch(colorDepth)
            {
                case 1:     PutImage1BPPEDS(left, top, image, stretch); break; 
                
                    #if (COLOR_DEPTH >= 4)
                case 4:     PutImage4BPPEDS(left, top, image, stretch); break; 
                    #endif
                    
                    #if (COLOR_DEPTH >= 8)
                case 8:     PutImage8BPPEDS(left, top, image, stretch); break;
                    #endif
                    
                    #if (COLOR_DEPTH == 16)
                case 16:    PutImage16BPPEDS(left, top, image, stretch); break;
                    #endif
                    
                default:    break;
            }

            break;


                #ifdef USE_BITMAP_EXTERNAL

        case EXTERNAL:

            // Get color depth
            ExternalMemoryCallback(image, 1, 1, &colorDepth);

            // Draw picture
            switch(colorDepth)
            {
                case 1:     PutImage1BPPExt(left, top, image, stretch); break;
                
                    #if (COLOR_DEPTH >= 4)
                case 4:     PutImage4BPPExt(left, top, image, stretch); break;
                    #endif
                
                    #if (COLOR_DEPTH >= 8)
                case 8:     PutImage8BPPExt(left, top, image, stretch); break;
                    #endif
                    
                    #if (COLOR_DEPTH == 16)
                case 16:    PutImage16BPPExt(left, top, image, stretch); break;
                    #endif
                    
                default:    break;
            }

            break;

                #endif //USE_BITMAP_EXTERNAL

        default:
            break;
    }

    // Restore current color
    SetColor(colorTemp);
   
    return (1);
}

/*********************************************************************
* Function:  SHORT DrvGetImageHeight(void* pImage)
*
* Overview: This function returns the image height for images in EDS.
*
* PreCondition: A valid image must exist in the location specified 
*               by the pointer pImage.
*
* Input: pImage - Pointer to the image.
*
* Output: Returns the image height in pixels.
*
* Side Effects: none
*
********************************************************************/
SHORT   DrvGetImageHeight(void *pImage)
{
    DWORD_VAL tempWorkArea1, tempWorkArea2;
    SHORT height;

#ifdef USE_COMP_IPU
    SHORT       resType;

    resType = *((SHORT *)pImage);
    
    if((resType & GFX_COMP_MASK) == COMP_IPU)
    {
        GFX_IMAGE_HEADER *iHeader = (GFX_IMAGE_HEADER*)pImage;
        return iHeader->height;
    }    
#endif

	// grab the image height using the RCCGPU

	// make sure there are no pending RCC GPU commands
	GFX_WaitForCommandQueue(16);

	// make sure the GPUs are not operating since changing the base addresses
	// will break the currently executing GPU command.
    GFX_WaitForGpu();

	// store the work areas temporarily
	tempWorkArea1.w[0] = G1W1ADRL; 
	tempWorkArea1.w[1] = G1W1ADRH; 
	tempWorkArea2.w[0] = G1W2ADRL; 
	tempWorkArea2.w[1] = G1W2ADRH; 

	// now change the work areass
	GFX_SetWorkArea1(((IMAGE_EXTERNAL*)pImage)->address);
    GFX_SetWorkArea2((WORD)&height);

    //GFX_RCC_SetColor(_color.Val);
    GFX_RCC_SetSrcOffset(1);
    GFX_RCC_SetDestOffset(0);
    GFX_RCC_SetSize(16/COLOR_DEPTH, 1);
    GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);

	// make sure there are no pending RCC GPU commands
	GFX_WaitForCommandQueue(16);

	// make sure the GPUs are not operating since changing the base addresses
	// will break the currently executing GPU command.
    GFX_WaitForGpu();
    
	GFX_SetWorkArea1(tempWorkArea1.Val);
	GFX_SetWorkArea2(tempWorkArea2.Val);
    
    /* Note: No need to wait for  complete execution of the command even for Blocking Mode. The next commands will be in the queue & hence will execute only after the completion of this command. */
    return (height);
}	

/*********************************************************************
* Function:  SHORT DrvGetImageWidth(void* pImage)
*
* Overview: This function returns the image width for images in EDS.
*
* PreCondition: A valid image must exist in the location specified 
*               by the pointer pImage.
*
* Input: pImage - Pointer to the image.
*
* Output: Returns the image height in pixels.
*
* Side Effects: none
*
********************************************************************/
SHORT   DrvGetImageWidth(void *pImage)
{
    DWORD_VAL tempWorkArea1, tempWorkArea2;
    SHORT width;

#ifdef USE_COMP_IPU
    SHORT       resType;

    resType = *((SHORT *)pImage);
    
    if((resType & GFX_COMP_MASK) == COMP_IPU)
    {
        GFX_IMAGE_HEADER *iHeader = (GFX_IMAGE_HEADER*)pImage;
        return iHeader->width;
    }    
#endif
	// grab the image width using the RCCGPU
    
	// make sure there are no pending RCC GPU commands
	GFX_WaitForCommandQueue(16);

	// make sure the GPUs are not operating since changing the base addresses
	// will break the currently executing GPU command.
    GFX_WaitForGpu();

	// store the work areas temporarily
	tempWorkArea1.w[0] = G1W1ADRL; 
	tempWorkArea1.w[1] = G1W1ADRH; 
	tempWorkArea2.w[0] = G1W2ADRL; 
	tempWorkArea2.w[1] = G1W2ADRH; 

	// now change the work areass
	GFX_SetWorkArea1(((IMAGE_EXTERNAL*)pImage)->address);
    GFX_SetWorkArea2((WORD)&width);
    

//    GFX_RCC_SetColor(_color.Val);
    GFX_RCC_SetSrcOffset(2);
    GFX_RCC_SetDestOffset(0);
    GFX_RCC_SetSize(16/COLOR_DEPTH, 1);
    GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);

	// make sure there are no pending RCC GPU commands
	GFX_WaitForCommandQueue(16);

	// make sure the GPUs are not operating since changing the base addresses
	// will break the currently executing GPU command.
    GFX_WaitForGpu();
    
	GFX_SetWorkArea1(tempWorkArea1.Val);
	GFX_SetWorkArea2(tempWorkArea2.Val);
    
    /* Note: No need to wait for  complete execution of the command even for Blocking Mode. The next commands will be in the queue & hence will execute only after the completion of this command. */
    return (width);

}

/*********************************************************************
* Function:  SHORT DrvGetImageColorDepth(void *pImage)
*
* Overview: This function returns the color depth for images in EDS.
*
* PreCondition: A valid image must exist in the location specified 
*               by the pointer pImage.
*
* Input: pImage - Pointer to the image.
*
* Output: Returns the color depth (bits per pixel) of the image.
*
* Side Effects: none
*
********************************************************************/
SHORT DrvGetImageColorDepth(void *pImage)
{
	// grab the image color depth using the RCCGPU
    DWORD_VAL tempWorkArea1, tempWorkArea2;
    SHORT bpp;
    
#ifdef USE_COMP_IPU
    GFX_IMAGE_HEADER *iHeader = (GFX_IMAGE_HEADER*)pImage;
    
    if((iHeader->type & GFX_COMP_MASK) == COMP_IPU)
    {
        return iHeader->colorDepth;
    }    
#endif

	// make sure there are no pending RCC GPU commands
	GFX_WaitForCommandQueue(16);

	// make sure the GPUs are not operating since changing the base addresses
	// will break the currently executing GPU command.
    GFX_WaitForGpu();

	// store the work areas temporarily
	tempWorkArea1.w[0] = G1W1ADRL; 
	tempWorkArea1.w[1] = G1W1ADRH; 
	tempWorkArea2.w[0] = G1W2ADRL; 
	tempWorkArea2.w[1] = G1W2ADRH; 

	// now change the work areass
	GFX_SetWorkArea1(((IMAGE_EXTERNAL*)pImage)->address);
    GFX_SetWorkArea2((WORD)&bpp);

    GFX_RCC_SetSrcOffset(0);
    GFX_RCC_SetDestOffset(0);
    GFX_RCC_SetSize(16/COLOR_DEPTH, 1);
    GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);

	// make sure there are no pending RCC GPU commands
	GFX_WaitForCommandQueue(16);

	// make sure the GPUs are not operating since changing the base addresses
	// will break the currently executing GPU command.
    GFX_WaitForGpu();
    
	GFX_SetWorkArea1(tempWorkArea1.Val);
	GFX_SetWorkArea2(tempWorkArea2.Val);
    
    /* Note: No need to wait for  complete execution of the command even for Blocking Mode. 
             The next commands will be in the queue & hence will execute only after the completion of this command. */
    return (bpp>>8);

}

#endif // USE_DRV_PUTIMAGE

/*********************************************************************
* Function:  BYTE Decompress(DWORD SrcAddress, DWORD DestAddress, DWORD nbytes);
*
* Overview: Decompresses the nbytes number of data at SrcAddress and
*           places starting from DestAddress. (Blocking)
*
* PreCondition: SrcAddress must point to the start of a compressed block.
*
* Input: SrcAddress  - Source address
*        DestAddress - Destination address
*        nbytes      - Number of bytes to be decompressed
*
* Output: error flag
*
* Side Effects: Modifies workarea_1 & workarea_2 registers.
*
********************************************************************/
BYTE Decompress(DWORD SrcAddress, DWORD DestAddress, DWORD nbytes)
{
    GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
    while(GFX_IsPuGpuBusy());
    
    GFX_SetWorkArea1(SrcAddress);
    GFX_IPU_SetSrcOffset(0);
    GFX_SetWorkArea2(DestAddress);
    GFX_IPU_SetDestOffset(0);
    GFX_IPU_Inflate(nbytes);
    
    while(!GFX_IPU_GetDecompressionDone())
    {
        if(GFX_IPU_GetDecompressionError())
        {
            return GFX_IPU_GetDecompressionError();
        }
    }
    
    return GFX_IPU_GetDecompressionError();
}

/*********************************************************************
* Function: void __GFX1Interrupt(void)
*
* Overview: This is an ISR to handle VMRGN Interrupt
*
* PreCondition: Interrupts must be enabled
*
* Input: none
*
* Output: none
*
* Side Effects: Sets the error flag blPaletteChangeError
*
********************************************************************/
void __attribute__((interrupt, shadow, no_auto_psv)) _GFX1Interrupt(void)
{
    _GFX1IF = 0;
    if(_VMRGNIF != 0)
    {
        _VMRGNIF = 0;

        #ifdef USE_PALETTE
        
        if(pPendingPalette != NULL)
        {
            blPaletteChangeError = SetPalette(pPendingPalette, PendingStartEntry, PendingLength);
            if(!blPaletteChangeError)
            {
                _palette = pPendingPalette;
                pPendingPalette = NULL;
            }
        }
        
        #endif
        
        #ifdef USE_DOUBLE_BUFFERING
        
        if(blDisplayUpdatePending)            
        {
            UpdateDisplayNow();
            blDisplayUpdatePending = 0;
        }
        
        #endif
    }    
}

/*********************************************************************
* Function: void StartVBlankInterrupt(void)
*
* Overview: Sets up the Vertical Blanking Interrupt
*
* PreCondition: Interrupts must be enabled
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void StartVBlankInterrupt(void)
{
    #if (GFX_LCD_TYPE == GFX_LCD_TFT)
    
        _VMRGNIF = 0;
        _VMRGNIE = 1;
        _GFX1IE = 1;
    
    #else
    
    #ifdef USE_PALETTE
    
    if(pPendingPalette != NULL)
    {
        blPaletteChangeError = SetPalette(pPendingPalette, PendingStartEntry, PendingLength);
        
        if(!blPaletteChangeError)
        {
            _palette = pPendingPalette;
            pPendingPalette = NULL;
        }
        
    #endif
        
    #ifdef USE_DOUBLE_BUFFERING
    
        if(blDisplayUpdatePending)            
        {
            UpdateDisplayNow();
            blDisplayUpdatePending = 0;
        }
        
    #endif
    
    }

    #endif
}

#ifdef USE_DOUBLE_BUFFERING

BYTE blInvalidateAll;
BYTE blEnableDoubleBuffering;
BYTE NoOfInvalidatedRectangleAreas;
RectangleArea InvalidatedArea[GFX_MAX_INVALIDATE_AREAS];

/*********************************************************************
* Function:  void InvalidateRectangle(WORD left, WORD top, WORD right, WORD bottom)
*
* Overview: Invalidates the specified rectangular area and if the
*           invalidated areas exceed the GFX_MAX_INVALIDATE_AREAS,
*           whole area is marked as invalidate
*
* PreCondition: None
*
* Input: left,top - top left corner coordinates,
*        right,bottom - bottom right corner coordinates
*
* Output: None
*
* Side Effects: Only copies back the invalidated areas to the draw -
*               buffer after the exchange of draw and frame buffers
*
********************************************************************/
void InvalidateRectangle(WORD left, WORD top, WORD right, WORD bottom)
{
    if(blInvalidateAll == 1 || blEnableDoubleBuffering == 0)
    {
        return;
    }
    
    if(NoOfInvalidatedRectangleAreas >= GFX_MAX_INVALIDATE_AREAS)
    {
        blInvalidateAll = 1;
        return;
    }
    else
    {
        WORD width, height;

    #if (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180)

        width   =   right - left + 1;
        height  =   bottom - top + 1;

    #elif (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)

        height  =   right - left + 1;
        width   =   bottom - top + 1;

    #endif

    #if (DISP_ORIENTATION == 90)

        bottom  =   DISP_VER_RESOLUTION - left;
        left    =   top;
        right   =   left + width;
        top     =   bottom - height;

    #elif (DISP_ORIENTATION == 180)

        right   =   DISP_HOR_RESOLUTION - left;
        bottom  =   DISP_VER_RESOLUTION - top;
        left    =   right - width;
        top     =   bottom - height;

    #elif (DISP_ORIENTATION == 270)

        right   =   DISP_HOR_RESOLUTION - top;
        top     =   left;
        bottom  =   top + height;
        left    =   right - width;

    #endif

        InvalidatedArea[NoOfInvalidatedRectangleAreas].X = left;
        InvalidatedArea[NoOfInvalidatedRectangleAreas].Y = top;
        InvalidatedArea[NoOfInvalidatedRectangleAreas].W = width;
        InvalidatedArea[NoOfInvalidatedRectangleAreas].H = height;
        NoOfInvalidatedRectangleAreas++;
    }
}

/*********************************************************************
* Function:  static void ExchangeDrawAndFrameBuffers(void);
*
* Overview: Interchanges Draw and Frame buffers and copies the contents
*           of current frame buffer to the draw buffer
*
* PreCondition: The graphical frame must be completely drawn.
*
* Input: None
*
* Output: None
*
* Side Effects: Always draw on draw buffer & not on frame buffer
*
********************************************************************/
static void ExchangeDrawAndFrameBuffers(void)
{
    DWORD SourceBuffer, DestBuffer;
    
    if(blEnableDoubleBuffering == 0)
    {
        return;
    }
    
    if(_drawbuffer == GFX_BUFFER1)
    {
        SourceBuffer = GFX_BUFFER1;
        DestBuffer   = GFX_BUFFER2;
    }
    else
    {
        SourceBuffer = GFX_BUFFER2;
        DestBuffer   = GFX_BUFFER1;
    }
    
    _drawbuffer = DestBuffer;
    GFX_SetDisplayArea(SourceBuffer);

    GFX_WaitForCommandQueue(GFX_COMMAND_QUEUE_LENGTH);
    while(GFX_IsPuGpuBusy());
    
    GFX_SetWorkArea1(SourceBuffer);
    GFX_SetWorkArea2(DestBuffer);

    if(blInvalidateAll == 1 || NoOfInvalidatedRectangleAreas > GFX_MAX_INVALIDATE_AREAS)
    {
        blInvalidateAll = 0;
        NoOfInvalidatedRectangleAreas = 0;
        GFX_RCC_SetSrcOffset(0);
        GFX_RCC_SetDestOffset(0);
        GFX_RCC_SetSize((WORD)DISP_HOR_RESOLUTION, (WORD)DISP_VER_RESOLUTION);
        GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_CONTINUOUS, RCC_DEST_ADDR_CONTINUOUS);
    }
    else if(NoOfInvalidatedRectangleAreas)
    {
        while(NoOfInvalidatedRectangleAreas)
        {
            DWORD address;
            NoOfInvalidatedRectangleAreas--;
            address = ((DWORD)InvalidatedArea[NoOfInvalidatedRectangleAreas].Y * DISP_HOR_RESOLUTION) + InvalidatedArea[NoOfInvalidatedRectangleAreas].X;
            GFX_WaitForCommandQueue(4);
            GFX_RCC_SetSrcOffset(address);
            GFX_RCC_SetDestOffset(address);
            GFX_RCC_SetSize(InvalidatedArea[NoOfInvalidatedRectangleAreas].W, InvalidatedArea[NoOfInvalidatedRectangleAreas].H);
            GFX_RCC_StartCopy(RCC_COPY, RCC_ROP_C, RCC_SRC_ADDR_DISCONTINUOUS, RCC_DEST_ADDR_DISCONTINUOUS);
        }
    }
}

/*********************************************************************
* Function:  void UpdateDisplayNow(void)
*
* Overview: Synchronizes the draw and frame buffers immediately
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void UpdateDisplayNow(void)
{
    ExchangeDrawAndFrameBuffers();
}

/*********************************************************************
* Function:  void RequestDisplayUpdate(void)
*
* Overview: Synchronizes the draw and frame buffers at next VBlank
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void RequestDisplayUpdate(void)
{
    if(blInvalidateAll == 1 || NoOfInvalidatedRectangleAreas > 0)
    {
        blDisplayUpdatePending = 1;
        StartVBlankInterrupt();
    }
}

#endif //USE_DOUBLE_BUFFERING

#ifdef USE_PALETTE

/*********************************************************************
* Function: void PaletteInit(void)
*
* Overview: Initializes the CLUT.
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void PaletteInit(void)
{
    while(_CLUTBUSY != 0)
    {
        Nop();
    }
    _CLUTRWEN = 0;
    _CLUTEN = 0;
}

/*********************************************************************
* Function: void EnablePalette(void)
*
* Overview: Enables the Palette mode
*
* PreCondition: A system palette has been set using RequestPaletteChange()
*               or SetPalette() and SetPaletteBpp() must have been called
*
* Input: none
*
* Output: none
*
* Side Effects: There may be a little flicker with SetPalette() which
*               can be avoided for TFT with RequestPaletteChange()
*
********************************************************************/
void EnablePalette(void)
{
    if(PaletteBpp <= GFX_16_BPP)
    {
        _PUBPP = PaletteBpp;
        _DPBPP = PaletteBpp;
        _CLUTEN = 1;
    }    
}

/*********************************************************************
* Function: void DisablePalette(void)
*
* Overview: Disables the Palette mode
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: There may be a little flicker which can be avoided for TFT
*               by preceding with a dummy RequestPaletteChange()
*
********************************************************************/
void DisablePalette(void)
{
    _CLUTEN = 0;
    _PUBPP  = GFX_BITS_PER_PIXEL;
    _DPBPP  = GFX_BITS_PER_PIXEL;
}

/*********************************************************************
* Function: BYTE IsPaletteEnabled(void)
*
* Overview: Returns if the Palette mode is enabled or not
*
* PreCondition: none
*
* Input: none
*
* Output: Enabled -> 1, Disabled -> 0
*
* Side Effects:
*
********************************************************************/
BYTE IsPaletteEnabled(void)
{
    return _CLUTEN;
}

/*********************************************************************
* Function: BYTE SetPaletteBpp(BYTE bpp)
*
* Overview: Sets the CLUT's number of valid entries.
*
* PreCondition: PaletteInit() must be called before.
*
* Input: bpp -> Bits per pixel
*
* Output: Status: Zero -> Success, Non-zero -> Error.
*
* Side Effects: Drawing mode will change to support palettes
*
********************************************************************/
BYTE SetPaletteBpp(BYTE bpp)
{
    switch(bpp)
    {
        case 1:         PaletteBpp = GFX_1_BPP;
                        break;

        case 2:         PaletteBpp = GFX_2_BPP;
                        break;

        case 4:         PaletteBpp = GFX_4_BPP;
                        break;

        case 8:         PaletteBpp = GFX_8_BPP;
                        break;

        default:        PaletteBpp = GFX_16_BPP;
                        return -1;                        
    }
    
    return 0;
}

/*********************************************************************
* Function: BYTE SetPaletteFlash(PALETTE_ENTRY *pPaletteEntry, WORD startEntry, WORD length)
*
* Overview: Loads the palettes from the flash immediately.
*
* PreCondition: PaletteInit() must be called before.
*
* Input: pPaletteEntry   - Pointer to the palette table in ROM
*        startEntry      - Start entry to load (inclusive)
*        length          - Number of entries
*
* Output: Status: Zero -> Success, Non-zero -> Error.
*
* Side Effects: There may be a slight flicker when the Palette entries
*               are getting loaded one by one.
*
********************************************************************/
BYTE SetPaletteFlash(PALETTE_ENTRY *pPaletteEntry, WORD startEntry, WORD length)
{
    WORD counter;

    if((pPaletteEntry == NULL) || ((startEntry + length) > 256))
    {
        return -1;
    }

    _CLUTRWEN = 1;
    for(counter = 0; counter < length; counter++)
    {
        _CLUTADR = counter;
        _CLUTWR  = pPaletteEntry[counter].value;
        while(_CLUTBUSY)
        {
            Nop();
        }
    }
    _CLUTRWEN = 0;

    return 0;
}

#endif // USE_PALETTE
