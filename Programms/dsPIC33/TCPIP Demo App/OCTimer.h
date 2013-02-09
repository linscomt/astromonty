#ifndef __OC_TIMER_H_
#define __OC_TIMER_H_

#ifdef __C30__
#   include "GenericTypeDefs.h"
#else
#   include "..\..\guidance\stdafx.h"
#endif
#include "device_control.h"

typedef union MOTOR_POSITION{
    BYTE Val;
    struct __16
    {        
        BYTE Step:4;
        BYTE b8:4;
    } b16;
    struct __8
    {        
        BYTE b0:1;
        BYTE Step:3;
        BYTE b8:4;
    } b8;
    struct __4
    {        
        BYTE b0:2;
        BYTE Step:2;
        BYTE b8:4;
    } b4;
    struct __2
    {        
        BYTE b0:3;
        BYTE Step:1;
        BYTE b8:4;
    } b2;
    struct __1
    {        
        BYTE b0:4;
        BYTE Step:1;
        BYTE b8:3;
    } b1;
} MOTOR_POSITION; 
#ifndef DATE_TIME_STRUCT
#define DATE_TIME_STRUCT
typedef struct DateTimeStruct {
    WORD Year;
    BYTE Month;
    BYTE Day;
    BYTE DayOfWeak;
    BYTE Hour;
    BYTE Min;
    BYTE Sec;
    double uSec;
} DateTime;
#endif
#define PI 3.1415926535897932384626433832795
#define PI2 6.283185307179586476925286766559
#define FREQ_STEP 20
#define BUF_SIZE 128
// �������� BUF_SIZE
#define BUF_SIZE_2 64
// ������� ������
#define CQ_SIZE 10

//static double Accelerate[ACCELERATE_SIZE];
#define Grad_to_Rad 0.017453292519943295
#define Rad_to_Grad 57.295779513082323

typedef union _STATE_VALUE{
    double Speed;
    double Angle;
    int Dir;
    BYTE Timer;
    BYTE Step;
    void* Null;
}STATE_VALUE;

// ��������� 

typedef enum GD_STATE { // ���������
    ST_STOP,            // ����������
    ST_ACCELERATE,      // �����������
    ST_RUN,             // �������� � ���������� ���������
    ST_CONTINOUS,       // �������������
    ST_DECELERATE,      // ��������
    ST_SET_TIMER,       // ��������� �������
    ST_SET_DIRECTION,   // ��������� ����������� ��������
    ST_SET_STEP,        // ��������� ���������� ����������
    ST_EMERGENCY_STOP,  // ��������� ���������
}GD_STATE;

// ������� ������. ���� �������� ����� 0, �� ��� ���� �� ������������, ���� ����������� �������������
typedef struct CMD_QUEUE{
    GD_STATE    State;      // �������
    STATE_VALUE Value;      // �������� ���������
    //BYTE        Direction;  // ����������� �������� � �������
    //double      Vend;       // ��������, ������� ����� ����������
    //double      deltaX;     // ����������, ������� ����� �������� ����� ���������� �������
    //DWORD       RunStep;    // ���������� ����� �� ���������� �������
}Cmd_Queue;

// ������� ������ ����������
typedef struct RUNCMD_QUEUE{
    GD_STATE    State;      // �������
    //STATE_VALUE Value;      // �������� ���������
    BYTE        Direction;  // ����������� �������� � �������
    BYTE        Timer;
    //double      Vend;       // ��������, ������� ����� ����������
    //double      deltaX;     // ����������, ������� ����� �������� ����� ���������� �������
    DWORD       RunStep;    // ���������� ����� �� ���������� �������
}Run_Cmd_Queue;

typedef struct _TIMERS_CON{
    BYTE divide;                // ����� �������� �� �������� �������
    BYTE SHRcount;              // divide = 2^SHRcount ������� ������ ��������
    double Period;              // ������ � �������� �������
} TIMERS_CON;

typedef struct _TABLE_ADDR {
    DWORD TableAddr;
    DWORD TableSize;
} TABLE_ADDR;

