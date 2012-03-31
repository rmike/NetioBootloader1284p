/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke jens (a) mundhenke.info
 Author:         Jens Mundhenke
 Remarks:
 known Problems: none
 Version:        25.01.2009
 				 13.02.2009 Simplified syslog
 Description:    universal console output functions for AVR
 				 based on Ulrich Radig's Webserver
				 input has to be handled by device dependent functions

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
------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "limits.h"
#include "config.h"
#include "console.h"

//----------------------------------------------------------------------------
#define CONSOLE_DEBUG(format, args...)	\
		{CONSOLE_PUSH(CONSOLE_SYSLOG);\
		 console_write_P(format , ## args);\
		 CONSOLE_POP;}

	// kein Debug
	//#define TELNET_DEBUG(...)


// the supported devices should be included here
#if USE_TELNET
#include "telnetd.h"
#endif
#if USE_USART
#include "usart.h"
#endif
#if USE_SYSLOG
#include "syslog.h"
#endif

#define CONSOLE_TX_BUFFERSIZE	60

char console_tx_buffer[CONSOLE_TX_BUFFERSIZE+1];	// plus 1 for string terminating 0
unsigned int console_tx_counter = 0;

char console_isinit = 0;			// Flag, dass das Modul initialisiert ist, vorher werden Ausgaben ignoriert

unsigned int console_mode;
unsigned int cm;


//----------------------------------------------------------------------------
//Initialisierung der console
void console_init (void)
{
	// Initialisiere Pufferverwaltung
	console_tx_counter 	= 0;

	// initiliaze device modules
	#if USE_TELNET
	if ( console_mode & CONSOLE_TELNET );
		telnetd_init();
	#endif
	#if USE_USART
	if ( console_mode & CONSOLE_USART );
//		usart_init(BAUDRATE);
		usart_init();
	#endif

	console_isinit = 1;
}


//----------------------------------------------------------------------------
//Ausgabe eines Strings
void console_write_str(char *str)
{
	while (str && *str)
		console_write_char(*str++);
}


//----------------------------------------------------------------------------
//Routine für die Ausgabe gepufferter Zeichen ueber zeilen-orientierte Devices
void console_flush(void)
{
	if ( !console_tx_counter )		// buffer is empty
		return;

   	console_tx_buffer[console_tx_counter] = 0;		// terminate buffered string
   	#if USE_TELNET
	if ( console_mode & CONSOLE_TELNET )
		telnetd_write_str ( console_tx_buffer );
	#endif
	#if USE_SYSLOG
	if ( console_mode & CONSOLE_SYSLOG )
		syslog_send ( console_tx_buffer );
	#endif
	console_tx_counter = 0;								// clear buffer
}

//----------------------------------------------------------------------------
//Routine für die Ausgabe eines Zeichens ueber definierte Devices
//
void console_write_char(char c)
{
	if ( !console_isinit )
		return;

	#if USE_USART
	if ( console_mode & CONSOLE_USART )
		usart_write_char (c);
	#endif

	if ( console_mode & (CONSOLE_TELNET | CONSOLE_SYSLOG) ) {
		console_tx_buffer[console_tx_counter++] = c;
	    if ( (console_tx_counter >= CONSOLE_TX_BUFFERSIZE - 1) || (c == '\n') ) {	// Buffer ist voll oder Zeilenende, absenden
	    	console_flush();
		}
	}
}


//------------------------------------------------------------------------------
void console_write_P (const char *Buffer,...)
{
	va_list ap;
	va_start (ap, Buffer);

	int format_flag;
	char str_buffer[20];
	char move = 0;
	char len, b;
	char by;
	char *ptr;

	//Ausgabe der Zeichen
    for(;;) {

		by = *Buffer++;
		if(by==0) break; // end of format string

		if (by == '%') {

			by = *Buffer++;

			while (isdigit(by)) {				// potential overflow of move
				move = by - '0' + move * 10;

 				by = *Buffer++;
			}

			switch (by) {
                case 's':
                    ptr = va_arg(ap,char *);
                    console_write_str(ptr);
                    break;
				case 'c':
					//Int to char
					format_flag = va_arg(ap,int);
					console_write_char (format_flag++);
					break;
				case 'i':
				case 'd':
					itoa(va_arg(ap,int), str_buffer,10);
					goto ConversionLoop;
				case 'u':
					utoa(va_arg(ap,unsigned int), str_buffer,10);
					goto ConversionLoop;
				case 'x':
					itoa(va_arg(ap,int), str_buffer,16);
					//****************************
					ConversionLoop:
					//****************************
					len = strlen (str_buffer);
					b = '0';
					while ( len < move ) {
					  console_write_char (b);
					  len++;
					}
					console_write_str (str_buffer);
					move = 0;
					break;
				} // switch
		} else
			console_write_char ( by );
	} // for
	va_end(ap);
}

