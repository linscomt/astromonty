
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "SpiDriver.h"

static SPI_DEVICE_LIST SPI_Devices[SPI_DEV_COUNT];
static BYTE DeviceBusy[SPI_PORT_COUNT];
static BYTE CurDevID[SPI_PORT_COUNT];


static BYTE GetPrescale(DWORD Speed)
{
    double Div;
    WORD IntDiv;
    WORD T = 1;
    BYTE PPS = 0;
    BYTE SPS = 0;
    BYTE Shift=0;
    Div=40000000.0/((double)Speed);
    IntDiv=(WORD)Div+1;
    while(T>0){
        T = IntDiv>>Shift;
        if((T>=1)&&(T<=8)) {
            PPS = 1<<(Shift);
            SPS = (BYTE)T;
            break;
        }  
        Shift += 2;                   
    }    
    return 0;
}

BYTE SPI_Init(void)
{
    WORD i;	
	for(i=0;i<SPI_DEV_COUNT;i++){		
		SPI_Devices[i].DevID=i;
		SPI_Devices[i].Port = 0;
		SPI_Devices[i].Status = 0;
		SPI_Devices[i].DevSelect = 0;
		SPI_Devices[i].DevDeselect = 0;
	}	
	for(i=0;i<SPI_PORT_COUNT;i++){
    	DeviceBusy[i]=0;
    	CurDevID[i]=0;
    }   
    GetPrescale(800000);	
    return 0;
}

static void Call_Dev(void* Dev)
{
    asm volatile (              
        "call %0 \n": :"r"(Dev)
    ); 
}




/*
����������� ������ ���������� �� ���� SPI
���������� DevId �� ������� ������� ����������
���������:
port   - ����� ����� SPI
DevOn  - ����� ������� ��� ������ ����������
DevOff - ����� ������� ��� ������������ ����������
*/
BYTE SPI_RegDevice(BYTE port,void* DevOn,void* DevOff) 
{   
    //���� ��������� ������� ������� 
    BYTE i;    
    BYTE j; 
    if((DevOn==0)||(DevOff==0)) return 2;//������������ ������� ������ ����������
    for(i=0;i<SPI_DEV_COUNT;i++){
        if(SPI_Devices[i].Status==0){
            SPI_Devices[i].Status = SPI_DEV_ACTIVE;
            SPI_Devices[i].DevSelect = DevOn;
		    SPI_Devices[i].DevDeselect = DevOff;            
            return 0; //��������� �������
        }         
    }
    return 1; // ��� ��������� ������� ��� ����������
}

/*
������ ������ � �����������
*/
BOOL SPI_Open(BYTE DevId)
{
    BYTE Port;
    if(SPI_Devices[DevId].Status&SPI_DEV_ACTIVE!=1)
        return 0;    
    Port = SPI_Devices[DevId].Port;
    //���� ���� ����������� ����������
    if(CurDevID[Port]!=DevId){
        while(DeviceBusy[Port]){
            Nop();       
        }
    }    
    //��������� ����
    DeviceBusy[Port]=1;
    CurDevID[Port]=DevId;
    SPI_Devices[DevId].Status |= SPI_DEV_CURRENT;
    //������������� ���������
    
    //SPIxSTATbits.SPIEN ��������� SPI
    
    
    switch(Port){
        case 1:
            
        break;        
        case 2:
            
        break;
    }
    
}
