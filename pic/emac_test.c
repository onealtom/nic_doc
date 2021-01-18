/**
*    EMAC - Basic Loopback
*
*    @date         7/21/2004
*    @author        Magdalena Iovescu
*
**/

#include "emac.h"
#include <stdio.h>
#include <cslr_emac.h>
#include <cslr_mdio.h>
#include <csl_intc.h>

#define TX_BUF    128
#define RX_BUF    1536

Uint8 packet_data[TX_BUF];
Uint8 packet_buffer1[RX_BUF];
Uint8 packet_buffer2[RX_BUF];
Uint8 packet_buffer3[RX_BUF];
Uint8 packet_buffer4[RX_BUF];
Uint8 packet_buffer5[RX_BUF];
Uint8 packet_buffer6[RX_BUF];
Uint8 packet_buffer7[RX_BUF];
Uint8 packet_buffer8[RX_BUF];
Uint8 packet_buffer9[RX_BUF];
Uint8 packet_buffer10[RX_BUF];

/*
 *  We use pDescBase for a base address. Its easier this way 
 *  because we can use indexing off the pointer.
 */
EMAC_Desc* pDescBase = ( EMAC_Desc* )_EMAC_DSC_BASE_ADDR;

/*
 *  The following are used to allow the ISR to communicate with
 *  the application.
 */
extern volatile int RxCount;
extern volatile int TxCount;
extern volatile int ErrCount;
extern volatile EMAC_Desc *pDescRx;
extern volatile EMAC_Desc *pDescTx;




Int16 verify_packet( EMAC_Desc* pDesc, Uint32 size, Uint32 flagCRC );

/* ------------------------------------------------------------------------ *
 *                                                                          *
 *  emac_test                                                               *
 *                                                                          *
 * ------------------------------------------------------------------------ */
