/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:
 known Problems: none
 Version:        18.01.2009
 Description:    Ethernet Stack,
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

#ifndef _STACK_H
	#define _STACK_H

#include <avr/interrupt.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/io.h>

#include "config.h"
#include "networkcard/enc28j60.h"
#include "networkcard/rtl8019.h"
#include "timer.h"

#define LBBL_ENDIAN_INT(x)	((x & 0x00FF)<<8)+((x & 0xFF00)>>8)
#define LBBL_ENDIAN_LONG(x) ((x & 0xFF000000)>>24)+((x & 0x00FF0000)>>8)+((x & 0x0000FF00)<<8)+((x & 0x000000FF)<<24)

unsigned char myip[4];

unsigned int IP_id_counter;

#if defined (__AVR_ATmega32__)
	#define MAX_ARP_ENTRY 5
	#define MTU_SIZE 600
#endif

#if defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__)
	#define MAX_ARP_ENTRY 5
	#define MTU_SIZE 1200
#endif

#if defined (__AVR_ATmega128__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega1281__) || defined (__AVR_ATmega1284P__)
	#define MAX_ARP_ENTRY 5
	#define MTU_SIZE 1200
#endif

#define ARP_REPLY_LEN		60
#define ICMP_REPLY_LEN		98
#define ARP_REQUEST_LEN		42

unsigned char eth_buffer[MTU_SIZE+1];
unsigned int  eth_len;

struct arp_table
{
	unsigned char arp_t_mac[6];
	unsigned long arp_t_ip;
};

//----------------------------------------------------------------------------
//Prototypen
void stack_init (void);
void check_packet (void);
void eth_get_data (void);
unsigned long get_eeprom_value (unsigned int eeprom_adresse,unsigned long default_value);

void new_eth_header (unsigned char *,unsigned long);

char arp_entry_search (unsigned long);
void arp_reply (void);
void arp_entry_add (void);
char arp_request (unsigned long);

void make_ip_header (unsigned char *,unsigned long);
void icmp_send (unsigned long,unsigned char,unsigned char,unsigned int,unsigned int);
unsigned int checksum (unsigned char *,unsigned int,unsigned long);

void udp_socket_process(void);

void create_new_udp_packet(	unsigned int,unsigned int,unsigned int,unsigned long);

void find_and_start (unsigned char index);
void add_udp_app (unsigned int, void(*fp1)(unsigned long src_ip, unsigned int src_port));
void change_port_tcp_app (unsigned int, unsigned int);

#define ETHER_OFFSET			0x00
#define ARP_OFFSET				0x0E
#define IP_OFFSET				0x0E
#define ICMP_OFFSET				0x22
#define ICMP_DATA				0x2A
#define UDP_OFFSET				0x22

struct arp_table arp_entry[MAX_ARP_ENTRY];

//IP Protocol Types
#define	PROT_ICMP				0x01	//zeigt an die Nutzlasten enthalten das ICMP Prot
#define	PROT_UDP				0x11	//zeigt an die Nutzlasten enthalten das UDP Prot.

//Defines für IF Abfrage
#define IF_MYIP 				(ip->IP_Destaddr==*((unsigned long*)&myip[0]))
#define IP_UDP_PACKET 			(ip->IP_Proto == PROT_UDP)
#define IP_ICMP_PACKET 			(ip->IP_Proto == PROT_ICMP)

//Defines für if Abfrage
#define ETHERNET_IP_DATAGRAMM	ethernet->EnetPacketType == 0x0008
#define ETHERNET_ARP_DATAGRAMM 	ethernet->EnetPacketType == 0x0608

#define IP_VERS_LEN 			20
#define UDP_HDR_LEN				8
#define ETH_HDR_LEN 			14
#define UDP_DATA_START			(IP_VERS_LEN + UDP_HDR_LEN + ETH_HDR_LEN)
#define UDP_DATA_END_VAR        (ETH_HDR_LEN + ((eth_buffer[IP_PKTLEN]<<8)+eth_buffer[IP_PKTLEN+1]) - UDP_HDR_LEN + 8)

#define	IP_PKTLEN				0x10
#define TCP_DATA_END_VAR 		(ETH_HDR_LEN + IP_VERS_LEN + ((eth_buffer[IP_PKTLEN]<<8)+eth_buffer[IP_PKTLEN+1]) - ((eth_buffer[TCP_HDRFLAGS_FIX] & 0xF0) >>2))

//----------------------------------------------------------------------------
//Aufbau eines Ethernetheader
//
//
//
//
struct Ethernet_Header	{
	unsigned char EnetPacketDest[6];	//MAC Zieladresse 6 Byte
	unsigned char EnetPacketSrc[6];	//MAC Quelladresse 6 Byte
	unsigned int EnetPacketType;		//Nutzlast 0x0800=IP Datagramm;0x0806 = ARP
};

