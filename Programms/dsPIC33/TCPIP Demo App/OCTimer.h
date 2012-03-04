#ifndef __OC_TIMER_H_
#define __OC_TIMER_H_
#include "..\..\guidance\stdafx.h"

#ifdef __C30__
#include "GenericTypeDefs.h"
#endif



#define OCM_MODULE_DISABLED         0x00 //000 Module Disabled Controlled by GPIO register �
#define OCM_ONE_SHOT_LOW            0x01 //001 Active-Low One-Shot 0 OCx rising edge
#define OCM_ONE_SHOT_HIGH           0x02 //010 Active-High One-Shot 1 OCx falling edge
#define OCM_TOGGLE                  0x03 //011 Toggle Current output is maintained OCx rising and falling edge
// ����������
#define OCM_DELAYED_ONE_SHOT        0x04 //100 Delayed One-Shot 0 OCx falling edge
#define OCM_CONTINUOUS_PULSE        0x05 //101 Continuous Pulse 0 OCx falling edge

#define OCM_PWM_WO_FAULT            0x06 //110 PWM without Fault Protection �0�, if OCxR is zero �1�, if OCxR is non-zero No interrupt
#define OCM_PWM_W_FAULT             0x07 //111 PWM with Fault Protection �0�, if OCxR is zero �1�, if OCxR is non-zero OCFA falling edge for OC1 to OC4

// �������� �������� �����
#define OCT_USE_TMR3                0x01 // 0 - ������������ ������ 2; 1 - ������ 3
#define OCT_USE_PERIOD_BUFFER       0x02 // 1 - ���������� ������ �������. 0 - � ������ ������� ������ 1 ��������
#define OCT_USE_PULSE_BUFFER        0x04 // 1 - ���������� ������ ������. 0 - � ������ ������ ������ 1 ��������
#define OCT_USE_STEPS_LEFT          0x08 // 1 - ���������� StepsLeft(). 0 - �������������� �������� �� ���������� ������������� ��������
#define OCT_IS_USE                  0x80 // ���� ������ ������������


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
#define FREQ_STEP 20
#define BUF_SIZE 64
// �������� BUF_SIZE
#define BUF_SIZE_2 32
// ������� ������
#define CQ_SIZE 10

//static double Accelerate[ACCELERATE_SIZE];
#define Grad_to_Rad 0.017453292519943295
#define Rad_to_Grad 57.295779513082323

// ��������� 

typedef struct ARR_TYPE {
    DWORD FixedPoint;   // ������� �����
    DWORD Interval;     // �������� �� ���������� �����
    WORD  Count;        // ���������� ����������
    WORD  Correction;   // ������� �� ������� ������� ��������� �� 32
} ARR_TYPE;

typedef enum Cmd{ // �������
    CM_STOP,            // ������������ (������� �������� �� ���������)
    CM_RUN_WITH_SPEED,  // ��������� � �������� ��������� �� ���������
    CM_RUN_TO_POINT,    // ��������� �� ���������� ����
}GD_CMD;

typedef enum State{     // ���������
    ST_STOP,            // ����������
    ST_ACCELERATE,      // �����������
    ST_RUN,             // �������� � ���������� ���������
    ST_DECELERATE,      // ��������
    ST_BUFFER_FREE      // ������������� �� �������� ������ ( ������� )
}GD_STATE;

// ������� ������. ���� �������� ����� 0, �� ��� ���� �� ������������, ���� ����������� �������������
typedef struct CMD_QUEUE{
    GD_STATE State;
    double Vend;
    double Xend;
    INT Direction;
    LONG RunStep; //  ���������� ����� �� ���������� �������
}Cmd_Queue;