// ������ �� 256�� ��� �������� ������ ������ ��
typedef struct _HEADER{
    char Magic[4];
    TABLE_ADDR SlowAcc;
    TABLE_ADDR FastAcc;
    TABLE_ADDR SlowDec;
    TABLE_ADDR FastDec;
    //double K;                                           // Kx + B ( K - ������� ���� ������� ������� ����������� ��������� ��������� �� �������� ��������
    //double B;                                           // B - ���������, ��������� ��������� � Hm �������� � �������� � ��� 
    double Mass;                                        // ����� ����������
    double Radius;                                      // ������ ���/����� ���������
    double Length;                                      // ������ ��� (�������������)
    double Reduction;                                   // ����������� �������� ��������� ���� (1/360)
    double TimerStep;
    WORD StepPerTurn;                                   // ���������� ����� ��������� �� ������ (200 - ���-200)
    WORD uStepPerStep;                                  // ���������� ���������� �� ��� (16)
    LONG XMinPosition;                                  // ����������� �������� ������ ���� (��������� ������� 0)
    LONG XMaxPosition;                                  // ������������ �������� ������ ����
    LONG XZenitPosition;                                // ����� ���� � ������
    LONG XParkPosition;                                 // �������� ������ ���� ����������� �������

    DWORD DataCheckSumm;
    DWORD HeadCheckSumm;
} HEADER;

// ��������� DMA ������ ��� ������ TOGGLE � OC
typedef struct _OC_RECORD {
    WORD r;
    WORD rs;
} OC_RECORD;

typedef struct RR{

    // ������� ������ ����
    Cmd_Queue               CmdQueue[CQ_SIZE];          // ������� ������
    BYTE                    CmdBeginQueue;              // ��������� �� ������ �������
    BYTE                    CmdEndQueue;                // ��������� �� ����� �������
    BYTE                    CmdCount;                   // ���������� ������ � �������
    // ������� ������ ����������
    Run_Cmd_Queue           RunQueue[CQ_SIZE];          // ������� ������ ����������
    BYTE                    RunBeginQueue;              // ��������� �� ������ �������
    BYTE                    RunEndQueue;                // ��������� �� ����� �������
    BYTE                    RunCount;                   // ���������� ������ � �������

    // ��������� ����������
    GD_STATE                RunState;                   // ��� �������, ����������� � ������ �����
    LONG                    XPosition;                  // ������� ����� ���� TODO: ���������� ������������ ���������
    BYTE                    RunDir;                     // ����������� �������� ��� �������� ( ������� �������� ������ Dir )    
    DWORD_VAL               T;

    // ��������� ������������
    GD_STATE                CacheState;                 // ��� �������, ���������� � ������ �����
    LONG                    XCachePos;                  // ������� ��������� � ��������
    double                  CacheSpeed;                 // ������� ��������
    BYTE                    CacheDir;                   // ����������� �������� ��� ��������
    DWORD                   CacheCmdCounter;            // ���������� ����� �� ��������� �������
    DWORD                   Interval;                   // ������� �������� ��������� (����� �� ������) ������?
    DWORD                   XaccBeg;                    // ��������� ������� ���������
    double                  d;                          // ��������� ��� ����������� ����������
    double                  a;  
    double                  c;
    DWORD                   T1;                         // �������� �������, ���������� � ���������� ������ Accelerate/Deccelerate

    // ��������� ����������� �� ������ ���������
    double                  Vend;                       // �������� ���������� �������
    double                  deltaX;                     // ���������� ���������� �������

    LONG                    XMinPosition;               // ����������� �������� ������ ���� (��������� ������� 0)
    LONG                    XMaxPosition;               // ������������ �������� ������ ����
    LONG                    XZenitPosition;             // ����� ���� � ������
    LONG                    XParkPosition;              // �������� ������ ���� ����������� �������        
    double                  MaxSpeed;
    double                  MaxAngle;
    double                  MinAngle;

    // ��������� (�����������)
    
    // ��������� ����������
    WORD DMABufferSize;                                 // ������ ������� �� ���� ������� DMA(����� ping-pong)
    DWORD  OneStepCalcTime;                             // �����(� �������� �������), ����������� ��� ������������ ����������
    DWORD  BufFillTime;                                 // �����(� �������� �������), ����������� �� ���������� ���� �������
    double K;                                           // Kx + B ( K - ������� ���� ������� ������� ����������� �������� ��������� �� �������� ��������
    double B;                                           // B - ���������, �������� ��������� � Hm �������� � �������� � ��� 
    double TimerStep;                                   // ��� ������� � �������� (25ns)
    double dx;                                          // ��� ���� � ��������
    double Mass;                                        // ����� ����������
    double Radius;                                      // ������ ���/����� ���������
    double Length;                                      // ������ ��� (�������������)
    double Reduction;                                   // ����������� �������� ��������� ���� (1/360)
    WORD StepPerTurn;                                   // ���������� ����� ��������� �� ������ (200 - ���-200)
    WORD uStepPerStep;                                  // ���������� ���������� �� ��� (16)
    OC_ID Index;                                        // ����� ������ (����� OC)
    TIMERS_ID TmrId;                                    // ����� ������� �� �������� �������� ����� (T2/T3)
    BYTE Enable;                                        // ������� ������������
    //BYTE Tmr2divide;                                    // �������� ������� 2(0 => 1, 3 => 8, 6 => 64, 8 => 256)
    //BYTE Tmr3divide;                                    // �������� ������� 3(0 => 1, 3 => 8, 6 => 64, 8 => 256)
    
    TABLE_ADDR SlowAcc;
    TABLE_ADDR FastAcc;
    TABLE_ADDR SlowDec;
    TABLE_ADDR FastDec;

}RR;


