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

#include "para.h"
#include "stack.h"
#include "bootloader/tftp.h"
#include "config.h"
#include "usart.h"

// dont forget to enable usart in config.h
//#define DEBUG usart_write	//mit Debugausgabe
#define DEBUG(...) 			//ohne Debugausgabe

unsigned char myip[4];

unsigned int IP_id_counter = 0;

unsigned char eth_buffer[MTU_SIZE+1];

struct arp_table arp_entry[MAX_ARP_ENTRY];


//----------------------------------------------------------------------------
//Trägt Anwendung in Anwendungsliste ein
void stack_init (void)
{
	//Timer starten
	timer_init();

#if USE_PARAMETERS_FROM_EEPROM
	//IP, NETMASK und ROUTER_IP aus EEPROM auslesen
    (*((unsigned long*)&myip[0])) = para_getip(IP_EEPROM_STORE,MYIP);

	//MAC Adresse setzen
	if ( para_getchar(MAC_EEPROM_STORE) != 0xFF )
		for ( int i=0; i<6; i++ )
			mymac[i] = para_getchar(MAC_EEPROM_STORE+i);
#else
    (*((unsigned long*)&myip[0])) = MYIP;
#endif

	/*NIC Initialisieren*/
	DEBUG_WRITE("\n\rNIC init:");
	ETH_INIT();
	DEBUG_WRITE("Ok\r\n");

#if USE_ENC28J60
	ETH_PACKET_SEND(60,eth_buffer);
	ETH_PACKET_SEND(60,eth_buffer);
#endif

	DEBUG_WRITE("My IP: %1i.%1i.%1i.%1i\r\n\r\n",myip[0],myip[1],myip[2],myip[3]);
}


//----------------------------------------------------------------------------
//ETH get data
void eth_get_data (void)
{
	unsigned int packet_lenght;

	packet_lenght = ETH_PACKET_RECEIVE(MTU_SIZE,eth_buffer);
	/*Wenn ein Packet angekommen ist, ist packet_lenght =! 0*/
	if(packet_lenght > 0)
	{
		packet_lenght = packet_lenght - 4;
		eth_buffer[packet_lenght+1] = 0;
		eth_len = packet_lenght;			// remember length for global access
		check_packet();
	}


}


//----------------------------------------------------------------------------
//Check Packet and call Stack for TCP or UDP
void check_packet (void)
{
	//Pointer auf Ethernet_Header
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&eth_buffer[ETHER_OFFSET];

	//Pointer auf IP_Header
	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];
#if USE_PING
	//Pointer auf ICMP_Header
	struct ICMP_Header *icmp;
	icmp = (struct ICMP_Header *)&eth_buffer[ICMP_OFFSET];
#endif

	if(ETHERNET_ARP_DATAGRAMM) {
		//Erzeugt ein ARP Reply Packet
		arp_reply();
	} else {
		if(ETHERNET_IP_DATAGRAMM && IF_MYIP) {
			arp_entry_add();			//Refresh des ARP Eintrages
#if USE_PING
			//Ist protokoll Byte = 1 dann ist es ein ICMP Packet
			if(IP_ICMP_PACKET) {
				switch ( icmp->ICMP_Type ) {
				case (0x08):
					//Echo-Request empfangen, erzeugen eines ICMP Reply Packet (PING Echo)
					icmp_send(ip->IP_Srcaddr,0x00,0x00,icmp->ICMP_SeqNum,icmp->ICMP_Id);
					break;
				case (0x00):
					//Echo-Reply Packet empfangen, Empfang melden
					//TODO: Erst Sequenznummer vergleichen?, Zeitmessung?
					DEBUG_WRITE("%i",(ip->IP_Srcaddr&0x000000FF));
					DEBUG_WRITE(".%i",((ip->IP_Srcaddr&0x0000FF00)>>8));
					DEBUG_WRITE(".%i",((ip->IP_Srcaddr&0x00FF0000)>>16));
					DEBUG_WRITE(".%i",((ip->IP_Srcaddr&0xFF000000)>>24));
					DEBUG_WRITE(": PONG!\r\n");
					break;
				}
				return;
			}
			else
#endif
			{
				if(IP_UDP_PACKET) udp_socket_process();
			}
		}
	}
	return;
}

