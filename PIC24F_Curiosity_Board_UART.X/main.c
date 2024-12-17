/* 
 * File:   main.c
 * Author: Richard Woodham
 *
 * Created on 29 January 2024, 14:10
 * Cut-down project to debug UART operation
 * Configuration bits and UART config done in main
 * Fast RC oscillator (no PLL)
 * 9600 baud - numbers lifted from datasheet
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * 
 */
#include <stdint.h>
#include <xc.h>

#include <../../PIC24F/h/p24FJ128GA204.h>

#include <string.h>

#define BAUDRATEREG 25  

// CONFIG4
#pragma config DSWDTPS = DSWDTPS1F      //  (1:68719476736 (25.7 Days))
#pragma config DSWDTOSC = LPRC          //  (DSWDT uses LPRC as reference clock)
#pragma config DSBOREN = OFF            // Deep Sleep BOR Enable bit (DSBOR Disabled)
#pragma config DSWDTEN = OFF            // Deep Sleep Watchdog Timer Enable (DSWDT Disabled)
#pragma config DSSWEN = OFF             // DSEN Bit Enable (Deep Sleep operation is always disabled)
#pragma config PLLDIV = PLL4X           // USB 96 MHz PLL Prescaler Select bits (4x PLL selected)
#pragma config I2C1SEL = DISABLE        // Alternate I2C1 enable bit (I2C1 uses SCL1 and SDA1 pins)
#pragma config IOL1WAY = OFF            // PPS IOLOCK Set Only Once Enable bit (The IOLOCK bit can be set and cleared using the unlock sequence)

// CONFIG3
#pragma config WPFP = WPFP127           // Write Protection Flash Page Segment Boundary (Page 127 (0x1FC00))
#pragma config SOSCSEL = OFF            // SOSC Selection bits (Digital (SCLKI) mode)
#pragma config WDTWIN = PS25_0          // Window Mode Watchdog Timer Window Width Select (Watch Dog Timer Window Width is 25 percent)
#pragma config PLLSS = PLL_FRC          // PLL Secondary Selection Configuration bit (PLL is fed by the on-chip Fast RC (FRC) oscillator)
#pragma config BOREN = ON               // Brown-out Reset Enable (Brown-out Reset Enable)
#pragma config WPDIS = WPDIS            // Segment Write Protection Disable (Disabled)
#pragma config WPCFG = WPCFGDIS         // Write Protect Configuration Page Select (Disabled)
#pragma config WPEND = WPENDMEM         // Segment Write Protection End Page Select (Write Protect from WPFP to the last page of memory)

// CONFIG2
/*
#pragma config POSCMD = NONE            // Primary Oscillator Select (Primary Oscillator Disabled)
#pragma config WDTCLK = FRC             // WDT Clock Source Select bits (WDT uses 31 kHz source from FRC when active in Windowed WDT)
#pragma config OSCIOFCN = OFF           // OSCO Pin Configuration (OSCO/CLKO/RA3 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Fail-Safe Clock Monitor Configuration bits (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRC              // Initial Oscillator Select (Fast RC Oscillator (FRC))
#pragma config ALTCMPI = CxINC_RX       // Alternate Comparator Input bit (C1INC, C2INC and C3INC are on RB9)
#pragma config WDTCMX = WDTCLK          // WDT Clock Source Select bits (WDT clock source is determined by the WDTCLK Configuration bits)
#pragma config IESO = OFF               // Internal External Switchover (Disabled)
*/
/*
 Search for: Dan1138, how do you connect UART to RB6 on PIC24FJ128GA204?
 * On the Microchip forum, then implement fix alike this:
 */
_CONFIG2(POSCMD_NONE & WDTCLK_FRC & OSCIOFCN_OFF & FCKSM_CSDCMD & FNOSC_FRC & ALTCMPI_CxINC_RX & WDTCMX_WDTCLK & IESO_OFF & 0xF7FF)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler Select (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler Ratio Select (1:128)
#pragma config WINDIS = ON              // Windowed WDT Disable (Standard Watchdog Timer)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config ICS = PGx1               // Emulator Pin Placement Select bits (Emulator functions are shared with PGEC1/PGED1)
#pragma config LPCFG = OFF              // Low power regulator control (Disabled - regardless of RETEN)
#pragma config GWRP = OFF               // General Segment Write Protect (Write to program memory allowed)
#pragma config GCP = OFF                // General Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (Disabled)


//------------------------------------------------------------------------------
//Global variables
//------------------------------------------------------------------------------

volatile unsigned char Rxdata[20] = "";
unsigned char DataAvailable=0;
unsigned char Txdata[20] = "Microchip\r\n";

void __attribute__ ((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
  int len =  strlen((const char *)Rxdata);        
	Rxdata[len] = U1RXREG;
	IFS0bits.U1RXIF = 0;        // Clear the Receive Interrupt Flag    
}

int main(void) {
    
    TRISAbits.TRISA9 = 0;           // LED on Demo board
    LATAbits.LATA9 = 0;
    
    RPINR18bits.U1RXR = 5;          // RB5, marked as RX on demo board
    RPOR3bits.RP6R = _RPOUT_U1TX;   // RB6, marked as TX on demo board  
    
    // configure U1MODE    
	U1MODE = 0x0000;
    U1STA = 0x0000;
	U1BRG = BAUDRATEREG;	// baud rate	

	IFS0bits.U1TXIF = 0;	// Clear the Transmit Interrupt Flag
	IEC0bits.U1TXIE = 0;	// Enable Transmit Interrupts
	IFS0bits.U1RXIF = 0;	// Clear the Receive Interrupt Flag
	IEC0bits.U1RXIE = 1;	// Enable Receive Interrupts

	U1MODEbits.UARTEN = 1;	// And turn the peripheral on
    U1STAbits.UTXEN = 1;
    
   int i = 0;        
     for ( i=0; i<511; i++)         // UART has been set idle high, so need a pause for PC receiver to know that initial rising edge is not part of UART frame  
    Nop();
   
        i=0;
     while( Txdata[i] != '\0')
        {
         // wait until the transmit shift register is empty  
          while(U1STAbits.UTXBF);
            U1TXREG = (uint16_t) Txdata[i];            
            i++;
        }                 
     
     //Main loop
    while(1)
    {      
        int len =  strlen((const char *)Rxdata); 
        if (len>0)
        {
            LATAbits.LATA9 = 1;         // LED on demo board indicates some data received           
        }
    }
}
    
