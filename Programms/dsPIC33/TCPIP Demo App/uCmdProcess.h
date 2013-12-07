#ifndef __uCMD_PROCESS_
#define __uCMD_PROCESS_

#ifdef __C30__
#   include "GenericTypeDefs.h"
#include "device_control.h"
#include "Queue.h"
#else
#include "stdafx.h"
#include "Queue.h"
#include "device_control.h"
#endif

#define Grad_to_Rad 0.017453292519943295
#define Rad_to_Grad 57.295779513082323

//#include "..\dsPIC33\TCPIP Demo App\OCTimer.h"


// ��������� DMA ������ ��� ������ Delayed One-Shot � OC
typedef struct _OC_BUF {
    WORD r;
    WORD rs;
} OC_BUF;

typedef enum _COMMAND { 
    CMD_STOP = 0x00,                        // ���������
    CMD_GO_TO_POSITION,                     // ������� �� �������; ���������: a, d
    CMD_GO_TO_POSITION_AND_GUIDE_STAR,      // ������� �� ������� � ������������ ������ ( ������� ������� �������) a, d
    CMD_GO_TO_POSITION_AND_GUIDE_SAT,       // ������� �� ������� � ������������ ������� ( ����� ��������� �������� ��������) a, d, a1, d1
    CMD_SET_GUIDE_TIME,                     // ���������� ����� ������������� �������
    CMD_GO_HOME,                            // ��������� �������� � ��������� ��������  
    CMD_OPEN_ROOF,                          // ������� ����� ( �������� ����� �� �������� URL) 
    CMD_CLOSE_ROOF,                         // ������� �����
    CMD_CREATE_SNAPSHOT,                    // ������ ������� �� ������
    CMD_GET_SNAPSHOT,                       // ����������� ���������� � ������ 
    CMD_SLOW_GO_HOME,                       // ��������� ����������� �� ����������� ������� ����� ������������� ��������������
    CMD_SET_AXIS,                           // ���������� ( ������������� ���������, ��� ������� ��������� ���������) a, d
    CMD_SET_GUIDE_SPEED,                    // ���������� ( ������������� �������� ������������� ������) guidespeed
    CMD_SET_AIM_SPEED,                      // ���������� ( ������������� �������� ���������) runspeed
    CMD_EMG_STOP = 0xFF,                    // ��������� ���������

} COMMAND;

 typedef enum _xCMD_STATE { // ���������
                           // ���������|��������
     xCMD_STOP,            //10 ����������
     //xCMD_START,           //10 ��������� ������
     xCMD_ACCELERATE,      //10 �����������
     xCMD_SET_SPEED,       //10 ������������� ������ �������� ��������� ��� xCMD_RUN (double)
     xCMD_RUN,             //10 �������� � ���������� ���������
     xCMD_DECELERATE,      //10 ��������
     xCMD_SET_TIMER,       //10 ��������� �������
     //xCMD_SET_TIMER_SOURCE,//10 ��������� ��������� ������� ��� �������
     xCMD_SET_DIRECTION,   //10 ��������� ����������� ��������
     //xCMD_SET_STEP,        //10 ��������� ���������� ����������
     xCMD_EMERGENCY_STOP,  //0 ��������� ���������
     //xCMD_BREAK,           //5 �������� ��� ������� �� ���� � ������� ��������� ( ������ mCMD )
    xCMD_SLOW_RUN,        //����� ������� ( �������� ������, ��� ����� (64 ����) �������� ����� ������� (��� ������� ������� ��� 9,6 ���))
     xCMD_ERROR            // ��������� ���������
 }xCMD_STATE;


// ������� ������� ����������� (������������ ������ � ������ DMA)
// ������������ � ����� ������ ����� ���� ��������� ������
typedef struct _xCMD_QUEUE{
    xCMD_STATE  State;                                  // �������
    DWORD       Value;                                  // ��������� ������� (���������� ����� �� ���������� �������)
} xCMD_QUEUE;

typedef struct _CMD_QUEUE{
    COMMAND     Command;                                // �������
    LONG       a;                                      // ���� � �����
    LONG       d;
    double       a1;                                     // �������� ���������
    double       d1;                                     // �������� �������������

} CMD_QUEUE;

#define BUFFER_QUEUE_SIZE 16
#define CMD_QUEUE_SIZE 10
#define uCMD_QUEUE_SIZE 24
#define mCMD_QUEUE_SIZE 10

typedef struct MOTOR_CONFIG {
    WORD                    StepPerTurn;                // ���������� ����� ��������� �� ������ (200 - ���-200)
    double                  K;                          // Kx + B ( K - ������� ���� ������� ������� ����������� �������� ��������� �� �������� ��������
    double                  B;                          // B - ���������, �������� ��������� � Hm �������� � �������� � ��� 
} MOTOR_CONFIG;

typedef struct MOTOR_DRIVER_CONFIG{
    WORD uStepPerStep;                                  // ���������� ���������� �� ��� (16)
    double dSTEPPulseWidth;                             // ������������ ������� � �������� 2us    
}MOTOR_DRIVER_CONFIG;