typedef struct RR{

    // �����
    // IntervalArray
    // |      NextWriteTo
    // |      |      NextReadFrom
    // |      |      |
    // v      v      v
    // 0======-------=========
    //
    // ��������� ���������� �������� ( �������� ��������� ��������)
    // | ������������� ���������
    // | | ����� ��������
    // | | |
    // v v v
    // -------========--------
    //                    
    //  
    DWORD                   Interval;
    ARR_TYPE                IntervalArray[BUF_SIZE];    // ������ �������� ������� (��������� ������)
    WORD                    NextReadFrom;               // ������ ������� �������. ��������� �� ������ �������� �������
    WORD                    NextWriteTo;                // ������ ������� �������. ��������� �� ������ ��������� �������
    WORD                    DataCount;                  // ���������� ������ � �������.
    DWORD_VAL               T;


    // �������
    GD_CMD                  Cmd;
    GD_STATE                CacheState;
    GD_STATE                RunState;

    Cmd_Queue               CmdQueue[CQ_SIZE];          // ������� ������
    WORD                    NextCacheCmd;
    WORD                    NextWriteCmd;
    WORD                    NextExecuteCmd;
    WORD                    CmdCount;

    // ��������� ����������
    LONG                    XPosition;
    int                     RunDir;                     // ����������� �������� ��� �������� ( ������� �������� ������ Dir )
    LONG                    RunCmdCounter;
    // ��������� ������������
    LONG                    XCachePos;                  // ������� ��������� � ��������
    int                     CalcDir;                    // ����������� �������� ��� ��������
    LONG                    CacheCmdCounter;
    // ���o���� ���������:
    DWORD                   TimeBeg;
    LONG                    XaccBeg;                    //��������� ������� ��������� (���������� ����� ����� �����)

    // ��������� ����������� �� ������ ���������
    double                  Vend;                       //(���� ����� ��������, �� ������� ���������� ���������)
    double                  Xend;


    // ��������� (�����������)
    double                  dX_acc_dec_pos;                 // ������� ��������� � �������� � ��������
    DWORD                  d;
    DWORD                  a;
    // ���������
    DWORD  e; // ���� �������� ������ ����� ��������, ��������� �� ������� ����������
    double K;
    double B;
    double TimerStep;
    double dx;
    double Mass;
    double Radius;
    double Length;
    double Reduction;
    WORD StepPerTurn;
    WORD uStepPerStep;
    BYTE Index;
    BYTE TmrId;
}RR;

#ifdef __C30__

#define MS1_Tris TRISGbits.TRISG12 // ����� MS1
#define MS2_Tris TRISGbits.TRISG13 // ����� MS2
#define SLEEP_Tris TRISGbits.TRISG14 // ����� SLEEP
#define RESET_Tris TRISGbits.TRISG15 // ����� RESET
#define MS1 LATGbits.LATG12 // ����� MS1
#define MS2 LATGbits.LATG13 // ����� MS2
#define SLEEP LATGbits.LATG14 // ����� SLEEP
#define RESET LATGbits.LATG15 // ����� RESET

//�������� ������� port1
#define PORT1_NULL_Tris TRISAbits.TRISA0 // ���� NULL
#define PORT1_POS_Tris TRISDbits.TRISD8 // ���� POS
#define PORT1_POS2_Tris TRISEbits.TRISE0 // ���� POS2
#define PORT1_NULL PORTAbits.RA0 // ���� NULL
#define PORT1_POS PORTDbits.RD8 // ���� POS
#define PORT1_POS2 PORTEbits.RE0 // ���� POS2
#define PORT1_ENABLE_Tris TRISAbits.TRISA9 // ����� ENABLE
#define PORT1_DIR_Tris TRISBbits.TRISB8 // ����� DIR
#define PORT1_STEP_Tris TRISDbits.TRISD0 // ����� STEP
#define PORT1_ENABLE LATAbits.LATA9 // ����� ENABLE
#define PORT1_DIR LATBbits.LATB8 // ����� DIR
#define PORT1_STEP LATDbits.LATD0 // ����� STEP

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

int OCInit(void);
int TmrInit(BYTE Num);
//int OCTimerInit(BYTE num, DWORD Steps, DWORD * Periods, DWORD * Pulses, BYTE TmrNum,WORD ocm);
int Run(RR * rr);
int Acceleration(RR * rr);
int Deceleration(RR * rr);
int Control(RR * rr);

int CacheNextCmd(RR * rr);
int PushCmdToQueue(RR * rr, GD_STATE State, double Vend, double Xend, int Direction );
int ProcessTimer(BYTE id, RR * rr);
DWORD GetBigTmrValue(BYTE id);
int DisableOC(BYTE oc);
int EnableOC(BYTE oc);
BOOL IsDisableOC(BYTE oc);
int SetOC(BYTE oc, WORD LW);
int ProcessOC(RR * rr);
int SetDirection(BYTE oc, BYTE Dir);

int CalculateBreakParam(RR * rr, GD_STATE State, int Direction, double Vbeg, double Xbeg, double * Vend, double * Xend, LONG * Xbreak);

int GDateToJD(DateTime GDate, int * JDN, double * JD);
int JDToGDate(double JD, DateTime * GDate );
#endif //__OC_TIMER_H_

