/*
 
 SDL_imageFilter - bytes-image "filter" routines 
 (uses inline x86 MMX optimizations if available)
 
 LGPL (c) A. Schiffler

 Note: Most MMX code is based on published routines 
 by Vladimir Kravtchenko at vk@cs.ubc.ca - credits to 
 him for his work.

*/

#include <stdio.h>
#include <stdlib.h>

#include "SDL_imageFilter.h"

#define swap_32(x) (((x) >> 24) | (((x) & 0x00ff0000) >> 8)  | (((x) & 0x0000ff00) << 8)  | ((x) << 24))

/* ------ Static variables ----- */

/* Toggle the use of the MMX routines - ON by default */

static int SDL_imageFilterUseMMX = 1;

/* MMX detection routine (with override flag) */

unsigned int cpuFlags()
{
    int flags = 0;

#ifdef USE_MMX
    asm volatile ("pusha		     \n\t" "mov    %1, %%eax     \n\t"	
		  "cpuid                \n\t"	
		  "mov    %%edx, %0     \n\t"	
		  "popa		     \n\t":"=m" (flags)	
		  :"i"(0x00000001)	
	);
#endif

    return (flags);
}

int SDL_imageFilterMMXdetect(void)
{
    unsigned int mmx_bit;

    /* Check override flag */
    if (SDL_imageFilterUseMMX == 0) {
	return (0);
    }

    mmx_bit = cpuFlags();
    mmx_bit &= 0x00800000;
    mmx_bit = (mmx_bit && 0x00800000);

    return (mmx_bit);
}

void SDL_imageFilterMMXoff()
{
    SDL_imageFilterUseMMX = 0;
}

void SDL_imageFilterMMXon()
{
    SDL_imageFilterUseMMX = 1;
}

int SDL_imageFilterAddMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov          %2, %%eax \n\t"	
      "mov          %1, %%ebx \n\t"	
      "mov          %0, %%edi \n\t"	
       "mov          %3, %%ecx \n\t"
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"
      ".L1010:                \n\t" "movq    (%%eax), %%mm1 \n\t"
      "paddusb (%%ebx), %%mm1 \n\t"
      "movq    %%mm1, (%%edi) \n\t"
       "add          $8, %%eax \n\t"
      "add          $8, %%ebx \n\t"	
      "add          $8, %%edi \n\t" "dec              %%ecx \n\t"
      "jnz             .L1010 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}

int SDL_imageFilterAdd(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* Use MMX assembly routine */
	SDL_imageFilterAddMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 + (int) *cursrc2;
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}

int SDL_imageFilterMeanMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length,
			   unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "movl         %4, %%edx \n\t"	
      "movq    (%%edx), %%mm0 \n\t"	
       "mov          %2, %%eax \n\t"	
      "mov          %1, %%ebx \n\t"	
      "mov          %0, %%edi \n\t"	
       "mov          %3, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L21011:                \n\t" 
      "movq    (%%eax), %%mm1 \n\t"	
      "movq    (%%ebx), %%mm2 \n\t"	
      
       "psrlw        $1, %%mm1 \n\t"	
      "psrlw        $1, %%mm2 \n\t"	

      ".byte     0x0f, 0xdb, 0xc8 \n\t"

      ".byte     0x0f, 0xdb, 0xd0 \n\t" 
      "paddusb   %%mm2, %%mm1 \n\t"	
      "movq    %%mm1, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%ebx \n\t"	
      "add          $8, %%edi \n\t" 
      "dec              %%ecx \n\t"	
      "jnz             .L21011 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length),		
      "m"(Mask)			
	);
#endif
    return (0);
}


