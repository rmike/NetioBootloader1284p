/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Used on an simplified IP-stack of Ulrich Radig
 				 Fixed configuration for ATmega644
 known Problems:
 Version:        18.01.2009
 Description:    Interpreter and Flasher for Intelhex-lines

 This module interprets a stream of intel hex information and flashes it
 to the programm store.

 Dieses Programm ist freie Software. Sie können es unter den Bedingungen der
 GNU General Public License, wie von der Free Software Foundation veröffentlicht,
 weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder
 (nach Ihrer Option) jeder späteren Version.

 Die Veröffentlichung dieses Programms erfolgt in der Hoffnung,
 daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE,
 sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT
 FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License.

 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 Programm erhalten haben.
 Falls nicht, schreiben Sie an die Free Software Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
----------------------------------------------------------------------------*/

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "config.h"
#include "usart.h"
#include "timer.h"
#include "bootloader/ihex.h"
#include "networkcard/enc28j60.h"

// states of line interpreter - what byte is expected to come next?
#define IHEX_START	1			// start byte ':'
#define IHEX_LENH	2			// high nibble of length
#define IHEX_LENL	3			// low nibble of length
#define	IHEX_ADDR	4			// one of the address bytes
#define IHEX_TYPEH	5			// high nibble of type
#define IHEX_TYPEL	6			// low nibble of type
#define IHEX_DATAH	7			// high nibble of one data byte
#define IHEX_DATAL	8			// low nibble of one data byte
#define IHEX_CHECKH	9			// high nibble of checksum
#define IHEX_CHECKL	10			// low nibble of checksum
#define IHEX_END	11			// line end char

// type bytes in line
#define IHEX_TYPEDATA				0		// data
#define IHEX_TYPEEND				1		// end marker, last line in intel hex file
#define IHEX_TYPE_EXTSEG			2		// extended segment address offset
#define IHEX_TYPE_STARTSEG			3		// start segment address offset
#define IHEX_TYPE_EXT_LIN_REC		4		// extended linear address record
#define IHEX_TYPE_START_LIN_REC		5		// start linear address record

#define IHEX_STARTBYTE	':'		// first byte in a line

#define IHEX_NOPAGE	-1			// flag for 'no block is processed in the moment'

uint16_t ihex_lastpage = IHEX_NOPAGE;// number of the open page
uint8_t ihex_linestate = IHEX_START;
uint8_t ihex_addrbyte;				// current addressbyte 3..0
uint8_t ihex_len;					// number of bytes in this line
uint8_t ihex_type;					// type of line
uint8_t ihex_data[IHEX_DATAMAX];	// data in that line
uint16_t ihex_dataidx;				// current data byte processed
uint8_t ihex_checksum;				// checksum calculated on the fly
uint16_t ihex_addr;					// addr found in the line
uint32_t ihex_addr_offset;			// addr-offset found in the line
uint8_t ihex_check;					// checksum in the line;


//----------------------------------------------------------------------------
// internal functions
char ihex_ascii2val (char c);
char ihex_program(uint32_t addr, char dat0, char dat1 );

//----------------------------------------------------------------------------
// debug-output don't forget to enable usart in config.h
//#define IHEX_DEBUG console_write
#define IHEX_DEBUG(...)


//----------------------------------------------------------------------------
// change location of interrupt vectors for the processor
// CAUTION: Do not optimize the Enable command
void ihex_intvector(char i)
{
	uint8_t temp;
	/* Get MCUCR*/
	temp = MCUCR;
	if ( i == IHEX_INTVECTOR_BOOT ) {
		/* Enable change of Interrupt Vectors */
		MCUCR = temp|(1<<IVCE);
		/* Move interrupts to Boot Flash section */
		MCUCR = temp | (1<<IVSEL);
	} else {
		/* Enable change of Interrupt Vectors */
		MCUCR = temp|(1<<IVCE);
		/* Move interrupts to Standard section */
		MCUCR = temp & ~(1<<IVSEL);
	}
}