emac_test( )
{
    Uint32 i;
    Uint32 testresult = 0;
    EMAC_Desc* pDesc;

    /* Init packet data array */
    for ( i = 0 ; i < TX_BUF ; i++ )
        packet_data[i] = i;

    /* Initialize our counts */
    TxCount = 0;
    RxCount = 0;
    ErrCount = 0;

    emac_init( );                           // Initialize EMAC

    testresult = EMAC_initIntc( );          // Init EMAC & ARM interrupts

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Setup RX packets                                                *
     *                                                                  *
     * ---------------------------------------------------------------- */
    pDesc            = pDescBase;
    pDesc->pNext     = pDescBase + 1;
    pDesc->pBuffer   = packet_buffer1;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 1;
    pDesc->pNext     = pDescBase + 2;
    pDesc->pBuffer   = packet_buffer2;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 2;
    pDesc->pNext     = pDescBase + 3;
    pDesc->pBuffer   = packet_buffer3;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 3;
    pDesc->pNext     = pDescBase + 4;
    pDesc->pBuffer   = packet_buffer4;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 4;
    pDesc->pNext     = pDescBase + 5;
    pDesc->pBuffer   = packet_buffer5;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 5;
    pDesc->pNext     = pDescBase + 6;
    pDesc->pBuffer   = packet_buffer6;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 6;
    pDesc->pNext     = pDescBase + 7;
    pDesc->pBuffer   = packet_buffer7;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 7;
    pDesc->pNext     = pDescBase + 8;
    pDesc->pBuffer   = packet_buffer8;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 8;
    pDesc->pNext     = pDescBase + 9;
    pDesc->pBuffer   = packet_buffer9;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDesc            = pDescBase + 9;
    pDesc->pNext     = 0;
    pDesc->pBuffer   = packet_buffer10;
    pDesc->BufOffLen = RX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    pDescRx = pDescBase;
    EMAC_REGS->RX0HDP = ( Uint32 )pDescRx;          // Start RX

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Setup TX packets                                                *
     *                                                                  *
     *  Send two copies of the same packet ( with different sizes )     *
     *                                                                  *
     * ---------------------------------------------------------------- */
    pDesc            = pDescBase + 10;
    pDesc->pNext     = pDescBase + 11;
    pDesc->pBuffer   = packet_data;
    pDesc->BufOffLen = 60;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER
                     | EMAC_DSC_FLAG_SOP
                     | EMAC_DSC_FLAG_EOP
                     | 60
                     ;

    pDesc            = pDescBase + 11;
    pDesc->pNext     = 0;
    pDesc->pBuffer   = packet_data;
    pDesc->BufOffLen = TX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER
                     | EMAC_DSC_FLAG_SOP
                     | EMAC_DSC_FLAG_EOP
                     | TX_BUF
                     ;

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Send TX packets                                                 *
     *                                                                  *
     * ---------------------------------------------------------------- */
    pDescTx           = pDescBase + 10;
    EMAC_REGS->TX0HDP = ( Uint32 )pDescTx;          // Start TX on Channel 0

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Wait for TX to send and RX to recv                              *
     *                                                                  *
     * ---------------------------------------------------------------- */

    EMAC_isr( );                                    // Just issue an ISR check

    while ( TxCount != 2 || RxCount != 2 )          // Wait for first two packets
    {
        if ( ErrCount )
            testresult++;
    }

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Check packet and results                                        *
     *                                                                  *
     * ---------------------------------------------------------------- */
    if ( pDescRx != pDescBase + 2 )                 // Verify Results
        testresult++;

    if ( verify_packet( pDescBase, 60, 0 ) )        // Verify Size + Contents
        testresult++;
    
    if ( verify_packet( pDescBase + 1, TX_BUF, 0 ) )// Verify Size + Contents
        testresult++;

    if ( ( i = EMAC_REGS->FRAME64 ) != 2 )          // No RX & TX frames w/ exactly 64 bytes
        testresult++;
    EMAC_REGS->FRAME64 = i;

    if ( ( i = EMAC_REGS->FRAME128T255 ) != 2 )
        testresult++;
    EMAC_REGS->FRAME128T255 = i;

    if ( ( i = EMAC_REGS->RXGOODFRAMES ) != 2 )
        testresult++;
    EMAC_REGS->RXGOODFRAMES = i;

    if ( ( i = EMAC_REGS->TXGOODFRAMES ) != 2 )
        testresult++;
    EMAC_REGS->TXGOODFRAMES = i;

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Setup TX descriptors for two more packet sends                  *
     *                                                                  *
     *  These will be used for the RXPASSCRC test passcrc flag is       *
     *  cleared => EMAC generates and appends 4-byte CRC, so resulting  *
     *  pkt size is 64                                                  *
     *                                                                  *
     * ---------------------------------------------------------------- */
    pDesc            = pDescBase + 12;
    pDesc->pNext     = pDescBase + 13;
    pDesc->pBuffer   = packet_data;
    pDesc->BufOffLen = 60;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER
                     | EMAC_DSC_FLAG_SOP
                     | EMAC_DSC_FLAG_EOP
                     | 60
                     ;

    pDesc            = pDescBase + 13;
    pDesc->pNext     = 0;
    pDesc->pBuffer   = packet_data;
    pDesc->BufOffLen = TX_BUF;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER
                     | EMAC_DSC_FLAG_SOP
                     | EMAC_DSC_FLAG_EOP
                     | TX_BUF
                     ;

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Send TX packets                                                 *
     *                                                                  *
     * ---------------------------------------------------------------- */
    /*
     *  Setup to pass CRC to memory 
     *  Rcv CRC is included in buffer descr pkt length
     */
    CSL_FINST( EMAC_REGS->RXMBPENABLE, EMAC_RXMBPENABLE_RXPASSCRC, INCLUDE );

    pDescTx           = pDescBase + 12;
    EMAC_REGS->TX0HDP = ( Uint32 )pDescTx;          // Start Transmitter on Channel 0

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Wait for TX to send and RX to recv                              *
     *                                                                  *
     * ---------------------------------------------------------------- */

    EMAC_isr( );                                    // Just issue an ISR check

    while ( TxCount != 4 || RxCount != 4 )          // Wait for next two packets
    {
        if ( ErrCount )
            testresult++;
    }

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Check packet and results                                        *
     *                                                                  *
     * ---------------------------------------------------------------- */
    if ( pDescRx != pDescBase + 4 )                 // Verify Results
        testresult++;

    if ( verify_packet( pDescBase + 2, 60, 1 ) )    // Verify Size + Contents
        testresult++;
    
    if ( verify_packet( pDescBase + 3, TX_BUF, 1 ) )// Verify Size + Contents
        testresult++;

    if ( ( i = EMAC_REGS->FRAME64 ) != 2 )
        testresult++;
    EMAC_REGS->FRAME64 = i;

    if ( ( i = EMAC_REGS->FRAME128T255 ) != 2 )
        testresult++;
    EMAC_REGS->FRAME128T255 = i;

    if ( ( i = EMAC_REGS->RXGOODFRAMES ) != 2 )
        testresult++;
    EMAC_REGS->RXGOODFRAMES = i;

    if ( ( i = EMAC_REGS->TXGOODFRAMES ) != 2 )
        testresult++;
    EMAC_REGS->TXGOODFRAMES = i;

    /* ---------------------------------------------------------------- *
     *                                                                  *
     *  Close EMAC                                                      *
     *                                                                  *
     * ---------------------------------------------------------------- */
    EMAC_closeIntc( );

    return testresult;
}

