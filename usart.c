/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:
 known Problems: none
 Version:        24.10.2007
 Description:    RS232 Routinen

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
#include "limits.h"
#include "usart.h"
#include "config.h"

//volatile unsigned int buffercounter = 0;

//char usart_rx_buffer[BUFFER_SIZE];
//char *rx_buffer_pointer_in	= &usart_rx_buffer[0];
//char *rx_buffer_pointer_out	= &usart_rx_buffer[0];

//----------------------------------------------------------------------------
//Init serielle Schnittstelle
//void usart_init(unsigned long baudrate)
void usart_init(void)
{
	//Serielle Schnittstelle 1
  	//Enable TXEN im Register UCR TX-Data Enable
//	UCR =(1 << TXEN | 1 << RXEN | 1<< RXCIE);
	UCR =(1 << TXEN | 1 << RXEN);
	// 0 = Parity Mode Disabled
	// 1 = Parity Mode Enabled, Even Parity
	// 2 = Parity Mode Enabled, Odd Parity
	//UCSRC = 0x06 + ((parity+1)<<4);
	//UCSRC |= (1<<USBS);
	//Teiler wird gesetzt
//	UBRR=(F_CPU / (baudrate * 16L) - 1);
	UBRR=(F_CPU / (BAUDRATE * 16L) - 1);
}

//----------------------------------------------------------------------------
//Routine für die Serielle Ausgabe eines Zeichens (Schnittstelle0)
void usart_write_char(char c)
{
	//Warten solange bis Zeichen gesendet wurde
    while(!(USR & (1<<UDRE)));
    //Ausgabe des Zeichens
    UDR = c;
}

//----------------------------------------------------------------------------
//Ausgabe eines Strings
void usart_write_str(char *str)
{
	while (*str)
		usart_write_char(*str++);
}

#if USE_USARTWRITE
//------------------------------------------------------------------------------
// formatierte Ausgabe per USART, nur als Debug Möglichkeit, nutze sonst console_write
void usart_write_P (const char *Buffer,...)
{
	va_list ap;
	va_start (ap, Buffer);

	int format_flag;
	char str_buffer[20];
	char str_null_buffer[20];
	char move = 0;
	int tmp = 0;
	char by;
	char *ptr;

	//Ausgabe der Zeichen
    for(;;)
	{
		by = *Buffer++;
		if(by==0) break; // end of format string

		if (by == '%')
		{
			by = *Buffer++;

			if (isdigit(by)>0)
				{

 				str_null_buffer[0] = by;
				str_null_buffer[1] = '\0';
				move = atoi(str_null_buffer);

				by = *Buffer++;
				}

			int b=0;
			switch (by)
				{
                case 's':
                    ptr = va_arg(ap,char *);
                    while(*ptr) { usart_write_char(*ptr++); }
                    break;
				case 'c':
					//Int to char
					format_flag = va_arg(ap,int);
					usart_write_char (format_flag++);
					break;
				case 'b':
					itoa(va_arg(ap,int),str_buffer,2);
					goto ConversionLoop;
				case 'i':
					itoa(va_arg(ap,int),str_buffer,10);
					goto ConversionLoop;
				case 'u':
					utoa(va_arg(ap,unsigned int),str_buffer,10);
					goto ConversionLoop;
				case 'l':
					ltoa(va_arg(ap,long),str_buffer,10);
					goto ConversionLoop;
				case 'o':
					itoa(va_arg(ap,int),str_buffer,8);
					goto ConversionLoop;
				case 'x':
					itoa(va_arg(ap,int),str_buffer,16);
					//****************************
					ConversionLoop:
					//****************************
					while (str_buffer[b++] != 0){};
					b--;
					if (b<move)
						{
						move -=b;
						for (tmp = 0;tmp<move;tmp++)
							{
							str_null_buffer[tmp] = '0';
							}
						//tmp ++;
						str_null_buffer[tmp] = '\0';
						strcat(str_null_buffer,str_buffer);
						strcpy(str_buffer,str_null_buffer);
						}
					usart_write_str (str_buffer);
					move =0;
					break;
				}

			}
		else
		{
			usart_write_char ( by );
		}
	}
	va_end(ap);
}

#endif
