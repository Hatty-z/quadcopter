/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_RTT_ASM_ARMv7M.S
Purpose : Assembler implementation of RTT functions for ARMv7M

Additional information:
  This module is written to be assembler-independent and works with
  GCC and clang (Embedded Studio) and IAR.
*/

 
#define SEGGER_RTT_ASM      // Used to control processed input from header file
#include "SEGGER_RTT.h"

//GNU汇编
.syntax unified
.cpu cortex-m4
.thumb

// Check if we should compile this file
#if defined (RTT_USE_ASM) && (RTT_USE_ASM == 1)

// External symbols
.extern _SEGGER_RTT

// Public symbols
.global SEGGER_RTT_ASM_WriteSkipNoLock
.section .text
.align 2
.thumb_func
.type SEGGER_RTT_ASM_WriteSkipNoLock, %function

SEGGER_RTT_ASM_WriteSkipNoLock:   // unsigned SEGGER_RTT_WriteSkipNoLock(unsigned BufferIndex, const void* pData, unsigned NumBytes) {
        //
        // Cases:
        //   1) RdOff <= WrOff => Space until wrap-around is sufficient
        //   2) RdOff <= WrOff => Space after wrap-around needed (copy in 2 chunks)
        //   3) RdOff <  WrOff => No space in buf
        //   4) RdOff >  WrOff => Space is sufficient
        //   5) RdOff >  WrOff => No space in buf
        //
        // 1) is the most common case for large buffers and assuming that J-Link reads the data fast enough
        //
        // Register usage:
        //   R0 Temporary needed as RdOff, <Tmp> register later on
        //   R1 pData
        //   R2 <NumBytes>
        //   R3 <Tmp> register. Hold free for subroutine calls
        //   R4 <Rem>
        //   R5 pRing->pBuffer
        //   R6 pRing (Points to active struct SEGGER_RTT_BUFFER_DOWN)
        //   R7 WrOff
        //
        push     {r4-r7}
        add      r3, r0, r0, lsl #1
        ldr      r0, =_SEGGER_RTT                 // pRing = &_SEGGER_RTT.aUp[BufferIndex];
        add      r0, r0, r3, lsl #3
        add      r6, r0, #24
        ldr      r0, [r6, #16]                    // RdOff = pRing->RdOff;
        ldr      r7, [r6, #12]                    // WrOff = pRing->WrOff;
        ldr      r5, [r6, #4]                     // pRing->pBuffer
        cmp      r7, r0
        blo      _CheckCase4                      // if (RdOff <= WrOff) {                           => Case 1), 2) or 3)
        //
        // Handling for case 1, later on identical to case 4
        //
        ldr      r3, [r6, #8]                     //  Avail = pRing->SizeOfBuffer - WrOff - 1u;      => Space until wrap-around (assume 1 byte not usable for case that RdOff == 0)
        subs     r4, r3, r7                       // <Rem> (Used in case we jump into case 2 afterwards)
        subs     r3, r4, #1                       // <Avail>
        cmp      r3, r2
        blo      _CheckCase2                      // if (Avail >= NumBytes) {  => Case 1)?
_Case4:
        add      r5, r7, r5                       // pBuffer += WrOff
        add      r0, r2, r7                       // v = WrOff + NumBytes
        //
        // 2x unrolling for the copy loop that is used most of the time
        // This is a special optimization for small SystemView packets and makes them even faster
        //
        .align 2
_LoopCopyStraight:                                // memcpy(pRing->pBuffer + WrOff, pData, NumBytes);
        ldrb     r3, [r1], #1
        strb     r3, [r5], #1                     // *pDest++ = *pSrc++
        subs     r2, r2, #1
        beq      _CSDone
        ldrb     r3, [r1], #1
        strb     r3, [r5], #1                     // *pDest++ = *pSrc++
        subs     r2, r2, #1
        bne      _LoopCopyStraight
_CSDone:
#ifdef _CORE_NEEDS_DMB                            // Do not slow down cores that do not need a DMB instruction here
        dmb                                       // Cortex-M7 may delay memory writes and also change the order in which the writes happen. Therefore, make sure that all buffer writes are finished, before updating the <WrOff> in the struct
#endif
        str      r0, [r6, #12]                    // pRing->WrOff = WrOff + NumBytes;
        movs     r0, #1
        pop      {r4-r7}
        bx       lr                               // Return 1
_CheckCase2:
        add      r0, r0, r3                       // Avail += RdOff; => Space incl. wrap-around
        cmp      r0, r2
        blo      _Case3                           // if (Avail >= NumBytes) {           => Case 2? => If not, we have case 3) (does not fit)
        //
        // Handling for case 2
        //
        add      r0, r7, r5                       // v = pRing->pBuffer + WrOff => Do not change pRing->pBuffer here because 2nd chunk needs org. value
        subs     r2, r2, r4                       // NumBytes -= Rem;  (Rem = pRing->SizeOfBuffer - WrOff; => Space until end of buffer)
_LoopCopyBeforeWrapAround:                        // memcpy(pRing->pBuffer + WrOff, pData, Rem); => Copy 1st chunk
        ldrb     r3, [r1], #1
        strb     r3, [r0], #1                     // *pDest++ = *pSrc++
        subs     r4, r4, #1
        bne      _LoopCopyBeforeWrapAround
        //
        // Special case: First check that assumed RdOff == 0 calculated that last element before wrap-around could not be used
        // But 2nd check (considering space until wrap-around and until RdOff) revealed that RdOff is not 0, so we can use the last element
        // In this case, we may use a copy straight until buffer end anyway without needing to copy 2 chunks
        // Therefore, check if 2nd memcpy is necessary at all
        //
        adds     r4, r2, #0                       // Save <NumBytes> (needed as counter in loop but must be written to <WrOff> after the loop). Also use this inst to update the flags to skip 2nd loop if possible
        beq      _No2ChunkNeeded                  // if (NumBytes) {
_LoopCopyAfterWrapAround:                         // memcpy(pRing->pBuffer, pData + Rem, NumBytes);
        ldrb     r3, [r1], #1                     // pData already points to the next src byte due to copy loop increment before this loop
        strb     r3, [r5], #1                     // *pDest++ = *pSrc++
        subs     r2, r2, #1
        bne      _LoopCopyAfterWrapAround
_No2ChunkNeeded:
#ifdef _CORE_NEEDS_DMB                            // Do not slow down cores that do not need a DMB instruction here
        dmb                                       // Cortex-M7 may delay memory writes and also change the order in which the writes happen. Therefore, make sure that all buffer writes are finished, before updating the <WrOff> in the struct
#endif
        str      r4, [r6, #12]                    // pRing->WrOff = NumBytes; => Must be written after copying data because J-Link may read control block asynchronously while writing into buffer
        movs     r0, #1
        pop      {r4-r7}
        bx       lr                               // Return 1
_CheckCase4:
        subs     r0, r0, r7
        subs     r0, r0, #1                       // Avail = RdOff - WrOff - 1u;
        cmp      r0, r2
        bhs      _Case4                           // if (Avail >= NumBytes) {      => Case 4) == 1) ? => If not, we have case 5) == 3) (does not fit)
_Case3:
        movs     r0, #0
        pop      {r4-r7}
        bx       lr                               // Return 0

.size SEGGER_RTT_ASM_WriteSkipNoLock, .-SEGGER_RTT_ASM_WriteSkipNoLock

#endif  // defined (RTT_USE_ASM) && (RTT_USE_ASM == 1)

.end


/*
/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
/* 
#define _CCIAR   0
#define _CCCLANG 1

#if (defined __SES_ARM) || (defined __GNUC__) || (defined __clang__)
  #define _CC_TYPE             _CCCLANG
  #define _PUB_SYM             .global
  #define _EXT_SYM             .extern
  #define _END                 .end
  #define _WEAK                .weak
  #define _THUMB_FUNC          .thumb_func
  #define _THUMB_CODE          .code 16
  #define _WORD                .word
  #define _SECTION(Sect, Type, AlignExp) .section Sect ##, "ax"
  #define _ALIGN(Exp)          .align Exp 
  #define _PLACE_LITS          .ltorg
  #define _DATA_SECT_START 
  #define _C_STARTUP           _start
  #define _STACK_END           __stack_end__
  #define _RAMFUNC
  //
  // .text     => Link to flash
  // .fast     => Link to RAM
  // OtherSect => Usually link to RAM
  // Alignment is 2^x
  //
#elif defined (__IASMARM__)
  #define _CC_TYPE             _CCIAR
  #define _PUB_SYM             PUBLIC
  #define _EXT_SYM             EXTERN
  #define _END                 END
  #define _WEAK                _WEAK
  #define _THUMB_FUNC
  #define _THUMB_CODE          THUMB
  #define _WORD                DCD
  #define _SECTION(Sect, Type, AlignExp) SECTION Sect ## : ## Type ## :REORDER:NOROOT ## (AlignExp)
  #define _ALIGN(Exp)          alignrom Exp 
  #define _PLACE_LITS
  #define _DATA_SECT_START     DATA
  #define _C_STARTUP           __iar_program_start
  #define _STACK_END           sfe(CSTACK)
  #define _RAMFUNC             SECTION_TYPE SHT_PROGBITS, SHF_WRITE | SHF_EXECINSTR
  //
  // .text     => Link to flash
  // .textrw   => Link to RAM
  // OtherSect => Usually link to RAM
  // NOROOT    => Allows linker to throw away the function, if not referenced
  // Alignment is 2^x
  //
#endif

#if (_CC_TYPE == _CCIAR)
        NAME SEGGER_RTT_ASM_ARMv7M
#else
        .syntax unified
#endif

#if defined (RTT_USE_ASM) && (RTT_USE_ASM == 1)
        #define SHT_PROGBITS 0x1

/*********************************************************************
*
*       Public / external symbols
*
**********************************************************************
*/

        _EXT_SYM __aeabi_memcpy
        _EXT_SYM __aeabi_memcpy4
        _EXT_SYM _SEGGER_RTT

        _PUB_SYM SEGGER_RTT_ASM_WriteSkipNoLock

/*********************************************************************
*
*       SEGGER_RTT_WriteSkipNoLock
*
*  Function description
*    Stores a specified number of characters in SEGGER RTT
*    control block which is then read by the host.
*    SEGGER_RTT_WriteSkipNoLock does not lock the application and
*    skips all data, if the data does not fit into the buffer.
*
*  Parameters
*    BufferIndex  Index of "Up"-buffer to be used (e.g. 0 for "Terminal").
*    pBuffer      Pointer to character array. Does not need to point to a \0 terminated string.
*    NumBytes     Number of bytes to be stored in the SEGGER RTT control block.
*                 MUST be > 0!!!
*                 This is done for performance reasons, so no initial check has do be done.
*
*  Return value
*    1: Data has been copied
*    0: No space, data has not been copied
*
*  Notes
*    (1) If there is not enough space in the "Up"-buffer, all data is dropped.
*    (2) For performance reasons this function does not call Init()
*        and may only be called after RTT has been initialized.
*        Either by calling SEGGER_RTT_Init() or calling another RTT API function first.
*/
/* 
        _SECTION(.text, CODE, 2)
        _ALIGN(2)
        _THUMB_FUNC
SEGGER_RTT_ASM_WriteSkipNoLock:   // unsigned SEGGER_RTT_WriteSkipNoLock(unsigned BufferIndex, const void* pData, unsigned NumBytes) {
        //
        // Cases:
        //   1) RdOff <= WrOff => Space until wrap-around is sufficient
        //   2) RdOff <= WrOff => Space after wrap-around needed (copy in 2 chunks)
        //   3) RdOff <  WrOff => No space in buf
        //   4) RdOff >  WrOff => Space is sufficient
        //   5) RdOff >  WrOff => No space in buf
        //
        // 1) is the most common case for large buffers and assuming that J-Link reads the data fast enough
        //
        // Register usage:
        //   R0 Temporary needed as RdOff, <Tmp> register later on
        //   R1 pData
        //   R2 <NumBytes>
        //   R3 <Tmp> register. Hold free for subroutine calls
        //   R4 <Rem>
        //   R5 pRing->pBuffer
        //   R6 pRing (Points to active struct SEGGER_RTT_BUFFER_DOWN)
        //   R7 WrOff
        //
        PUSH     {R4-R7}
        ADD      R3,R0,R0, LSL #+1
        LDR.W    R0,=_SEGGER_RTT                 // pRing = &_SEGGER_RTT.aUp[BufferIndex];
        ADD      R0,R0,R3, LSL #+3
        ADD      R6,R0,#+24
        LDR      R0,[R6, #+16]                   // RdOff = pRing->RdOff;
        LDR      R7,[R6, #+12]                   // WrOff = pRing->WrOff;
        LDR      R5,[R6, #+4]                    // pRing->pBuffer
        CMP      R7,R0
        BCC.N    _CheckCase4                     // if (RdOff <= WrOff) {                           => Case 1), 2) or 3)
        //
        // Handling for case 1, later on identical to case 4
        //
        LDR      R3,[R6, #+8]                    //  Avail = pRing->SizeOfBuffer - WrOff - 1u;      => Space until wrap-around (assume 1 byte not usable for case that RdOff == 0)
        SUBS     R4,R3,R7                        // <Rem> (Used in case we jump into case 2 afterwards)
        SUBS     R3,R4,#+1                       // <Avail>
        CMP      R3,R2
        BCC.N    _CheckCase2                     // if (Avail >= NumBytes) {  => Case 1)?
_Case4:
        ADDS     R5,R7,R5                        // pBuffer += WrOff
        ADDS     R0,R2,R7                        // v = WrOff + NumBytes
        //
        // 2x unrolling for the copy loop that is used most of the time
        // This is a special optimization for small SystemView packets and makes them even faster
        //
        _ALIGN(2)
_LoopCopyStraight:                               // memcpy(pRing->pBuffer + WrOff, pData, NumBytes);
        LDRB     R3,[R1], #+1
        STRB     R3,[R5], #+1                    // *pDest++ = *pSrc++
        SUBS     R2,R2,#+1
        BEQ      _CSDone
        LDRB     R3,[R1], #+1
        STRB     R3,[R5], #+1                    // *pDest++ = *pSrc++
        SUBS     R2,R2,#+1
        BNE      _LoopCopyStraight
_CSDone:
#if _CORE_NEEDS_DMB                              // Do not slow down cores that do not need a DMB instruction here
        DMB                                      // Cortex-M7 may delay memory writes and also change the order in which the writes happen. Therefore, make sure that all buffer writes are finished, before updating the <WrOff> in the struct
#endif
        STR      R0,[R6, #+12]                   // pRing->WrOff = WrOff + NumBytes;
        MOVS     R0,#+1
        POP      {R4-R7}
        BX       LR                              // Return 1
_CheckCase2:
        ADDS     R0,R0,R3                        // Avail += RdOff; => Space incl. wrap-around
        CMP      R0,R2
        BCC.N    _Case3                          // if (Avail >= NumBytes) {           => Case 2? => If not, we have case 3) (does not fit)
        //
        // Handling for case 2
        //
        ADDS     R0,R7,R5                        // v = pRing->pBuffer + WrOff => Do not change pRing->pBuffer here because 2nd chunk needs org. value
        SUBS     R2,R2,R4                        // NumBytes -= Rem;  (Rem = pRing->SizeOfBuffer - WrOff; => Space until end of buffer)
_LoopCopyBeforeWrapAround:                       // memcpy(pRing->pBuffer + WrOff, pData, Rem); => Copy 1st chunk
        LDRB     R3,[R1], #+1
        STRB     R3,[R0], #+1                    // *pDest++ = *pSrc++
        SUBS     R4,R4,#+1
        BNE      _LoopCopyBeforeWrapAround
        //
        // Special case: First check that assumed RdOff == 0 calculated that last element before wrap-around could not be used
        // But 2nd check (considering space until wrap-around and until RdOff) revealed that RdOff is not 0, so we can use the last element
        // In this case, we may use a copy straight until buffer end anyway without needing to copy 2 chunks
        // Therefore, check if 2nd memcpy is necessary at all
        //
        ADDS     R4,R2,#+0                       // Save <NumBytes> (needed as counter in loop but must be written to <WrOff> after the loop). Also use this inst to update the flags to skip 2nd loop if possible
        BEQ.N    _No2ChunkNeeded                 // if (NumBytes) {
_LoopCopyAfterWrapAround:                        // memcpy(pRing->pBuffer, pData + Rem, NumBytes);
        LDRB     R3,[R1], #+1                    // pData already points to the next src byte due to copy loop increment before this loop
        STRB     R3,[R5], #+1                    // *pDest++ = *pSrc++
        SUBS     R2,R2,#+1
        BNE      _LoopCopyAfterWrapAround
_No2ChunkNeeded:
#if _CORE_NEEDS_DMB                              // Do not slow down cores that do not need a DMB instruction here
        DMB                                      // Cortex-M7 may delay memory writes and also change the order in which the writes happen. Therefore, make sure that all buffer writes are finished, before updating the <WrOff> in the struct
#endif
        STR      R4,[R6, #+12]                   // pRing->WrOff = NumBytes; => Must be written after copying data because J-Link may read control block asynchronously while writing into buffer
        MOVS     R0,#+1
        POP      {R4-R7}
        BX       LR                              // Return 1
_CheckCase4:
        SUBS     R0,R0,R7
        SUBS     R0,R0,#+1                       // Avail = RdOff - WrOff - 1u;
        CMP      R0,R2
        BCS.N    _Case4                          // if (Avail >= NumBytes) {      => Case 4) == 1) ? => If not, we have case 5) == 3) (does not fit)
_Case3:
        MOVS     R0,#+0
        POP      {R4-R7}
        BX       LR                              // Return 0
        _PLACE_LITS

#endif  // defined (RTT_USE_ASM) && (RTT_USE_ASM == 1)
        _END

/*************************** End of file ****************************/
