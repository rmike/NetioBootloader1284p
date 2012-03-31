/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:
 known Problems: none
 Version:        24.10.2007
 Description:    Timer Routinen
 				 simplified for use in a bootloader by Jens Mundhenke

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
#include "timer.h"
#include "bootloader/tftp.h"


//----------------------------------------------------------------------------
//Diese Routine startet und inizialisiert den Timer
void timer_init (void)
{
	TCCR1B |= (1<<WGM12) | (1<<CS10 | 0<<CS11 | 1<<CS12);
	TCNT1 = 0;
	OCR1A = (F_CPU / 1024) - 1;
	TIMSK1 |= (1 << OCIE1A);
};

//----------------------------------------------------------------------------
//Timer Interrupt
ISR (TIMER1_COMPA_vect)
{

//   	tftp_watchdogtick (1000);		// parameter notes the length of interval in ms
//	if ( tftp_watchdog == TFTP_WATCHDOG_OFF )
//		return;

	if (tftp_watchdog > 0)
		tftp_watchdog -= 1000;
}