//----------------------------------------------------------------------------
//Aufbau eines ARP Header
//
//	2 BYTE Hardware Typ				|	2 BYTE Protokoll Typ
//	1 BYTE Länge Hardwareadresse	|	1 BYTE Länge Protokolladresse
//	2 BYTE Operation
//	6 BYTE MAC Adresse Absender		|	4 BYTE IP Adresse Absender
//	6 BYTE MAC Adresse Empfänger	|	4 BYTE IP Adresse Empfänger
struct ARP_Header	{
		unsigned int ARP_HWType;		//Hardware Typ enthält den Code für Ethernet oder andere Link Layer
		unsigned int ARP_PRType;		//Protokoll Typ enthält den Code für IP o. anderes Übertragungsprotokoll
		unsigned char ARP_HWLen;		//Länge der Hardwareadresse enthält 6 für 6 Byte lange MAC Adressen
		unsigned char ARP_PRLen;		//Länge der Protocolladresse enthält 4 für 4 Byte lange IP Adressen
		unsigned int ARP_Op;			//Enthält Code der signalisiert ob es sich um eine Anfrage o. Antwort handelt
		unsigned char ARP_SHAddr[6];	//Enthält die MAC Adresse des Anfragenden
		unsigned long ARP_SIPAddr;    //Enthält die IP Adresse des Absenders
		unsigned char ARP_THAddr[6];	//MAC Adresse Ziel, ist in diesem Fall 6 * 00,da die Adresse erst noch herausgefunden werden soll (ARP Request)
		unsigned long ARP_TIPAddr;    //IP Adresse enthält die IP Adresse zu der die Kommunikation aufgebaut werden soll
};

//----------------------------------------------------------------------------
//Aufbau eines IP Datagramms (B=BIT)
//
//4B Version	|4B Headergr.	|8B Tos	|16B Gesamtlänge in Bytes
//16B Identifikation			|3B Schalter	|13B Fragmentierungsposition
//8B Time to Live	|8B Protokoll	|16B Header Prüfsumme
//32B IP Quelladresse
//32B IB Zieladresse
struct IP_Header	{
	unsigned char	IP_Vers_Len;	//4 BIT Die Versionsnummer von IP,
									//meistens also 4 + 4Bit Headergröße
	unsigned char	IP_Tos;			//Type of Service
	unsigned int	IP_Pktlen;		//16 Bit Komplette Läng des IP Datagrams in Bytes
	unsigned int	IP_Id;			//ID des Packet für Fragmentierung oder
									//Reassemblierung
	unsigned int	IP_Frag_Offset;	//wird benutzt um ein fragmentiertes
									//IP Packet wieder korrekt zusammenzusetzen
	unsigned char	IP_ttl;			//8 Bit Time to Live die lebenszeit eines Paket
	unsigned char	IP_Proto;		//Zeigt das höherschichtige Protokoll an
									//(TCP, UDP, ICMP)
	unsigned int	IP_Hdr_Cksum;	//Checksumme des IP Headers
	unsigned long	IP_Srcaddr;		//32 Bit IP Quelladresse
	unsigned long	IP_Destaddr;	//32 Bit IP Zieladresse
};

//----------------------------------------------------------------------------
//Aufbau einer ICMP Nachricht
//
//8 BIT Typ	|8 BIT Code	|16 BIT Prüfsumme
//Je nach Typ			|Nachricht unterschiedlich
//Testdaten
//
//8 BIT Typ = 0 Echo Reply oder 8 Echo request
struct ICMP_Header {
		unsigned char     	ICMP_Type;		//8 bit typ identifiziert Aufgabe der Nachricht
											//0 = Echo Reply 8 = Echo Request
		unsigned char     	ICMP_Code;		//8 Bit Code enthält Detailangaben zu bestimmten Typen
		unsigned int     	ICMP_Cksum;		//16 Bit Prüfsumme enthält die CRC Prüfsumme
		unsigned int     	ICMP_Id;		//2 byte Identifizierung
		unsigned int     	ICMP_SeqNum;	//Sequenznummer
};

//----------------------------------------------------------------------------
//UDP Header Layout
//
//
struct UDP_Header {
	unsigned int 	udp_SrcPort;	//der Quellport für die Verbindung (Socket)
	unsigned int 	udp_DestPort;	//der Zielport für die Verbindung (Socket)
	unsigned int   udp_Hdrlen;
	unsigned int	udp_Chksum;	//Enthält eine Prüfsumme über Header und Datenbereich
};

#endif // #define _STACK_H
