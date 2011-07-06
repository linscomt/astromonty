
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "SpiDriver.h"

static SPI_DEVICE_LIST SPI_Devices[SPI_DEV_COUNT];
static BYTE DeviceBusy[SPI_PORT_COUNT];
static BYTE CurDevID[SPI_PORT_COUNT];
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
        return false;    
    Port = SPI_Devices[DevId].Port;
    //���� ���� ����������� ����������
    if(CurDevID[Port]!=DevId){
        while(DeviceBusy[Port]){
            Nop();       
        }
    }    
    //��������� ����
    DeviceBusy[Port]=1;
    CurDevId[Port]=DevId;
    SPI_Devices[DevId].Status |= SPI_DEV_CURRENT;
    //������������� ���������
    switch(Port){
        case 1:
            
        break;        
        case 2:
            
        break;
    }
    
}