/* ------------------------------------------------------------------------ *
 *                                                                          *
 *  verify_packet                                                           *
 *                                                                          *
 * ------------------------------------------------------------------------ */
Int16 verify_packet( EMAC_Desc* pDesc, Uint32 size, Uint32 flagCRC )
{
    int i;
    Uint32 SizeCRC      = ( flagCRC ) ? size + 4 : size;
    Uint32 packet_flags = pDesc->PktFlgLen;

    /* We must own this packet */
    if ( packet_flags & EMAC_DSC_FLAG_OWNER )
        return 1;

    /* Must be SOP and EOP */
    if ( ( packet_flags & ( EMAC_DSC_FLAG_SOP | EMAC_DSC_FLAG_EOP ) ) 
                       != ( EMAC_DSC_FLAG_SOP | EMAC_DSC_FLAG_EOP ) )
        return 2;


    /* If flagCRC is set, it must have a CRC */
    if ( flagCRC )
        if ( ( packet_flags & EMAC_DSC_FLAG_PASSCRC ) != EMAC_DSC_FLAG_PASSCRC )
            return 3;


    /* Packet must not have any error bits set */
    if ( packet_flags & ( EMAC_DSC_FLAG_JABBER
                        | EMAC_DSC_FLAG_OVERSIZE
                        | EMAC_DSC_FLAG_FRAGMENT
                        | EMAC_DSC_FLAG_UNDERSIZED
                        | EMAC_DSC_FLAG_CONTROL
                        | EMAC_DSC_FLAG_OVERRUN
                        | EMAC_DSC_FLAG_CODEERROR
                        | EMAC_DSC_FLAG_ALIGNERROR
                        | EMAC_DSC_FLAG_CRCERROR
                        | EMAC_DSC_FLAG_NOMATCH
                        | EMAC_DSC_FLAG_EOQ
                        | EMAC_DSC_FLAG_TDOWNCMPLT ) )
        return 4;

    /* Packet must be correct size */
    if ( ( packet_flags & 0xFFFF ) != SizeCRC )
        return 5;

    /* Offset must be zero, packet size unchanged */
    if ( pDesc->BufOffLen != SizeCRC )
        return 6;

    /* Validate the data */
    for ( i = 0 ; i < size ; i++ )
        if ( pDesc->pBuffer[i] != i )
            return 7;

    return 0;
}
