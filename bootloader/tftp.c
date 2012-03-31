/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Used in an simplified IP-stack of Ulrich Radig
 known Problems: activating to much debug will lead to timing and memory problems
 Version:        18.01.2009
 Description:    TFTP-Client for Bootloader use

 The module is a simple TFTP-client. TFTP server IP and filename are stored in EEPROM
 tftp_request starts a download, the answers are handled by tftp_get. It is expected
 that this function is called by ipstack if a UDP packet arrives.

 The data received is sent byte by byte to an intel hex interpreter and flasher.

 This is not an universal TFTP-client, the simple structure is designed to work in a
 bootloader. It only supports download.

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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#include "config.h"
#include "para.h"
#include "stack.h"
#include "console.h"
#include "bootloader/tftp.h"
#include "bootloader/ihex.h"

//----------------------------------------------------------------------------
// structure of a TFTP packet, see RFC1350
typedef struct TFTP_header_st {
	uint16_t	type;			// constants in tftp.h
	union {
		char raw[0];			// raw data for the request
		struct {				// data with block number
			uint16_t block;
			char data[0];
		} data;
		struct {				// ack, only a block number
			uint16_t block;
		} ack;
		struct {				// error message with err code
			uint16_t code;
			char msg[1];
		} error;
	} u;
} TFTP_header_t;


unsigned char tftp_server_ip[4];// IP address of the server, set by default in config.h
								// or from EEPROM


uint16_t tftp_block = TFTP_OFF;		// number of TFTP-block number that is expected

volatile int tftp_watchdog=TFTP_WATCHDOG_OFF;	// count down watchdog for timeout after sending a packet


//----------------------------------------------------------------------------
//Outputs - only available if USE_USART is set
#if USE_CONSOLE
//#define TFTP_DEBUG usart_write
//#define TFTP_DEBUG console_write
#define TFTP_DEBUG(...)
#define TFTP_ERROR console_write
#define TFTP_MSG   console_write
#else
#define TFTP_DEBUG(...)
#define TFTP_ERROR(...)
#define TFTP_MSG(...)
#endif

//----------------------------------------------------------------------------
//Initialisierung des TFTP Ports (für den Datenempfang)
void tftp_init (void)
{
	ihex_init();			// initialize the interpreter and flasher
	tftp_watchdog = TFTP_WATCHDOG_OFF;	// reset timer for answer

	tftp_block = TFTP_OFF;	// init the number of the last TFTP block received
	return;
}


//----------------------------------------------------------------------------
//Anforderung einer Datei vom TFTP-Server
void tftp_request (void)
{
	char name[TFTP_FILENAME_MAX+1];
	char mode[9];
	int i;

	TFTP_header_t *tftp_header_p = (TFTP_header_t *)(eth_buffer+UDP_DATA_START);

	TFTP_DEBUG("TFTP Req\r\n");
	if ( tftp_block != TFTP_OFF ) {
//	  TFTP_ERROR("TFTP running!\r\n");
	  ihex_appstart();
	}

	(*((unsigned long*)&tftp_server_ip[0])) = para_getip(TFTP_IP_EEPROM_STORE,TFTP_IP);
	TFTP_MSG("Svr %3i.%3i.%3i.%3i\r\n",tftp_server_ip[0],tftp_server_ip[1],tftp_server_ip[2],tftp_server_ip[3]);

	//Arp-Request senden
	unsigned long tmp_ip = (*(unsigned long*)&tftp_server_ip[0]);

	for ( i=0; i<TFTP_ARPRETRIES && (arp_request(tmp_ip) != 1); i++ );

	if (i>=TFTP_ARPRETRIES) {
	  TFTP_ERROR("Svr?\r\n");
	  ihex_appstart();
	}

	// setting name and mode from constants, prepared for a more flexible solution
	strcpy ( mode, TFTP_MODE );

	para_getstring (name, (unsigned char *)TFTP_FILE_EEPROM_STORE, TFTP_FILENAME_MAX, TFTP_NAME);
	TFTP_MSG("File %s\r\n", name);

	// building the header structure
	tftp_header_p->type = LBBL_ENDIAN_INT((int)TFTP_RRQ);
	strcpy ( tftp_header_p->u.raw, name );
	strcpy ( &(tftp_header_p->u.raw[strlen(name)+1]), mode );
	create_new_udp_packet(strlen(name)+strlen(mode)+4,TFTP_CLIENT_PORT,TFTP_SERVER_PORT,tmp_ip);
	TFTP_DEBUG("TFTP Req sent\r\n");

	tftp_watchdogrestart();		// start watchdog

	tftp_block = TFTP_REC;
}