//----------------------------------------------------------------------------
//erzeugt einen ARP - Eintrag wenn noch nicht vorhanden
void arp_entry_add (void)
{
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&eth_buffer[ETHER_OFFSET];

	struct ARP_Header *arp;
	arp = (struct ARP_Header *)&eth_buffer[ARP_OFFSET];

	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];

	//Eintrag schon vorhanden?
	for (unsigned char a = 0;a<MAX_ARP_ENTRY;a++)
	{
		if(ETHERNET_ARP_DATAGRAMM)
		{
			if(arp_entry[a].arp_t_ip == arp->ARP_SIPAddr)
			{
			return;
			}
		}
		if(ETHERNET_IP_DATAGRAMM)
		{
			if(arp_entry[a].arp_t_ip == ip->IP_Srcaddr)
			{
			return;
			}
		}
	}

	//Freien Eintrag finden
	for (unsigned char b = 0;b<MAX_ARP_ENTRY;b++)
	{
		if(arp_entry[b].arp_t_ip == 0)
		{
			if(ETHERNET_ARP_DATAGRAMM)
			{
				for(unsigned char a = 0; a < 6; a++)
				{
					arp_entry[b].arp_t_mac[a] = ethernet->EnetPacketSrc[a];
				}
				arp_entry[b].arp_t_ip = arp->ARP_SIPAddr;
				return;
			}
			if(ETHERNET_IP_DATAGRAMM)
			{
				for(unsigned char a = 0; a < 6; a++)
				{
					arp_entry[b].arp_t_mac[a] = ethernet->EnetPacketSrc[a];
				}
				arp_entry[b].arp_t_ip = ip->IP_Srcaddr;
				return;
			}

		DEBUG("No ARP/IP packet!\r\n");
		return;
		}
	}
	//Eintrag konnte nicht mehr aufgenommen werden
	DEBUG("ARP table full!\r\n");
	return;
}

//----------------------------------------------------------------------------
//Diese Routine such anhand der IP den ARP eintrag
char arp_entry_search (unsigned long dest_ip)
{
	for (unsigned char b = 0;b<MAX_ARP_ENTRY;b++)
	{
		if(arp_entry[b].arp_t_ip == dest_ip)
		{
			return(b);
		}
	}
	return (MAX_ARP_ENTRY);
}

//----------------------------------------------------------------------------
//Diese Routine Erzeugt ein neuen Ethernetheader
void new_eth_header (unsigned char *buffer,unsigned long dest_ip)
{
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&buffer[ETHER_OFFSET];

	unsigned char b = arp_entry_search (dest_ip);

	if (b != MAX_ARP_ENTRY) //Eintrag gefunden wenn ungleich
	{
		for(unsigned char a = 0; a < 6; a++)
		{
			//MAC Destadresse wird geschrieben mit MAC Sourceadresse
			ethernet->EnetPacketDest[a] = arp_entry[b].arp_t_mac[a];
			//Meine MAC Adresse wird in Sourceadresse geschrieben
			ethernet->EnetPacketSrc[a] = mymac[a];
		}
		return;
	}
	DEBUG("No ARP entry found\r\n");

	for(unsigned char a = 0; a < 6; a++)
	{
		//MAC Destadresse wird geschrieben mit MAC Sourceadresse
		ethernet->EnetPacketDest[a] = 0xFF;
		//Meine MAC Adresse wird in Sourceadresse geschrieben
		ethernet->EnetPacketSrc[a] = mymac[a];
	}
	return;

}