//----------------------------------------------------------------------------
// start normal application
void ihex_appstart(void)
{
	cli();								// disable interrupts

#if USE_USART
    while(!(USR & (1<<UDRE)));			// wait for pending usart transfers
	UCR = 0; 							// disable usart and usart interrupts
	USR = (1<<TXC);						// clear interrupt flags
#endif

	TIMSK1 = 0;							// disable timer interrupts
	TIFR1 = 0xFF; 						// clear interrupt flags
	TCCR1B = 0;							// disable timer

	SPCR = 0;							// disable SPI
	SPSR = 0;

	ihex_intvector(IHEX_INTVECTOR_STD);	// switch back to standard interrupt vector table

	asm volatile ( "jmp 0x0000" );		// start application
}


//----------------------------------------------------------------------------
// dummy converter, expects upper Letters and numbers
char ihex_ascii2val ( char c )
{
	if (c <= '9') return (c-'0');
	else 		  return (c-55);
}


//----------------------------------------------------------------------------
// init this module
void ihex_init(void)
{
	ihex_linestate = IHEX_START;
}


//----------------------------------------------------------------------------
// taking the intel hex file byte by byte, waiting for a complete line, flashing it
//
// CAUTION:
// due to the nature of the buffer a page will be deleted before it is flashed
// if the contents of a page arrives not in one sequence, only the last part
// will be kept
//
// the END-line of IntelHex is needed to complete the flashing of the last page
//
char ihex_flash ( char c )
{
	unsigned char sreg;		// stores the INT-state while int is disabled
	char rc;				// return code
	int i;

	c = toupper(c);

//	IHEX_DEBUG ( "IHEX %c, State %i\r\n", c, ihex_linestate );

	// only 0..9 and A..F is valid inside the line
	if ( !(ihex_linestate == IHEX_START || ihex_linestate == IHEX_END) )
		if ( (c < '0') || ( c > '9' && c < 'A' ) || ( c > 'F' ) ) {
			ihex_linestate = IHEX_START;
			return IHEX_ERRCHAR;
		}

	// interpret the char
	switch ( ihex_linestate ) {
		case IHEX_START:						// start byte ':'
			if ( c == IHEX_STARTBYTE ) {
				ihex_addrbyte = 3;
				ihex_addr = 0;
				ihex_dataidx = 0;
				ihex_linestate = IHEX_LENH;
			}
			break;
		case IHEX_LENH:							// high nibble of length
			ihex_len = ihex_ascii2val(c) << 4;
			ihex_linestate = IHEX_LENL;
			break;
		case IHEX_LENL:							// low nibble of length
			ihex_len += ihex_ascii2val(c);
			ihex_checksum = ihex_len;			// start calculating the checksum
			if ( ihex_len <= IHEX_DATAMAX ) {
				ihex_linestate = IHEX_ADDR;
			} else {
				ihex_linestate = IHEX_START;
				return IHEX_ERRLEN;
			}
			break;
		case IHEX_ADDR:							// one of the address bytes
			ihex_addr <<= 4;					// shift the result of the nibbles before
			ihex_addr += ihex_ascii2val(c);		// add the current nibble
			if ( !(ihex_addrbyte & 0x01) ) {	// calc checksum for second and forth byte of addr
				ihex_checksum += (ihex_addr & 0xff);
			}
			if ( !(ihex_addrbyte--) )			// last byte of address processed?
				ihex_linestate = IHEX_TYPEH;	// go to next state
			break;
		case IHEX_TYPEH:						// high nibble of type
			ihex_type = ihex_ascii2val(c) << 4;
			ihex_linestate = IHEX_TYPEL;
			break;
		case IHEX_TYPEL:						// low nibble of type
			ihex_type += ihex_ascii2val(c);
			ihex_checksum += ihex_type;			// calculate the checksum
			ihex_linestate = ( ihex_len )?IHEX_DATAH:IHEX_CHECKH;
			break;
		case IHEX_DATAH:						// high nibble of one data byte
			ihex_data[ihex_dataidx] = ihex_ascii2val(c) << 4;
			ihex_linestate = IHEX_DATAL;
			break;
		case IHEX_DATAL:						// low nibble of one data byte
			ihex_data[ihex_dataidx] += ihex_ascii2val(c);
			ihex_checksum += ihex_data[ihex_dataidx];
												// loop for next data byte or terminate data
			ihex_linestate = ( ++ihex_dataidx < ihex_len )?IHEX_DATAH:IHEX_CHECKH;
			break;
		case IHEX_CHECKH:						// high nibble of checksum
			ihex_check = ihex_ascii2val(c) << 4;
			ihex_linestate = IHEX_CHECKL;
			break;
		case IHEX_CHECKL:						// low nibble of checksum
			ihex_check += ihex_ascii2val(c);
			ihex_checksum += ihex_check;		// calculate the checksum
			if ( ihex_checksum ) {				// checksum should be zero in this state
				IHEX_DEBUG ( "IHEX wrong Checksum %x!\n\r", ihex_checksum );
				ihex_linestate = IHEX_START;
				return IHEX_ERRCHECK;
			}
			ihex_linestate = IHEX_END;
			break;
		case IHEX_END:
			ihex_linestate = IHEX_START;		// we are starting a new line
			if ( c == 0x0a || c == 0x0d ) {		// line is terminated
				IHEX_DEBUG ( "IHEX Hurra!\n\r" );
				sreg = SREG;
				cli();
				rc = 0;
				switch ( ihex_type ) {
					case IHEX_TYPEDATA:
						for ( i=0; (i<ihex_len) && (!rc); i+=2, ihex_addr+=2 )
				  			rc = ihex_program (ihex_addr + ihex_addr_offset, ihex_data[i], ihex_data[i+1]);
				  		break;
				  	case IHEX_TYPEEND:
				  		rc = ihex_program (ihex_addr + ihex_addr_offset, 0, 0);	// complete flash process of last page
				  		break;
				  	case IHEX_TYPE_EXTSEG:
				  		ihex_addr_offset = ((uint16_t)ihex_data[0] << 8) + ihex_data[1];
				  		ihex_addr_offset <<= 4;
				  		rc = IHEX_NOERR;
				  		break;
				  	case IHEX_TYPE_STARTSEG:
				  		// found this type at end of hex file. ignore this line.
				  		rc = IHEX_NOERR;
				  		break;
				  	default:
				  		rc = IHEX_ERRTYPE;		// other intel commands are not supported
				  								// will be needed for addresses > 0xffff
				}
				SREG = sreg;
				return rc;
			} else
				return IHEX_ERRLINE;
			break;
		default:
			IHEX_DEBUG ( "IHEX wrong linestate %i!\n\r", (int)ihex_linestate );
			return IHEX_ERRSTATE;
	}

	return IHEX_NOERR;
}