int SDL_imageFilterMean(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    static unsigned char Mask[8] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMeanMMX(Src1, Src2, Dest, length, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 / 2 + (int) *cursrc2 / 2;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterSubMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	
      "mov %1, %%ebx \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       ".align 16       \n\t"	
      ".L1012:         \n\t" "movq    (%%eax), %%mm1 \n\t"	
      "psubusb (%%ebx), %%mm1 \n\t"	
      "movq    %%mm1, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%ebx \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L1012    \n\t"	
       "emms          \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}

int SDL_imageFilterSub(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterSubMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 - (int) *cursrc2;
	if (result < 0)
	    result = 0;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterAbsDiffMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	
      "mov %1, %%ebx \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       ".align 16       \n\t"	
      ".L1013:         \n\t" "movq    (%%eax), %%mm1 \n\t"	
      "movq    (%%ebx), %%mm2 \n\t"	
      "psubusb (%%ebx), %%mm1 \n\t"	
      "psubusb (%%eax), %%mm2 \n\t"	
      "por       %%mm2, %%mm1 \n\t"	
      "movq    %%mm1, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%ebx \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L1013    \n\t"	
       "emms          \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterAbsDiff(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterAbsDiffMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = abs((int) *cursrc1 - (int) *cursrc2);
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterMultMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	
      "mov %1, %%ebx \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       "pxor      %%mm0, %%mm0 \n\t"	
       ".align 16       \n\t"	
      ".L1014:         \n\t" "movq    (%%eax), %%mm1 \n\t"	
      "movq    (%%ebx), %%mm3 \n\t"	
      "movq      %%mm1, %%mm2 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
       "punpcklbw %%mm0, %%mm1 \n\t"	
      "punpckhbw %%mm0, %%mm2 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
       "pmullw    %%mm3, %%mm1 \n\t"	
      "pmullw    %%mm4, %%mm2 \n\t"	
      
       "movq      %%mm1, %%mm5 \n\t"	
      "movq      %%mm2, %%mm6 \n\t"	
       "psraw       $15, %%mm5 \n\t"	
      "psraw       $15, %%mm6 \n\t"	
       "pxor      %%mm5, %%mm1 \n\t"	
      "pxor      %%mm6, %%mm2 \n\t"	
       "psubsw    %%mm5, %%mm1 \n\t"	
      "psubsw    %%mm6, %%mm2 \n\t"	
       "packuswb  %%mm2, %%mm1 \n\t"	
       "movq    %%mm1, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%ebx \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L1014    \n\t"	
       "emms          \n\t"	
      "popa \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterMult(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMultMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {

	/* NOTE: this is probably wrong - dunno what the MMX code does */

	result = (int) *cursrc1 * (int) *cursrc2;
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterMultNorASM(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%edx \n\t"	
      "mov %1, %%esi \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
       ".align 16       \n\t"	
      ".L10141:        \n\t" "mov  (%%edx), %%al \n\t"	
      "mulb (%%esi)       \n\t"	
       ".L10142:           \n\t" "mov %%al, (%%edi)  \n\t"	
       "inc %%edx \n\t"		
      "inc %%esi \n\t"		
      "inc %%edi \n\t" "dec %%ecx      \n\t"	
      "jnz .L10141    \n\t"	
       "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterMultNor(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if (length > 0) {
	/* ASM routine */
	SDL_imageFilterMultNorASM(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* No bytes - we are done */
	return (0);
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 * (int) *cursrc2;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterMultDivby2MMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha \n\t" "mov %2, %%eax \n\t"	
      "mov %1, %%ebx \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       "pxor      %%mm0, %%mm0 \n\t"	
       ".align 16       \n\t"	
      ".L1015:         \n\t" "movq    (%%eax), %%mm1 \n\t"	
      "movq    (%%ebx), %%mm3 \n\t"	
      "movq      %%mm1, %%mm2 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
       "punpcklbw %%mm0, %%mm1 \n\t"	
      "punpckhbw %%mm0, %%mm2 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
       "psrlw        $1, %%mm1 \n\t"	
      "psrlw        $1, %%mm2 \n\t"	
       "pmullw    %%mm3, %%mm1 \n\t"	
      "pmullw    %%mm4, %%mm2 \n\t"	
       "packuswb  %%mm2, %%mm1 \n\t"	
       "movq    %%mm1, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%ebx \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L1015    \n\t"	
       "emms          \n\t"	
      "popa \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterMultDivby2(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMultDivby2MMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = ((int) *cursrc1 / 2) * (int) *cursrc2;
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterMultDivby4MMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	
      "mov %1, %%ebx \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       "pxor      %%mm0, %%mm0 \n\t"	
       ".align 16       \n\t"	
      ".L1016:         \n\t" "movq    (%%eax), %%mm1 \n\t"	
      "movq    (%%ebx), %%mm3 \n\t"	
      "movq      %%mm1, %%mm2 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
       "punpcklbw %%mm0, %%mm1 \n\t"	
      "punpckhbw %%mm0, %%mm2 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
       "psrlw        $1, %%mm1 \n\t"	
      "psrlw        $1, %%mm2 \n\t"	
      "psrlw        $1, %%mm3 \n\t"	
      "psrlw        $1, %%mm4 \n\t"	
       "pmullw    %%mm3, %%mm1 \n\t"	
      "pmullw    %%mm4, %%mm2 \n\t"	
       "packuswb  %%mm2, %%mm1 \n\t"	
       "movq    %%mm1, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%ebx \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L1016    \n\t"	
       "emms          \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterMultDivby4(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterMultDivby4MMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = ((int) *cursrc1 / 2) * ((int) *cursrc2 / 2);
	if (result > 255)
	    result = 255;
	*curdst = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterBitAndMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	
      "mov %1, %%ebx \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       ".align 16       \n\t"	
      ".L1017:         \n\t" "movq    (%%eax), %%mm1 \n\t"	
      "pand    (%%ebx), %%mm1 \n\t"	
      "movq    %%mm1, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%ebx \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L1017    \n\t"	
       "emms          \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterBitAnd(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;


    if (length > 7) {
	/* Call MMX routine */

	SDL_imageFilterBitAndMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {

	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdst = (*cursrc1) & (*cursrc2);
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterBitOrMMX(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov %2, %%eax \n\t"	
      "mov %1, %%ebx \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       ".align 16       \n\t"	
      ".L91017:        \n\t" "movq    (%%eax), %%mm1 \n\t"	
      "por     (%%ebx), %%mm1 \n\t"	
      "movq    %%mm1, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%ebx \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L91017   \n\t"	
       "emms          \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterBitOr(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *cursrc2, *curdst;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterBitOrMMX(Src1, Src2, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    cursrc2 = &Src2[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	cursrc2 = Src2;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdst = *cursrc1 | *cursrc2;
	/* Advance pointers */
	cursrc1++;
	cursrc2++;
	curdst++;
    }
    return (0);
}


int SDL_imageFilterDivASM(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha \n\t" "mov %2, %%edx \n\t"	
      "mov %1, %%esi \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %3, %%ecx \n\t"	
       ".align 16     \n\t"	
      ".L10191:      \n\t" "mov  (%%esi), %%bl  \n\t"	
      "cmp       $0, %%bl  \n\t"	
      "jnz .L10192         \n\t" "movb  $255, (%%edi) \n\t"	
      "jmp  .L10193        \n\t" ".L10192:            \n\t" "xor   %%ah, %%ah    \n\t"	
      "mov   (%%edx), %%al \n\t"	
      "div   %%bl          \n\t"	
      "mov   %%al, (%%edi) \n\t"	
       ".L10193:            \n\t" "inc %%edx \n\t"	
      "inc %%esi \n\t"		
      "inc %%edi \n\t" "dec %%ecx    \n\t"	
      "jnz .L10191  \n\t"	
       "popa \n\t":"=m" (Dest)	
      :"m"(Src2),		
      "m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterDiv(unsigned char *Src1, unsigned char *Src2, unsigned char *Dest, int length)
{
    if (length > 0) {
	/* Call ASM routine */
	SDL_imageFilterDivASM(Src1, Src2, Dest, length);

	/* Never unaligned bytes - we are done */
	return (0);
    } else {
	return (-1);
    }
}




int SDL_imageFilterBitNegationMMX(unsigned char *Src1, unsigned char *Dest, int length)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "pcmpeqb   %%mm1, %%mm1 \n\t"	
       "mov %1, %%eax \n\t"	
      "mov %0, %%edi \n\t"	
       "mov %2, %%ecx \n\t"	
      "shr $3, %%ecx \n\t"	
       ".align 16       \n\t"	
      ".L91117:        \n\t" "movq    (%%eax), %%mm0 \n\t"	
      "pxor      %%mm1, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
       "add $8, %%eax \n\t"	
      "add $8, %%edi \n\t" "dec %%ecx     \n\t"	
      "jnz .L91117   \n\t"	
       "emms          \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length)		
	);
#endif
    return (0);
}


int SDL_imageFilterBitNegation(unsigned char *Src1, unsigned char *Dest, int length)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdst;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {
	/* MMX routine */
	SDL_imageFilterBitNegationMMX(Src1, Dest, length);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdst = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdst = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdst = ~(*cursrc1);
	/* Advance pointers */
	cursrc1++;
	curdst++;
    }

    return (0);
}


int SDL_imageFilterAddByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "mov           %3, %%al \n\t"	
      "mov         %%al, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L1021:                \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	
      "paddusb   %%mm1, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L1021 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(C)			
	);
#endif
    return (0);
}


int SDL_imageFilterAddByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1, *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterAddByteMMX(Src1, Dest, length, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 + iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }
    return (0);
}


int SDL_imageFilterAddUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C, unsigned int Cs)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "mov          %3, %%eax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "mov          %4, %%eax \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
      "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L11023:                \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	
      "paddusb   %%mm1, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
      "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L11023 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(C),			
      "m"(Cs)			
	);
#endif
    return (0);
}


int SDL_imageFilterAddUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C)
{
    unsigned int i, j, istart, D;
    int iC[4];
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	D=swap_32(C);
	SDL_imageFilterAddUintMMX(Src1, Dest, length, C, D);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC[0] = (int) ((C >> 24) & 0xff);
    iC[1] = (int) ((C >> 16) & 0xff);
    iC[2] = (int) ((C >>  8) & 0xff);
    iC[3] = (int) ((C >>  0) & 0xff);
    for (i = istart; i < length; i += 4) {
     for (j = 0; j < 4; j++) {
      if ((i+j)<length) {
	result = (int) *cursrc1 + iC[j];
	if (result > 255) result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
      }
     }
    }
    return (0);
}



int SDL_imageFilterAddByteToHalfMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C,
				    unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "mov           %3, %%al \n\t"	
      "mov         %%al, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
       "movl         %4, %%edx \n\t"	
      "movq    (%%edx), %%mm0 \n\t"	
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L1022:                \n\t" 
      "movq    (%%eax), %%mm2 \n\t"	
      "psrlw        $1, %%mm2 \n\t"	
      
      ".byte     0x0f, 0xdb, 0xd0 \n\t" 
      "paddusb   %%mm1, %%mm2 \n\t"	
      "movq    %%mm2, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L1022 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(C),			
      "m"(Mask)			
	);