typedef struct MOUNT_CONFIG {
    double Mass;                                        // ����� ����������
    double Radius;                                      // ������ ���/����� ���������
    double Length;                                      // ������ ��� (�������������)
    double Reduction;                                   // ����������� �������� ��������� ���� (1/360)
} MOUNT_CONFIG;

typedef struct OC_CONFIG {
    OC_ID                   Index;                      // ����� ������ (����� OC)
    OC_WORK_MODE            WorkMode;                   // ����� ������ ������ OC 
    WORD                    OCxCon;                     // ������������ ������
    // TODO: ���� ���� ���������� ��� �� �����
    // TMR_PRESCALER Tmr2divide;                                    // �������� ������� 2(0 => 1, 3 => 8, 6 => 64, 8 => 256)
    // TMR_PRESCALER Tmr3divide;                                    // �������� ������� 3(0 => 1, 3 => 8, 6 => 64, 8 => 256)


    /*double                  d;                          // ��������� ��� ����������� ����������
    double                  a;  
    double                  c;
    LONG                    XMinPosition;               // ����������� �������� ������ ���� (��������� ������� 0)
    LONG                    XMaxPosition;               // ������������ �������� ������ ����
    LONG                    XZenitPosition;             // ����� ���� � ������
    LONG                    XParkPosition;              // �������� ������ ���� ����������� �������        
    double                  MaxSpeed;
    double                  MaxAngle;
    double                  MinAngle;*/
}OC_CONFIG;

typedef struct DMA_CONFIG {
    DMA_ID                  DmaId;
    WORD                    DmaCfg;
    WORD                    DMABufSize;
} DMA_CONFIG;

typedef struct CHANEL_CONFIG {
    MOUNT_CONFIG            MntConfig;
    MOTOR_CONFIG            MConfig;
    MOTOR_DRIVER_CONFIG     DrvConfig;
    OC_CONFIG               OCConfig;
    DMA_CONFIG              DMAConfig;
    TIMERS_ID               TmrId;                      // ����� ������� �� �������� �������� ����� (T2/T3)
    double                  dx;                         // ��� ���� � ��������
    WORD                    wSTEPPulseWidth;            // ������������ ������� ��� � ���������� �������
    DWORD                   AccBaseAddress;             // ����� ������ ������� ��������� �� ������� ������
    DWORD                   AccRecordCount;             // ���������� ������� � ������� ��������� �� ������� ������
    double                  K;                          // ����������� ��������� ���������
    double                  B;
    double                  Xmax;                       
    double                  Vmax;                       
    double                  Tmax;
    double                  GuideSpeed;                 // �������� ������������� ������
}CHANEL_CONFIG;

typedef struct mCMD_STATUS {
    WORD                    AccX;                       // ���������� X � ������� ��������� 
    WORD                    RUN_Interval;             // ������� ��������
    xCMD_STATE              State;
    BYTE                    Priority;
} mCMD_STATUS;

typedef struct OC_CHANEL_STATE{
        
    PRIORITY_QUEUE          uCmdQueue;                  // ������� �����������
    PRIORITY_QUEUE          BufferQueue;                  // ������� �����������
    LONG                    XPosition;                  // ������� ����� ���� TODO: ���������� ������������ ���������
    BYTE                    CurrentDirection;           // ����������� �������� ��� �������� ( ������� �������� ������ Dir )    

    PRIORITY_QUEUE          mCmdQueue;                  // ������� ����������
    mCMD_STATUS             mCMD_Status;                // ����������, ������������ ��� ��������� ���������� (�����������)

    xCMD_STATE              CurrentState;
    
    WORD                    T;
    BYTE                    Enable;                     // ������� ������������
    BYTE                    Pulse;                      // ���� ��� Togle ������ � OC (������������)
    
    CHANEL_CONFIG *         Config;
    

}OC_CHANEL_STATE;

#ifdef _WINDOWS_
#if !defined _APP_CONFIG_TYPE
#define _APP_CONFIG_TYPE
typedef struct { 
    DWORD_VAL MyIPAddr;
    DWORD_VAL MyMask;
    DWORD_VAL MyGateway;
    DWORD_VAL PrimaryDNSServer;
    DWORD_VAL SecondaryDNSServer;
    CHANEL_CONFIG    ChanellsConfig[3];      // ������������ �������
    char NetBIOSName[16];
    DWORD Time;
} APP_CONFIG;
#endif //#if !defined _APP_CONFIG_TYPE
#endif // _WINDOWS_

int Cmd_Init(void);
int Cmd_Process(void);
void uCmd_DefaultConfig(CHANEL_CONFIG * Config, BYTE Number);
void CreateAccTable();
LONG GetAngle(BYTE ch);
DWORD GetStepsPerGrad(BYTE ch);
BYTE GetStatus(BYTE ch);
#endif //__uCMD_PROCESS_