typedef  struct 
#ifdef __C30__
__attribute__((__packed__))
#endif 
{
    DWORD Timestamp;                                    // ����� ������� UTC
    LONG XPosition;                  					// ������� ����� ���� TODO: ���������� ������������ ���������
    WORD uStepPerStep;                                  // ������� ���������� ���������� �� ��� (16)
}RRRAMSave;


typedef struct
#ifdef __C30__
__attribute__((__packed__))
#endif 
{
    RRRAMSave RRSave[3];
}RAMSaveConfig;

#ifdef __C30__

#define MS1_Tris            TRISGbits.TRISG12   // ����� MS1
#define MS2_Tris            TRISGbits.TRISG13   // ����� MS2
#define SLEEP_Tris          TRISGbits.TRISG14   // ����� SLEEP
#define RESET_Tris          TRISGbits.TRISG15   // ����� RESET
#define MS1                 LATGbits.LATG12     // ����� MS1
#define MS2                 LATGbits.LATG13     // ����� MS2
#define SLEEP               LATGbits.LATG14     // ����� SLEEP
#define RESET               LATGbits.LATG15     // ����� RESET

//�������� ������� port1
#define PORT1_NULL_Tris     TRISAbits.TRISA0    // ���� NULL
#define PORT1_POS_Tris      TRISDbits.TRISD8    // ���� POS
#define PORT1_POS2_Tris     TRISEbits.TRISE0    // ���� POS2
#define PORT1_NULL          PORTAbits.RA0       // ���� NULL
#define PORT1_POS           PORTDbits.RD8       // ���� POS
#define PORT1_POS2          PORTEbits.RE0       // ���� POS2
#define PORT1_ENABLE_Tris   TRISAbits.TRISA9    // ����� ENABLE
#define PORT1_DIR_Tris      TRISBbits.TRISB8    // ����� DIR
#define PORT1_STEP_Tris     TRISDbits.TRISD0    // ����� STEP
#define PORT1_ENABLE        LATAbits.LATA9      // ����� ENABLE
#define PORT1_DIR           LATBbits.LATB8      // ����� DIR
#define PORT1_STEP          LATDbits.LATD0      // ����� STEP

