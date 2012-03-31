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

#include <avr/eeprom.h>
#include <string.h>

#include "config.h"
#include "para.h"

//----------------------------------------------------------------------------
//
unsigned long para_getip (unsigned char *eeprom_address,unsigned long default_value)
{
#if USE_PARAMETERS_FROM_EEPROM

	unsigned char value[4];

	for (unsigned char count = 0; count<4;count++)
	{
		//eeprom_busy_wait ();
		value[count] = eeprom_read_byte(eeprom_address + count);
	}

	//Ist der EEPROM Inhalt leer?
	if ((*((unsigned long*)&value[0])) == 0xFFFFFFFF)
	{
		return(default_value);
	}
	return((*((unsigned long*)&value[0])));

#else

	return(default_value);

#endif
}



//----------------------------------------------------------------------------
// reading a string parameter from EEPROM
void para_getstring (char *name, unsigned char *eeprom_address, char len, char *default_value)
{
#if USE_PARAMETERS_FROM_EEPROM

	int i;
	char c;
	i = 0;

	// copy string from eeprom to memory
	do {
		c = eeprom_read_byte (eeprom_address+i);
		name[i] = c;
	} while ( (c != 0) && (c != 0xFF) && (i++ < len) );

	// if there is empty eeprom or no termination, copy default value
	if ( (c == 0xFF) || (i > len) )
		strcpy ( name, default_value );

#else

	strcpy ( name, default_value );

#endif
}

//------------------------------------------------------------------------------
//
unsigned char para_getchar (unsigned char *eeprom_address)
{
	return ( eeprom_read_byte (eeprom_address) );
}