//----------------------------------------------------------------------------
//Diese Routine Antwortet auf ein ARP Packet
void arp_reply (void)
{
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&eth_buffer[ETHER_OFFSET];

	struct ARP_Header *arp;
	arp = (struct ARP_Header *)&eth_buffer[ARP_OFFSET];

	//2 Byte Hardware Typ: Enthält den Code für Ethernet
	if(		arp->ARP_HWType == 0x0100 &&

			//2 Byte Protokoll Typ: Enthält den Code für IP
			arp->ARP_PRType == 0x0008  &&

			//1Byte Länge der Hardwareadresse:Enthält 6 für 6 Byte MAC Addresse
			arp->ARP_HWLen == 0x06 &&

			//1Byte Länge der Protokolladresse:Enthält 4 für 4 Byte Adressen
			arp->ARP_PRLen == 0x04 &&

			//Ist das ARP Packet für meine IP Addresse bestimmt
			//Vergleiche ARP Target IP Adresse mit meiner IP
			arp->ARP_TIPAddr == *((unsigned long*)&myip[0]))
	{
		//Operation handelt es sich um eine anfrage
		if (arp->ARP_Op == 0x0100)
		{
			//Rechner Eingetragen wenn noch nicht geschehen?
			arp_entry_add();

			new_eth_header (eth_buffer, arp->ARP_SIPAddr); //Erzeugt ein neuen Ethernetheader

			ethernet->EnetPacketType = 0x0608; //Nutzlast 0x0800=IP Datagramm;0x0806 = ARP

			unsigned char b = arp_entry_search (arp->ARP_SIPAddr);
			if (b != MAX_ARP_ENTRY) //Eintrag gefunden wenn ungleich
			{
				for(unsigned char a = 0; a < 6; a++)
				{
					//ARP MAC Targetadresse wird geschrieben mit ARP Sourceadresse
					arp->ARP_THAddr[a] = arp_entry[b].arp_t_mac[a];
					//ARP MAC Sourceadresse wird geschrieben mit My MAC Adresse
					arp->ARP_SHAddr[a] = mymac[a];
				}
			}
			else
			{
				DEBUG("No ARP entry found\r\n");//Unwarscheinlich das das jemals passiert!
			}

			//ARP operation wird auf 2 gesetzt damit der andere merkt es ist ein ECHO
			arp->ARP_Op = 0x0200;
			//ARP Target IP Adresse wird geschrieben mit ARP Source IP Adresse
			arp->ARP_TIPAddr = arp->ARP_SIPAddr;
			//Meine IP Adresse wird in ARP Source IP Adresse geschrieben
			arp->ARP_SIPAddr = *((unsigned long *)&myip[0]);

			//Nun ist das ARP-Packet fertig zum Senden !!!
			//Sendet das erzeugte ARP Packet
			ETH_PACKET_SEND(ARP_REPLY_LEN,eth_buffer);
			return;
		}
		//es handelt sich um ein REPLY von einem anderen Client
		if (arp->ARP_Op == 0x0200)
		{
			//Rechner Eingetragen wenn noch nicht geschehen?
			arp_entry_add();

			DEBUG("ARP REPLY!\r\n");
		}
	}
	return;
}

//----------------------------------------------------------------------------
//Diese Routine erzeugt ein ARP Request
char arp_request (unsigned long dest_ip)
{
	unsigned char buffer[ARP_REQUEST_LEN];
	unsigned char index = 0;
	unsigned long dest_ip_store;

	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&buffer[ETHER_OFFSET];

	struct ARP_Header *arp;
	arp = (struct ARP_Header *)&buffer[ARP_OFFSET];

	dest_ip_store = dest_ip;

	//Nutzlast 0x0800=IP Datagramm;0x0806 = ARP
	ethernet->EnetPacketType = 0x0608;

	new_eth_header (buffer,dest_ip);

	//Meine IP Adresse wird in ARP Source IP Adresse geschrieben
	arp->ARP_SIPAddr = *((unsigned long *)&myip[0]);

	//Ziel IP wird in Dest IP geschrieben
	arp->ARP_TIPAddr = dest_ip;

	for(unsigned char count = 0; count < 6; count++)
	{
		  arp->ARP_SHAddr[count] = mymac[count];
		  arp->ARP_THAddr[count] = 0x00;
	}

	arp->ARP_HWType = 0x0100;
	arp->ARP_PRType = 0x0008;
	arp->ARP_HWLen 	= 0x06;
	arp->ARP_PRLen 	= 0x04;
	arp->ARP_Op 	= 0x0100;

	//Nun ist das ARP-Packet fertig zum Senden !!!
	//Sendet das erzeugte ARP Packet
	ETH_PACKET_SEND(ARP_REQUEST_LEN, buffer);

	for(unsigned char count = 0;count<20;count++)
	{
		unsigned char index_tmp = arp_entry_search(dest_ip_store);
		index = arp_entry_search(dest_ip);
		if (index < MAX_ARP_ENTRY || index_tmp < MAX_ARP_ENTRY)
		{
//			DEBUG("ARP EINTRAG GEFUNDEN!\r\n");
			if (index_tmp < MAX_ARP_ENTRY) return(1);//OK
			arp_entry[index].arp_t_ip = dest_ip_store;
			return(1);//OK
		}
		for(unsigned long a=0;a<10000;a++){asm volatile ("nop" ::);};
		eth_get_data();
		DEBUG("*No ARP entry found*\r\n");
	}
	return(0);//keine Antwort
}

