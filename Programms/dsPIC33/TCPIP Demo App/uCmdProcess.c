#include "stdafx.h"

#include "device_control.h"
#include "Queue.h"
#include "uCmdProcess.h"
#include <math.h>

#ifdef __C30__
#   define INTERRUPT void __attribute__((__interrupt__,__no_auto_psv__))
#include "TCPIP Stack/TCPIP.h"
#include "TCPIP Stack/SPIFlash.h"
#include "GenericTypeDefs.h"
extern APP_CONFIG AppConfig;
#else
#   define INTERRUPT void
#define Nop()
extern _IFS0bits IFS0bits;
typedef enum _WAIT_READY
{
    NO_WAIT_READ_COMPLETE = 0,
    WAIT_READ_COMPLETE = 1,
} WAIT_READY;

void SPIFlashReadArray(DWORD dwAddress, BYTE* vData, WORD wLen, WAIT_READY WaitData);
void SPIFlashWriteArray(BYTE* vData, WORD wLen);
void SPIFlashBeginWrite(DWORD Addr);

APP_CONFIG AppConfig;

DWORD dwFlashAddr = 0;
unsigned char FileSystem[64*1024*256];
#define __attribute__(far) 
#endif

QUEUE_ELEMENT   BufferQueueKeys1[BUFFER_QUEUE_SIZE];
BYTE            BufferValues1[BUFFER_QUEUE_SIZE][OC1_DMA_BUF_LEN] __attribute__ ((far));
QUEUE_ELEMENT   BufferQueueKeys2[BUFFER_QUEUE_SIZE];
BYTE            BufferValues2[BUFFER_QUEUE_SIZE][OC2_DMA_BUF_LEN] __attribute__ ((far));

QUEUE_ELEMENT   CMDQueueKeys[CMD_QUEUE_SIZE];
CMD_QUEUE       CMDQueueValues[CMD_QUEUE_SIZE];
PRIORITY_QUEUE  CmdQueue;                  // ������� ������
xCMD_QUEUE      uCmdQueueValues1[uCMD_QUEUE_SIZE];     // values
QUEUE_ELEMENT   uCMDQueueKeys1[uCMD_QUEUE_SIZE];    // keys
xCMD_QUEUE      mCmdQueueValues1[mCMD_QUEUE_SIZE];     // values
QUEUE_ELEMENT   mCMDQueueKeys1[uCMD_QUEUE_SIZE];    // keys
xCMD_QUEUE      uCmdQueueValues2[uCMD_QUEUE_SIZE];     // values
QUEUE_ELEMENT   uCMDQueueKeys2[uCMD_QUEUE_SIZE];    // keys
xCMD_QUEUE      mCmdQueueValues2[mCMD_QUEUE_SIZE];     // values
QUEUE_ELEMENT   mCMDQueueKeys2[uCMD_QUEUE_SIZE];    // keys
OC_CHANEL_STATE OControll1;
OC_CHANEL_STATE OControll2;
//OC_CHANEL_STATE OControll3;

int uCmd_Init(void);
int xCmdStart(BYTE Id);
int SetDirection(OC_ID id, BYTE Dir);
int mCmd_DMACallback( void * _This, BYTE* Buf, WORD BufLen);
int uCmd_OCCallback(void * _This);
int uCmd_ICCallback(void * _This);
double GetInterval(double X, double K, double B);
double QuadraticEquationRoot1(double A, double B, double C);
int Aim(OC_CHANEL_STATE * OCN, LONG destang, BYTE priority);
int mCmd_Process( OC_CHANEL_STATE * OCN, WORD BufLen);

int GetDestAngWGuide(double TgtRefPos, double TgtSpeed, CHANEL_CONFIG * Config, LONG * DestAng);

#ifndef __C30__
void SPIFlashReadArray(DWORD dwAddress, BYTE* vData, WORD wLen, WAIT_READY WaitData)
{
    memcpy(vData, &FileSystem[dwAddress], wLen);
}
void SPIFlashBeginWrite(DWORD Addr)
{
    dwFlashAddr = Addr;
}
void SPIFlashWriteArray(BYTE* vData, WORD wLen)
{
    memcpy(&FileSystem[dwFlashAddr], vData, wLen);
}

extern WORD OC1R;
extern WORD OC2R;

#endif


int xCmdStart(BYTE Id)
{
    OC_CHANEL_STATE * OCN;
    if(Id == 0){
        OCN = &OControll1;
    } else if(Id == 1){
        OCN = &OControll2;
    } else return 0;

    if((OCN->CurrentState == xCMD_STOP) || (OCN->CurrentState == xCMD_EMERGENCY_STOP)){
        DMADisable(OCN->Config->DMAConfig.DmaId);
        OCSetMode(OCN->Config->OCConfig.Index, OC_DISABLED);
        IFS0bits.U1TXIF = 1;
        while(IFS0bits.U1TXIF){
            Nop();
            Nop();
        }
        DMASetDataCount(OCN->Config->DMAConfig.DmaId, OCN->Config->DMAConfig.DMABufSize);
        DMAPrepBuffer(OCN->Config->DMAConfig.DmaId);
        DMAEnable(OCN->Config->DMAConfig.DmaId);
        DMAForceTransfer(OCN->Config->DMAConfig.DmaId);
        OCInit(OCN->Config->OCConfig.Index,OCN->Config->OCConfig.OCxCon);
        return 0;
    } else return -1;
}

