#include "stdafx.h"
#include "Queue.h"
#include "uCmdProcess.h"
#define SPIFLASH_CS_TRIS 
#include "..\dsPIC33\Microchip\Include\TCPIP Stack\SPIFlash.h"


xCMD_QUEUE      uCmdQueueValues1[uCMD_QUEUE_SIZE];     // values
QUEUE_ELEMENT   uCMDQueueKeys1[uCMD_QUEUE_SIZE];    // keys
xCMD_QUEUE      mCmdQueueValues1[mCMD_QUEUE_SIZE];     // values
QUEUE_ELEMENT   mCMDQueueKeys1[uCMD_QUEUE_SIZE];    // keys
OC_CHANEL_STATE OC1;



int SetDirection(OC_ID id, BYTE Dir);

int uCmd_Init()
{
    //                              
    // ������� -> ������� ������ -> ���� ������� -> ������� ������� -> ������������ -> ������� ����������� -> ����������
    // 0. ��������� �������
    Queue_Init(&OC1.uCmdQueue, uCMDQueueKeys1, uCMD_QUEUE_SIZE, (BYTE*)uCmdQueueValues1, sizeof(xCMD_QUEUE));
    Queue_Init(&OC1.mCmdQueue, mCMDQueueKeys1, mCMD_QUEUE_SIZE, (BYTE*)mCmdQueueValues1, sizeof(xCMD_QUEUE));
    // 1. ������� ��������� �� flash
    // 2. ��������� OC, DMA
    // 3. 
    // 

    // ��� ������
    xCMD_QUEUE command;
    command.State = xCMD_STOP;
    command.Value = (DWORD)0x0000;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_SET_TIMER;
    command.Value = (DWORD)OC_TMR3;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_START;
    command.Value = (DWORD)0x0000;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_ACCELERATE;
    command.Value = (DWORD)0x0001;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_RUN;
    command.Value = (DWORD)0x0001;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_DECELERATE;
    command.Value = (DWORD)0x0001;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_STOP;
    command.Value = (DWORD)0x0000;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);

    uCmd_OCCallback((void*)&OC1);
    uCmd_OCCallback((void*)&OC1);
    uCmd_OCCallback((void*)&OC1);
    uCmd_OCCallback((void*)&OC1);
    command.State = xCMD_START;
    command.Value = (DWORD)0x0000;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_ACCELERATE;
    command.Value = (DWORD)0x0000;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_DECELERATE;
    command.Value = (DWORD)0x0001;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    command.State = xCMD_STOP;
    command.Value = (DWORD)0x0000;    
    Queue_Insert(&OC1.uCmdQueue, 10, (BYTE*)&command);
    uCmd_OCCallback((void*)&OC1);
    uCmd_OCCallback((void*)&OC1);
    uCmd_OCCallback((void*)&OC1);
    uCmd_OCCallback((void*)&OC1);

    
    return 0;
}

