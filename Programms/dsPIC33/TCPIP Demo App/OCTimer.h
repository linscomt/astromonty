#ifndef __OC_TIMER_H_
#define __OC_TIMER_H_
#include "GenericTypeDefs.h"
//struct OC_CONFIG {
	
//};

// �������� �������� �����
#define OC_TIMER_USE_TMR3		0x01	// 0 - ������������ ������ 2; 1 - ������ 3
#define OC_TIMER_IS_USE 		0x80	// ���� ������ ������������
#define OC_TIMER_ONE_SPEED		0x40	// ������ �������� �� ����� ��������(�� ������������ �����)
#define OC_TIMER_CONTINUED		0x20	// ������ �������� ��������� (������������ �������� StepsLeft)

#define OCM_MODULE_DISABLED		0x00	//000 Module Disabled Controlled by GPIO register �
#define OCM_ONE_SHOT_LOW		0x01	//001 Active-Low One-Shot 0 OCx rising edge
#define OCM_ONE_SHOT_HIGH		0x02	//010 Active-High One-Shot 1 OCx falling edge
#define OCM_TOGGLE				0x03	//011 Toggle Current output is maintained OCx rising and falling edge
// ����������
#define OCM_DELAYED_ONE_SHOT	0x04	//100 Delayed One-Shot 0 OCx falling edge
#define OCM_CONTINUOUS_PULSE	0x05	//101 Continuous Pulse 0 OCx falling edge

#define OCM_PWM_WO_FAULT		0x06	//110 PWM without Fault Protection �0�, if OCxR is zero �1�, if OCxR is non-zero No interrupt
#define OCM_PWM_W_FAULT			0x07	//111 PWM with Fault Protection �0�, if OCxR is zero �1�, if OCxR is non-zero OCFA falling edge for OC1 to OC4

struct OC_TIMER_TYPE {
	BYTE Flags;					// ����� ������ 		
	BYTE PeriodsLeft; 			// ���������� ���������� �������� � ������
	WORD OCxCON;				// ��������� OC
	DWORD StepsCurrent;			// ���������� ���������� �����
	DWORD StepsLeft;		    // �������� �����
	DWORD_VAL Period;			// ������� ������
	DWORD_VAL Pulse;			// ������� �����	
	DWORD * Periods;			// ����� ��������
	DWORD * Pulses;
} OCTimers[8];


#define MS1_Tris 			TRISGbits.TRISG12 // ����� MS1
#define MS2_Tris 			TRISGbits.TRISG13 // ����� MS2
#define SLEEP_Tris 			TRISGbits.TRISG14 // ����� SLEEP		
#define RESET_Tris 			TRISGbits.TRISG15 // ����� RESET	
#define MS1 				LATGbits.LATG12   // ����� MS1  
#define MS2 				LATGbits.LATG13   // ����� MS2  
#define SLEEP 				LATGbits.LATG14   // ����� SLEEP
#define RESET 				LATGbits.LATG15   // ����� RESET

//�������� ������� port1
#define PORT1_NULL_Tris   	TRISAbits.TRISA0  // ���� NULL
#define PORT1_POS_Tris    	TRISDbits.TRISD8  // ���� POS
#define PORT1_POS2_Tris   	TRISEbits.TRISE0  // ���� POS2			
#define PORT1_NULL        	PORTAbits.RA0	  // ���� NULL       
#define PORT1_POS 			PORTDbits.RD8     // ���� POS       
#define PORT1_POS2 			PORTEbits.RE0     // ���� POS2	
#define PORT1_ENABLE_Tris 	TRISAbits.TRISA9  // ����� ENABLE
#define PORT1_DIR_Tris    	TRISBbits.TRISB8  // ����� DIR
#define PORT1_STEP_Tris   	TRISDbits.TRISD0  // ����� STEP		
#define PORT1_ENABLE 		LATAbits.LATA9    // ����� ENABLE   
#define PORT1_DIR 			LATBbits.LATB8    // ����� DIR      
#define PORT1_STEP 			LATDbits.LATD0    // ����� STEP	