//----------------------------------------------------------------------------
// acknowledges the block number stored in tftp_block
void tftp_ack(unsigned long src_ip, unsigned int src_port, TFTP_header_t *tftp_header_p)
{
	tftp_header_p->type = LBBL_ENDIAN_INT((int)TFTP_ACK);			// set packet type to ACK
	tftp_header_p->u.ack.block = LBBL_ENDIAN_INT((int)tftp_block);	// ack last packet
	create_new_udp_packet(4,TFTP_CLIENT_PORT,src_port,src_ip);		// send ACK
	tftp_watchdogrestart();											// restart watchdog
}

//----------------------------------------------------------------------------
// called by ip stack, this function interprets the tftp-answers of the server
void tftp_get(unsigned long src_ip, unsigned int src_port)
{
	TFTP_header_t *tftp_header_p = (TFTP_header_t *)(eth_buffer+UDP_DATA_START);
	int data_len;		// length of TFTP-data in the packet
	unsigned int block;	// blocknumber in the packet
	int i;				// counter for the bytes in data part of TFTP-packet
	char rc;			// return code of the flasher function

	TFTP_DEBUG("TFTP Receive, Type %i!!\r\n", LBBL_ENDIAN_INT(tftp_header_p->type) );

	// we are not in the mood, due to error in former data or because no tftp-data was requested
	if ( tftp_block == TFTP_OFF ) {
//		TFTP_ERROR ( "Packet w/o Req\r\n" );
		return;
	}

	//
	switch ( LBBL_ENDIAN_INT(tftp_header_p->type) ) {
		case TFTP_ERR:
			TFTP_ERROR ( "Err %s\r\n", tftp_header_p->u.error.msg );
			tftp_block = TFTP_OFF;
	  		ihex_appstart();
			break;

		case TFTP_DATA:
			data_len = UDP_DATA_END_VAR - UDP_DATA_START - 4;
			block = LBBL_ENDIAN_INT(tftp_header_p->u.data.block);

			TFTP_DEBUG ( "TFTP Datablock %i, Size: %i\r\n", block, data_len );

			// interpret the packet
			// warning: we are using the ethernet buffer, may this be overwritten?!? No, Int is disabled
			if ( block == tftp_block ) {	// the expected next packet?
				rc = 0;
				for (i=0; (i<data_len) && !rc; i++)
					rc = ihex_flash ( tftp_header_p->u.data.data[i] );

				if (rc) {					// error while interpreting the intel hex contents?
					TFTP_ERROR ( "Err %i@%i\r\n", (int)rc, i-1 );
					tftp_block = TFTP_OFF;
					return;
				} else {					// everything went fine, acknowledge the block
					TFTP_MSG ( "." );
					tftp_ack(src_ip, src_port, tftp_header_p);
					tftp_block++;
				}

			} else {						// wrong block number
				TFTP_ERROR ( "Block %i, %i exp.\r\n", block, tftp_block );
				tftp_ack(src_ip, src_port, tftp_header_p);	// ack the last block that was correct
			}

			// if the packet has less the 512 bytes of data transmission is over
			if ( data_len != 512 ) {
			  TFTP_MSG ( "done\r\n" );
			  tftp_block = TFTP_OFF;
			  ihex_appstart();
			}
			break;
		default:
			TFTP_ERROR ( "Err Type %i\r\n", LBBL_ENDIAN_INT(tftp_header_p->type) );
			tftp_block = TFTP_OFF;
			break;
	}
	return;
}

//----------------------------------------------------------------------------
// Watchdog functions: terminate bootloader if waiting for answers times out
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// restart countdown of the watchdog
void tftp_watchdogrestart()
{
	tftp_watchdog = TFTP_WATCHDOG;
}

//----------------------------------------------------------------------------
// check if watchdog has reached the end (value <= 0)
void tftp_watchdogcheck()
{
	if ( tftp_watchdog <= 0 ) {
		tftp_block = TFTP_OFF;
		TFTP_ERROR ( "Tout\r\n" );
		ihex_appstart();
	}
}

/*
//----------------------------------------------------------------------------
// tick the countdown of the watchdog
void tftp_watchdogtick(unsigned int ms)
{
	if ( tftp_watchdog == TFTP_WATCHDOG_OFF )
		return;

	if (tftp_watchdog > 0)
		tftp_watchdog -= ms;
}
*/