//#define IHEX_DEBUG usart_write


char ihex_program(uint32_t addr, char dat0, char dat1 )
{
	uint16_t page = addr / SPM_PAGESIZE;
	unsigned int w;

//	IHEX_DEBUG("%2x%2x@%x\r\n", (int)dat0, (int)dat1, addr );

	if ( addr >= BOOTLOADERSTARTADR )
		return	IHEX_ERRADDR;		// protect yourself!

	eeprom_busy_wait();

	// if we change the page or got the END-line, flash the current page
	if ( ihex_lastpage != IHEX_NOPAGE
	  && (ihex_lastpage != page || ihex_type == IHEX_TYPEEND) ) {
//		IHEX_DEBUG ( "W %x ", (uint16_t)ihex_lastpage*SPM_PAGESIZE );
		boot_page_write ((uint32_t)ihex_lastpage*SPM_PAGESIZE);	// store buffer in flash
		while (boot_rww_busy())
			boot_rww_enable();
		ihex_lastpage = IHEX_NOPAGE;					// page is done, forget it
	}

	switch ( ihex_type ) {
		case IHEX_TYPEEND:
			boot_rww_enable();			// reenable RWW-section
			break;
		case IHEX_TYPEDATA:
			if ( ihex_len == 0 )		// nothing to do...
				break;
			if ( ihex_lastpage == IHEX_NOPAGE )	{	// page is not prepared for reprogramming
//				IHEX_DEBUG ( "E %x ", addr );
				boot_page_erase (addr);				// erase page we have data for
				while (boot_rww_busy())
					boot_rww_enable();
				ihex_lastpage = page;				// remember the page we have prepared
			}
			w = dat0;					// Set up little-endian word.
			w += dat1 << 8;
			boot_page_fill ( addr, w );	// write it to buffer
//			IHEX_DEBUG ("%4x@%4x ", w, addr );
			break;
		default:
			return IHEX_ERRTYPE;	// unsuported type of data
	}

	return IHEX_NOERR;
}