#endif
    return (0);
}


int SDL_imageFilterAddByteToHalf(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    static unsigned char Mask[8] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterAddByteToHalfMMX(Src1, Dest, length, C, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) (*cursrc1 / 2) + iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterSubByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "mov           %3, %%al \n\t"	
      "mov         %%al, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
      "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L1023:                \n\t" "movq    (%%eax), %%mm0 \n\t"	
      "psubusb   %%mm1, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L1023 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(C)			
	);
#endif
    return (0);
}


int SDL_imageFilterSubByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterSubByteMMX(Src1, Dest, length, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 - iC;
	if (result < 0)
	    result = 0;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }
    return (0);
}


int SDL_imageFilterSubUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C, unsigned int Cs)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "mov          %3, %%eax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "mov          %4, %%eax \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
      "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L11024:                \n\t" "movq    (%%eax), %%mm0 \n\t"	
      "psubusb   %%mm1, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L11024 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(C),			
      "m"(Cs)			
	);
#endif
    return (0);
}


int SDL_imageFilterSubUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned int C)
{
    unsigned int i, j, istart, D;
    int iC[4];
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	D=swap_32(C);
	SDL_imageFilterSubUintMMX(Src1, Dest, length, C, D);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC[0] = (int) ((C >> 24) & 0xff);
    iC[1] = (int) ((C >> 16) & 0xff);
    iC[2] = (int) ((C >>  8) & 0xff);
    iC[3] = (int) ((C >>  0) & 0xff);
    for (i = istart; i < length; i += 4) {
     for (j = 0; j < 4; j++) {
      if ((i+j)<length) {
	result = (int) *cursrc1 - iC[j];
	if (result < 0) result = 0;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
      }
     }
    }
    return (0);
}



int SDL_imageFilterShiftRightMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
				 unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "movl         %4, %%edx \n\t"	
      "movq    (%%edx), %%mm0 \n\t"	
       "xor       %%ecx, %%ecx \n\t"	
      "mov           %3, %%cl \n\t"	
      "movd      %%ecx, %%mm3 \n\t"	
       "pcmpeqb   %%mm1, %%mm1 \n\t"	
       ".L10240:               \n\t"	
       "psrlw        $1, %%mm1 \n\t"	
      
      ".byte     0x0f, 0xdb, 0xc8 \n\t" 
      "dec               %%cl \n\t"	
      "jnz            .L10240 \n\t"	
      
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L10241:               \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	
      "psrlw     %%mm3, %%mm0 \n\t"	
      
      ".byte     0x0f, 0xdb, 0xc1 \n\t" 
      "movq    %%mm0, (%%edi) \n\t"	
      "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz            .L10241 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(N),			
      "m"(Mask)			
	);
#endif
    return (0);
}


int SDL_imageFilterShiftRight(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    static unsigned char Mask[8] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;

    /* Check shift */
    if ((N > 8) || (N < 1)) {
	return (-1);
    }

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	/* MMX routine */
	SDL_imageFilterShiftRightMMX(Src1, Dest, length, N, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdest = (unsigned char) *cursrc1 >> N;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterShiftRightUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L13023:                \n\t" "movq    (%%eax), %%mm0 \n\t"	
      "psrld   %3, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
      "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L13023 \n\t"	
      "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(N)			
	);
#endif
    return (0);
}


int SDL_imageFilterShiftRightUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    unsigned int *icursrc1, *icurdest;
    int result;

    
    if ((N > 32) || (N < 1)) return (-1);

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftRightUintMMX(Src1, Dest, length, N);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    icursrc1=(unsigned int *)cursrc1;
    icurdest=(unsigned int *)curdest;
    for (i = istart; i < length; i += 4) {
     if ((i+4)<length) {
	result = ((unsigned int)*icursrc1 >> N);
	*icurdest = (unsigned int)result;
     }
     /* Advance pointers */
     icursrc1++;
     icurdest++;
    }

    return (0);
}


int SDL_imageFilterMultByByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "mov           %3, %%al \n\t"	
      "xor         %%ah, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
       "pxor      %%mm0, %%mm0 \n\t"	
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       "cmp         $128, %%al \n\t"	
      "jg             .L10251 \n\t" ".align 16              \n\t"	
      ".L10250:               \n\t" "movq    (%%eax), %%mm3 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
      "pmullw    %%mm1, %%mm3 \n\t"	
      "pmullw    %%mm1, %%mm4 \n\t"	
      "packuswb  %%mm4, %%mm3 \n\t"	
      "movq    %%mm3, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz            .L10250 \n\t"	
      "jmp            .L10252 \n\t" ".align 16              \n\t"	
      ".L10251:               \n\t" "movq    (%%eax), %%mm3 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
      "pmullw    %%mm1, %%mm3 \n\t"	
      "pmullw    %%mm1, %%mm4 \n\t"	
      
       "movq      %%mm3, %%mm5 \n\t"	
      "movq      %%mm4, %%mm6 \n\t"	
      "psraw       $15, %%mm5 \n\t"	
      "psraw       $15, %%mm6 \n\t"	
      "pxor      %%mm5, %%mm3 \n\t"	
      "pxor      %%mm6, %%mm4 \n\t"	
      "psubsw    %%mm5, %%mm3 \n\t"	
      "psubsw    %%mm6, %%mm4 \n\t"	
      "packuswb  %%mm4, %%mm3 \n\t"	
      "movq    %%mm3, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz            .L10251 \n\t"	
       ".L10252:               \n\t" "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(C)			
	);
#endif
    return (0);
}


int SDL_imageFilterMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterMultByByteMMX(Src1, Dest, length, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 * iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterShiftRightAndMultByByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
					      unsigned char C)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "mov           %4, %%al \n\t"	
      "xor         %%ah, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
       "xor       %%ecx, %%ecx \n\t"	
      "mov           %3, %%cl \n\t"	
      "movd      %%ecx, %%mm7 \n\t"	
       "pxor      %%mm0, %%mm0 \n\t"	
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L1026:                \n\t" "movq    (%%eax), %%mm3 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
      "psrlw     %%mm7, %%mm3 \n\t"	
      "psrlw     %%mm7, %%mm4 \n\t"	
      "pmullw    %%mm1, %%mm3 \n\t"	
      "pmullw    %%mm1, %%mm4 \n\t"	
      "packuswb  %%mm4, %%mm3 \n\t"	
      "movq    %%mm3, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L1026 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(N),			
      "m"(C)			
	);
#endif
    return (0);
}


int SDL_imageFilterShiftRightAndMultByByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
					   unsigned char C)
{
    unsigned int i, istart;
    int iC;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int result;

    /* Check shift */
    if ((N > 8) || (N < 1)) {
	return (-1);
    }

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftRightAndMultByByteMMX(Src1, Dest, length, N, C);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    iC = (int) C;
    for (i = istart; i < length; i++) {
	result = (int) (*cursrc1 >> N) * iC;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterShiftLeftByteMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N,
				    unsigned char *Mask)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "movl         %4, %%edx \n\t"	
      "movq    (%%edx), %%mm0 \n\t"	
       "xor       %%ecx, %%ecx \n\t"	
      "mov           %3, %%cl \n\t"	
      "movd      %%ecx, %%mm3 \n\t"	
       "pcmpeqb   %%mm1, %%mm1 \n\t"	
       ".L10270:               \n\t"	
       "psllw        $1, %%mm1 \n\t"	
      
      ".byte     0x0f, 0xdb, 0xc8 \n\t" "dec %%cl               \n\t"	
      "jnz            .L10270 \n\t"	
      
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L10271:               \n\t" "movq    (%%eax), %%mm0 \n\t"	
      "psllw     %%mm3, %%mm0 \n\t"	
      
      ".byte     0x0f, 0xdb, 0xc1 \n\t" "movq    %%mm0, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz            .L10271 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(N),			
      "m"(Mask)			
	);