int uCmd_Init(void)
{
    // ��� ������
    //xCMD_QUEUE command;
    WORD DMAcfg;
    WORD OCConfig;
    BYTE * Buf1;
    BYTE * Buf2;
    BYTE i = 0;
    OC_CHANEL_STATE * nOC;

    //                              
    // ������� -> ������� ������ -> ���� ������� -> ������� ������� -> ������������ -> ������� ����������� -> ����������
    // 0. ��������� �������
    Queue_Init(&OControll1.uCmdQueue, uCMDQueueKeys1, uCMD_QUEUE_SIZE, (BYTE*)uCmdQueueValues1, (BYTE)sizeof(xCMD_QUEUE));
    Queue_Init(&OControll1.mCmdQueue, mCMDQueueKeys1, mCMD_QUEUE_SIZE, (BYTE*)mCmdQueueValues1, (BYTE)sizeof(xCMD_QUEUE));
    Queue_Init(&OControll1.BufferQueue, BufferQueueKeys1, BUFFER_QUEUE_SIZE, (BYTE*)BufferValues1, OC1_DMA_BUF_LEN);

    Queue_Init(&OControll2.uCmdQueue, uCMDQueueKeys2, uCMD_QUEUE_SIZE, (BYTE*)uCmdQueueValues2, (BYTE)sizeof(xCMD_QUEUE));
    Queue_Init(&OControll2.mCmdQueue, mCMDQueueKeys2, mCMD_QUEUE_SIZE, (BYTE*)mCmdQueueValues2, (BYTE)sizeof(xCMD_QUEUE));
    Queue_Init(&OControll2.BufferQueue, BufferQueueKeys2, BUFFER_QUEUE_SIZE, (BYTE*)BufferValues2, OC2_DMA_BUF_LEN);

    OControll1.Config = &AppConfig.ChanellsConfig[0];
    OControll1.CurrentDirection = 0;
    OControll2.Config = &AppConfig.ChanellsConfig[1];
    OControll2.CurrentDirection = 0;
    OControll1.Pulse = 0;
    OControll2.Pulse = 0;
    // TODO: ��������
    OControll1.XPosition = 288000;
    OControll2.XPosition = 288000;

    //uCmd_DefaultConfig(&AppConfig.ChanellsConfig[0], 0);
#ifdef __C30__
    T2CONbits.TON = 0;
    T3CONbits.TON = 0;
#else
    uCmd_DefaultConfig(&AppConfig.ChanellsConfig[0], 0);
    CreateAccTable();

    {
        LONG DestAng = 0;
        GetDestAngWGuide(10.0, 1.0, &AppConfig.ChanellsConfig[0], &DestAng);
        GetDestAngWGuide(10.0, -1.0, &AppConfig.ChanellsConfig[0], &DestAng);
        GetDestAngWGuide(-10.0, 1.0, &AppConfig.ChanellsConfig[0], &DestAng);
        GetDestAngWGuide(-10.0, -1.0, &AppConfig.ChanellsConfig[0], &DestAng);
        GetDestAngWGuide(30.0, 1.0, &AppConfig.ChanellsConfig[0], &DestAng);
        GetDestAngWGuide(30.0, -1.0, &AppConfig.ChanellsConfig[0], &DestAng);
        GetDestAngWGuide(-30.0, 1.0, &AppConfig.ChanellsConfig[0], &DestAng);
        GetDestAngWGuide(-30.0, -1.0, &AppConfig.ChanellsConfig[0], &DestAng);
    }
#endif 


    TimerInit(TIMER2, CLOCK_SOURCE_INTERNAL, GATED_DISABLE, PRE_1_64, IDLE_ENABLE, BIT_16, SYNC_DISABLE);
    TimerSetValue(TIMER2, 0x0000, 0xFFFF);
    TimerSetCallback(TIMER2, NULL);
    TimerSetInt(TIMER2, 0, FALSE);

    TimerInit(TIMER3, CLOCK_SOURCE_INTERNAL, GATED_DISABLE, PRE_1_64, IDLE_ENABLE, BIT_16, SYNC_DISABLE);
    TimerSetValue(TIMER3, 0x0000, 0xFFFF);
    TimerSetCallback(TIMER3, NULL);
    TimerSetInt(TIMER3, 0, FALSE);

    //TimerSetState(TIMER3, TRUE); // ������� ��������� ����� �������
    //TimerSetState(TIMER2, TRUE);
#ifdef __C30__
    T2CONbits.TON = 1;
    T3CONbits.TON = 1;
#endif

    DMAInit();
    DMAcfg = DMACreateConfig(SIZE_WORD, RAM_TO_DEVICE, FULL_BLOCK, NORMAL_OPS, REG_INDIRECT_W_POST_INC, CONTINUE_PP);

    OCConfig = OCCreateConfig(IDLE_DISABLE, OC_TMR2, TOGGLE);
    //OCInit(ID_OC1, OCConfig);
    OCSetInt(ID_OC1, OC_INT_LEVEL, TRUE);

    DMASetConfig(OC1_DMA_ID, DMAcfg);
    Buf1 = DMAGetBuffer(OC1_DMA_BUF_LEN);
    Buf2 = DMAGetBuffer(OC1_DMA_BUF_LEN);
    DMASelectDevice(OC1_DMA_ID, IRQ_OC1, (int)&OC1R);
    DMASetInt(OC1_DMA_ID, OC_DMA_INT_LEVEL, TRUE);
    DMASetBuffers(OC1_DMA_ID, Buf1, Buf2);
    DMASetDataCount(OC1_DMA_ID, OC1_DMA_BUF_LEN);

    //OCInit(ID_OC2, OCConfig);
    OCSetInt(ID_OC2, OC_INT_LEVEL, TRUE);

    DMASetConfig(OC2_DMA_ID, DMAcfg);
    Buf1 = DMAGetBuffer(OC2_DMA_BUF_LEN);
    Buf2 = DMAGetBuffer(OC2_DMA_BUF_LEN);
    DMASelectDevice(OC2_DMA_ID, IRQ_OC2, (int)&OC2R);
    DMASetInt(OC2_DMA_ID, OC_DMA_INT_LEVEL, TRUE);
    DMASetBuffers(OC2_DMA_ID, Buf1, Buf2);
    DMASetDataCount(OC2_DMA_ID, OC2_DMA_BUF_LEN);



    for (i = 0; i < 2; i++){
        if(i == 0){
            nOC = &OControll1;
        } else {
            nOC = &OControll2;
        }
        nOC->Config->DMAConfig.DmaCfg = DMAcfg;
        nOC->Config->OCConfig.OCxCon = OCConfig;
        nOC->T = 0;
        nOC->Pulse = 0;
        nOC->mCMD_Status.AccX = 0;
        nOC->mCMD_Status.RUN_Interval = 0;
        nOC->mCMD_Status.State = xCMD_STOP;
        nOC->mCMD_Status.Priority = 255;

        if(nOC->Config->OCConfig.Index == ID_OC1){
            OCSetCallback(ID_OC1, (void*)&OControll1,  uCmd_OCCallback);
            DMASetCallback(OC1_DMA_ID, (void*)&OControll1, mCmd_DMACallback, mCmd_DMACallback);
            nOC->Config->DMAConfig.DMABufSize = OC1_DMA_BUF_LEN;
        } else 
        if(nOC->Config->OCConfig.Index == ID_OC2){
            OCSetCallback(ID_OC2, (void*)&OControll2,  uCmd_OCCallback);
            DMASetCallback(OC2_DMA_ID, (void*)&OControll2, mCmd_DMACallback, mCmd_DMACallback);
            nOC->Config->DMAConfig.DMABufSize = OC2_DMA_BUF_LEN;
        }
    }
#ifdef __C30__
    MS1         = 1;    // ����� MS1
    MS2         = 1;     // ����� MS2
    SLEEP       = 1;     // ����� SLEEP
    RESET       = 1;     // ����� RESET

    MS1_Tris    = 0;     // ����� MS1
    MS2_Tris    = 0;     // ����� MS2
    SLEEP_Tris  = 0;    // ����� SLEEP
    RESET_Tris  = 0;    // ����� RESET

    //������������� �����1
    {
        PORT1_NULL_Tris   = 1; // ���� NULL
        PORT1_POS_Tris    = 1; // ���� POS
        PORT1_POS2_Tris   = 1; // ���� POS2

        PORT1_ENABLE      = 0;// ����� ENABLE
        PORT1_DIR         = 0;// ����� DIR
        PORT1_STEP        = 0;// ����� STEP

        PORT1_ENABLE_Tris = 0;// ����� ENABLE
        PORT1_DIR_Tris    = 0;// ����� DIR
        PORT1_STEP_Tris   = 0;// ����� STEP
    }

    //������������� �����2
    {
        PORT2_NULL_Tris   = 1; // ���� NULL
        PORT2_POS_Tris    = 1; // ���� POS
        PORT2_POS2_Tris   = 1; // ���� POS2

        PORT2_ENABLE      = 0;// ����� ENABLE
        PORT2_DIR         = 0;// ����� DIR
        PORT2_STEP        = 0;// ����� STEP

        PORT2_ENABLE_Tris = 0;// ����� ENABLE
        PORT2_DIR_Tris    = 0;// ����� DIR
        PORT2_STEP_Tris   = 0;// ����� STEP
    }
    // U1TX
    IFS0bits.U1TXIF = 0;
    IEC0bits.U1TXIE = 1;
    IPC3bits.U1TXIP = 1;

#endif
    
    return 0;
}

