/*----------------------------------------------------------------------------
 Copyright:      Jens Mundhenke
 Author:         Jens Mundhenke
 Remarks:		 Used on an simplified IP-stack of Ulrich Radig
 known Problems:
 Version:        18.01.2009
 Description:    Interpreter und Flasher für Intelhex-Zeilen

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

#ifndef _IHEX_H
#define _IHEX_H

// errorcodes
#define IHEX_NOERR		0		// hurray
#define IHEX_ERRCHAR	1		// non-Hexachar to convert
#define IHEX_ERRLEN		2		// to much bytes in line for databuffer size
#define IHEX_ERRSTATE	3		// unkown state, should never happen
#define IHEX_ERRCHECK	4		// checksum was wrong
#define IHEX_ERRLINE	5		// line is too long
#define IHEX_ERRTYPE	6		// unsupported line type
#define IHEX_ERRADDR	7		// address out of range

#define IHEX_DATAMAX	16		// max. number of bytes in one line

// arguments for ihex_intvector
#define IHEX_INTVECTOR_STD	0	// use standard vectortable
#define IHEX_INTVECTOR_BOOT	1	// use vectortable of bootloader

// functions
void ihex_init (void);			// has to be called before using the other functions
char ihex_flash ( char c );		// interpret c an a part of an intel hex file
void ihex_intvector(char i);	// switch interrupt vector table betw. standard and boot
void ihex_appstart(void);		// start the application

#endif
