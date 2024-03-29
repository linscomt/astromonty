/*********************************************************************
 *
 *  Berekely TCP lient demo application.
 *  This application uses the BSD socket APIs and start a client
 *
 *********************************************************************
 * FileName:        BerkeleyTCPClientDemo.c
 * Company:         Microchip Technology, Inc.
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 * Author               Date    	Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Aseem Swalah         4/17/08  	Original
 ********************************************************************/

#include "TCPIPConfig.h" 

#if defined(STACK_USE_BERKELEY_API)

#include "TCPIP Stack/TCPIP.h"
#include "..\..\dsPIC33\protocol.h"


#define PORTNUM 9764
static ROM BYTE ServerName[] =  "ASTROMONTY";
// This is specific to this HTTP Client example
static BYTE sendRequest[] = "GET /search?as_q=Microchip&as_sitesearch=microchip.com HTTP/1.0\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";

/*********************************************************************
 * Function:        void BerkeleyTCPClientDemo(void)
 *
 * PreCondition:    Stack is initialized()
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void BerkeleyTCPClientDemo(void)
{
	#if defined(STACK_USE_DNS)
    static SOCKET bsdClientSocket;
    static struct sockaddr_in addr;
    char recvBuffer[32];
    int i;
    int addrlen;
    BYTE res = 0;
    BYTE BlobLen;
    static enum
    {
	    DNS_START_RESOLUTION = 0,
	    DNS_GET_RESULT,
        BSD_START,
        BSD_CONNECT,
        //BSD_SEND,
        BSD_OPERATION,
        BSD_CLOSE,
        BSD_DONE
    } BSDClientState = BSD_DONE;

    switch(BSDClientState)
    {
	    case DNS_START_RESOLUTION:
	    	if(DNSBeginUsage())
	    	{
		    	DNSResolveROM(ServerName, DNS_TYPE_A);
		    	BSDClientState = DNS_GET_RESULT;
		    }
	    	break;
	    
	    case DNS_GET_RESULT:
	    	if(!DNSIsResolved((IP_ADDR*)&addr.sin_addr.S_un.S_addr))
	    		break;
	    		
	    	if(!DNSEndUsage())
	    	{
				#if defined(STACK_USE_UART)
				putrsUART((ROM char*)"Could not resolve ServerName[] to IP address.\r\n");
				#endif
		    	BSDClientState = BSD_DONE;
		    	break;
		    }

	    	BSDClientState = BSD_START;			
	    	// No break; here.
	    
        case BSD_START:
            // Create a socket for this client to connect with 
            if((bsdClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET )
                return;
         
			#if defined(STACK_USE_UART)
			putrsUART((ROM char*)"\r\n\r\nConnecting using Berkeley Sockets TCP API...\r\n");
			putrsUART((ROM char*)"   Note: this demo will do nothing if an underlying TCP_PURPOSE_BERKELEY_CLIENT type \r\n"
								 "   socket is unavailable, as declared by the TCPSocketInitializer[] array in \r\n"
								 "   TCPIPConfig.h.\r\n\r\n");
			#endif
         	         
            BSDClientState = BSD_CONNECT;
            break;

        case BSD_CONNECT:
            // addr.sin_addr.S_un.S_addr destination IP address was set earlier in DNS step
            addr.sin_port = PORTNUM;
            addrlen = sizeof(struct sockaddr);
            if(connect( bsdClientSocket, (struct sockaddr*)&addr, addrlen) < 0)
            	return;

            BSDClientState = BSD_OPERATION;
            // No break needed
        
        case BSD_OPERATION:
            // Obtian and print the server reply
            while(1)
            {
	            i = recv(bsdClientSocket, recvBuffer, sizeof(recvBuffer), 0); //get the data from the recv queue
                res = RunClient(recvBuffer, sizeof(recvBuffer), &i);
                switch(res){
	            case STR_NEED_ANSWER:  
	            	send(bsdClientSocket, recvBuffer, i, 0);
	            case STR_OK:
	            	break;
	            case STR_NEED_DISCONNECT:	            	
	            default:
	            	BSDClientState = BSD_CLOSE;
	            	break;
                }     
			    break;
            }
            break;
         
        case BSD_CLOSE:
            closesocket(bsdClientSocket);
            BSDClientState = BSD_DONE;
            // No break needed
            
        case BSD_DONE:
            //if(BUTTON2_IO == 0u)
            	//BSDClientState = DNS_START_RESOLUTION;
            	BSDClientState = BSD_START;
            addr.sin_addr.S_un.S_un_b.s_b1 = 192;
            addr.sin_addr.S_un.S_un_b.s_b2 = 168;
            addr.sin_addr.S_un.S_un_b.s_b3 = 1;
            addr.sin_addr.S_un.S_un_b.s_b4 = 33;
            break;
         
        default:
            return;
    }


	//#if defined(STACK_USE_DNS)
	#else
		#warning You must define STACK_USE_DNS for BerkeleyTCPClientDemo to work
	#endif
}

#endif //#if defined(STACK_USE_BERKELEY_API)

