/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Used in an simplified IP-stack of Ulrich Radig
 known Problems:
 Version:        18.01.2009
 Description:    TFTP-Client for Bootloader use

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

#ifndef _TFTP_H
#define _TFTP_H

// TFTP port numbers
#define TFTP_SERVER_PORT	69		// fixed port due to RFC
#define TFTP_CLIENT_PORT	42069	// random port

// TFTP watchdog
#define	TFTP_WATCHDOG		2000	// waiting time in milliseconds
#define TFTP_WATCHDOG_OFF	30000	// value for watchdog: the watchdog timer is stopped

// TFTP packet types
#define TFTP_RRQ	1	// read request
#define TFTP_WRQ	2	// write request
#define TFTP_DATA	3	// data
#define TFTP_ACK	4	// acknowledge
#define TFTP_ERR	5	// error

// TFTP States of Receiving
#define TFTP_OFF	0	// nothing to do
#define TFTP_REC	1	// receiving

// TFTP protocol handling internals
#define TFTP_MODE	"octet"		// requested transmission mode
#define TFTP_ARPRETRIES	3		// number of ARP requests before we give up
								// to reach the server
// TFTP functions
void tftp_init (void);
void tftp_request (void);
void tftp_get(unsigned long src_ip, unsigned int src_port);
void tftp_watchdogrestart(void);
void tftp_watchdogcheck(void);
void tftp_watchdogtick(unsigned int ms);

extern volatile int tftp_watchdog;	// count down watchdog for timeout after sending a packet

#endif
