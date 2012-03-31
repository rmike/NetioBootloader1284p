/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:
 known Problems: none
 Version:        24.10.2007
 Description:    RTL8019 Treiber
----------------------------------------------------------------------------*/

#include "rtl8019.h"

#if USE_RTL8019
unsigned char mymac[6] = MAC;

//----------------------------------------------------------------------------
// Schreibt ein Byte in ein angegebenes Register der Netzwerkkarte
void WriteRTL (unsigned char rtl_addr,unsigned char rtl_data)
{
	//Der Datenport wird auf output gesetzt vom µC zum NIC schreiben
	DATA_CTRL_RLT = OUTPUT;
	//write Adress to Port
	ADDR_PORT_RLT = rtl_addr;
	//write Data to Port
	DATA_PORT_RLT_WRITE = rtl_data;
	//set write Pin to Low
	RTL_WR_OFF();
	//wait a short time
        nop(); //gib mal ein bißchen GAS Ulrich ;)
        nop();

	//set write Pin to High
	RTL_WR_ON();
	//write to RTL complete
}


//----------------------------------------------------------------------------
// Auslesen eines Byte von einen angegebenes Register der Netzwerkkarte
unsigned char ReadRTL (unsigned char rtl_addr)
{
	unsigned char rtl_data;
	//Der Datenport wir auf input gesetzt vom NIC zum µC lesen
	DATA_CTRL_RLT = INPUT;
	//write Adress to Port
	ADDR_PORT_RLT = rtl_addr;
	//set read Pin to Low
	RTL_RD_OFF();
	//wait a short time
	nop();
	nop();
	//read from port rtldata
	rtl_data = DATA_PORT_RLT_READ;
	//set Read Pin to High
	RTL_RD_ON();
	//read from RTL complete
	return (rtl_data);
}

//----------------------------------------------------------------------------
//Hier wird die Netzwerkkarte Grundinizialisiert
void Init_Realtek_Network_Card (void)
{

#if defined (__AVR_ATmega128__)
	//Set Addr Port Direction = Output
	DATA_ADDR_RLT = OUTPUT;
#endif

#if defined (__AVR_ATmega32__)
	//Set Addr Port Direction = Output
	DATA_ADDR_RLT = OUTPUT;
#endif

#if defined (__AVR_ATmega644__)
	//Set Addr Port Direction = Output
	DATA_ADDR_RLT = OUTPUT;
#endif
	//Set all Addrport High
#ifndef ISA_CTRL
	ADDR_PORT_RLT = OUTPUT;
#else
	// für ISP-CTRL
	ADDR_PORT_RLT = ADDR_OUTPUT;                  // nur die 5 Adressleitungen
	CTRL_LINES_DIR |=  (1<<READ_PIN);
    CTRL_LINES_DIR |= (1<<RESET_PIN);
	CTRL_LINES_DIR |= (1<<WRITE_PIN);
#endif
	//set Data Port Direction = Input
	DATA_CTRL_RLT = INPUT;

	//printf ("\nInit Network Card: ");

	//Set Reset Pin Low
	RTL_RESET_OFF();

	//added by holgi
	//meine Karten laufen nur wenn man hier noch ca. 100ms wartet
	//wait a long time
	delay_ms(100);
	//end added by holgi

	ReadRTL (RSTPORT);
	WriteRTL (RSTPORT , 0xFF);

	delay_ms(10);

	WriteRTL (CR , (1<<STP| 1<<RD2));

	delay_ms(10);

	WriteRTL (DCR , DCRVAL);
	WriteRTL (RBCR0 , 0x00);
	WriteRTL (RBCR1 , 0x00);
	WriteRTL (RCR , 0x04);
	WriteRTL (TPSR , RXSTART);
	WriteRTL (TCR , 0x02);
	WriteRTL (PSTART , RXSTART);
	WriteRTL (BNRY , RXSTART);
	WriteRTL (PSTOP , RXSTOP);
	WriteRTL (CR ,(1<<STP| 1<<RD2|1<<PS0));

	delay_ms(10);

	WriteRTL (CURR , RXSTART);

	for(int a = 0; a<=5; a++)
	{
		WriteRTL ((PAR0 + a) , mymac[a]);
	}

	WriteRTL (CR , (1<<STP| 1<<RD2));
	WriteRTL (DCR , DCRVAL);
	WriteRTL (CR , (1<<STA| 1<<RD2));
	WriteRTL (RTL_ISR ,(1<<PRX|1<<PTX|1<<RXE|1<<TXE|1<<OVW|1<<CNT|1<<RDC|1<<RST));
	WriteRTL (IMR , IMRVAL);
	WriteRTL (TCR , TCRVAL);
	//printf (" Init Ready! \n");

	//startet die Network Card
	WriteRTL (CR ,(1<<STA| 1<<RD2));
    DATA_PORT_RLT_WRITE = 255; //Schreibt eine 255 auf den BUS
	if((ReadRTL(0)) == 34)
	{
		DEBUG("\n\rNIC: Init RTL8019: Ok\r\n");
	}
	else
	{
		DEBUG("\n\rNIC: Init RTL8019: ERR\n\r");
	}
	ETH_INT_TRIGGER_MODE;
}