#if USE_PING
//----------------------------------------------------------------------------
//Diese Routine erzeugt ein neues ICMP Packet
void icmp_send (unsigned long dest_ip, unsigned char icmp_type,
                unsigned char icmp_code, unsigned int icmp_sn,
                unsigned int icmp_id)
{
	//Variablen zur Berechnung der Checksumme
	unsigned int result16;

	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];

	struct ICMP_Header *icmp;
	icmp = (struct ICMP_Header *)&eth_buffer[ICMP_OFFSET];

	//Das ist ein Echo Reply Packet
	icmp->ICMP_Type   = icmp_type;
	icmp->ICMP_Code   = icmp_code;
	icmp->ICMP_Id     = icmp_id;
	icmp->ICMP_SeqNum = icmp_sn;

	//Berechnung der ICMP Checksumme
	//Alle Daten im ICMP Header werden addiert checksum wird deshalb
	//ersteinmal auf null gesetzt
	icmp->ICMP_Cksum = 0x0000;

	//Hier wird erstmal der IP Header neu erstellt

	ip->IP_Pktlen = 0x5400;                 // 0x54 = 84
 	ip->IP_Proto  = PROT_ICMP;
	make_ip_header (eth_buffer,dest_ip);

	//Berechnung der ICMP Header lÃ¤nge
	result16 = LBBL_ENDIAN_INT(ip->IP_Pktlen);
	result16 = result16 - ((ip->IP_Vers_Len & 0x0F) << 2);

	//pointer wird auf das erste Packet im ICMP Header gesetzt
	//jetzt wird die Checksumme berechnet
	result16 = checksum (&icmp->ICMP_Type, result16, 0);

	//schreibt Checksumme ins Packet
	icmp->ICMP_Cksum = LBBL_ENDIAN_INT(result16);

	//Sendet das erzeugte ICMP Packet

    ETH_PACKET_SEND(ICMP_REPLY_LEN,eth_buffer);
}
#endif

//----------------------------------------------------------------------------
//Diese Routine erzeugt eine Cecksumme
unsigned int checksum (unsigned char *pointer,unsigned int result16,unsigned long result32)
{
	unsigned int result16_1 = 0x0000;
	unsigned char DataH;
	unsigned char DataL;

	//Jetzt werden alle Packete in einer While Schleife addiert
	while(result16 > 1)
	{
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH=*pointer++;

		//schreibt Inhalt Pointer nach DATAL danach inc Pointer
		DataL=*pointer++;

		//erzeugt Int aus Data L und Data H
		result16_1 = ((DataH << 8)+DataL);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
		//decrimiert Länge von TCP Headerschleife um 2
		result16 -=2;
	}

	//Ist der Wert result16 ungerade ist DataL = 0
	if(result16 > 0)
	{
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH=*pointer;
		//erzeugt Int aus Data L ist 0 (ist nicht in der Berechnung) und Data H
		result16_1 = (DataH << 8);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
	}

	//Komplementbildung (addiert Long INT_H Byte mit Long INT L Byte)
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));
	result16 =~(result32 & 0x0000FFFF);

	return (result16);
}