#endif
    return (0);
}


int SDL_imageFilterShiftLeftByte(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    static unsigned char Mask[8] = { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE };
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    int result;

    if ((N > 8) || (N < 1))
	return (-1);		
    

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftLeftByteMMX(Src1, Dest, length, N, Mask);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = ((int) *cursrc1 << N) & 0xff;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterShiftLeftUintMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L12023:                \n\t" "movq    (%%eax), %%mm0 \n\t"	
      "pslld   %3, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
      "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L12023 \n\t"	
      "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(N)			
	);
#endif
    return (0);
}


int SDL_imageFilterShiftLeftUint(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    unsigned int *icursrc1, *icurdest;
    int result;

    
    if ((N > 32) || (N < 1)) return (-1);

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftLeftUintMMX(Src1, Dest, length, N);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    icursrc1=(unsigned int *)cursrc1;
    icurdest=(unsigned int *)curdest;
    for (i = istart; i < length; i += 4) {
     if ((i+4)<length) {
	result = ((unsigned int)*icursrc1 << N);
	*icurdest = (unsigned int)result;
     }
     /* Advance pointers */
     icursrc1++;
     icurdest++;
    }

    return (0);
}


int SDL_imageFilterShiftLeftMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "xor       %%eax, %%eax \n\t"	
      "mov           %3, %%al \n\t"	
      "movd      %%eax, %%mm7 \n\t"	
       "pxor      %%mm0, %%mm0 \n\t"	
       "mov         %1, %%eax  \n\t"	
      "mov         %0, %%edi  \n\t"	
      "mov         %2, %%ecx  \n\t"	
      "shr         $3, %%ecx  \n\t"	
       "cmp           $7, %%al \n\t"	
      "jg             .L10281 \n\t" ".align 16              \n\t"	
      ".L10280:               \n\t" "movq    (%%eax), %%mm3 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
      "psllw     %%mm7, %%mm3 \n\t"	
      "psllw     %%mm7, %%mm4 \n\t"	
      "packuswb  %%mm4, %%mm3 \n\t"	
      "movq    %%mm3, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz            .L10280 \n\t"	
      "jmp            .L10282 \n\t" ".align 16              \n\t"	
      ".L10281:               \n\t" "movq    (%%eax), %%mm3 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
      "punpcklbw %%mm0, %%mm3 \n\t"	
      "punpckhbw %%mm0, %%mm4 \n\t"	
      "psllw     %%mm7, %%mm3 \n\t"	
      "psllw     %%mm7, %%mm4 \n\t"	
      
       "movq      %%mm3, %%mm5 \n\t"	
      "movq      %%mm4, %%mm6 \n\t"	
      "psraw       $15, %%mm5 \n\t"	
      "psraw       $15, %%mm6 \n\t"	
      "pxor      %%mm5, %%mm3 \n\t"	
      "pxor      %%mm6, %%mm4 \n\t"	
      "psubsw    %%mm5, %%mm3 \n\t"	
      "psubsw    %%mm6, %%mm4 \n\t"	
       "packuswb  %%mm4, %%mm3 \n\t"	
      "movq    %%mm3, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz            .L10281 \n\t"	
       ".L10282:               \n\t" "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(N)			
	);
#endif
    return (0);
}


int SDL_imageFilterShiftLeft(unsigned char *Src1, unsigned char *Dest, int length, unsigned char N)
{
    unsigned int i, istart;
    unsigned char *cursrc1, *curdest;
    int result;

    if ((N > 8) || (N < 1))
	return (-1);		
    

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterShiftLeftMMX(Src1, Dest, length, N);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	result = (int) *cursrc1 << N;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterBinarizeUsingThresholdMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char T)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t"
      
      "pcmpeqb   %%mm1, %%mm1 \n\t"	
      "pcmpeqb   %%mm2, %%mm2 \n\t"	
       "mov           %3, %%al \n\t"	
      "mov         %%al, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm3 \n\t"	
      "movd      %%eax, %%mm4 \n\t"	
      "punpckldq %%mm4, %%mm3 \n\t"	
       "psubusb   %%mm3, %%mm2 \n\t"	
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L1029:                \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	
      "paddusb   %%mm2, %%mm0 \n\t"	
      "pcmpeqb   %%mm1, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L1029 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(T)			
	);
#endif
    return (0);
}


int SDL_imageFilterBinarizeUsingThreshold(unsigned char *Src1, unsigned char *Dest, int length, unsigned char T)
{
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterBinarizeUsingThresholdMMX(Src1, Dest, length, T);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	*curdest = ((unsigned char) *cursrc1 >= T) ? 255 : 0;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterClipToRangeMMX(unsigned char *Src1, unsigned char *Dest, int length, unsigned char Tmin,
				  unsigned char Tmax)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "pcmpeqb   %%mm1, %%mm1 \n\t"	
      
       "mov           %4, %%al \n\t"	
      "mov         %%al, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm3 \n\t"	
      "movd      %%eax, %%mm4 \n\t"	
      "punpckldq %%mm4, %%mm3 \n\t"	
       "psubusb   %%mm3, %%mm1 \n\t"	
      
       "mov           %3, %%al \n\t"	
      "mov         %%al, %%ah \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm5 \n\t"	
      "movd      %%eax, %%mm4 \n\t"	
      "punpckldq %%mm4, %%mm5 \n\t"	
       "movq      %%mm5, %%mm7 \n\t"	
      "paddusb   %%mm1, %%mm7 \n\t"	
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L1030:                \n\t" 
      "movq    (%%eax), %%mm0 \n\t"	
      "paddusb   %%mm1, %%mm0 \n\t"	
      "psubusb   %%mm7, %%mm0 \n\t"	
      "paddusb   %%mm5, %%mm0 \n\t"	
      "movq    %%mm0, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L1030 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(Tmin),		
      "m"(Tmax)			
	);
#endif
    return (0);
}


