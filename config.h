/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:
 known Problems: none
 Version:        03.11.2007
 Description:    Webserver Config-File
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

#ifndef _CONFIG_H_
	#define _CONFIG_H_

	//ETH_M32_EX (www.ulrichradig.de) and Pollins AVR-NET-IO
	#define USE_ENC28J60			1

	//Holger Buss (www.mikrocontroller.com) Mega32-Board
	#define USE_RTL8019		0

	// use Pollins AVR NET IO board (change two pins to ENC28J60)
	#define USE_AVRNETIO	1

	//Umrechnung von IP zu unsigned long
	#define IP(a,b,c,d) ((unsigned long)(d)<<24)+((unsigned long)(c)<<16)+((unsigned long)(b)<<8)+a

	// Set 0 here if unsure
	#define USE_PARAMETERS_FROM_EEPROM		0

	//IP des Webservers
	#define MYIP		IP(192,168,1,90)

	//IP des Syslog-Servers
	#define SYSLOG_IP	IP(192,168,1,136)

	//IP und Filename für TFTP bootloader
	#define TFTP_IP		IP(192,168,1,136)
	#define TFTP_NAME	"avr.hex"
	#define TFTP_MSGMSK	IP(3,0,0,0)			// dummy IP, only first number is used

	//Ping - answer pings while bootloader is active
	// disabled due to space limitations
	#define USE_PING	0

	//MAC Adresse des Webservers
	#define MAC				{0x00,0x22,0xf9,0x01,0x0c,0x65}

	//Timertakt intern oder extern
	#define EXTCLOCK 0 //0=Intern 1=Externer Uhrenquarz

	//Baudrate der seriellen Schnittstelle
	#define BAUDRATE 9600

	//Use Console for Debug and Messages
	#define USE_CONSOLE	1

	// console devices
	#define USE_TELNET	0		// can't be used in bootloader
	#define USE_SYSLOG	1		// destination syslog for messages and errors
	#define USE_USART	1		// destination usart

	// setting the debug output function
	//#define DEBUG_WRITE console_write
	#define DEBUG_WRITE(...)

	#define USE_USARTWRITE	0		//printf like usart_write

#endif //_CONFIG_H