//----------------------------------------------------------------------------
void Write_Ethernet_Frame (unsigned int bufferlen,unsigned char *buffer)
{
 unsigned int a;

	bufferlen = bufferlen;
	//Buffer auf eine min. länge von 60Bytes überprüfen
	if (bufferlen < 0x40)
	{
		bufferlen = 0x40;
	}

	WriteRTL (CR,(1<<STA| 1<<RD2));
	WriteRTL (TPSR,TXSTART);
	WriteRTL (RSAR0,0x00);
	WriteRTL (RSAR1,0x40);
	WriteRTL (RTL_ISR,(1<<PRX|1<<PTX|1<<RXE|1<<TXE|1<<OVW|1<<CNT|1<<RDC|1<<RST));
	WriteRTL (RBCR0,(bufferlen & 0x00ff));
	WriteRTL (RBCR1,(bufferlen & 0xff00) >>8);
	WriteRTL (CR,(1<<STA| 1<<RD1));

	//Der Datenport wir auf output gesetzt vom µC zum NIC schreiben
	DATA_CTRL_RLT = OUTPUT;
	//write Adress to Port
	ADDR_PORT_RLT = RDMAPORT;

	for (a = 0; a < (bufferlen);a++)
	{
		//write Data to Port
		DATA_PORT_RLT_WRITE = buffer[a];
		//set write Pin to Low
		RTL_WR_OFF();
		//wait a short time
		nop();
		nop();
		//set write Pin to High
		RTL_WR_ON();
		//write to RTL complete
	}

	while ((ReadRTL(RTL_ISR)&(1<<RDC)) == 0) {}

	WriteRTL (TBCR0,(bufferlen & 0x00ff));
	WriteRTL (TBCR1,(bufferlen & 0xff00) >>8);
	WriteRTL (CR,(1<<TXP| 1<<RD2));
}

//----------------------------------------------------------------------------
unsigned int Read_Ethernet_Frame (unsigned int bufferlen_max, unsigned char *buffer)
{
	unsigned int bufferlen;
	unsigned int tmp1,tmp2;
	unsigned int a;
	unsigned char by = 0;

	WriteRTL(CR ,(1<<STA| 1<<RD0| 1<<RD1));
	ReadRTL(RDMAPORT);
	ReadRTL(RDMAPORT);
	tmp1 = ReadRTL(RDMAPORT);
	bufferlen = (ReadRTL(RDMAPORT)<<8)+tmp1;

	//Der Datenport wir auf input gesetzt vom NIC zum µC lesen
	DATA_CTRL_RLT = INPUT;
	//write Adress to Port
	ADDR_PORT_RLT = RDMAPORT;

    tmp1=bufferlen;	//Annehmen *bufferlen passt in den Buffer
    tmp2=0;		//Rest ist erstmal 0

    if(bufferlen>( bufferlen_max-1)) //Mehr Daten vorhanden als in den Buffer passen
	{
		tmp1=( bufferlen_max-1);    //Daten nur bis MTU_SIZE-1 in den Buffer übernehmen
		tmp2=bufferlen-tmp1; //den Rest abholen, aber ignorieren !
    }

    //Daten in den Buffer übernehmen solange sie reinpassen
    for(a=0; a<tmp1; a++)
	{
		//set read Pin to Low
		RTL_RD_OFF();
		//wait a short time
		nop();
		nop();
        //read from port rtldata
		buffer[a] = DATA_PORT_RLT_READ;
		//set Read Pin to High
		RTL_RD_ON();
		//read from RTL complete
	}

	//Daten abholen die NICHT mehr in den Buffer passen
	//Anmerkung holgi: Kommt das überhaupt einmal vor ?
	for(a=0; a<tmp2; a++)
	{
		//set read Pin to Low
		RTL_RD_OFF();
		//wait a short time
		nop();
		nop();
		//read from port rtldata
		by = DATA_PORT_RLT_READ;
		//set Read Pin to High
		RTL_RD_ON();
		//read from RTL complete
	}
	buffer[tmp1 + 1] = 0;

	by = by & RDC;

	if ( by != 64 )
	{
		ReadRTL(RTL_ISR);
	}

	return (bufferlen);
}
#endif
