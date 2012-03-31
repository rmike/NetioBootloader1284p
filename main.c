/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Based on a webserver of Ulrich Radig
 known Problems:
 Version:        18.01.2009
 Description:    Bootloader main

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


#include <avr/io.h>
#include <math.h>
#include "config.h"
#include "para.h"
#if USE_CONSOLE
	#include "console.h"
#endif
#include "syslog.h"
#include "networkcard/enc28j60.h"
#include "networkcard/rtl8019.h"
#include "stack.h"
#include "timer.h"
#include "bootloader/tftp.h"
#include "bootloader/ihex.h"

//----------------------------------------------------------------------------
// wdt_init - Watchdog Init used to disable the CPU watchdog
//         placed in Startcode, no call needed
#include <avr/wdt.h>

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init1")));

void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}


//----------------------------------------------------------------------------
//Hier startet das Hauptprogramm
int main(void)
{
	ihex_intvector(IHEX_INTVECTOR_BOOT);

	//Applikationen starten
	stack_init();
	tftp_init();

	//Globale Interrupts einschalten
	sei();

#if USE_CONSOLE
//	console_mode = (char)para_getip(TFTP_MSG_EEPROM_STORE, TFTP_MSGMSK);

#if USE_SYSLOG
	console_mode = CONSOLE_SYSLOG;
#endif

#if USE_USART
	console_mode |= CONSOLE_USART;
#endif

	console_init();
	console_write("\n\rBoot\n\r");
#endif

	tftp_request();				// send tftp request for bootfile

    while(1)
	{
	    eth_get_data();			// handle timer and incoming packets
	    tftp_watchdogcheck();	// check for timeout in tftp communication
    }
}