//������������� ������� port2
#define PORT2_NULL_Tris TRISAbits.TRISA1 // ���� NULL
#define PORT2_POS_Tris TRISDbits.TRISD9 // ���� POS
#define PORT2_POS2_Tris TRISEbits.TRISE1 // ���� POS2
#define PORT2_NULL PORTAbits.RA1 // ���� NULL
#define PORT2_POS PORTDbits.RD9 // ���� POS
#define PORT2_POS2 PORTEbits.RE1 // ���� POS2
#define PORT2_ENABLE_Tris TRISAbits.TRISA10 // ����� ENABLE
#define PORT2_DIR_Tris TRISBbits.TRISB9 // ����� DIR
#define PORT2_STEP_Tris TRISDbits.TRISD1 // ����� STEP
#define PORT2_ENABLE LATAbits.LATA10 // ����� ENABLE
#define PORT2_DIR LATBbits.LATB9 // ����� DIR
#define PORT2_STEP LATDbits.LATD1 // ����� STEP

//������������� ������� port3
#define PORT3_NULL_Tris TRISAbits.TRISA2 // ���� NULL
#define PORT3_POS_Tris TRISDbits.TRISD10 // ���� POS
#define PORT3_POS2_Tris TRISEbits.TRISE2 // ���� POS2
#define PORT3_NULL PORTAbits.RA2 // ���� NULL
#define PORT3_POS PORTDbits.RD10 // ���� POS
#define PORT3_POS2 PORTEbits.RE2 // ���� POS2
#define PORT3_ENABLE_Tris TRISAbits.TRISA9 // ����� ENABLE
#define PORT3_DIR_Tris TRISBbits.TRISB10 // ����� DIR
#define PORT3_STEP_Tris TRISDbits.TRISD2 // ����� STEP
#define PORT3_ENABLE LATAbits.LATA9 // ����� ENABLE
#define PORT3_DIR LATBbits.LATB10 // ����� DIR
#define PORT3_STEP LATDbits.LATD2 // ����� STEP

//������������� ������� port4
#define PORT4_NULL_Tris TRISAbits.TRISA3 // ���� NULL
#define PORT4_POS_Tris TRISDbits.TRISD11 // ���� POS
#define PORT4_POS2_Tris TRISEbits.TRISE3 // ���� POS2
#define PORT4_NULL PORTAbits.RA3 // ���� NULL
#define PORT4_POS PORTDbits.RD11 // ���� POS
#define PORT4_POS2 PORTEbits.RE3 // ���� POS2
#define PORT4_ENABLE_Tris TRISAbits.TRISA9 // ����� ENABLE
#define PORT4_DIR_Tris TRISBbits.TRISB11 // ����� DIR
#define PORT4_STEP_Tris TRISDbits.TRISD3 // ����� STEP
#define PORT4_ENABLE LATAbits.LATA9 // ����� ENABLE
#define PORT4_DIR LATBbits.LATB11 // ����� DIR
#define PORT4_STEP LATDbits.LATD3 // ����� STEP

//������������� ������� port5
#define PORT5_NULL_Tris TRISAbits.TRISA4 // ���� NULL
#define PORT5_POS_Tris TRISDbits.TRISD12 // ���� POS
#define PORT5_POS2_Tris TRISEbits.TRISE4 // ���� POS2
#define PORT5_NULL PORTAbits.RA4 // ���� NULL
#define PORT5_POS PORTDbits.RD12 // ���� POS
#define PORT5_POS2 PORTEbits.RE4 // ���� POS2
#define PORT5_ENABLE_Tris TRISAbits.TRISA9 // ����� ENABLE
#define PORT5_DIR_Tris TRISBbits.TRISB12 // ����� DIR
#define PORT5_STEP_Tris TRISDbits.TRISD4 // ����� STEP
#define PORT5_ENABLE LATAbits.LATA9 // ����� ENABLE
#define PORT5_DIR LATBbits.LATB12 // ����� DIR
#define PORT5_STEP LATDbits.LATD4 // ����� STEP

