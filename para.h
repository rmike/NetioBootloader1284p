/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Used on an simplified IP-stack of Ulrich Radig
 				 Collection of eeprom functions spreaded over the project
 known Problems:
 Version:        18.01.2009
 Description:    EEPROM-functions for the IP stack and applications

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
#ifndef _PARA_H
#define _PARA_H

#include "config.h"

// Adress-Offsets im EEPROM
#define MAC_EEPROM_STORE 		((unsigned char *)24)	// 6 byte
#define IP_EEPROM_STORE 		((unsigned char *)30)	// 4 byte
#define NETMASK_EEPROM_STORE 	((unsigned char *)34)	// 4 byte
#define ROUTER_IP_EEPROM_STORE 	((unsigned char *)38)	// 4 byte
#define NTP_IP_EEPROM_STORE 	((unsigned char *)50)	// 4 byte
#define TFTP_MSG_EEPROM_STORE	((unsigned char *)56)	// 4 byte, output mask for bootloader
														// 0x01 : usart, 0x02 : syslog
#define	TFTP_IP_EEPROM_STORE	((unsigned char *)60)	// 4 byte, IP address of TFTP server
#define TFTP_FILE_EEPROM_STORE	((unsigned char *)64)	// name of boot file, max 16 byte
#define WOL_MAC_EEPROM_STORE	((unsigned char *)84) 	// 6 Byte
#define WOL_BCAST_EEPROM_STORE	((unsigned char *)90) 	// 4 Byte


#define TFTP_FILENAME_MAX		16


unsigned long para_getip (unsigned char *eeprom_address, unsigned long default_value);
void para_getstring (char *name, unsigned char* eeprom_address, char len, char *default_value);
unsigned char para_getchar (unsigned char *eeprom_address);

#endif
