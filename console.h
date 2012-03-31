/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke jens (a) mundhenke.info
 Author:         Jens Mundhenke
 Remarks:
 known Problems: none
 Version:        25.01.2009
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

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "config.h"

void console_init(void);
void console_write_char(char c);
void console_write_str(char *cp);
void console_flush(void);

void console_write_P (const char *Buffer,...);
#define console_write(format, args...)   console_write_P(format , ## args)

extern unsigned int console_mode;
extern unsigned int cm;				// buffer, if mode is changed by PUSH

#define CONSOLE_TELNET	0x04
#define CONSOLE_SYSLOG	0x02
#define CONSOLE_USART	0x01

#define CONSOLE_SET(x)	console_mode = (x)
#define CONSOLE_ADD(x)	console_mode |= (x)
#define CONSOLE_SUB(x)	console_mode &= ~(x)
#define CONSOLE_PUSH(x)	{cm = console_mode; console_mode = (x);}
#define CONSOLE_POP		{console_mode = cm;}

#endif