//������������� ������� port6
#define PORT6_NULL_Tris TRISAbits.TRISA5 // ���� NULL
#define PORT6_POS_Tris TRISDbits.TRISD13 // ���� POS
#define PORT6_POS2_Tris TRISEbits.TRISE5 // ���� POS2
#define PORT6_NULL PORTAbits.RA5 // ���� NULL
#define PORT6_POS PORTDbits.RD13 // ���� POS
#define PORT6_POS2 PORTEbits.RE5 // ���� POS2
#define PORT6_ENABLE_Tris TRISAbits.TRISA9 // ����� ENABLE
#define PORT6_DIR_Tris TRISBbits.TRISB13 // ����� DIR
#define PORT6_STEP_Tris TRISDbits.TRISD5 // ����� STEP
#define PORT6_ENABLE LATAbits.LATA9 // ����� ENABLE
#define PORT6_DIR LATBbits.LATB13 // ����� DIR
#define PORT6_STEP LATDbits.LATD5 // ����� STEP

//������������� ������� port7
#define PORT7_NULL_Tris TRISAbits.TRISA6 // ���� NULL
#define PORT7_POS_Tris TRISDbits.TRISD14 // ���� POS
#define PORT7_POS2_Tris TRISEbits.TRISE6 // ���� POS2
#define PORT7_NULL PORTAbits.RA6 // ���� NULL
#define PORT7_POS PORTDbits.RD14 // ���� POS
#define PORT7_POS2 PORTEbits.RE6 // ���� POS2
#define PORT7_ENABLE_Tris TRISAbits.TRISA9 // ����� ENABLE
#define PORT7_DIR_Tris TRISBbits.TRISB14 // ����� DIR
#define PORT7_STEP_Tris TRISDbits.TRISD6 // ����� STEP
#define PORT7_ENABLE LATAbits.LATA9 // ����� ENABLE
#define PORT7_DIR LATBbits.LATB14 // ����� DIR
#define PORT7_STEP LATDbits.LATD6 // ����� STEP

//������������� ������� port8
#define PORT8_NULL_Tris TRISAbits.TRISA7 // ���� NULL
#define PORT8_POS_Tris TRISDbits.TRISD15 // ���� POS
#define PORT8_POS2_Tris TRISEbits.TRISE7 // ���� POS2
#define PORT8_NULL PORTAbits.RA7 // ���� NULL
#define PORT8_POS PORTDbits.RD15 // ���� POS
#define PORT8_POS2 PORTEbits.RE7 // ���� POS2
#define PORT8_ENABLE_Tris TRISAbits.TRISA9 // ����� ENABLE
#define PORT8_DIR_Tris TRISBbits.TRISB15 // ����� DIR
#define PORT8_STEP_Tris TRISDbits.TRISD7 // ����� STEP
#define PORT8_ENABLE LATAbits.LATA9 // ����� ENABLE
#define PORT8_DIR LATBbits.LATB15 // ����� DIR
#define PORT8_STEP LATDbits.LATD7 // ����� STEP

#endif

int OCSetup(void);
int ProcessCmd(RR * rr);
int Control(void * _This, WORD* Buf, WORD BufSize);
WORD CalculateMove(RR * rr, OC_RECORD* buf, WORD count);
//int GoToCmd(RR * rr, double VTarget, double XTarget, DWORD Tick);
double GetAngle(WORD n);
int GetStatus(WORD n);
DWORD GetInterval(DWORD T, DWORD Xa, double dx, double a, double d);
/*
int GDateToJD(DateTime GDate, int * JDN, double * JD);
int JDToGDate(double JD, DateTime * GDate );
*/
double JulDay (BYTE D, BYTE M,WORD Y,BYTE h, BYTE m, BYTE s);
double LM_Sidereal_Time (double jd, double longitude);

void FS_ClearSector(WORD Sector);
void FS_ClearAll();
void FS_WriteArray(DWORD Addr, BYTE* val, WORD len);
void FS_ReadArray(DWORD Addr, BYTE* val, WORD len);

#endif //__OC_TIMER_H_

