/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Used on the IP-stack of Ulrich Radig
 known Problems:
 Version:        18.01.2009
 				 13.02.2009 simplified
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

#ifndef _SYSLOG_H
#define _SYSLOG_H

#define SYSLOG_SERVER_PORT			514
#define SYSLOG_CLIENT_PORT			42514

#define	SYSLOG_IP_EEPROM_STORE		((unsigned char *)80)

#define SYSLOG_ARPRETRIES			3

#define SYSLOG_PRIOEMERGENCY		0
#define SYSLOG_PRIOALERT			1
#define SYSLOG_PRIOCRITICAL			2
#define SYSLOG_PRIOERROR			3
#define SYSLOG_PRIOWARNING			4
#define SYSLOG_PRIONOTICE			5
#define SYSLOG_PRIOINFORMATIONAL	6
#define SYSLOG_PRIODEBUG			7

#define	SYSLOG_FACKERNEL			0
#define	SYSLOG_FACUSER				1
#define SYSLOG_FACMAIL				2
#define SYSLOG_FACDAEMONS			3
#define SYSLOG_FACSECURITY			4
#define SYSLOG_FACSYSLOGD			5
#define SYSLOG_FACPRINTER			6
#define SYSLOG_FACNETWORK			7
#define SYSLOG_FACUUCP				8
#define SYSLOG_FACCLOCK				9
#define SYSLOG_FACSECURITY2			10
#define SYSLOG_FACFTP				11
#define SYSLOG_FACNTP				12
#define SYSLOG_FACLOGAUDIT			13
#define SYSLOG_FACLOGALERT			14
#define SYSLOG_FACCLOCK2			15
#define SYSLOG_FACLOCAL0			16
#define SYSLOG_FACLOCAL1			17
#define SYSLOG_FACLOCAL2			18
#define SYSLOG_FACLOCAL3			19
#define SYSLOG_FACLOCAL4			20
#define SYSLOG_FACLOCAL5			21
#define SYSLOG_FACLOCAL6			22
#define SYSLOG_FACLOCAL7			23


void syslog_send ( char *msg );

#endif