//----------------------------------------------------------------------------
//Diese Routine erzeugt ein IP Packet
void make_ip_header (unsigned char *buffer,unsigned long dest_ip)
{

	//------------------------------------------------------------------------
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&buffer[ETHER_OFFSET];
	new_eth_header (buffer, dest_ip); //Erzeugt ein neuen Ethernetheader
	ethernet->EnetPacketType = 0x0008; //Nutzlast 0x0800=IP
	struct IP_Header *ip;
	//------------------------------------------------------------------------

	//Variablen zur Berechnung der Checksumme
	unsigned int result16;

	ip = (struct IP_Header *)&buffer[IP_OFFSET];

	//don't fragment
	ip->IP_Frag_Offset = 0x0040;

	//max. hops
	ip->IP_ttl = 128;
	IP_id_counter++;
	ip->IP_Id = LBBL_ENDIAN_INT(IP_id_counter);
	ip->IP_Vers_Len = 0x45;	//4 BIT Die Versionsnummer von IP,
							//meistens also 4 + 4Bit Headergröße
	ip->IP_Tos = 0x00;

	//unsigned int	IP_Pktlen;		//16 Bit Komplette Läng des IP Datagrams in Bytes
	//unsigned char	IP_Proto;		//Zeigt das höherschichtige Protokoll an
									//(TCP, UDP, ICMP)

	//IP Destadresse wird geschrieben mit IP Sourceadresse
	//das packet soll ja zurückgeschickt werden :-)
	ip->IP_Destaddr	= dest_ip;
	ip->IP_Srcaddr	= *((unsigned long *)&myip[0]);

	//Berechnung der IP Checksumme
	//Alle Daten im IP Header werden addiert checksum wird deshalb
	//ersteinmal auf null gesetzt
	ip->IP_Hdr_Cksum = 0x0000;

	//Berechnung der IP Header länge
	result16 = (ip->IP_Vers_Len & 0x0F) << 2;

	//jetzt wird die Checksumme berechnet
	result16 = checksum (&ip->IP_Vers_Len, result16, 0);

	//schreibt Checksumme ins Packet
	ip->IP_Hdr_Cksum = LBBL_ENDIAN_INT(result16);
	return;
}


//----------------------------------------------------------------------------
//Diese Routine verwaltet die UDP Ports
void udp_socket_process(void)
{
	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];

	struct UDP_Header *udp;
	udp = (struct UDP_Header *)&eth_buffer[UDP_OFFSET];

	//zugehörige Anwendung ausführen - hier immer der tftp-client
	DEBUG("UDP Anwendung gefunden an Port %i, Src-Port %i!\r\n", LBBL_ENDIAN_INT(udp->udp_DestPort), LBBL_ENDIAN_INT(udp->udp_SrcPort));

	tftp_get(ip->IP_Srcaddr, LBBL_ENDIAN_INT(udp->udp_SrcPort));
	return;
}

//----------------------------------------------------------------------------
//Diese Routine Erzeugt ein neues UDP Packet
void create_new_udp_packet(	unsigned int data_length,
							unsigned int src_port,
							unsigned int dest_port,
							unsigned long dest_ip)
{
	DEBUG("UDP wird gesendet an %i.%i.%i.%i:%u, Len %u!\r\n", (int)(dest_ip&0xFF), (int)(dest_ip>>8&0xFF), (int)(dest_ip>>16&0xFF), (int)(dest_ip>>24), dest_port, data_length);
    unsigned int result16;
	unsigned long result32;

	struct UDP_Header *udp;
	udp = (struct UDP_Header *)&eth_buffer[UDP_OFFSET];

	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];

	udp->udp_SrcPort = LBBL_ENDIAN_INT(src_port);
	udp->udp_DestPort = LBBL_ENDIAN_INT(dest_port);

	//UDP Packetlänge
	data_length = UDP_HDR_LEN + data_length;
	udp->udp_Hdrlen = LBBL_ENDIAN_INT(data_length);
	//IP Headerlänge + UDP Headerlänge
	data_length = IP_VERS_LEN + data_length;
	//Hier wird erstmal der IP Header neu erstellt
	ip->IP_Pktlen = LBBL_ENDIAN_INT(data_length);
	data_length += ETH_HDR_LEN;
	ip->IP_Proto = PROT_UDP;
	make_ip_header (eth_buffer,dest_ip);

	//Alle Daten im UDP Header werden addiert checksum wird deshalb
	//ersteinmal auf null gesetzt
	udp->udp_Chksum = 0;

	//Berechnet Headerlänge und Addiert Pseudoheaderlänge 2XIP = 8
	result16 = LBBL_ENDIAN_INT(ip->IP_Pktlen) + 8;
	result16 = result16 - ((ip->IP_Vers_Len & 0x0F) << 2);
	result32 = result16 + 0x09;

	//Routine berechnet die Checksumme
	result16 = checksum ((&ip->IP_Vers_Len+12), result16, result32);
	udp->udp_Chksum = LBBL_ENDIAN_INT(result16);

	//Sendet das erzeugte UDP Packet
    ETH_PACKET_SEND(data_length,eth_buffer);
	return;
}
