/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Used on the IP-stack of Ulrich Radig
  known Problems:
 Version:        18.01.2009
 Description:    Sending syslog-Messages

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
#include <stdlib.h>
#include "config.h"
#include "console.h"
#include "usart.h"
#include "syslog.h"
#include "stack.h"
#include "para.h"

#if USE_SYSLOG

//#define SYSLOG_DEBUG	DEBUG_WRITE
#define SYSLOG_DEBUG(...)
//#define SYSLOG_ERROR	DEBUG_WRITE
#define SYSLOG_ERROR(...)

unsigned char syslog_server_ip[4];	// IP address of the server, set by default in config.h
									// or from EEPROM


//-----------------------------------------------------------------------------------
// sending msg to syslog server defined in EEPROM
void syslog_send ( char *msg )
{
	char *packet = (char *)(eth_buffer+UDP_DATA_START);
	char *pp = packet;
	int i = 0;

	SYSLOG_DEBUG("SYSLOG send %s\r\n", msg);

	(*((unsigned long*)&syslog_server_ip[0])) = para_getip(SYSLOG_IP_EEPROM_STORE,SYSLOG_IP);
	SYSLOG_DEBUG("Server: %1i.%1i.%1i.%1i\r\n",syslog_server_ip[0],syslog_server_ip[1],syslog_server_ip[2],syslog_server_ip[3]);

	//Arp-Request senden
	unsigned long tmp_ip = (*(unsigned long*)&syslog_server_ip[0]);
	if ( syslog_server_ip[3] != 0xFF )	// quickhack for broadcast address
		if ( arp_entry_search(tmp_ip) >= MAX_ARP_ENTRY ) //Arp-Request senden?
			for ( i=0; i<SYSLOG_ARPRETRIES && (arp_request(tmp_ip) != 1); i++ );

	if (i>=SYSLOG_ARPRETRIES) {
	  SYSLOG_ERROR("No SYSLOG server!\r\n");
	}

	// building the packet structure
	*pp++ = '<';
	*pp++ = '0';
	*pp++ = '>';
	strcpy ( pp, msg );
	create_new_udp_packet(strlen(packet),SYSLOG_CLIENT_PORT,SYSLOG_SERVER_PORT,tmp_ip);
}

#endif
