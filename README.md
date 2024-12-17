# Curiosity-Demo-UART
MPLAB source code for testing UART on PIC24F Curiosity Dev board
Uses the Micro-bus socket for UART connections marked RX & TX
Needs a Microchip special kludge to get RB6/TX working
TX success is receiving "Microchip" on a teminal emulator (e.g. TeraTerm) set to 9600 baud
RX success is LED1 illuminating on reception of any character from e.g. TeraTerm