//������������� ������� port2
#define PORT2_NULL_Tris   	TRISAbits.TRISA1  // ���� NULL
#define PORT2_POS_Tris    	TRISDbits.TRISD9  // ���� POS
#define PORT2_POS2_Tris   	TRISEbits.TRISE1  // ���� POS2			
#define PORT2_NULL        	PORTAbits.RA1    // ���� NULL       
#define PORT2_POS 			PORTDbits.RD9    // ���� POS       
#define PORT2_POS2 			PORTEbits.RE1    // ���� POS2	
#define PORT2_ENABLE_Tris 	TRISAbits.TRISA10 // ����� ENABLE
#define PORT2_DIR_Tris    	TRISBbits.TRISB9  // ����� DIR
#define PORT2_STEP_Tris   	TRISDbits.TRISD1  // ����� STEP		
#define PORT2_ENABLE 		LATAbits.LATA10   // ����� ENABLE   
#define PORT2_DIR 			LATBbits.LATB9    // ����� DIR      
#define PORT2_STEP 			LATDbits.LATD1    // ����� STEP	

//������������� ������� port3
#define PORT3_NULL_Tris   	TRISAbits.TRISA2  // ���� NULL
#define PORT3_POS_Tris    	TRISDbits.TRISD10 // ���� POS
#define PORT3_POS2_Tris   	TRISEbits.TRISE2  // ���� POS2			
#define PORT3_NULL        	PORTAbits.RA2    // ���� NULL       
#define PORT3_POS 			PORTDbits.RD10   // ���� POS       
#define PORT3_POS2 			PORTEbits.RE2    // ���� POS2	
#define PORT3_ENABLE_Tris 	TRISAbits.TRISA9  // ����� ENABLE
#define PORT3_DIR_Tris    	TRISBbits.TRISB10 // ����� DIR
#define PORT3_STEP_Tris   	TRISDbits.TRISD2  // ����� STEP		
#define PORT3_ENABLE 		LATAbits.LATA9    // ����� ENABLE   
#define PORT3_DIR 			LATBbits.LATB10   // ����� DIR      
#define PORT3_STEP 			LATDbits.LATD2    // ����� STEP	

//������������� ������� port4
#define PORT4_NULL_Tris   	TRISAbits.TRISA3  // ���� NULL
#define PORT4_POS_Tris    	TRISDbits.TRISD11 // ���� POS
#define PORT4_POS2_Tris   	TRISEbits.TRISE3  // ���� POS2			
#define PORT4_NULL        	PORTAbits.RA3    // ���� NULL       
#define PORT4_POS 			PORTDbits.RD11   // ���� POS       
#define PORT4_POS2 			PORTEbits.RE3    // ���� POS2	
#define PORT4_ENABLE_Tris 	TRISAbits.TRISA9  // ����� ENABLE
#define PORT4_DIR_Tris    	TRISBbits.TRISB11 // ����� DIR
#define PORT4_STEP_Tris   	TRISDbits.TRISD3  // ����� STEP		
#define PORT4_ENABLE 		LATAbits.LATA9    // ����� ENABLE   
#define PORT4_DIR 			LATBbits.LATB11   // ����� DIR      
#define PORT4_STEP 			LATDbits.LATD3    // ����� STEP	

//������������� ������� port5
#define PORT5_NULL_Tris   	TRISAbits.TRISA4  // ���� NULL
#define PORT5_POS_Tris    	TRISDbits.TRISD12 // ���� POS
#define PORT5_POS2_Tris   	TRISEbits.TRISE4  // ���� POS2			
#define PORT5_NULL        	PORTAbits.RA4    // ���� NULL       
#define PORT5_POS 			PORTDbits.RD12   // ���� POS       
#define PORT5_POS2 			PORTEbits.RE4    // ���� POS2	
#define PORT5_ENABLE_Tris 	TRISAbits.TRISA9  // ����� ENABLE
#define PORT5_DIR_Tris    	TRISBbits.TRISB12 // ����� DIR
#define PORT5_STEP_Tris   	TRISDbits.TRISD4  // ����� STEP		
#define PORT5_ENABLE 		LATAbits.LATA9    // ����� ENABLE   
#define PORT5_DIR 			LATBbits.LATB12   // ����� DIR      
#define PORT5_STEP 			LATDbits.LATD4    // ����� STEP	

