/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:
 known Problems: none
 Version:        24.10.2007
 Description:    RTL8019 Treiber
----------------------------------------------------------------------------*/

#ifndef _RTL_8019_H
	#define _RTL_8019_H

	#include <avr/io.h>
	#include "config.h"
	#include "usart.h"

#if USE_RTL8019

	extern unsigned char mymac[6];

	//Anschluﬂ des Webservers nach meiner Bauanleitung, oder die von
	//Holger Buss (www.mikrocontroller.com) Mega32-Board

	#if defined (__AVR_ATmega32__)
		#define ETH_INT_ENABLE 			GICR |= (1<<INT0)
		#define ETH_INT_DISABLE 		GICR &= ~(1<<INT0)
		#define ETH_INT_TRIGGER_MODE 	MCUCR |= 0x03;
		#define ETH_INTERRUPT 			INT0_vect
		#define ISA_CTRL
	#endif

	#if defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__)
		#define ETH_INT_ENABLE 			EIMSK |= (1<<INT0)
		#define ETH_INT_DISABLE 		EIMSK &= ~(1<<INT0)
		#define ETH_INT_TRIGGER_MODE  	EICRA |= 0x03;
		#define ETH_INTERRUPT 			INT0_vect
		#define ISA_CTRL
	#endif

	#if defined (__AVR_ATmega128__)
		#define ETH_INT_ENABLE 			EIMSK |= (1<<INT4)
		#define ETH_INT_DISABLE 		EIMSK &= ~(1<<INT4)
		#define ETH_INT_TRIGGER_MODE  	EICRB = 0x03;
		#define ETH_INTERRUPT 			INT4_vect
	#endif

	//Prototype
	extern void WriteRTL (unsigned char,unsigned char);
	extern unsigned char ReadRTL (unsigned char);
	extern void Init_Realtek_Network_Card (void);
	extern void Write_Ethernet_Frame (unsigned int,unsigned char *);
	extern unsigned int Read_Ethernet_Frame (unsigned int,unsigned char *);

	#define ETH_INIT			Init_Realtek_Network_Card
	#define ETH_PACKET_RECEIVE	Read_Ethernet_Frame
	#define ETH_PACKET_SEND		Write_Ethernet_Frame

	#define nop()  __asm__ __volatile__ ("nop" ::)

	//----------------------------------------------------------------------------
	// Standard Belegung f¸r meine Webserver
	#ifndef ISA_CTRL
		#define RTL_WR_OFF() 	ADDR_PORT_RLT&=~(1<<WRITE_PIN);
		#define RTL_WR_ON() 	ADDR_PORT_RLT|=(1<<WRITE_PIN);
		#define RTL_RD_OFF() 	ADDR_PORT_RLT&=~(1<<READ_PIN);
		#define RTL_RD_ON()  	ADDR_PORT_RLT|=(1<<READ_PIN);
		#define RTL_RESET_OFF() ADDR_PORT_RLT&=~(1<<RESET_PIN);

		//Ethernet Card Ports
		#define DATA_CTRL_RLT  			DDRA
		#define DATA_ADDR_RLT			DDRC	//F¸r ATMega128/32 wird bei ATMega102 nicht benˆ.
		#define ADDR_PORT_RLT  			PORTC
		#define DATA_PORT_RLT_WRITE  	PORTA
		#define DATA_PORT_RLT_READ 		PINA	//Read
		#define WRITE_PIN				6		//Pin an dem WriteIO Signal angeschlossen
		#define READ_PIN				5		//Pin an dem ReadIO Signal angeschlossen
		#define RESET_PIN				7		//Pin an dem Reset angeschlossen ist
	#else

	//----------------------------------------------------------------------------
	// Belegung f¸r Holger Buss Mega32 WebBoard
		#define RTL_WR_OFF() 	CTRL_LINES&=~(1<<WRITE_PIN);
		#define RTL_WR_ON() 	CTRL_LINES|=(1<<WRITE_PIN);
		#define RTL_RD_OFF() 	CTRL_LINES&=~(1<<READ_PIN);
		#define RTL_RD_ON()  	CTRL_LINES|=(1<<READ_PIN);
		#define RTL_RESET_OFF() CTRL_LINES&=~(1<<RESET_PIN);

		//Ethernet Card Ports f¸r ISP-CTRL
		#define DATA_CTRL_RLT  			DDRC
		#define DATA_ADDR_RLT			DDRA	//F¸r ATMega128/32 wird bei ATMega102 nicht benˆ.
		#define ADDR_PORT_RLT  			PORTA
		#define DATA_PORT_RLT_WRITE  	PORTC
		#define DATA_PORT_RLT_READ 		PINC	//Read
		#define CTRL_LINES              PORTB
		#define CTRL_LINES_DIR          DDRB
		#define WRITE_PIN				1		//Pin an dem WriteIO Signal angeschlossen
		#define READ_PIN				2		//Pin an dem ReadIO Signal angeschlossen
		#define RESET_PIN				0		//Pin an dem Reset angeschlossen ist
		#define ADDR_OUTPUT             0x1f
	#endif //define ISP-CTRL

	#define delay_ms(x) for(unsigned long a=0;a<((unsigned long)x*100);a++){asm("nop");};

	#define INPUT					0x00	//setzt einen Port auf Input
	#define OUTPUT					0xff	//setzt einen Port auf Output

	#define Networkcard_INT_RES()  WriteRTL (RTL_ISR, (1<<PRX|1<<PTX|1<<RXE|1<<TXE|1<<OVW|1<<CNT|1<<RDC|1<<RST));
	#define Networkcard_Start() WriteRTL (CR ,(1<<STA|1<<RD2));

	//Realtek Netzwerkkarten Register
	#define RTL_REG_OFFSET				96
	#define CR						RTL_REG_OFFSET+0x00
	#define PSTART					RTL_REG_OFFSET+0x01
	#define PAR0					RTL_REG_OFFSET+0x01
	#define PSTOP					RTL_REG_OFFSET+0x02
	#define BNRY					RTL_REG_OFFSET+0x03
	#define TPSR					RTL_REG_OFFSET+0x04
	#define TBCR0					RTL_REG_OFFSET+0x05
	#define TBCR1					RTL_REG_OFFSET+0x06
	#define RTL_ISR					RTL_REG_OFFSET+0x07       // renamed (avr-gcc 3.4.5)
	#define CURR					RTL_REG_OFFSET+0x07
	#define RSAR0					RTL_REG_OFFSET+0x08
	#define CRDA0					RTL_REG_OFFSET+0x08
	#define RSAR1					RTL_REG_OFFSET+0x09
	#define CRDAl					RTL_REG_OFFSET+0x09
	#define RBCR0					RTL_REG_OFFSET+0x0A
	#define RBCR1					RTL_REG_OFFSET+0x0B
	#define RSR						RTL_REG_OFFSET+0x0C
	#define RCR						RTL_REG_OFFSET+0x0C
	#define TCR						RTL_REG_OFFSET+0x0D
	#define CNTR0					RTL_REG_OFFSET+0x0D
	#define DCR						RTL_REG_OFFSET+0x0E
	#define CNTR1					RTL_REG_OFFSET+0x0E
	#define IMR						RTL_REG_OFFSET+0x0F
	#define CNTR2					RTL_REG_OFFSET+0x0F
	#define RDMAPORT				RTL_REG_OFFSET+0x10
	#define RSTPORT					RTL_REG_OFFSET+0x18

	//RTL8019AS initial register Werte
	#define RCRVAL					0x04
	#define TCRVAL					0x00
	#define DCRVAL					0x58

	//PXR and OVW interrupt enabled
	#define IMRVAL					0x11
	#define TXSTART					0x40
	#define RXSTART					0x46
	#define RXSTOP					0x60

	//RTL8019AS ISR Register Bits
	#define PRX						0	//BIT 0 0x01
	#define	PTX						1	//BIT 1 0x02
	#define	RXE						2	//BIT 2 0x04
	#define	TXE						3	//BIT 3 0x08
	#define	OVW						4	//BIT 4 0x10
	#define	CNT						5	//BIT 5 0x20
	#define	RDC						6	//BIT 6 0x40
	#define	RST						7	//BIT 7 0x80

	//RTL8019AS CR Register Bits
	#define STP						0	//BIT 0 0x01
	#define	STA						1	//BIT 1 0x02
	#define	TXP						2	//BIT 2 0x04
	#define	RD0						3	//BIT 3 0x08
	#define	RD1						4	//BIT 4 0x10
	#define	RD2						5	//BIT 5 0x20
	#define	PS0						6	//BIT 6 0x40
	#define	PS1						7	//BIT 7 0x80

#endif
#endif // _RTL_8019_H