// ��� ������, ����������� � ������,
// ������������ ������� �� ������������
int uCmd_OCCallback( void * _This )
{
    OC_CHANEL_STATE * OCN = (OC_CHANEL_STATE*)_This;
    xCMD_QUEUE * Value = NULL;
    BYTE ProcessCmd = 1;
    xCMD_QUEUE StopCommand;
    StopCommand.State = xCMD_STOP;
    StopCommand.Value = (DWORD)0x0000;    

    while (ProcessCmd){

        if(Queue_First( &OCN->uCmdQueue, NULL, (BYTE**)&Value) != 0){
            // ���� ��� �������, �� �������
            Value = &StopCommand;
            ProcessCmd = 0;
        }

        switch (Value->State) {

        case xCMD_ACCELERATE:
        case xCMD_RUN:
        case xCMD_DECELERATE: // �������� Value - ���������� �����, ������� ��������� � �����
            (OCN->CurrentDirection == 0) ? OCN->XPosition++ : OCN->XPosition--;
            if(Value->Value > 0){
                Value->Value--;
                ProcessCmd = 0;
                OCN->CurrentState = Value->State;
            }
            break;

        case xCMD_SET_TIMER: // �������� Value - �������� ���� OC_TMR_SELECT - ����� �������
            OCSetTmr(OCN->Config.OCConfig.Index, (OC_TMR_SELECT)Value->Value);
            OCN->CurrentState = Value->State;
            break;

        case xCMD_SET_DIRECTION: // �������� Value - ����������� ��������
            OCN->CurrentDirection = Value->Value;
            OCN->CurrentState = Value->State;
            SetDirection(OCN->Config.OCConfig.Index, (BYTE)Value->Value);            
            break;

        case xCMD_STOP: // ��������� ������
            OCN->CurrentState = Value->State;
            OCSetMode(OCN->Config.OCConfig.Index, OC_DISABLED);
            DMADisable(OCN->Config.DmaId);
            break;

        case xCMD_START: // ������ ������
            DMAEnable(OCN->Config.DmaId);
            DMAForceTransfer(OCN->Config.DmaId);
            OCSetMode(OCN->Config.OCConfig.Index, OCN->Config.OCConfig.WorkMode);
            //ProcessCmd = 0;
            break;

        case xCMD_EMERGENCY_STOP:
            OCN->CurrentState = Value->State;
            OCSetMode(OCN->Config.OCConfig.Index, OC_DISABLED);
            DMADisable(OCN->Config.DmaId);
            ProcessCmd = 0;
            break;

        default:
            OCN->CurrentState = xCMD_ERROR;
            break;
        }
        if(ProcessCmd != 0)
            Queue_Delete(&OCN->uCmdQueue);
    }
    return 0;
}
#define WAIT_READ_COMPLETE 0
#define NO_WAIT_READ_COMPLETE 0
int uCmd_DMACallback( void * _This, BYTE* Buf, WORD BufLen)
{
    OC_CHANEL_STATE * OCN = (OC_CHANEL_STATE*)_This;
    xCMD_QUEUE * Value = NULL;
    BYTE ProcessCmd = 1;
    BYTE Priority = 0;
    xCMD_QUEUE Command;
    WORD TmpBuf1[8];
    WORD TmpBuf2[8];
    WORD * TmpBufPtr = TmpBuf1;
    WORD Addr = 0;
    WORD i = 0;
    WORD j = 0;
    WORD Pulse = OCN->Config.wSTEPPulseWidth;
    OC_BUF * BufPtr = (OC_BUF*)Buf;
    Command.State = xCMD_STOP;
    Command.Value = (DWORD)0x0000;    

    while (ProcessCmd){

        if(Queue_ExtractAllToMin( &OCN->mCmdQueue, &Priority, (BYTE**)&Value ) != 0){
            // ���� ��� �������, �� �������
            Value = &Command;
            ProcessCmd = 0;
        }

        switch (Value->State) {

        case xCMD_ACCELERATE:
            Value->Value;  // ��� ����� AccX �� �������� ���� �����
            OCN->mCMD_Status.AccX; // ��� ������� ����� AccX
            OCN->Config.AccBaseAddress; // ����� ������� �������
            OCN->Config.AccRecordCount; // ���������� ������� � ������� �������
            SPIFlashReadArray(Addr , (BYTE*)TmpBuf1, 16, WAIT_READ_COMPLETE);
            for (i = 0; i < 8; i++) {
                if(i & 1 == 0){
                    SPIFlashReadArray(Addr , (BYTE*)TmpBuf2, 16, NO_WAIT_READ_COMPLETE);
                    TmpBufPtr = TmpBuf1;
                } else {
                    SPIFlashReadArray(Addr , (BYTE*)TmpBuf1, 16, NO_WAIT_READ_COMPLETE);
                    TmpBufPtr = TmpBuf2;
                }
                for(j = 0; j< 8; j++){
                    BufPtr->r = *TmpBufPtr;
                    BufPtr->rs = *TmpBufPtr + Pulse;
                    BufPtr++;
                    TmpBufPtr++;
                }
            }
            break;
        case xCMD_RUN:
            break;
        case xCMD_DECELERATE:
            break;
        case xCMD_SET_TIMER:
            Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Value);
            break;
        case xCMD_SET_DIRECTION:
            Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Value);
            break;
        case xCMD_STOP:
            Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Value);
            break;
        case xCMD_START:
            Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Value);
            break;
        case xCMD_EMERGENCY_STOP:
            // ��� ��������
            break;
        default:
            break;
        }
    }
    return 0;
}

//------------------------------------------------------------------------------------------------
int SetDirection(OC_ID id, BYTE Dir)
//------------------------------------------------------------------------------------------------
{
#ifdef __C30__
    switch ( id ) {
    case ID_OC1:
        PORT1_DIR = Dir;
        break;
    case ID_OC2: 
        PORT2_DIR = Dir;
        break;
    case ID_OC3: 
        PORT3_DIR = Dir;
        break;
    case ID_OC4: 
        PORT4_DIR = Dir;
        break;
    case ID_OC5: 
        PORT5_DIR = Dir;
        break;
    case ID_OC6: 
        PORT6_DIR = Dir;
        break;
    case ID_OC7: 
        PORT7_DIR = Dir;
        break;
    case ID_OC8:
        PORT8_DIR = Dir;
    	break;
    }
#endif
    return 0;
}