//������������� ������� port6
#define PORT6_NULL_Tris   	TRISAbits.TRISA5  // ���� NULL
#define PORT6_POS_Tris    	TRISDbits.TRISD13 // ���� POS
#define PORT6_POS2_Tris   	TRISEbits.TRISE5  // ���� POS2			
#define PORT6_NULL        	PORTAbits.RA5    // ���� NULL       
#define PORT6_POS 			PORTDbits.RD13   // ���� POS       
#define PORT6_POS2 			PORTEbits.RE5    // ���� POS2	
#define PORT6_ENABLE_Tris 	TRISAbits.TRISA9  // ����� ENABLE
#define PORT6_DIR_Tris    	TRISBbits.TRISB13 // ����� DIR
#define PORT6_STEP_Tris   	TRISDbits.TRISD5  // ����� STEP		
#define PORT6_ENABLE 		LATAbits.LATA9    // ����� ENABLE   
#define PORT6_DIR 			LATBbits.LATB13   // ����� DIR      
#define PORT6_STEP 			LATDbits.LATD5    // ����� STEP	

//������������� ������� port7
#define PORT7_NULL_Tris   	TRISAbits.TRISA6  // ���� NULL
#define PORT7_POS_Tris    	TRISDbits.TRISD14 // ���� POS
#define PORT7_POS2_Tris   	TRISEbits.TRISE6  // ���� POS2			
#define PORT7_NULL        	PORTAbits.RA6    // ���� NULL       
#define PORT7_POS 			PORTDbits.RD14   // ���� POS       
#define PORT7_POS2 			PORTEbits.RE6    // ���� POS2	
#define PORT7_ENABLE_Tris 	TRISAbits.TRISA9  // ����� ENABLE
#define PORT7_DIR_Tris    	TRISBbits.TRISB14 // ����� DIR
#define PORT7_STEP_Tris   	TRISDbits.TRISD6  // ����� STEP		
#define PORT7_ENABLE 		LATAbits.LATA9    // ����� ENABLE   
#define PORT7_DIR 			LATBbits.LATB14   // ����� DIR      
#define PORT7_STEP 			LATDbits.LATD6    // ����� STEP	

//������������� ������� port8
#define PORT8_NULL_Tris   	TRISAbits.TRISA7  // ���� NULL
#define PORT8_POS_Tris    	TRISDbits.TRISD15 // ���� POS
#define PORT8_POS2_Tris   	TRISEbits.TRISE7  // ���� POS2			
#define PORT8_NULL        	PORTAbits.RA7    // ���� NULL       
#define PORT8_POS 			PORTDbits.RD15   // ���� POS       
#define PORT8_POS2 			PORTEbits.RE7    // ���� POS2	
#define PORT8_ENABLE_Tris 	TRISAbits.TRISA9  // ����� ENABLE
#define PORT8_DIR_Tris    	TRISBbits.TRISB15 // ����� DIR
#define PORT8_STEP_Tris   	TRISDbits.TRISD7  // ����� STEP		
#define PORT8_ENABLE 		LATAbits.LATA9    // ����� ENABLE   
#define PORT8_DIR 			LATBbits.LATB15   // ����� DIR      
#define PORT8_STEP 			LATDbits.LATD7    // ����� STEP	

int OCInit(void);
int TmrInit(BYTE Num);
//int OCTimerInit(BYTE num, DWORD Steps, DWORD * Periods, DWORD * Pulses, BYTE TmrNum,WORD ocm);



#endif