// ��� ������, ����������� � ������,
// ������������ ������� �� ������������
int uCmd_OCCallback( void * _This )
{
    OC_CHANEL_STATE * OCN = (OC_CHANEL_STATE*)_This;
    xCMD_QUEUE * Value = NULL;
    xCMD_QUEUE StopCommand;
    BYTE ProcessCmd = 1;
    
    
    if(OCN->Pulse){
        OCN->Pulse = 0;
        if(OCN->CurrentState != xCMD_STOP){
            return 0;
        }
    }
    

    /*
    switch(OCN->Config->OCConfig.Index){
    case ID_OC1: 
    #ifdef __C30__
        if(PORTDbits.RD0 == 1) return 0; 
    #endif
        break;
    case ID_OC2: 
    #ifdef __C30__
        if(PORTDbits.RD1 == 1) return 0; 
    #endif
        break;
    default:
        break;
    }
    */
    while (ProcessCmd){
        // ������������ ������� ������, ���� OCN->CurrentState == xCMD_SLOW_RUN

        if(Queue_First( &OCN->uCmdQueue, NULL, (BYTE**)&Value) != 0){
            // ���� ��� �������, �� �������
            if((OCN->CurrentState != xCMD_STOP) && (OCN->CurrentState != xCMD_EMERGENCY_STOP)){
                StopCommand.State = xCMD_STOP;
                StopCommand.Value = (DWORD)0x0000;
                Value = &StopCommand;
                ProcessCmd = 0;
            } else {
                return -1;
            }
        }

        switch (Value->State) {

        case xCMD_ACCELERATE:
        case xCMD_RUN:
        case xCMD_SLOW_RUN:
        case xCMD_DECELERATE: // �������� Value - ���������� �����, ������� ��������� � �����
            if(Value->Value > 0){
                (OCN->CurrentDirection == 0) ? OCN->XPosition++ : OCN->XPosition--;
                Value->Value--;
                ProcessCmd = 0;
                OCN->CurrentState = Value->State;
                OCN->Pulse = 1;
            } else {
                Queue_Delete(&OCN->uCmdQueue);
            }
            break;
        case xCMD_SET_TIMER: // �������� Value - �������� ���� OC_TMR_SELECT - ����� �������
            OCSetMode(OCN->Config->OCConfig.Index, OC_DISABLED);
            OCSetTmr(OCN->Config->OCConfig.Index, (OC_TMR_SELECT)Value->Value);
            OCSetMode(OCN->Config->OCConfig.Index, OCN->Config->OCConfig.WorkMode);
            OCN->CurrentState = Value->State;
            Queue_Delete(&OCN->uCmdQueue);
            break;

        case xCMD_SET_DIRECTION: // �������� Value - ����������� ��������
            OCN->CurrentDirection = (BYTE)Value->Value;
            OCN->CurrentState = Value->State;
            SetDirection(OCN->Config->OCConfig.Index, (BYTE)Value->Value);
            Queue_Delete(&OCN->uCmdQueue);
            break;

        case xCMD_STOP: // ��������� ������
            if(OCN->CurrentState != xCMD_STOP){
                OCN->CurrentState = Value->State;
                OCSetMode(OCN->Config->OCConfig.Index, OC_DISABLED);
                DMADisable(OCN->Config->DMAConfig.DmaId);
            }
            Queue_Delete(&OCN->uCmdQueue);
            //ProcessCmd = 0;
            break;

        default:
            OCN->CurrentState = xCMD_ERROR;
            Queue_Delete(&OCN->uCmdQueue);
            break;
        }
    }
    return 0;
}

int mCmd_DMACallback( void * _This, BYTE* Buf, WORD BufLen)
{
    OC_CHANEL_STATE * OCN = (OC_CHANEL_STATE*)_This;

    if(Queue_ExtractMin( &OCN->BufferQueue, NULL, Buf ) !=0 ){
        Nop();
        Nop();
    }

    // �������� ����������
    IFS0bits.U1TXIF = 1;
    
    return 0;
}
#define TMP_SIZE 64
#define TMP_SIZE_MASK TMP_SIZE-1
#define bTMP_SIZE TMP_SIZE*2

// ������� ���������� �������, ����������� � ������� uCmdQueue �, ��������� � ���, ������� BufferQueue, �������� mCmd_Process
int mCmd_RevertCmd( OC_CHANEL_STATE * OCN)
{
    xCMD_QUEUE * Value = NULL;
    //xCMD_QUEUE * Value1 = NULL;
    BYTE Priority = 0;
    //DWORD DataSize = 0;
    volatile WORD BufCount = TMP_SIZE; // ��������� ���� � ������ � ��������� ( (WORD)r + (WORD)rs )
    //WORD TmpT = 0;
    OC_BUF * BufPtr = NULL;

    while (1){


        if(OCN->CurrentState == xCMD_SLOW_RUN){
            // �� xCMD_SLOW_RUN ���������� �� �����
            if(OCN->uCmdQueue.Count == 0 )
                return 0;
        } else {
            // ���� 2 ��� ����� ������� ������, �� ��������� �����
            if(OCN->BufferQueue.Count <= 2 )
                return 0;
        }
        
        if(Queue_Last( &OCN->uCmdQueue, &Priority, (BYTE**)&Value ) != 0){
            return -1;
        }

        OCN->mCMD_Status.State = Value->State;
        //TmpT = OCN->T;
        switch (Value->State) {
            case xCMD_ACCELERATE:
                OCN->mCMD_Status.AccX -= (WORD)Value->Value;
                BufCount -= (WORD)Value->Value;
                LOCK(OC_INT_LEVEL, Queue_Revert(&OCN->uCmdQueue));
                break;
            case xCMD_DECELERATE:
                OCN->mCMD_Status.AccX += (WORD)Value->Value;
                BufCount -= (WORD)Value->Value;
                LOCK(OC_INT_LEVEL, Queue_Revert(&OCN->uCmdQueue));
                break;
            case xCMD_RUN:
            case xCMD_SLOW_RUN:
                BufCount -= (WORD)Value->Value;
                LOCK(OC_INT_LEVEL, Queue_Revert(&OCN->uCmdQueue));
                break;
            default:
                LOCK(OC_INT_LEVEL, Queue_Revert(&OCN->uCmdQueue));
                break;
        }

        if(BufCount == 0){
            if(Queue_Last( &OCN->BufferQueue, NULL, (BYTE**)&BufPtr ) != 0){
                return -1;
            }
            OCN->T = BufPtr[TMP_SIZE - 1].r;
            LOCK(OC_DMA_INT_LEVEL, Queue_Revert(&OCN->BufferQueue));
            BufCount = TMP_SIZE;
        }
    }
}