int SDL_imageFilterClipToRange(unsigned char *Src1, unsigned char *Dest, int length, unsigned char Tmin,
			       unsigned char Tmax)
{
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterClipToRangeMMX(Src1, Dest, length, Tmin, Tmax);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    for (i = istart; i < length; i++) {
	if (*cursrc1 < Tmin) {
	    *curdest = Tmin;
	} else if (*cursrc1 > Tmax) {
	    *curdest = Tmax;
	} else {
	    *curdest = *cursrc1;
	}
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}


int SDL_imageFilterNormalizeLinearMMX(unsigned char *Src1, unsigned char *Dest, int length, int Cmin, int Cmax,
				      int Nmin, int Nmax)
{
#ifdef USE_MMX
    asm volatile
     ("pusha		     \n\t" "mov           %6, %%ax \n\t"	
      "mov           %4, %%bx \n\t"	
      "sub           %5, %%ax \n\t"	
      "sub           %3, %%bx \n\t"	
      "jz             .L10311 \n\t"	
      "xor         %%dx, %%dx \n\t"	
      "div               %%bx \n\t"	
      "jmp            .L10312 \n\t" ".L10311:               \n\t" "mov         $255, %%ax \n\t"	
       ".L10312:               \n\t"	
       "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm0 \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "punpckldq %%mm1, %%mm0 \n\t"	
      
       "mov           %3, %%ax \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm1 \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "punpckldq %%mm2, %%mm1 \n\t"	
      
       "mov           %5, %%ax \n\t"	
      "mov         %%ax, %%bx \n\t"	
      "shl         $16, %%eax \n\t"	
      "mov         %%bx, %%ax \n\t"	
      "movd      %%eax, %%mm2 \n\t"	
      "movd      %%eax, %%mm3 \n\t"	
      "punpckldq %%mm3, %%mm2 \n\t"	
       "pxor      %%mm7, %%mm7 \n\t"	
       "mov          %1, %%eax \n\t"	
      "mov          %0, %%edi \n\t"	
      "mov          %2, %%ecx \n\t"	
      "shr          $3, %%ecx \n\t"	
       ".align 16              \n\t"	
      ".L1031:                \n\t" 
      "movq    (%%eax), %%mm3 \n\t"	
      "movq      %%mm3, %%mm4 \n\t"	
      "punpcklbw %%mm7, %%mm3 \n\t"	
      "punpckhbw %%mm7, %%mm4 \n\t"	
      "psubusb   %%mm1, %%mm3 \n\t"	
      "psubusb   %%mm1, %%mm4 \n\t"	
      "pmullw    %%mm0, %%mm3 \n\t"	
      "pmullw    %%mm0, %%mm4 \n\t"	
      "paddusb   %%mm2, %%mm3 \n\t"	
      "paddusb   %%mm2, %%mm4 \n\t"	
      
       "movq      %%mm3, %%mm5 \n\t"	
      "movq      %%mm4, %%mm6 \n\t"	
      "psraw       $15, %%mm5 \n\t"	
      "psraw       $15, %%mm6 \n\t"	
      "pxor      %%mm5, %%mm3 \n\t"	
      "pxor      %%mm6, %%mm4 \n\t"	
      "psubsw    %%mm5, %%mm3 \n\t"	
      "psubsw    %%mm6, %%mm4 \n\t"	
      "packuswb  %%mm4, %%mm3 \n\t"	
       "movq    %%mm3, (%%edi) \n\t"	
       "add          $8, %%eax \n\t"	
      "add          $8, %%edi \n\t"	
      "dec              %%ecx \n\t"	
      "jnz             .L1031 \n\t"	
       "emms                   \n\t"	
      "popa                   \n\t":"=m" (Dest)	
      :"m"(Src1),		
      "m"(length),		
      "m"(Cmin),		
      "m"(Cmax),		
      "m"(Nmin),		
      "m"(Nmax)			
	);
#endif
    return (0);
}


int SDL_imageFilterNormalizeLinear(unsigned char *Src1, unsigned char *Dest, int length, int Cmin, int Cmax, int Nmin,
				   int Nmax)
{
    unsigned int i, istart;
    unsigned char *cursrc1;
    unsigned char *curdest;
    int dN, dC, factor;
    int result;

    if ((SDL_imageFilterMMXdetect()) && (length > 7)) {

	SDL_imageFilterNormalizeLinearMMX(Src1, Dest, length, Cmin, Cmax, Nmin, Nmax);

	/* Check for unaligned bytes */
	if ((length & 7) > 0) {
	    /* Setup to process unaligned bytes */
	    istart = length & 0xfffffff8;
	    cursrc1 = &Src1[istart];
	    curdest = &Dest[istart];
	} else {
	    /* No unaligned bytes - we are done */
	    return (0);
	}
    } else {
	/* Setup to process whole image */
	istart = 0;
	cursrc1 = Src1;
	curdest = Dest;
    }

    /* C routine to process image */
    dC = Cmax - Cmin;
    if (dC == 0)
	return (0);
    dN = Nmax - Nmin;
    factor = dN / dC;
    for (i = istart; i < length; i++) {
	result = factor * ((int) (*cursrc1) - Cmin) + Nmin;
	if (result > 255)
	    result = 255;
	*curdest = (unsigned char) result;
	/* Advance pointers */
	cursrc1++;
	curdest++;
    }

    return (0);
}




int SDL_imageFilterConvolveKernel3x3Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 3) || (rows < 3) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	  "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "movq    (%%edx), %%mm5 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm6 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm7 \n\t"	

	  "mov          %3, %%eax \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "inc              %%edi \n\t"	
	   "mov          %2, %%edx \n\t"	
	  "sub          $2, %%edx \n\t"	

	  ".L10320:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $2, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10322:               \n\t"

	  "movq    (%%esi), %%mm1 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%esi), %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%esi), %%mm3 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpcklbw %%mm0, %%mm3 \n\t"	
	   "pmullw    %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm6, %%mm2 \n\t"	
	  "pmullw    %%mm7, %%mm3 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm3, %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "psrlq       $32, %%mm1 \n\t"	
	  "paddsw    %%mm2, %%mm1 \n\t"	
	  "movq      %%mm1, %%mm3 \n\t"	
	  "psrlq       $16, %%mm1 \n\t"	
	  "paddsw    %%mm3, %%mm1 \n\t"	

	  "movd      %%eax, %%mm2 \n\t"	
	  "movd      %%edx, %%mm3 \n\t"	
	  "movd      %%mm1, %%eax \n\t"	
	  "psraw       $15, %%mm1 \n\t"	
	  "movd      %%mm1, %%edx \n\t"	
	  "idivw             %%bx \n\t"	
	  "movd      %%eax, %%mm1 \n\t"	
	  "packuswb  %%mm0, %%mm1 \n\t"	
	  "movd      %%mm1, %%eax \n\t"	
	  "mov      %%al, (%%edi) \n\t"	
	  "movd      %%mm3, %%edx \n\t"	
	  "movd      %%mm2, %%eax \n\t"	

	  "sub       %%eax, %%esi \n\t"	
	  "sub       %%eax, %%esi \n\t"	
	  "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10322 \n\t"	
	   "add          $2, %%esi \n\t"	
	  "add          $2, %%edi \n\t"	
	   "dec              %%edx \n\t"	
	  "jnz            .L10320 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(Divisor)		
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterConvolveKernel5x5Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 5) || (rows < 5) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	  "movd      %%ebx, %%mm5 \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add          $2, %%edi \n\t"	
	  "mov          %3, %%eax \n\t"	
	  "shl          $1, %%eax \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "shr          $1, %%eax \n\t"	
	   "mov          %2, %%ebx \n\t"	
	  "sub          $4, %%ebx \n\t"	

	  ".L10330:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $4, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10332:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	
	  "movd      %%esi, %%mm6 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "paddsw    %%mm3, %%mm7 \n\t"	
	  "movq      %%mm7, %%mm2 \n\t"	
	  "psrlq       $16, %%mm7 \n\t"	
	  "paddsw    %%mm2, %%mm7 \n\t"	

	  "movd      %%eax, %%mm1 \n\t"	
	  "movd      %%ebx, %%mm2 \n\t"	
	  "movd      %%edx, %%mm3 \n\t"	
	   "movd      %%mm7, %%eax \n\t"	
	  "psraw       $15, %%mm7 \n\t"	
	  "movd      %%mm5, %%ebx \n\t"	
	  "movd      %%mm7, %%edx \n\t"	
	  "idivw             %%bx \n\t"	
	  "movd      %%eax, %%mm7 \n\t"	
	  "packuswb  %%mm0, %%mm7 \n\t"	
	   "movd      %%mm7, %%eax \n\t"	
	  "mov      %%al, (%%edi) \n\t"	
	   "movd      %%mm3, %%edx \n\t"	
	  "movd      %%mm2, %%ebx \n\t"	
	  "movd      %%mm1, %%eax \n\t"	

	  "movd      %%mm6, %%esi \n\t"	
	  "sub         $72, %%edx \n\t"	
	  "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10332 \n\t"	
	   "add          $4, %%esi \n\t"	
	  "add          $4, %%edi \n\t"	
	   "dec              %%ebx \n\t"	
	  "jnz            .L10330 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(Divisor)		
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterConvolveKernel7x7Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 7) || (rows < 7) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	  "movd      %%ebx, %%mm5 \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add          $3, %%edi \n\t"	
	  "mov          %3, %%eax \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	
	  "sub          $6, %%ebx \n\t"	

	  ".L10340:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $6, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10342:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	
	  "movd      %%esi, %%mm6 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "paddsw    %%mm3, %%mm7 \n\t"	
	  "movq      %%mm7, %%mm2 \n\t"	
	  "psrlq       $16, %%mm7 \n\t"	
	  "paddsw    %%mm2, %%mm7 \n\t"	

	  "movd      %%eax, %%mm1 \n\t"	
	  "movd      %%ebx, %%mm2 \n\t"	
	  "movd      %%edx, %%mm3 \n\t"	
	   "movd      %%mm7, %%eax \n\t"	
	  "psraw       $15, %%mm7 \n\t"	
	  "movd      %%mm5, %%ebx \n\t"	
	  "movd      %%mm7, %%edx \n\t"	
	  "idivw             %%bx \n\t"	
	  "movd      %%eax, %%mm7 \n\t"	
	  "packuswb  %%mm0, %%mm7 \n\t"	
	   "movd      %%mm7, %%eax \n\t"	
	  "mov      %%al, (%%edi) \n\t"	
	   "movd      %%mm3, %%edx \n\t"	
	  "movd      %%mm2, %%ebx \n\t"	
	  "movd      %%mm1, %%eax \n\t"	

	  "movd      %%mm6, %%esi \n\t"	
	  "sub        $104, %%edx \n\t"	
	  "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10342 \n\t"	
	   "add          $6, %%esi \n\t"	
	  "add          $6, %%edi \n\t"	
	   "dec              %%ebx \n\t"	
	  "jnz            .L10340 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(Divisor)		
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterConvolveKernel9x9Divide(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					   signed short *Kernel, unsigned char Divisor)
{
    if ((columns < 9) || (rows < 9) || (Divisor == 0))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	  "movd      %%ebx, %%mm5 \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add          $4, %%edi \n\t"	
	  "mov          %3, %%eax \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	
	  "sub          $8, %%ebx \n\t"	

	  ".L10350:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $8, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10352:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	
	  "movd      %%esi, %%mm6 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "paddsw    %%mm3, %%mm7 \n\t"	
	  "movq      %%mm7, %%mm2 \n\t"	
	  "psrlq       $16, %%mm7 \n\t"	
	  "paddsw    %%mm2, %%mm7 \n\t"	

	  "movd      %%eax, %%mm1 \n\t"	
	  "movd      %%ebx, %%mm2 \n\t"	
	  "movd      %%edx, %%mm3 \n\t"	
	   "movd      %%mm7, %%eax \n\t"	
	  "psraw       $15, %%mm7 \n\t"	
	  "movd      %%mm5, %%ebx \n\t"	
	  "movd      %%mm7, %%edx \n\t"	
	  "idivw             %%bx \n\t"	
	  "movd      %%eax, %%mm7 \n\t"	
	  "packuswb  %%mm0, %%mm7 \n\t"	
	   "movd      %%mm7, %%eax \n\t"	
	  "mov      %%al, (%%edi) \n\t"	
	   "movd      %%mm3, %%edx \n\t"	
	  "movd      %%mm2, %%ebx \n\t"	
	  "movd      %%mm1, %%eax \n\t"	

	  "movd      %%mm6, %%esi \n\t"	
	  "sub        $208, %%edx \n\t"	
	  "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10352 \n\t"	
	   "add          $8, %%esi \n\t"	
	  "add          $8, %%edi \n\t"	
	   "dec              %%ebx \n\t"	
	  "jnz            .L10350 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(Divisor)		
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterConvolveKernel3x3ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 3) || (rows < 3) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	  "movd      %%ebx, %%mm4 \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "movq    (%%edx), %%mm5 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm6 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm7 \n\t"	

	  "mov          %3, %%eax \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "inc              %%edi \n\t"	
	   "mov          %2, %%edx \n\t"	
	  "sub          $2, %%edx \n\t"	

	  ".L10360:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $2, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10362:               \n\t"

	  "movq    (%%esi), %%mm1 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%esi), %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%esi), %%mm3 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpcklbw %%mm0, %%mm3 \n\t"	
	   "psrlw     %%mm4, %%mm1 \n\t"	
	  "psrlw     %%mm4, %%mm2 \n\t"	
	  "psrlw     %%mm4, %%mm3 \n\t"	
	   "pmullw    %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm6, %%mm2 \n\t"	
	  "pmullw    %%mm7, %%mm3 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm3, %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "psrlq       $32, %%mm1 \n\t"	
	  "paddsw    %%mm2, %%mm1 \n\t"	
	  "movq      %%mm1, %%mm3 \n\t"	
	  "psrlq       $16, %%mm1 \n\t"	
	  "paddsw    %%mm3, %%mm1 \n\t"	
	  "packuswb  %%mm0, %%mm1 \n\t"	
	  "movd      %%mm1, %%ebx \n\t"	
	  "mov      %%bl, (%%edi) \n\t"	

	  "sub       %%eax, %%esi \n\t"	
	  "sub       %%eax, %%esi \n\t" "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10362 \n\t"	
	   "add          $2, %%esi \n\t"	
	  "add          $2, %%edi \n\t"	
	   "dec              %%edx \n\t"	
	  "jnz            .L10360 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(NRightShift)	
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterConvolveKernel5x5ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 5) || (rows < 5) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	  "movd      %%ebx, %%mm5 \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add          $2, %%edi \n\t"	
	  "mov          %3, %%eax \n\t"	
	  "shl          $1, %%eax \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "shr          $1, %%eax \n\t"	
	   "mov          %2, %%ebx \n\t"	
	  "sub          $4, %%ebx \n\t"	

	  ".L10370:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $4, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10372:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	
	  "movd      %%esi, %%mm6 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "paddsw    %%mm3, %%mm7 \n\t"	
	  "movq      %%mm7, %%mm2 \n\t"	
	  "psrlq       $16, %%mm7 \n\t"	
	  "paddsw    %%mm2, %%mm7 \n\t"	
	  "movd      %%eax, %%mm1 \n\t"	
	  "packuswb  %%mm0, %%mm7 \n\t"	
	  "movd      %%mm7, %%eax \n\t"	
	  "mov      %%al, (%%edi) \n\t"	
	  "movd      %%mm1, %%eax \n\t"	

	  "movd      %%mm6, %%esi \n\t"	
	  "sub         $72, %%edx \n\t"	
	  "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10372 \n\t"	
	   "add          $4, %%esi \n\t"	
	  "add          $4, %%edi \n\t"	
	   "dec              %%ebx \n\t"	
	  "jnz            .L10370 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(NRightShift)	
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterConvolveKernel7x7ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 7) || (rows < 7) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	  "movd      %%ebx, %%mm5 \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add          $3, %%edi \n\t"	
	  "mov          %3, %%eax \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	
	  "sub          $6, %%ebx \n\t"	

	  ".L10380:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $6, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10382:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	
	  "movd      %%esi, %%mm6 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "paddsw    %%mm3, %%mm7 \n\t"	
	  "movq      %%mm7, %%mm2 \n\t"	
	  "psrlq       $16, %%mm7 \n\t"	
	  "paddsw    %%mm2, %%mm7 \n\t"	
	  "movd      %%eax, %%mm1 \n\t"	
	  "packuswb  %%mm0, %%mm7 \n\t"	
	  "movd      %%mm7, %%eax \n\t"	
	  "mov      %%al, (%%edi) \n\t"	
	  "movd      %%mm1, %%eax \n\t"	

	  "movd      %%mm6, %%esi \n\t"	
	  "sub        $104, %%edx \n\t"	
	  "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10382 \n\t"	
	   "add          $6, %%esi \n\t"	
	  "add          $6, %%edi \n\t"	
	   "dec              %%ebx \n\t"	
	  "jnz            .L10380 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(NRightShift)	
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterConvolveKernel9x9ShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
					       signed short *Kernel, unsigned char NRightShift)
{
    if ((columns < 9) || (rows < 9) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %5, %%bl \n\t"	
	  "movd      %%ebx, %%mm5 \n\t"	
	   "mov          %4, %%edx \n\t"	
	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add          $4, %%edi \n\t"	
	  "mov          %3, %%eax \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "add       %%eax, %%edi \n\t" "mov          %2, %%ebx \n\t"	
	  "sub          $8, %%ebx \n\t"	

	  ".L10390:               \n\t" "mov       %%eax, %%ecx \n\t"	
	  "sub          $8, %%ecx \n\t"	
	   ".align 16              \n\t"	
	  ".L10392:               \n\t" "pxor      %%mm7, %%mm7 \n\t"	
	  "movd      %%esi, %%mm6 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "dec              %%esi \n\t" "add       %%eax, %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq    (%%esi), %%mm1 \n\t"	
	  "movq      %%mm1, %%mm2 \n\t"	
	  "inc              %%esi \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	  "add          $8, %%edx \n\t"	
	  "movq    (%%edx), %%mm4 \n\t"	
	  "add          $8, %%edx \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "punpckhbw %%mm0, %%mm2 \n\t"	
	   "psrlw     %%mm5, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm2 \n\t"	
	   "pmullw    %%mm3, %%mm1 \n\t"	
	  "pmullw    %%mm4, %%mm2 \n\t"	
	   "paddsw    %%mm2, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	
	   "movq    (%%esi), %%mm1 \n\t"	
	  "movq    (%%edx), %%mm3 \n\t"	
	   "punpcklbw %%mm0, %%mm1 \n\t"	
	  "psrlw     %%mm5, %%mm1 \n\t"	
	  "pmullw    %%mm3, %%mm1 \n\t"	
	  "paddsw    %%mm1, %%mm7 \n\t"	

	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "paddsw    %%mm3, %%mm7 \n\t"	
	  "movq      %%mm7, %%mm2 \n\t"	
	  "psrlq       $16, %%mm7 \n\t"	
	  "paddsw    %%mm2, %%mm7 \n\t"	
	  "movd      %%eax, %%mm1 \n\t"	
	  "packuswb  %%mm0, %%mm7 \n\t"	
	  "movd      %%mm7, %%eax \n\t"	
	  "mov      %%al, (%%edi) \n\t"	
	  "movd      %%mm1, %%eax \n\t"	

	  "movd      %%mm6, %%esi \n\t"	
	  "sub        $208, %%edx \n\t"	
	  "inc              %%esi \n\t"	
	  "inc              %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10392 \n\t"	
	   "add          $8, %%esi \n\t"	
	  "add          $8, %%edi \n\t"	
	   "dec              %%ebx \n\t"	
	  "jnz            .L10390 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(Kernel),		
	  "m"(NRightShift)	
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}




int SDL_imageFilterSobelX(unsigned char *Src, unsigned char *Dest, int rows, int columns)
{
    if ((columns < 8) || (rows < 3))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	  "mov          %3, %%eax \n\t"	

	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "inc              %%edi \n\t"	
	   "mov          %2, %%edx \n\t"	
	  "sub          $2, %%edx \n\t"	

	  ".L10400:                \n\t" "mov       %%eax, %%ecx \n\t"	
	  "shr          $3, %%ecx \n\t"	
	   "mov       %%esi, %%ebx \n\t"	
	  "movd      %%edi, %%mm1 \n\t"	
	   ".align 16              \n\t"	
	  ".L10402:               \n\t"

	  "movq    (%%esi), %%mm4 \n\t"	
	  "movq      %%mm4, %%mm5 \n\t"	
	  "add          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm4 \n\t"	
	  "punpckhbw %%mm0, %%mm5 \n\t"	
	   "movq    (%%esi), %%mm6 \n\t"	
	  "movq      %%mm6, %%mm7 \n\t"	
	  "sub          $2, %%esi \n\t"	
	   "punpcklbw %%mm0, %%mm6 \n\t"	
	  "punpckhbw %%mm0, %%mm7 \n\t"	
	   "add       %%eax, %%esi \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "add          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm4 \n\t"	
	  "paddw     %%mm3, %%mm5 \n\t"	
	  "paddw     %%mm2, %%mm4 \n\t"	
	  "paddw     %%mm3, %%mm5 \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "sub          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm6 \n\t"	
	  "paddw     %%mm3, %%mm7 \n\t"	
	  "paddw     %%mm2, %%mm6 \n\t"	
	  "paddw     %%mm3, %%mm7 \n\t"	
	   "add       %%eax, %%esi \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "add          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm4 \n\t"	
	  "paddw     %%mm3, %%mm5 \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "sub          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm6 \n\t"	
	  "paddw     %%mm3, %%mm7 \n\t"	

	  "movq      %%mm4, %%mm2 \n\t"	
	  "psrlq       $32, %%mm4 \n\t"	
	  "psubw     %%mm2, %%mm4 \n\t"	
	  "movq      %%mm6, %%mm3 \n\t"	
	  "psrlq       $32, %%mm6 \n\t"	
	  "psubw     %%mm3, %%mm6 \n\t"	
	  "punpckldq %%mm6, %%mm4 \n\t"	
	   "movq      %%mm5, %%mm2 \n\t"	
	  "psrlq       $32, %%mm5 \n\t"	
	  "psubw     %%mm2, %%mm5 \n\t"	
	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "psubw     %%mm3, %%mm7 \n\t"	
	  "punpckldq %%mm7, %%mm5 \n\t"	
	  
	  "movq      %%mm4, %%mm6 \n\t"	
	  "movq      %%mm5, %%mm7 \n\t"	
	  "psraw       $15, %%mm6 \n\t"	
	  "psraw       $15, %%mm7 \n\t"	
	  "pxor      %%mm6, %%mm4 \n\t"	
	  "pxor      %%mm7, %%mm5 \n\t"	
	  "psubsw    %%mm6, %%mm4 \n\t"	
	  "psubsw    %%mm7, %%mm5 \n\t"	
	   "packuswb  %%mm5, %%mm4 \n\t"	
	  "movq    %%mm4, (%%edi) \n\t"	

	  "sub       %%eax, %%esi \n\t"	
	  "sub       %%eax, %%esi \n\t" "add $8,          %%esi \n\t"	
	  "add $8,          %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10402 \n\t"	
	   "mov       %%ebx, %%esi \n\t"	
	  "movd      %%mm1, %%edi \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "add       %%eax, %%edi \n\t"	
	   "dec              %%edx \n\t"	
	  "jnz            .L10400 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns)		
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}


int SDL_imageFilterSobelXShiftRight(unsigned char *Src, unsigned char *Dest, int rows, int columns,
				    unsigned char NRightShift)
{
    if ((columns < 8) || (rows < 3) || (NRightShift > 7))
	return (-1);

    if ((SDL_imageFilterMMXdetect())) {
#ifdef USE_MMX
	asm volatile
	 ("pusha		     \n\t" "pxor      %%mm0, %%mm0 \n\t"	
	  "mov          %3, %%eax \n\t"	
	   "xor       %%ebx, %%ebx \n\t"	
	  "mov           %4, %%bl \n\t"	
	  "movd      %%ebx, %%mm1 \n\t"	

	  "mov          %1, %%esi \n\t"	
	  "mov          %0, %%edi \n\t"	
	  "add       %%eax, %%edi \n\t"	
	  "inc              %%edi \n\t"	
	  
	  "subl            $2, %2 \n\t"	

	  ".L10410:                \n\t" "mov       %%eax, %%ecx \n\t"	
	  "shr          $3, %%ecx \n\t"	
	   "mov       %%esi, %%ebx \n\t"	
	  "mov       %%edi, %%edx \n\t"	
	   ".align 16              \n\t"	
	  ".L10412:               \n\t"

	  "movq    (%%esi), %%mm4 \n\t"	
	  "movq      %%mm4, %%mm5 \n\t"	
	  "add          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm4 \n\t"	
	  "punpckhbw %%mm0, %%mm5 \n\t"	
	   "psrlw     %%mm1, %%mm4 \n\t"	
	  "psrlw     %%mm1, %%mm5 \n\t"	
	   "movq    (%%esi), %%mm6 \n\t"	
	  "movq      %%mm6, %%mm7 \n\t"	
	  "sub          $2, %%esi \n\t"	
	   "punpcklbw %%mm0, %%mm6 \n\t"	
	  "punpckhbw %%mm0, %%mm7 \n\t"	
	   "psrlw     %%mm1, %%mm6 \n\t"	
	  "psrlw     %%mm1, %%mm7 \n\t"	
	   "add       %%eax, %%esi \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "add          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "psrlw     %%mm1, %%mm2 \n\t"	
	  "psrlw     %%mm1, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm4 \n\t"	
	  "paddw     %%mm3, %%mm5 \n\t"	
	  "paddw     %%mm2, %%mm4 \n\t"	
	  "paddw     %%mm3, %%mm5 \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "sub          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "psrlw     %%mm1, %%mm2 \n\t"	
	  "psrlw     %%mm1, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm6 \n\t"	
	  "paddw     %%mm3, %%mm7 \n\t"	
	  "paddw     %%mm2, %%mm6 \n\t"	
	  "paddw     %%mm3, %%mm7 \n\t"	
	   "add       %%eax, %%esi \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "add          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "psrlw     %%mm1, %%mm2 \n\t"	
	  "psrlw     %%mm1, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm4 \n\t"	
	  "paddw     %%mm3, %%mm5 \n\t"	
	   "movq    (%%esi), %%mm2 \n\t"	
	  "movq      %%mm2, %%mm3 \n\t"	
	  "sub          $2, %%esi \n\t"	
	  "punpcklbw %%mm0, %%mm2 \n\t"	
	  "punpckhbw %%mm0, %%mm3 \n\t"	
	   "psrlw     %%mm1, %%mm2 \n\t"	
	  "psrlw     %%mm1, %%mm3 \n\t"	
	   "paddw     %%mm2, %%mm6 \n\t"	
	  "paddw     %%mm3, %%mm7 \n\t"	

	  "movq      %%mm4, %%mm2 \n\t"	
	  "psrlq       $32, %%mm4 \n\t"	
	  "psubw     %%mm2, %%mm4 \n\t"	
	  "movq      %%mm6, %%mm3 \n\t"	
	  "psrlq       $32, %%mm6 \n\t"	
	  "psubw     %%mm3, %%mm6 \n\t"	
	  "punpckldq %%mm6, %%mm4 \n\t"	
	   "movq      %%mm5, %%mm2 \n\t"	
	  "psrlq       $32, %%mm5 \n\t"	
	  "psubw     %%mm2, %%mm5 \n\t"	
	  "movq      %%mm7, %%mm3 \n\t"	
	  "psrlq       $32, %%mm7 \n\t"	
	  "psubw     %%mm3, %%mm7 \n\t"	
	  "punpckldq %%mm7, %%mm5 \n\t"	
	  
	  "movq      %%mm4, %%mm6 \n\t"	
	  "movq      %%mm5, %%mm7 \n\t"	
	  "psraw       $15, %%mm6 \n\t"	
	  "psraw       $15, %%mm7 \n\t"	
	  "pxor      %%mm6, %%mm4 \n\t"	
	  "pxor      %%mm7, %%mm5 \n\t"	
	  "psubsw    %%mm6, %%mm4 \n\t"	
	  "psubsw    %%mm7, %%mm5 \n\t"	
	   "packuswb  %%mm5, %%mm4 \n\t"	
	  "movq    %%mm4, (%%edi) \n\t"	

	  "sub       %%eax, %%esi \n\t"	
	  "sub       %%eax, %%esi \n\t" "add $8,          %%esi \n\t"	
	  "add $8,          %%edi \n\t"	

	  "dec              %%ecx \n\t"	
	  "jnz            .L10412 \n\t"	
	   "mov       %%ebx, %%esi \n\t"	
	  "mov       %%edx, %%edi \n\t"	
	  "add       %%eax, %%esi \n\t"	
	  "add       %%eax, %%edi \n\t"	
	   "decl                %2 \n\t"	
	  "jnz            .L10410 \n\t"	

	  "emms                   \n\t"	
	  "popa                   \n\t":"=m" (Dest)	
	  :"m"(Src),		
	  "m"(rows),		
	  "m"(columns),		
	  "m"(NRightShift)	
	    );
#endif
	return (0);
    } else {
	/* No non-MMX implementation yet */
	return (-1);
    }
}

/* Align stack to 32 byte boundary */
void SDL_imageFilterAlignStack(void)
{
#ifdef USE_MMX
    asm volatile
     (				
	 "mov       %%esp, %%ebx \n\t"	
	 "sub          $4, %%ebx \n\t"	
	 "and        $-32, %%ebx \n\t"	
	 "mov     %%esp, (%%ebx) \n\t"	
	 "mov       %%ebx, %%esp \n\t"	
	 ::);
#endif
}

/* Restore previously aligned stack */
void SDL_imageFilterRestoreStack(void)
{
#ifdef USE_MMX
    asm volatile
     (				
	 "mov     (%%esp), %%ebx \n\t"	
	 "mov       %%ebx, %%esp \n\t"	
	 ::);
#endif
}
