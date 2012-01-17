#define __OCTIMER_C
#include "TCPIP Stack/TCPIP.h"
#include "OCTimer.h"



static DWORD_VAL Timer2Big;
static DWORD_VAL Timer3Big;
static RR rr1;
static RR rr2;
static RR rr3;

void __attribute__((__interrupt__,__no_auto_psv__)) _OC1Interrupt( void )
{       
   	if(rr1.DataCount>0){
       	rr1.DataCount--;
       	if((IFS1bits.U2RXIF != 1) && (rr1.DataCount <= BUF_SIZE_2)) {
            IFS1bits.U2RXIF = 1;    //  1 = ����� �������� ������   	
       	}
   	} else {
   	    rr1.RunState = ST_STOP;
   	    OC1CONbits.OCM = 0b000; // ��������� ������ OC
   	    IFS0bits.OC1IF = 0; // Clear OC1 interrupt flag
   	    return;
   	}
	rr1.T.Val = rr1.IntervalArray[rr1.NextReadFrom]; 	
   	rr1.NextReadFrom++;
   	if(rr1.NextReadFrom >= BUF_SIZE) rr1.NextReadFrom -= BUF_SIZE;
   	
   	OC1CONbits.OCM = 0b000; // ��������� ������ OC	   	
   	OC1R = rr1.T.word.LW;		// �������� �������� OCxR
   	OC1RS = rr1.T.word.LW + 50; // �������� �������� OCxRS
   	Timer2Big.word.LW = TMR2;   	
    //if((Timer2Big.Val <= rr1.T.Val)&&(Timer2Big.Val + 0x00010000 >= rr1.T.Val)){     
	OC1CONbits.OCM = 0b100;	// �������� ������ OC
	//}
    IFS0bits.OC1IF = 0; // Clear OC1 interrupt flag
    
}
void __attribute__((__interrupt__,__no_auto_psv__)) _OC2Interrupt( void )
{

    IFS0bits.OC2IF = 0; // Clear OC2 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _OC3Interrupt( void )
{

    IFS1bits.OC3IF = 0; // Clear OC3 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _OC4Interrupt( void )
{

    IFS1bits.OC4IF = 0; // Clear OC4 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _OC5Interrupt( void )
{

    IFS2bits.OC5IF = 0; // Clear OC5 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _OC6Interrupt( void )
{

    IFS2bits.OC6IF = 0; // Clear OC6 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _OC7Interrupt( void )
{

    IFS2bits.OC7IF = 0; // Clear OC7 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _OC8Interrupt( void )
{

    IFS2bits.OC8IF = 0; // Clear OC1 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _T2Interrupt( void )
{
    Timer2Big.word.HW++;
    Timer2Big.word.LW = TMR2;	
    /*
    if(OC1CONbits.OCM == 0b000){
    	if(rr1.RunState != ST_STOP){               	
            if((Timer2Big.Val <= rr1.T.Val)&&(Timer2Big.Val + 0x00010000 >= rr1.T.Val)){     
        	    OC1CONbits.OCM = 0b100;	// �������� ������ OC
        	}
        	if(Timer2Big.Val >= rr1.T.Val){     
        	    OC1CONbits.OCM = 0b100;	// �������� ������ OC
        	}	
    	}
	}*/
    IFS0bits.T2IF = 0; // Clear T2 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _T3Interrupt( void )
{
	
    IFS0bits.T3IF = 0; // Clear T3 interrupt flag
}
void __attribute__((__interrupt__,__no_auto_psv__)) _U2RXInterrupt( void )
{
    
    // ���������� ��� ������ ����������
    if(rr1.State != ST_STOP){
        Acceleration(&rr1);
    }        
    IFS1bits.U2RXIF = 0;
} 

int OCInit(void)
{
	BYTE i;		
	
	// MS1 = 1; MS2 = 1; 1/16
	MS1         = 1;    // ����� MS1  
	MS2         = 1; 	// ����� MS2  
	SLEEP       = 0; 	// ����� SLEEP
	RESET       = 0; 	// ����� RESET
	
	MS1_Tris    = 0; 	// ����� MS1
	MS2_Tris    = 0; 	// ����� MS2
	SLEEP_Tris  = 0;	// ����� SLEEP		
	RESET_Tris  = 0;	// ����� RESET	
	// ������������� ���������� ��� ���������� ������
	IFS1bits.U2RXIF = 0;
	IPC7bits.U2RXIP = 6;		// Priority level 6
	IEC1bits.U2RXIE = 1;
	IFS1bits.U2RXIF = 0;    //  1 = ����� 
	
	//������������� �����1
	{
    	
        {
        	PORT1_NULL_Tris   = 1; // ���� NULL
        	PORT1_POS_Tris    = 1; // ���� POS
        	PORT1_POS2_Tris   = 1; // ���� POS2	
        	
        	PORT1_ENABLE 	  = 0;// ����� ENABLE   
        	PORT1_DIR 		  = 0;// ����� DIR      
        	PORT1_STEP 		  = 0;// ����� STEP	
        
        	PORT1_ENABLE_Tris = 0;// ����� ENABLE
        	PORT1_DIR_Tris    = 0;// ����� DIR
        	PORT1_STEP_Tris   = 0;// ����� STEP		
        }    
    
        TmrInit(2);
        InitRR(&rr1);        
        InitRR(&rr2);
        InitRR(&rr3);
        IFS1bits.U2RXIF = 1;
        
        // ������������� OC1
    	{
        	OC1CONbits.OCM = 0b000;
            OC1CONbits.OCTSEL = 0;  	// ������� Timer2
            IPC0bits.OC1IP = 7;		    // ������� ��������� ���������� ��� OC1
            IFS0bits.OC1IF = 0;			// �������� ���� ����������
            IEC0bits.OC1IE = 1;			// ��������� ���������� �� OC1    
        }
        IFS0bits.OC1IF = 1;
    }       
    
	return 0;
}
/*
11 = 1:256
10 = 1:64
01 = 1:8
00 = 1:1
*/
int TmrInit(BYTE Num)
{	
	switch(Num){
		case 2:			
			T2CONbits.TON = 0; 		// Disable Timer
		    T2CONbits.TCS = 0; 		// Select internal instruction cycle clock
		    T2CONbits.TGATE = 0; 	// Disable Gated Timer mode
		    T2CONbits.TCKPS = 0b01; // Select 8:1 Prescaler 200ns
		    TMR2 = 0x00; 			// Clear timer register
		    PR2 = 0xFFFF; 			// Load the period value
		    IPC1bits.T2IP = 7; 	// Set Timer2 Interrupt Priority Level
		    IFS0bits.T2IF = 0; 		// Clear Timer2 Interrupt Flag
		    IEC0bits.T2IE = 1; 		// Enable Timer2 interrupt
		    T2CONbits.TON = 1; 		// Start Timer   
		    break;
		case 3:		
			T3CONbits.TON = 0; 		// Disable Timer
		    T3CONbits.TCS = 0; 		// Select internal instruction cycle clock
		    T3CONbits.TGATE = 0; 	// Disable Gated Timer mode
		    T3CONbits.TCKPS = 0b01; // Select 8:1 Prescaler 200ns
		    TMR3 = 0x00; 			// Clear timer register
		    PR3 = 0xFFFF; 			// Load the period value
		    IPC2bits.T3IP = 0; 		// Set Timer3 Interrupt Priority Level
		    IFS0bits.T3IF = 0; 		// Clear Timer3 Interrupt Flag
		    IEC0bits.T3IE = 1; 		// Enable Timer3 interrupt
		    T3CONbits.TON = 1; 		// Start Timer   			
			break;
		default: return -1;
	}		
	return 0;
}

int InitRR(RR * rr)
{
    double I;
    rr->Mass = 500.0f;
    rr->Radius = 0.30f;
    rr->Length = 2.0f;
    rr->Reduction = 360.0f;
    I = ((rr->Mass*rr->Radius*rr->Radius/4) + (rr->Mass*rr->Length*rr->Length/12))/rr->Reduction;     
    rr->K = (-0.000349812 * 200 * 180/PI)/I;
    rr->B = 0.751428571 / I;
    rr->dx = PI/(180.0*200.0*16.0); // ��� ����������� � ��������
    rr->TimerStep = 0.0000002; // ��� �������
    rr->Vend = 0 * Grad_to_Rad;
    rr->Xend = 1 * Grad_to_Rad; // ����� ��������� ����������. �.�. ����� ��������� ����� ���� ����������
    rr->DataCount = 0;
    rr->NextWriteTo = 0;
    rr->NextReadFrom = 0;    
    rr->XaccBeg = 0;
    rr->Xbeg = 0;
    rr->TimeBeg = Timer2Big.Val + (ARR_TYPE)(0.000035*BUF_SIZE/rr->TimerStep);       
    rr->State = ST_ACCELERATE;
    rr->NextState = ST_STOP;  
    rr->RunState = ST_ACCELERATE;
    rr->T.Val = 0;
    rr->Interval = 0xFFFFFFFF;
        
}
int Run(RR * rr)
{
    // x = V*T
    // T = X/V;
    WORD i;
    WORD j;
    WORD FreeData = BUF_SIZE - rr->DataCount;  
    double X = 0.0;
    //X = rr->Vend * rr->TimeBeg;
    DWORD Xb = rr->Xbeg/rr->dx;
    DWORD Xe = rr->Xend/rr->dx;
    ARR_TYPE T = 0;
    ARR_TYPE T1 = rr->TimeBeg;// = 0;
    for (i = 0; i < FreeData; i++){
        j = rr->NextWriteTo + i;
        if(j >= BUF_SIZE) j -= BUF_SIZE;  
        X += rr->dx;
        Xb++;
        T = rr->TimeBeg + (ARR_TYPE)(X / (rr->Vend * rr->TimerStep));
        if((Xe != 0) && (Xb >= Xe)){
            FreeData = i;
            rr->State = rr->NextState;                                
            break;
        }
        rr->IntervalArray[j] = T;
        T1 = T;
    }	
    rr->TimeBeg = T1;        
    rr->DataCount += FreeData;
    rr->NextWriteTo += FreeData;
    if(rr->NextWriteTo >= BUF_SIZE) rr->NextWriteTo -= BUF_SIZE;
    rr->Xbeg = Xb * rr->dx;
    return 0;
}
// ������ � ������� �������� �� ���������
int Acceleration(RR * rr)
{
    WORD j;        
    WORD FreeData = BUF_SIZE - rr->DataCount;
    ARR_TYPE T = 0;
    ARR_TYPE T1 = 0;
    ARR_TYPE T2 = 0;
    ARR_TYPE T3 = 0;
    ARR_TYPE dT = 0;
    double X;       // ��������� ���������� 
    ARR_TYPE Tb = 0.0;  
    double D;      
    DWORD Xb = rr->Xbeg/rr->dx;
    DWORD Xe = rr->Xend/rr->dx;
    double K = rr->K;
    double B = rr->B;
    double VKpB = 0.0; 
    
    double dx;
    double a;
    double c;
    double d;    
    WORD i = 0;
    ARR_TYPE e;
    WORD k = 0;
    
    e = 0.00007 / rr->TimerStep; //70us
    dx = rr->dx;
    X = rr->XaccBeg * rr->dx; 
    d = K/(2.0 * B * rr->TimerStep);
    c = K*d;
    a = 4.0 * B/(K*K);    
    
    /* TODO: ��-�������� �� �����, �.�. ����� ���������� ���������� ��������
    if(rr->XaccBeg > 0){        
        D = X *(X + a);
        if(D >= 0.0){
            T1 = (ARR_TYPE)((-X - sqrtf(D))*d);    
        }         
    }*/
    Tb = rr->TimeBeg - rr->T1;       	
   	T1 = rr->T1;
   	T = T1;
   	
   	// ���������� ������� ���������
    if(rr->Vend != 0.0){       
        VKpB = rr->Vend * K + B;
        D = B * VKpB;
        dT = (-VKpB + sqrtf(D))/(-K * VKpB * rr->TimerStep);
    }
    // �������������� 35 uSec (1431.5 ������ �� ���)
    for( i = 0; i < FreeData; i++) {
        if(rr->Interval >= e){
            // ���������� ������� ����
            j = rr->NextWriteTo + i;
            if(j >= BUF_SIZE) j -= BUF_SIZE;        
            Xb++;
    	    X += dx;            
            D = X *(X + a);
            if(D >= 0.0){
                T = (ARR_TYPE)((-X - sqrtf(D))*d);    
            }               
            if(((dT != 0)&&(T >= dT))||((Xe != 0)&&(Xb >= Xe))){
                FreeData = i;
                rr->State = rr->NextState;                                
                break;
            } 	    
            rr->IntervalArray[j] = Tb + T;    
            rr->Interval = T - T1;
            T1 = T;
        } else {
            // "������" ����������
            j = rr->NextWriteTo + i;
            if(j >= BUF_SIZE) j -= BUF_SIZE;        
            Xb++;
            if(k == 0){
                // ��������� ����� ����� 16 �����
                X += dx*16.0;            
                D = X *(X + a);
                if(D >= 0.0){
                    T2 = (ARR_TYPE)((-X - sqrtf(D))*d);    
                } 
                rr->Interval = (T2 - T1) / 16;   // �����, ������� ����� ����������                                                                
            } else {
                T += rr->Interval;                         
            }    
            if(((dT != 0)&&(T >= dT))||((Xe != 0)&&(Xb >= Xe))){
                FreeData = i;
                rr->State = rr->NextState;                                
                break;
            } 	    
            k++;
            if(k >= 16){
                rr->IntervalArray[j] = Tb + T2;                
                rr->Interval = T2 - T1;
                T1 = T2;
                k = 0;
            } else {
                rr->IntervalArray[j] = Tb + T;
                rr->Interval = T - T1;
                T1 = T;    
            }    
            
        }    
    }
    rr->T1 = T1;
    rr->TimeBeg = Tb + T1;
    rr->XaccBeg += FreeData; 
    rr->DataCount += FreeData;
    rr->NextWriteTo += FreeData;
    if(rr->NextWriteTo >= BUF_SIZE) rr->NextWriteTo -= BUF_SIZE;
    rr->Xbeg = Xb * rr->dx;
    return 0;
}

ARR_TYPE CalculateT(double X, double K, double B, double TimerStep)
{
    double D;	
    double Kx;

    Kx = X * K;
    D = Kx * Kx + 4.0 * X * B;
    if(D >= 0.0){
        return (ARR_TYPE)((-Kx + sqrtf(D))/(2.0 * TimerStep * B ));    
    }
    return (ARR_TYPE) 0;
}

// ���������� � ������� �������� �� ���������
int Deceleration(RR * rr)
{    
    WORD j;        
    WORD FreeData = BUF_SIZE - rr->DataCount;
    ARR_TYPE T = 0;
    ARR_TYPE T1 = 0;
    ARR_TYPE dT = 0;
    double X;       // ��������� ���������� 
    ARR_TYPE Tb = 0.0; 
    double K = rr->K;
    double B = rr->B;
    double VKpB = 0.0; 
    DWORD Xb = rr->Xbeg/rr->dx;
    DWORD Xe = rr->Xend/rr->dx;
    double TimerStep = rr->TimerStep;
    WORD i;

    X = rr->XaccBeg * rr->dx; 
    if(rr->XaccBeg > 0){
        T1 = CalculateT( X, K, B, TimerStep);         
    }
    Tb = rr->TimeBeg + T1;    

    if(rr->Vend != 0.0){        
        VKpB = rr->Vend * K + B;
        dT = (-VKpB + sqrt(B * VKpB))/(-K * VKpB * TimerStep);
    }

    for(i = 0; i < FreeData; i++) {        
        j = rr->NextWriteTo + i;
        if(j >= BUF_SIZE) j -= BUF_SIZE;
        X -= rr->dx;  
        if(X<=0.0){
            FreeData = i;
            rr->State = rr->NextState;            
            break;
        }
        Xb++;
        T = CalculateT( X, K, B, TimerStep);           
        if(((dT!=0)&&( T <= dT))||((Xe != 0)&&(Xb >= Xe))){
            FreeData = i;
            rr->State = rr->NextState;                                
            break;
        }
        rr->IntervalArray[j] = Tb - T;  
        T1 = T;

    }
    rr->TimeBeg = Tb - T1;
    rr->XaccBeg -= FreeData; 
    rr->DataCount += FreeData;
    rr->NextWriteTo += FreeData;
    if(rr->NextWriteTo >= BUF_SIZE) rr->NextWriteTo -= BUF_SIZE;
    rr->Xbeg = Xb * rr->dx;
    return 0;
}