int mCmd_Process( OC_CHANEL_STATE * OCN, WORD BufLen)
{


    xCMD_QUEUE * Value = NULL;
    BYTE ProcessCmd = 1;
    BYTE Priority = 0;
    xCMD_QUEUE Command;
    WORD TmpBuf1[TMP_SIZE];
    //WORD TmpBuf2[TMP_SIZE];
    WORD * TmpBufPtr = NULL;
    DWORD Addr = 0;
    WORD i = 0;
    //WORD j = 0;
    WORD Pulse = OCN->Config->wSTEPPulseWidth;
    //OC_BUF * BufPtr;
    DWORD DataSize = 0;
    //BYTE BufLoad = 0;
    //BYTE BufProcess = 0;
    //BYTE NeedStart = 0;
    WORD BufCount = TMP_SIZE; // ��������� ���� � ������ � ��������� ( (WORD)r + (WORD)rs )
    //WORD Buf1Cnt = 0;           // ���������� ������ � ������ 1
    //WORD Buf2Cnt = 0;           // ���������� ������ � ������ 2
   // WORD TmpSize2 = 0;
    WORD TmpSize = 0;              
    //BYTE FirstPass = 1;         // ������ ������
    //BYTE Buf1Ready = 0;         // ������� ���������� ������
    //BYTE Buf2Ready = 0;
    //WAIT_READY Buf1Wait = NO_WAIT_READ_COMPLETE;
    //WAIT_READY Buf2Wait = NO_WAIT_READ_COMPLETE;
    WORD Interval = 0;
    OC_BUF Buf[TMP_SIZE];
    OC_BUF * BufPtr = Buf;
    //BYTE CurrInt = 0;
    WORD TmpT = 0;

    while (BufCount){

        if(Queue_ExtractAllToMin( &OCN->mCmdQueue, &Priority, (BYTE**)&Value ) != 0){
            // ���� ��� �������, �� �������
            //Command.State = xCMD_STOP;
            //Command.Value = (DWORD)0x0000; 
            //Value = &Command;
            //ProcessCmd = 0;
            return -1;
        }

        if(OCN->mCMD_Status.Priority > Priority){
            mCmd_RevertCmd(OCN);
        }

        if((OCN->BufferQueue.Count >= OCN->BufferQueue.Size)||(OCN->uCmdQueue.Count >= OCN->uCmdQueue.Size))
            return -1;
        
        OCN->mCMD_Status.Priority = Priority;
        OCN->mCMD_Status.State = Value->State;
        TmpT = OCN->T;
        switch (Value->State) {

        case xCMD_ACCELERATE:
            // �������� �� ������������ ������ 
            if(OCN->mCMD_Status.AccX > OCN->Config->AccRecordCount){
                OCN->mCMD_Status.AccX = (WORD)OCN->Config->AccRecordCount;
            }
            if(Value->Value > OCN->Config->AccRecordCount){
                Value->Value = OCN->Config->AccRecordCount;
            }
            
            if(Value->Value < OCN->mCMD_Status.AccX){
                Value->State = xCMD_DECELERATE;
                break;
            }

            // ��������� ���������� ������
            DataSize = Value->Value - OCN->mCMD_Status.AccX;
            if (DataSize > BufCount) {
                DataSize = BufCount;
            } 
            // ��������� ������
            Addr = OCN->Config->AccBaseAddress + (OCN->mCMD_Status.AccX) * sizeof(WORD);

            /*
            BufLoad = 0;
            BufProcess = 0;
            Buf1Ready = 0;
            Buf2Ready = 0;
            Buf2Wait = NO_WAIT_READ_COMPLETE;
            Buf1Wait = NO_WAIT_READ_COMPLETE;
            FirstPass = 1;
            */

            // ��������� ���������� ������ �������
            if (DataSize > 0)
            {
                //TmpSize2 = (WORD)DataSize;
                /*
                while(TmpSize2 > 0){
                    if((BufLoad == 0)  && (Buf1Ready == 0) && (TmpSize2 > 0)){
                        if(TmpSize2 > TMP_SIZE){
                            Buf1Cnt = TMP_SIZE;
                        } else {
                            Buf1Cnt = TmpSize2;
                        }
                        TmpSize2 -= Buf1Cnt;
                        if(TmpSize2 == 0) {
                            Buf1Wait = WAIT_READ_COMPLETE;
                            Buf1Ready = 1;
                        }
                        SPIFlashReadArray(Addr, (BYTE*)TmpBuf1, Buf1Cnt * sizeof(WORD), Buf1Wait);
                        Addr += Buf1Cnt * sizeof(WORD);
                        if(FirstPass != 1 ) Buf2Ready = 1; // ������ ������ �� ������ ������
                        BufLoad = 1;
                    }  
                    if((((BufLoad == 1) && (Buf2Ready == 0)) || (FirstPass == 1)) && (TmpSize2 > 0)){
                        if(TmpSize2 > TMP_SIZE){
                            Buf2Cnt = TMP_SIZE;
                        } else {
                            Buf2Cnt = TmpSize2;
                        }
                        TmpSize2 -= Buf2Cnt;
                        if(TmpSize2 == 0) {
                            Buf2Wait = WAIT_READ_COMPLETE;
                            Buf2Ready = 1;
                        }
                        SPIFlashReadArray(Addr, (BYTE*)TmpBuf2, Buf2Cnt * sizeof(WORD) , Buf2Wait);
                        Addr += Buf2Cnt * sizeof(WORD);
                        FirstPass = 0;      // ������ ������
                        Buf1Ready = 1;      // ������ ������ � ������ ������
                        BufLoad = 0;
                    }
                    while(Buf1Ready||Buf2Ready){
                        if(Buf1Ready && (BufProcess == 0)){
                            TmpBufPtr = TmpBuf1;
                            Buf1Ready = 0;
                            TmpSize = Buf1Cnt;
                            BufProcess = 1;
                        } else
                        if(Buf2Ready && (BufProcess == 1)){
                            TmpBufPtr = TmpBuf2;
                            Buf2Ready = 0;
                            TmpSize = Buf2Cnt;
                            BufProcess = 0;
                        }
                        while(TmpSize > 0){
                            Interval = *TmpBufPtr;
                            TmpBufPtr++;
                            OCN->T += Interval;       // �� flash �������� ��������� ( � TmpBufPtr)
                            BufPtr->r = OCN->T;
                            BufPtr->rs = OCN->T + Pulse;
                            if(BufPtr->r == 0) BufPtr->r++; 
                            BufPtr++;
                            TmpSize--;
                        }
                    }
                }*/
                
                SPIFlashReadArray(Addr, (BYTE*)TmpBuf1, (WORD)DataSize * sizeof(WORD), WAIT_READ_COMPLETE);
                Addr += DataSize * sizeof(WORD);
                TmpBufPtr = TmpBuf1;
                TmpSize = (WORD)DataSize;

                while(TmpSize > 0){

                    Interval = *TmpBufPtr;
                    TmpBufPtr++;
                    OCN->T += Interval;       // �� flash �������� ��������� ( � TmpBufPtr)
                    BufPtr->r = OCN->T;
                    BufPtr->rs = OCN->T + Pulse;
                    if(BufPtr->r == 0) BufPtr->r++; 
//                     if(OCN->T <= TmpT){
//                         Nop();
//                         Nop();
//                     }
//                     if(Interval > 1000){
//                         Nop();
//                         Nop();
//                     }
                    TmpT = OCN->T;
                    BufPtr++;
                    TmpSize--;
                }
            
                OCN->mCMD_Status.AccX += (WORD)DataSize;
                BufCount -= (WORD)DataSize;
                OCN->mCMD_Status.RUN_Interval = Interval;   // ��� ���������� ��������� �� �������� ���������� ���������
                // ��������� ������� � �������
                Command.State = Value->State;
                Command.Value = DataSize; 
                //LOCK(OC_DMA_INT_LEVEL, Queue_Insert(&OCN->BufferQueue, Priority, (BYTE*)Buf));
                LOCK(OC_INT_LEVEL, Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Command));

            }
            if(OCN->mCMD_Status.AccX >= Value->Value) {
                Queue_Delete(&OCN->mCmdQueue);
            }
            break;
        case xCMD_SLOW_RUN:
        case xCMD_RUN:

            // ��������� �������� ������
            DataSize = (DWORD)Value->Value;

            if (DataSize > BufCount) {
                DataSize = BufCount;
            }
            if(DataSize < BufCount){
                Nop();
                Nop();
            }
            if (DataSize > 0){
                for (i = 0; i < DataSize; i++) {

                    OCN->T += (WORD)(OCN->mCMD_Status.RUN_Interval);
                    BufPtr->r = OCN->T;
                    BufPtr->rs = OCN->T + Pulse;
                    if(BufPtr->r == 0) BufPtr->r++; 
//                     if(OCN->T <= TmpT){
//                         Nop();
//                         Nop();
//                     }
//                     TmpT = OCN->T;
//                     if(Interval > 1000){
//                         Nop();
//                         Nop();
//                     }
                    BufPtr++;
                }

                BufCount -= (WORD)DataSize;
                Value->Value -= DataSize;
                // ��������� ������� � �������
                Command.State = Value->State;
                Command.Value = DataSize; 
                LOCK(OC_INT_LEVEL, Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Command));

            } 
            if(Value->Value == 0) {
                Queue_Delete(&OCN->mCmdQueue);
            }
            
            break;
        case xCMD_DECELERATE:
            
            // �������� �� ������������ ������ 
            if(OCN->mCMD_Status.AccX > OCN->Config->AccRecordCount){
                OCN->mCMD_Status.AccX = (WORD)OCN->Config->AccRecordCount;
            }
            if(Value->Value > OCN->Config->AccRecordCount){
                Value->Value = OCN->Config->AccRecordCount;
            }

            if(Value->Value > OCN->mCMD_Status.AccX){
                Value->State = xCMD_ACCELERATE;
                break;
            }

            // ��������� ���������� ������
            DataSize = (DWORD)OCN->mCMD_Status.AccX - Value->Value;
            if (DataSize > BufCount) {
                DataSize = BufCount;
            }
            // ��������� ������
            Addr = OCN->Config->AccBaseAddress + ((DWORD)OCN->mCMD_Status.AccX) * sizeof(WORD);

            /*
            BufLoad = 0;
            BufProcess = 0;
            FirstPass = 1;
            Buf1Ready = 0;
            Buf2Ready = 0;
            Buf2Wait = NO_WAIT_READ_COMPLETE;
            Buf1Wait = NO_WAIT_READ_COMPLETE;
            */
            // ��������� ���������� ������ �������
            if (DataSize > 0)
            {

                //TmpSize2 = (WORD)DataSize;
                /* while(TmpSize2 > 0){
                    if((BufLoad == 0)  && (Buf1Ready == 0) && (TmpSize2 > 0)){
                        if(TmpSize2 > TMP_SIZE){
                            Buf1Cnt = TMP_SIZE;
                        } else {
                            Buf1Cnt = TmpSize2;
                        }
                        Addr -= Buf1Cnt * sizeof(WORD);
                        TmpSize2 -= Buf1Cnt;
                        if(TmpSize2 == 0) {
                            Buf1Wait = WAIT_READ_COMPLETE;
                            Buf1Ready = 1;
                        }
                        SPIFlashReadArray(Addr, (BYTE*)TmpBuf1, Buf1Cnt * sizeof(WORD), Buf1Wait);
                        if(FirstPass != 1 ) Buf2Ready = 1; // ������ ������ �� ������ ������
                        BufLoad = 1;
                    }  
                    if((((BufLoad == 1) && (Buf2Ready == 0)) || (FirstPass == 1)) && (TmpSize2 > 0)){
                        if(TmpSize2 > TMP_SIZE){
                            Buf2Cnt = TMP_SIZE;
                        } else {
                            Buf2Cnt = TmpSize2;
                        }
                        Addr -= Buf2Cnt * sizeof(WORD);
                        TmpSize2 -= Buf2Cnt;
                        if(TmpSize2 == 0) {
                            Buf2Wait = WAIT_READ_COMPLETE;
                            Buf2Ready = 1;
                        }
                        SPIFlashReadArray(Addr, (BYTE*)TmpBuf2, Buf2Cnt * sizeof(WORD) , Buf2Wait);
                        FirstPass = 0;      // ������ ������
                        Buf1Ready = 1;      // ������ ������ � ������ ������
                        BufLoad = 0;
                    }
                    while(Buf1Ready||Buf2Ready){
                        if(Buf1Ready && (BufProcess == 0)){
                            TmpBufPtr = TmpBuf1 + Buf1Cnt;
                            Buf1Ready = 0;
                            TmpSize = Buf1Cnt;
                            BufProcess = 1;
                        } else 
                        if(Buf2Ready && (BufProcess == 1)){
                            TmpBufPtr = TmpBuf2 + Buf2Cnt;
                            Buf2Ready = 0;
                            TmpSize = Buf2Cnt;
                            BufProcess = 0;
                        }
                        while(TmpSize > 0){
                            TmpBufPtr--;
                            Interval = *TmpBufPtr;
                            OCN->T += Interval;       // �� flash �������� ��������� ( � TmpBufPtr)
                            BufPtr->r = OCN->T;
                            BufPtr->rs = OCN->T + Pulse;
                            if(BufPtr->r == 0) BufPtr->r++; 
                            BufPtr++;
                            TmpSize--;
                        }
                    }
                } */
                
                Addr -= DataSize * sizeof(WORD);
                SPIFlashReadArray(Addr, (BYTE*)TmpBuf1, (WORD)DataSize * sizeof(WORD), WAIT_READ_COMPLETE);
                TmpBufPtr = TmpBuf1 + DataSize;
                TmpSize = (WORD)DataSize;
                while(TmpSize > 0){
                    TmpBufPtr--;
                    Interval = *TmpBufPtr;
                    OCN->T += Interval;       // �� flash �������� ��������� ( � TmpBufPtr)
                    BufPtr->r = OCN->T;
                    BufPtr->rs = OCN->T + Pulse;
                    if(BufPtr->r == 0) BufPtr->r++; 
//                     if(OCN->T <= TmpT){
//                         Nop();
//                         Nop();
//                     }
//                     TmpT = OCN->T;
//                     if(Interval > 1000){
//                         Nop();
//                         Nop();
//                     }
                    BufPtr++;
                    TmpSize--;
                }
                
                OCN->mCMD_Status.RUN_Interval = Interval;   // ��� ���������� ��������� �� �������� ���������� ���������
                OCN->mCMD_Status.AccX -= (WORD)DataSize;
                BufCount -= (WORD)DataSize;
                // ��������� ������� � �������
                Command.State = Value->State;
                Command.Value = DataSize; 
                //LOCK(OC_DMA_INT_LEVEL, Queue_Insert(&OCN->BufferQueue, Priority, (BYTE*)Buf));
                LOCK(OC_INT_LEVEL, Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Command));
            } 
            if(OCN->mCMD_Status.AccX <= Value->Value) {
                Queue_Delete(&OCN->mCmdQueue);
            }

            break;
        case xCMD_SET_SPEED: //����� �������� ��������� ��� xCMD_RUN ( � ������, ���� �� ������, ������������ ��������� �������� xCMD_ACCELERATE/xCMD_DECELERATE)
            // ��� uCMD �� ������������
            // TODO: ���� ��������� ����������
            OCN->mCMD_Status.RUN_Interval = (WORD)Value->Value;
            OCN->mCMD_Status.State = Value->State;
            Queue_Delete(&OCN->mCmdQueue);
            break;
        case xCMD_SET_TIMER: // TODO: ��������, � mCMD �� �����
        case xCMD_SET_DIRECTION:
        case xCMD_STOP:
            Command.State = Value->State;
            Command.Value = Value->Value; 
            LOCK(OC_INT_LEVEL, Queue_Insert(&OCN->uCmdQueue, Priority, (BYTE*)&Command));
            Queue_Delete(&OCN->mCmdQueue);
            OCN->mCMD_Status.State = Value->State;
            //ProcessCmd = 0;
            break;
        case xCMD_EMERGENCY_STOP:
            // ��� ��������
            #ifdef __C30__
                PORT1_ENABLE = 1;
                PORT2_ENABLE = 1;
                PORT3_ENABLE = 1;
            #endif
            OCN->CurrentState = Value->State;
            OCSetMode(OCN->Config->OCConfig.Index, OC_DISABLED);
            DMADisable(OCN->Config->DMAConfig.DmaId);
            ProcessCmd = 0;
            OCN->mCMD_Status.State = Value->State;
            break;
        default:
            break;
        }
    }
    if(BufCount == 0){
        LOCK(OC_DMA_INT_LEVEL, Queue_Insert(&OCN->BufferQueue, Priority, (BYTE*)Buf));
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

void uCmd_DefaultConfig(CHANEL_CONFIG * Config, BYTE Number)
{
    
    Config->AccBaseAddress = 0x40000;
    Config->AccRecordCount = 32767; // 10 ��������
    switch(Number){
    case 0:
        Config->DMAConfig.DmaId = OC1_DMA_ID;
        Config->OCConfig.Index = ID_OC1;
        Config->DMAConfig.DMABufSize = OC1_DMA_BUF_LEN;
        break;
    case 1:
        Config->DMAConfig.DmaId = OC2_DMA_ID;
        Config->OCConfig.Index = ID_OC2;
        Config->DMAConfig.DMABufSize = OC2_DMA_BUF_LEN;
        break;
    case 2:
        Config->DMAConfig.DmaId = OC1_DMA_ID;  // ��� ������� DMA � OC3 ���
        Config->OCConfig.Index = ID_OC3;
        break;
    }
    Config->DrvConfig.dSTEPPulseWidth = 0.000002; // 2 uS
    Config->wSTEPPulseWidth = 70;//(WORD)(Config->DrvConfig.dSTEPPulseWidth / 0.0000016);
    Config->DrvConfig.uStepPerStep = 16;
    Config->MntConfig.Length = 3.0;
    Config->MntConfig.Mass = 500.0;
    Config->MntConfig.Radius = 0.3;
    Config->MntConfig.Reduction = 360.0;
    Config->OCConfig.WorkMode = TOGGLE;
    Config->TmrId = TIMER2;
    Config->MConfig.StepPerTurn = 200;
    //TODO: ��������� � � B �� ��������� ������ ����������
    

    //TODO: ���-�� ������ ������
    {
        double I;
        // ����� �� ������� ��������� (F1,P1)(F2,P2)
        double F1 = 100;
        double F2 = 1000;
        double P1 = 0.76;
        double P2 = 0.46;
        double MaxAccLength = 0.0;
        double T2;
        I = 2.0*(((Config->MntConfig.Mass*Config->MntConfig.Radius*Config->MntConfig.Radius/4) + (Config->MntConfig.Mass*Config->MntConfig.Length*Config->MntConfig.Length/12))/Config->MntConfig.Reduction)/Rad_to_Grad;

        Config->MConfig.K = (P2 - P1)/(F2-F1);
        Config->MConfig.B = P1 - F1 * Config->MConfig.K;
        // ��������������� ����������. ����� �������������� ����������.
        Config->K = Config->MConfig.K * Config->MConfig.StepPerTurn / I;  // ����������� 1/���
        Config->B = Config->MConfig.B / I; // ����������� 1/���^2
        Config->dx = 360.0/(Config->MntConfig.Reduction * Config->DrvConfig.uStepPerStep * Config->MConfig.StepPerTurn);
        MaxAccLength = Config->AccRecordCount * Config->dx;
        Config->Tmax = GetInterval(MaxAccLength, Config->K, Config->B);
        T2 = GetInterval(MaxAccLength + Config->dx, Config->K, Config->B);
        Config->Vmax = Config->dx / (T2 - Config->Tmax);
        Config->Xmax = MaxAccLength;
        
    }
    //rr->d = (-(rr->K)/(2.0 * rr->B * rr->TimerStep));
    //rr->a = (4.0 * rr->B/(rr->K * rr->K));
    //rr->OneStepCalcTime = (DWORD)(0.000160 / rr->TimerStep); //160us
    //{
        //LONG DestAng = 0;
        //GetDestAngWGuide(20.0, 1.0, Config, &DestAng);
    //}
}

//roots of the quadratic equation
double QuadraticEquationRoot1(double A, double B, double C)
{
    // ���������� ����� ����������� ���������
    volatile double D;
    volatile double V;
    volatile double BB;
    volatile double Am4mC;
    volatile double Am2;
    volatile double sqrtDd2A;
    // �������� ������ ��������� ����������, �.�. ���������� �30 ���������
    BB = B * B;
    Am4mC =  A * C;
    Am4mC *= 4.0;
    D = BB + Am4mC;
    Am2 = A * 2.0;
    sqrtDd2A = sqrt(D); 
    sqrtDd2A -= B;
    V = sqrtDd2A / Am2;
    return V;
}


double GetInterval(double X, double k, double b)
{
    // ���������� ����� ����������� ���������
    return QuadraticEquationRoot1(b, k * X, X);
}


// ��������� ����� � ������� ���������� ����������� ���������� ��������� � ����
// diffX - ������� ����� ������� ���������� � ���������� ����
// TargetSpeed �������� ����
// K, B -   ��������� ���������
// dx       ��� 
// ��������� ���������:
// AccX     ���������� ����� ��������� � ����������
// RunX     ���������� ����� ��������
// RunSpeed ��������
int GetDestAngWGuide(double diffX, double TgtSpeed, CHANEL_CONFIG * Config, LONG * DestAng)
{
    volatile double A;
    volatile double B;
    volatile double B1;
    volatile double C;
    volatile double T;
    volatile double XX;
    volatile double X;
    BYTE sign = 0;
    //volatile float diffX;
    volatile double Tmax2;
    volatile double Xmax2;
    volatile double Vavgmax;
    volatile double Tcross;
    volatile double Xcross;

    if (DestAng == NULL) 
        return -1;

    if(diffX < 0){
        diffX = -diffX;
        TgtSpeed = -TgtSpeed;
        sign = 1;
    }
    {
        // ��������� ����������
        Tmax2 = Config->Tmax * 2.0;
        Vavgmax = Config->Xmax / Config->Tmax;
        Tcross = diffX / (Vavgmax - TgtSpeed);
        Xcross = Vavgmax * Tcross;
        Xmax2 = Config->Xmax * 2;
        
    }

    if(((-Xmax2) < Xcross) && (Xcross < Xmax2)) {
        // ��������� �����, ����������� �� ���������
        // ��� diffX < Acc+Dec

        A = TgtSpeed * Config->K;
        A += Config->B;
        B = diffX * Config->K;
        B1 = TgtSpeed * 2.0;
        B -= B1;
        C = 2.0 * diffX;
        T = QuadraticEquationRoot1(A, B, C);

    } else {
        // ���������� ����������
        X = TgtSpeed * Tmax2 + diffX;
        T = (X - Xmax2)/(Config->Vmax - TgtSpeed);
    }

    XX = ((TgtSpeed * (T + Tmax2)) + diffX) / Config->dx;
    if(sign){
        *DestAng = -(LONG)XX;
    } else {
        *DestAng = (LONG)XX;
    }
    
    return 0;
}

void CreateAccTable()
{
    
    
        WORD i;
        
        WORD Buf[128];
        WORD * Bufptr = Buf;
        double K = AppConfig.ChanellsConfig[0].K;
        double B = AppConfig.ChanellsConfig[0].B;
        double dx = AppConfig.ChanellsConfig[0].dx;
        //double dT = 0.0000002;
        volatile double X;
        volatile double V;
        volatile DWORD Value = 0;
        volatile DWORD Value1 = 0;
        volatile DWORD Res;
        SPIFlashBeginWrite(262144);
        for(i = 0; i < 32768; i++){
            if(((i & 0x7f) == 0) && (i > 0)){
                SPIFlashWriteArray((BYTE*)Buf, 256);
                Bufptr = Buf;
                //Nop();
            }
            X = (double)i*dx;
            V = 5000000.0 * GetInterval(X, K, B);
            Value = (DWORD)(V);
            if(Value < Value1){
                Nop();
                Nop();
            }
            Res = Value - Value1;
            *Bufptr = (WORD)Res;
            Value1 = Value;
            Bufptr++;
        }
    
}
LONG GetAngle(BYTE ch)
{
    volatile LONG value = 0;
    switch (ch)
    {
    case 0: value = OControll1.XPosition;
        break;
    case 1: value = OControll2.XPosition;
        break;
    case 3: value = 0;
        break;
    default:
        value = 0;
    }
    return value;
}

DWORD GetStepsPerGrad(BYTE ch)
{
    volatile DWORD value = 0;
    switch (ch)
    {
    case 0: value = (DWORD)((OControll1.Config->MntConfig.Reduction * OControll1.Config->DrvConfig.uStepPerStep * OControll1.Config->MConfig.StepPerTurn)/360);
        break;
    case 1: value = (DWORD)((OControll2.Config->MntConfig.Reduction * OControll2.Config->DrvConfig.uStepPerStep * OControll2.Config->MConfig.StepPerTurn)/360);
        break;
    case 3: value = 0;
        break;
    default:
        value = 0;
    }
    return value;
}
BYTE GetStatus(BYTE ch){
    volatile BYTE value = 0;
    switch (ch)
    {
    case 0: value = (OControll1.CurrentState);
        break;
    case 1: value = (OControll2.CurrentState);
        break;
    case 3: value = 0;//(OControll3.CurrentState);
        break;
    default:
        value = 0;
    }
    return value;
}

int Cmd_Init(void)
{
    Queue_Init(&CmdQueue, CMDQueueKeys, CMD_QUEUE_SIZE, (BYTE*)CMDQueueValues, sizeof(CMD_QUEUE));
    if(0){
        CMD_QUEUE Cmd;
        Cmd.a  = 300000;
        Cmd.a1 = 288000;
        Cmd.Command = CMD_GO_TO_POSITION;
        Queue_Insert(&CmdQueue, 50, (BYTE*)&Cmd);
    }
    return uCmd_Init();
}

int Cmd_Process()
{
    CMD_QUEUE * Value = NULL;
    //CMD_QUEUE StopCommand;
    xCMD_QUEUE mCmd;
    //BYTE ProcessCmd = 1;
    BYTE Priority = 50;
    static double GuideTime = 10.0; // 10 ������
    
    if(Queue_ExtractAllToMin( &CmdQueue, &Priority, (BYTE**)&Value ) == 0){

        switch (Value->Command) {

        case CMD_STOP:                                  // ���������
            mCmd.State = xCMD_DECELERATE;
            mCmd.Value = 0;
            LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
            
            //mCmd.State = xCMD_STOP;
            //LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));

            mCmd.State = xCMD_DECELERATE;
            mCmd.Value = 0;
            LOCK(1,Queue_Insert(&OControll2.mCmdQueue, Priority, (BYTE*)&mCmd));
            
            //mCmd.State = xCMD_STOP;
            //LOCK(1,Queue_Insert(&OControll2.mCmdQueue, Priority, (BYTE*)&mCmd));
            Queue_Delete(&CmdQueue);
            break;
        case CMD_GO_TO_POSITION:                        // ������� �� �������; ���������: a, d,
            // ���� ����� ��������, �� ���������
            if((OControll1.CurrentState == xCMD_STOP) && (OControll2.CurrentState == xCMD_STOP)){
                // ��������� � ���������
                if(Aim(&OControll1, (LONG)(Value->a), Priority) == 0){
                    xCmdStart(0);
                }
                if(Aim(&OControll2, (LONG)(Value->d), Priority) == 0){
                    xCmdStart(1);
                }
                Queue_Delete(&CmdQueue);
            }
            break;
        case CMD_GO_TO_POSITION_AND_GUIDE_STAR:         // ������� �� ������� � ������������ ������ ( ������� ������� �������) a, d,
            if(0){
                mCmd.State = xCMD_SET_DIRECTION;
                mCmd.Value = 1;
                LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
                mCmd.State = xCMD_SET_TIMER;
                mCmd.Value = (DWORD)OC_TMR3;
                LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
                mCmd.State = xCMD_SET_SPEED;
                mCmd.Value = 46500;
                LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
                mCmd.State = xCMD_SLOW_RUN;
                mCmd.Value = 288000;
                LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
            }
            Queue_Delete(&CmdQueue);
            break;
        case CMD_SET_GUIDE_TIME:                        // ���������� ����� ������������� ������� a ( � ��� )
            GuideTime = (double)Value->a;
            Queue_Delete(&CmdQueue);
            break;
        case CMD_GO_HOME:                               // ��������� �������� � ��������� ��������  
            Queue_Delete(&CmdQueue);
            break;
        case CMD_SLOW_GO_HOME:                          // ��������� ����������� �� ����������� ������� ����� ������������� ��������������
            Queue_Delete(&CmdQueue);
            break;
        case CMD_SET_AXIS:                              // ���������� ( ������������� ������� ��������� ���������) a, d
            OControll1.XPosition = Value->a;
            OControll2.XPosition = Value->d;
            Queue_Delete(&CmdQueue);
            break;
        case CMD_SET_GUIDE_SPEED:                       // ���������� ( ������������� �������� ������������� ������) guidespeed
            OControll1.Config->GuideSpeed = Value->a1;
            OControll2.Config->GuideSpeed = Value->d1;
            Queue_Delete(&CmdQueue);
            break;
        case CMD_SET_AIM_SPEED:                         // ���������� ( ������������� �������� ���������) runspeed
            Queue_Delete(&CmdQueue);
            break;
        case CMD_EMG_STOP:                              // ��������� ���������
            mCmd.State = xCMD_EMERGENCY_STOP;
            LOCK(1,Queue_Insert(&OControll1.mCmdQueue, 0, (BYTE*)&mCmd));
            LOCK(1,Queue_Insert(&OControll2.mCmdQueue, 0, (BYTE*)&mCmd));
            Queue_Delete(&CmdQueue);
            break;
        default:
            Queue_Delete(&CmdQueue);
            break;
        }
    }
    IFS0bits.U1TXIF = 1;
    return 0;
}

int Aim(OC_CHANEL_STATE * OCN, LONG destang, BYTE Priority)
{
    
    LONG diff;
    LONG diff_2;
    xCMD_QUEUE mCmd;
    /*
    if(destang < 0) {
        destang += 1152000;
    } else if(destang > 1152000){
        destang -= 1152000;
    }
    if (OCN->XPosition < 0) {
        OCN->XPosition += 1152000;
    }else if(OCN->XPosition > 1152000){
        OCN->XPosition -= 1152000;
    }*/
    
    diff = destang - OCN->XPosition;
    if(diff == 0) return -1;
    // ���������� ����������� ��������
    if(diff < 0){
        diff = -diff;
        mCmd.State = xCMD_SET_DIRECTION;
        mCmd.Value = 1;
        LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
    } else {
        mCmd.State = xCMD_SET_DIRECTION;
        mCmd.Value = 0;
        LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
    }
    mCmd.State = xCMD_SET_TIMER;
    mCmd.Value = (DWORD)OC_TMR2;
    LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));

    if(0){
        mCmd.State = xCMD_SET_SPEED;
        mCmd.Value = 1562;
        LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
        mCmd.State = xCMD_RUN;
        mCmd.Value = diff;
        LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
    } else {
    
        diff_2 = diff / 2;
        if(diff_2 < (LONG)OCN->Config->AccRecordCount){
            mCmd.State = xCMD_ACCELERATE;
            mCmd.Value = diff_2;
            LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
        } else {
            mCmd.State = xCMD_ACCELERATE;
            mCmd.Value = OCN->Config->AccRecordCount;
           LOCK(1, Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
            mCmd.State = xCMD_RUN;
            mCmd.Value = diff - OCN->Config->AccRecordCount * 2;
            LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
        }
        mCmd.State = xCMD_DECELERATE;
        mCmd.Value = 0;
        LOCK(1,Queue_Insert(&OCN->mCmdQueue, Priority, (BYTE*)&mCmd));
        mCmd.State = xCMD_STOP;
        LOCK(1,Queue_Insert(&OCN->mCmdQueue, 250, (BYTE*)&mCmd));
    }
    return 0;
}
// ��������� � ������������� ����
// ���������:
// destang          ���� ���� �� ������ ������ ��������
// TargetSpeed      �������� �������� ���� �� �������� ��� ( ���� ��������� ����������� )
// GuideTime        ����� ������������� ���� ( �������� ������ 0 ) 
//                  ���� �������� ����� 0, �� ��������������� ����������� ��������� 
//                  �������� � ���������� ����������� ( ����� �������� ����� ����� ��������)
// Priority         ���������
int AimAndGuide(OC_CHANEL_STATE * OCN, LONG destang, double TargetSpeed, double GuideTime, BYTE Priority)
{
    LONG diff;
    LONG diff_2;
    double GuideTimerStep = 0.0000016;
    xCMD_QUEUE mCmd;
    diff = destang - OCN->XPosition;

    GetDestAngWGuide(diff, TargetSpeed, OCN->Config, &diff_2);
    Aim(OCN, diff_2 + OCN->XPosition, Priority);

    if(TargetSpeed != 0.0){
        mCmd.State = xCMD_SET_DIRECTION;
        if(TargetSpeed >= 0){
            mCmd.Value = 0;
        } else {
            mCmd.Value = 1;
            TargetSpeed = -TargetSpeed;
        }
        LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
        mCmd.State = xCMD_SET_TIMER;
        mCmd.Value = (DWORD)OC_TMR3;
        LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
        
        mCmd.State = xCMD_SET_SPEED;
        mCmd.Value = (DWORD)(OCN->Config->dx / (GuideTimerStep * TargetSpeed));
        LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
        mCmd.State = xCMD_SLOW_RUN;
        mCmd.Value =  (DWORD)(GuideTime / GuideTimerStep);
        LOCK(1,Queue_Insert(&OControll1.mCmdQueue, Priority, (BYTE*)&mCmd));
    }
    return 0;
}

// INT_LEVEL = 1
INTERRUPT _U1TXInterrupt(void)
{
    int res = 0;
    IFS0bits.U1TXIF = 0;
    while(res > -2) {
        res = 0;
        res += mCmd_Process(&OControll1, OC1_DMA_BUF_LEN);
        res += mCmd_Process(&OControll2, OC2_DMA_BUF_LEN);
    }
}
