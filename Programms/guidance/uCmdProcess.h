#ifndef __uCMD_PROCESS_
#define __uCMD_PROCESS_

#ifdef __C30__
#   include "GenericTypeDefs.h"
#else
#   include "stdafx.h"
#endif

#include "..\dsPIC33\TCPIP Demo App\device_control.h"
#include "..\dsPIC33\TCPIP Demo App\OCTimer.h"


// ��������� DMA ������ ��� ������ Delayed One-Shot � OC
typedef struct OC_BUF {
    WORD r;
    WORD rs;
} OC_BUF;


 typedef enum xCMD_STATE { // ���������
                           // ���������|��������
     xCMD_STOP,            //10 ����������
     xCMD_START,           //10 ��������� ������
     xCMD_ACCELERATE,      //10 �����������
     xCMD_RUN,             //10 �������� � ���������� ���������
     xCMD_DECELERATE,      //10 ��������
     xCMD_SET_TIMER,       //10 ��������� �������
     //xCMD_SET_TIMER_SOURCE,//10 ��������� ��������� ������� ��� �������
     xCMD_SET_DIRECTION,   //10 ��������� ����������� ��������
     //xCMD_SET_STEP,        //10 ��������� ���������� ����������
     xCMD_EMERGENCY_STOP,  //0 ��������� ���������
     xCMD_BREAK,           //5 �������� ��� ������� �� ���� � ������� ��������� ( ������ mCMD )
     xCMD_ERROR            // ��������� ���������
 }xCMD_STATE;


// ������� ������� ����������� (������������ ������ � ������ DMA)
// ������������ � ����� ������ ����� ���� ��������� ������
typedef struct xCMD_QUEUE{
    xCMD_STATE  State;                                  // �������
    DWORD       Value;                                  // ��������� ������� (���������� ����� �� ���������� �������)
} xCMD_QUEUE;

#define uCMD_QUEUE_SIZE 10
#define mCMD_QUEUE_SIZE 10

typedef struct MOTOR_CONFIG {
    WORD StepPerTurn;                                   // ���������� ����� ��������� �� ������ (200 - ���-200)
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

typedef struct CHANEL_CONFIG {
    MOUNT_CONFIG            MConfig;
    MOTOR_DRIVER_CONFIG     DrvConfig;
    OC_CONFIG               OCConfig;
    TIMERS_ID               TmrId;                      // ����� ������� �� �������� �������� ����� (T2/T3)
    double                  dx;                         // ��� ���� � ��������
    WORD                    wSTEPPulseWidth;            // ������������ ������� ��� � ���������� �������
    DMA_ID                  DmaId;
    DWORD                   AccBaseAddress;             // ����� ������ ������� ��������� �� ������� ������
    DWORD                   AccRecordCount;             // ���������� ������� � ������� ��������� �� ������� ������
}CHANEL_CONFIG;

typedef struct mCMD_STATUS {
    WORD                    AccX;                       // ���������� X � ������� ��������� 
} mCMD_STATUS;

typedef struct OC_CHANEL_STATE{
        
    PRIORITY_QUEUE          uCmdQueue;                  // ������� �����������
    LONG                    XPosition;                  // ������� ����� ���� TODO: ���������� ������������ ���������
    BYTE                    CurrentDirection;           // ����������� �������� ��� �������� ( ������� �������� ������ Dir )

    PRIORITY_QUEUE          mCmdQueue;                  // ������� ����������
    mCMD_STATUS             mCMD_Status;                // ����������, ������������ ��� ��������� ���������� (�����������)

    xCMD_STATE              CurrentState;
    
    DWORD_VAL               T;
    BYTE                    Enable;                     // ������� ������������
    
    CHANEL_CONFIG           Config;
    

}OC_CHANEL_STATE;



int uCmd_Init();
int uCmd_DMACallback(void*, BYTE*, WORD);
int uCmd_OCCallback(void * _This);
int uCmd_ICCallback(void * _This);
#endif //__uCMD_PROCESS_