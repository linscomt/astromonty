// guidance.cpp: ���������� ����� ����� ��� ����������.
//

#include "stdafx.h"
#include "guidance.h"
#include <math.h>
#include <stdio.h>


#define MAX_LOADSTRING 100

#define PI 3.1415926535897932384626433832795
#define ACCELERATE_SIZE 111
#define FREQ_STEP 20
#define BUF_SIZE 64
//static double Accelerate[ACCELERATE_SIZE];
typedef double ARR_TYPE;
typedef struct FREQ_POWER {
    WORD Freq;
    double Power;
} FREQ_POWER;
static FREQ_POWER FreqPower[] = {
    {0,     0.850000000},
    {100,   0.764642857},
    {250,   0.666666667},
    {500,   0.600000000},
    {750,   0.533333333},
    {1000,  0.466666667},
    {1250,  0.400000000},
    {1500,  0.333333333},
    {1750,  0.222222222},
    {2000,  0.100000000},
    {2200,  0.000000000}
};
typedef enum Cmd {                  // �������
	CM_STOP,                // ������������ (������� �������� �� ���������)
	CM_RUN_WITH_SPEED,      // ��������� � �������� ��������� �� ��������� 
	CM_RUN_TO_POINT,        // ��������� �� ���������� ����        
} GD_CMD;
typedef enum State {                // ���������
        ST_FREE,                // ������� ��������� ������� (��������, ��������������� ����� ������� ���������)
	ST_STOP,                // ����������
	ST_ACCELERATE,          // �����������
	ST_RUN,                 // �������� � ���������� ���������
	ST_DECELERATE           // ��������
} GD_STATE;
typedef struct RR {
    
    // �����
    // IntervalArray
    // |      NextWriteTo
    // |      |      NextReadFrom
    // |      |      | 
    // v      v      v
    // 0======-------=========
    // 
    //       ��������� ���������� �������� ( �������� ��������� ��������)
    //       |   ������������� ���������
    //       |   |   ����� �������� 
    //       |   |   |
    //       v   v   v 
    // -------========--------
    // 
    
    ARR_TYPE    IntervalArray[BUF_SIZE];    // ������ �������� ������� (��������� ������)
    WORD        NextReadFrom;               // ������ ������� �������. ��������� �� ������ �������� �������
    WORD        NextWriteTo;                // ������ ������� �������. ��������� �� ������ ��������� �������
    WORD        DataCount;                  // ���������� ������ � �������.

    // �������
    GD_CMD      Cmd;
    GD_STATE    State;    
    GD_STATE    NextState;
    
    // ��������� ���������
    DWORD       CurrentX;                   // ������� ����� ����
    BYTE        Direction;                  // ����������� ��������
    
    // ���������� ����������
    double      cmdTargetSpeed;             // ����������� �� ��������
    double      cmdTargetAngle;             // ��������� �� ����
    //DWORD   TargetX;
    
    
    // ���o���� ���������:
    ARR_TYPE    TimeBeg;  
    DWORD       XaccBeg;                    //(���������� ����� ����� �����)
    //���������, ������ � �������� ������
    ARR_TYPE    TimePos;
    DWORD       Xpos;
    DWORD       XaccPos;                    //(����� ����� �����) 

    // ��������� ����������� �� ������ ���������
    double      Vend;                       //(���� ����� ��������, �� ������� ���������� ���������)
    double      Tend;                       //(��� ��������� ������ ����� ����� ����� ����� � �����, ����� ��������� ������)
    double      Xend;
    DWORD       XaccEnd;                    //���������� ��������� (DWORD)

    // ���������
    double      K;
    double      B;
    double      TimerStep;
    double      dx;

} RR;
RR rr1;
// �������� ��� ��������
// CM_STOP
// 1. ���������� state = ST_DECELERATE
//    ������ Vend = 0.0
//    ���������� NextState = ST_STOP
//
// CM_RUN_WITH_SPEED
// 1. 


// ���������� ����������:
#define FIRST_TIMER 1
int nTimerID;
HINSTANCE hInst;// ������� ���������
HWND hWindow;

TCHAR szTitle[MAX_LOADSTRING];					// ����� ������ ���������
TCHAR szWindowClass[MAX_LOADSTRING];			// ��� ������ �������� ����

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
 

int Run(RR * rr);
int Acceleration(RR * rr);
int Deceleration(RR * rr);

void Calc(HWND hWnd, HDC hdc);
DWORD MaxAcceleration(DWORD Xb, DWORD Xe,double dx, double K, double B, double * T, DWORD Len, DWORD * Xpos);



int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���������� ��� �����.

	MSG msg;
	HACCEL hAccelTable;

	// ������������� ���������� �����
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GUIDANCE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ��������� ������������� ����������:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
		
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUIDANCE));

	SetTimer(hWindow, FIRST_TIMER, 1, (TIMERPROC) NULL);
	// ���� ��������� ���������:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  �������: MyRegisterClass()
//
//  ����������: ������������ ����� ����.
//
//  �����������:
//
//    ��� ������� � �� ������������� ���������� ������ � ������, ���� �����, ����� ������ ���
//    ��� ��������� � ��������� Win32, �� �������� ������� RegisterClassEx'
//    ������� ���� ��������� � Windows 95. ����� ���� ������� ����� ��� ����,
//    ����� ���������� �������� "������������" ������ ������ � ���������� �����
//    � ����.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GUIDANCE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_GUIDANCE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �������: InitInstance(HINSTANCE, int)
//
//   ����������: ��������� ��������� ���������� � ������� ������� ����.
//
//   �����������:
//
//        � ������ ������� ���������� ���������� ����������� � ���������� ����������, � �����
//        ��������� � ��������� �� ����� ������� ���� ���������.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // ��������� ���������� ���������� � ���������� ����������

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	hWindow = hWnd;
	return TRUE;
}

//
//  �������: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����������:  ������������ ��������� � ������� ����.
//
//  WM_COMMAND	- ��������� ���� ����������
//  WM_PAINT	-��������� ������� ����
//  WM_DESTROY	 - ������ ��������� � ������ � ���������.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	static int T = 0;
	static bool k = false;
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// ��������� ����� � ����:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_TIMER: 

		switch (wParam) 
		{ 
		case FIRST_TIMER: 	
			T++;
			if(T>=360) T-=360;
			//GetClientRect(hWnd, &rect);
			rect.top = 100;
			rect.bottom = 300;
			rect.left = 100;
			rect.right = 300;
			//InvalidateRect(hWnd, &rect, TRUE);
			k = true;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
				//LineTo(hdc, 100,100);
		// TODO: �������� ����� ��� ���������...

				
// 		if(k)
// 		{
// 			double X = 200+100*sin(T*PI/180);
// 			double Y = 200+100*cos(T*PI/180);
// 			MoveToEx(hdc, 200,200, NULL);
// 			LineTo(hdc,(int)X, (int)Y);
// 			k=false;
// 		} else 
			Calc(hWnd, hdc);	
		EndPaint(hWnd, &ps);
	

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
		
	return 0;
}

// ���������� ��������� ��� ���� "� ���������".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

double LinInt(double x1,double y1,double x2,double y2, double x)
{
	if(x1!=x2) {
		return y1+(y2-y1)*(x-x1)/(x2-x1);
	}else return y1;
}
void Calc(HWND hWnd, HDC hdc)
{
    double D = 0.0;
    static double A = 0.0; //��������� � �������� � ��� �� ���
    static double A1 = 0.0; //��������� � �������� � ��� �� ���
    static double A2 = 0.0; //��������� � �������� � ��� �� ���
    static double V1 = 0.0; //��������� � �������� � ��� �� ���
    static double dt = 0.0; // ��������� �������
    static double V = 0.0;  // ���������� �������� � ��������
    //static double dX = 1.0/(200.0*16.0);// ��� ����������� � �������� (� 1 ������� 3200 �����)
    static double dX = PI/(180.0*200.0*16.0); // ��� ����������� � ��������
    static double X = 0;    // ������ ����������� � ��������
    static double X1 = 0;    // ������ ����������� � ��������
    static double T = 0;    // ������ �����
    static double T1 = 0;    // ������ �����
    static double T2 = 0;    // ������ �����
    double dt1 = 0.0; // ��������� �������
    int W = 0;       // ����� ��� ����������� ������
    int K1 = -1;
    int W1 = 0;
    int K3 = -1;
    double timer1 = 0;  // �������� �������
    //DWORD F = 0;
    DWORD i = 0;
    //DWORD k = 1;
    RECT rect;
    static double Mass = 500.0f;
    static double Radius = 0.30f;
    static double Length = 2.0f;
    static double Reduction = 360.0f;
    static double Grad_to_Rad = PI / 180.0;
    static double Rad_to_Grad = 180.0 / PI;
    static double I = ((Mass*Radius*Radius/4) + (Mass*Length*Length/12))/Reduction; 
    static double K = (-0.000349812 * 200 * 180/PI)/I;
    //static double K = (-0.000154286 * 200 * 180/PI)/I;
    //static double B = 0.79962406 / I;
    static double B = 0.751428571 / I;

    static DWORD SizeX = 200;
    static DWORD SizeY = 50;
    static double Pi = PI;
    static double TT[64];
    static DWORD TTLen = 64;
    static DWORD Count = 0;
    DWORD XPos = 0;

    HGDIOBJ original = NULL;

    //Save original object.
    original = SelectObject(hdc,GetStockObject(DC_PEN));

    //double Vf = 180 * 200/PI;
    //char RRR[256];
    GetClientRect(hWnd, &rect);
    MoveToEx(hdc, rect.left+9, rect.bottom - 9, NULL);
    LineTo(hdc, rect.right - 9, rect.bottom - 9);
    MoveToEx(hdc, rect.left+9, rect.bottom - 9, NULL);
    LineTo(hdc, rect.left+9, rect.top);        
    

    for (DWORD i = 0; i < rect.bottom/SizeY ; i++) {
        if(i % 10 == 0){
            SetDCPenColor(hdc,RGB(0,0,0));
        } else {
            SetDCPenColor(hdc,RGB(200,200,200));
        }
        MoveToEx(hdc, rect.left + 10, rect.bottom - 10 - (int)(i*SizeY), NULL);
        LineTo(hdc,   rect.right -10, rect.bottom - 10 - (int)(i*SizeY) );  
    }
    for (DWORD i = 0; i < rect.right*10/(SizeX) ; i++) {
        if(i % 10 == 0){
            SetDCPenColor(hdc,RGB(0,0,0));
        } else {
            SetDCPenColor(hdc,RGB(200,200,200));
        }
        MoveToEx(hdc, rect.left + 10 + (int)(i*SizeX/10), rect.bottom - 10, NULL);
        LineTo(  hdc, rect.left + 10 + (int)(i*SizeX/10), rect.top); 
    }

    DWORD Px = rect.left + 10;
    DWORD Py = rect.bottom - 10 ;//- (rect.bottom/4);
    POINT TX = {Px,Py};
    POINT TV = {Px,Py};
    POINT TA = {Px,Py};
    POINT VA = {Px,Py};

       
    SetDCPenColor(hdc,RGB(0,200,0));
    MoveToEx(hdc, Px, Py - (int)(0.004166667*SizeY), NULL);
    LineTo(hdc, rect.right - 9, Py - (int)(0.004166667*SizeY));

    dt = 0.0;
    T = 0.0;
    T1 = 0.0;
    X = 0.0;
    V = 0.0;
    V1 = 0.0;
    rr1.B = B;
    rr1.K = K;
    rr1.dx = dX;
    rr1.TimerStep = 1;
    rr1.Vend = 10.0 * Grad_to_Rad;
    rr1.DataCount = 0;
    rr1.NextWriteTo = 0;
    rr1.NextReadFrom = 0;
    rr1.Tend = 0.0;
    
    rr1.Xpos = 0;
    rr1.State = ST_ACCELERATE;
    rr1.NextState = ST_STOP;
   

//     do{
//         rr1.DataCount = 0;
//         Acceleration(&rr1);        
//     }while(rr1.State == ST_ACCELERATE);
// 
//     Run(&rr1);
//     rr1.DataCount = 150;
//     Run(&rr1);
//     rr1.DataCount = 150;
//     Run(&rr1);
    //     rr1.State = ST_DECELERATE;
    //     rr1.TargetSpeed = 1.0 * Grad_to_Rad;
    do{		
        //Count = MaxAcceleration(XPos, 180*3200, dX, K, B, TT, TTLen, &XPos);
        switch(rr1.State){
            case ST_ACCELERATE:
                Acceleration(&rr1);                  
                break;
            case ST_RUN:
                Run(&rr1);
                break;
            case ST_DECELERATE:
                Deceleration(&rr1);
                break;
            case ST_STOP:
                break;
            case ST_FREE:
                rr1.State = ST_DECELERATE;                
                break;
        }
        int j;
        //if(rr1.DataCount == 0) rr1.State = ST_STOP;
        for( i = 0; i < rr1.DataCount; i++) {            
            T1 = T;
            V1 = V;
            T = rr1.IntervalArray[rr1.NextReadFrom] * rr1.TimerStep;
            rr1.NextReadFrom++;
            if(rr1.NextReadFrom >= BUF_SIZE) rr1.NextReadFrom -= BUF_SIZE;
            
            if(T-T1 != 0.0){
                X += dX;
                V = dX/(T-T1);
            }else {
                V = 0.0;
            }
            
            A = (V-V1)/(T-T1);
              
//             T = TT[i];
//             X += dX;
//             V = B*T/(1-K*T);
//             A = B/(1-K*T);
        
            K3 = (int)(T*SizeX);
            //if( K3 != K1)
            {
                //Change the DC pen color
                SetDCPenColor(hdc,RGB(0,0,255));
                MoveToEx(hdc, TX.x, TX.y, NULL);
                TX.x = Px + (int)(T*SizeX);
                TX.y = Py - (int)(X*Rad_to_Grad*SizeY/2);
                LineTo(hdc, TX.x, TX.y);  

                SetDCPenColor(hdc,RGB(0,255,0));
                MoveToEx(hdc, TV.x, TV.y, NULL);
                TV.x = Px + (int)(T*SizeX);
                TV.y = Py - (int)(V*Rad_to_Grad*SizeY);
                LineTo(hdc, TV.x, TV.y);  

                SetDCPenColor(hdc,RGB(255,0,0));
                MoveToEx(hdc, TA.x, TA.y, NULL);
                TA.x = Px + (int)(T*SizeX);
                TA.y = Py - (int)(A*Rad_to_Grad*SizeY/10);
                LineTo(hdc, TA.x, TA.y);  

                SetDCPenColor(hdc,RGB(200,200,0));
                MoveToEx(hdc, VA.x, VA.y, NULL);
                VA.x = Px + (int)(V*Rad_to_Grad*SizeY/10);
                VA.y = Py - (int)(A*Rad_to_Grad*SizeY/10);
                LineTo(hdc, VA.x, VA.y);                  

                K1 = K3;
            }
        }     
        rr1.DataCount = 0;
    }while ( (X < 0.590) && (rr1.State!= ST_STOP)) /*rr1.State!=ST_STOP)*/;       
    //Restore original object.
    SelectObject(hdc,original);

}

// �����
// IntervalArray
// |      NextWriteTo
// |	  |      NextReadFrom
// |      |      | 
// v      v      v
// 0======-------=========
// 


int Run(RR * rr)
{
	// x = V*T
	// T = X/V;
// 	WORD i;
// 	WORD j;
// 	WORD FreeData = BUF_SIZE - rr->DataCount;
//         rr->X = rr->dwX * rr->dx;
// 	for (i = 0; i < FreeData; i++){
// 	    j = rr->NextWriteTo + i;
// 	    if(j >= BUF_SIZE) j -= BUF_SIZE;
// 	    rr->IntervalArray[j] = (ARR_TYPE)(rr->X / (rr->BufCurSpeed * rr->TimerStep));		
// 	    rr->X += rr->dx;
// 	}	
//         rr->dwX += FreeData;
// 	rr->DataCount += FreeData;
// 	rr->NextWriteTo += FreeData;
// 	if(rr->NextWriteTo >= BUF_SIZE) rr->NextWriteTo -= BUF_SIZE;
	
	return 0;
}
// ������ � ������� �������� �� ���������
//    ARR_TYPE    TimeBeg;  
//    DWORD       XaccBeg;                    //(���������� ����� ����� �����)

int Acceleration(RR * rr)
{
	double D;	
        double Kx;
	DWORD Count;
        WORD j;        
        WORD FreeData = BUF_SIZE - rr->DataCount;
        ARR_TYPE T;
        double X;
        // V = B*T/(1-K*T)
        // T = V/(B+K*V)
	X = rr->XaccBeg*rr->dx;
        if(rr->Tend == 0.0)
            rr->Tend = (ARR_TYPE)(rr->Vend/((rr->B + rr->Vend * rr->K) * rr->TimerStep));

	for(WORD i = 0; i < FreeData; i++) {        
            j = rr->NextWriteTo + i;
            if(j >= BUF_SIZE) j -= BUF_SIZE;
	    Kx = X * rr->K;
	    D = Kx * Kx + 4.0 * X * rr->B;
	    if(D >= 0.0){
                T = (ARR_TYPE)((-Kx + sqrt(D))/(2.0 * rr->TimerStep * rr->B ));
                if(T >= rr->Tend){
                    FreeData = i;
                    rr->State = rr->NextState;
                    rr->Tend = 0.0;                    
                    break;
                }
                rr->IntervalArray[j] = T; 
	    }
	    X += rr->dx;            
	}
	rr->XaccBeg += FreeData; 
        rr->DataCount += FreeData;
        rr->NextWriteTo += FreeData;
        if(rr->NextWriteTo >= BUF_SIZE) rr->NextWriteTo -= BUF_SIZE;
        return 0;
}

// ���������� � ������� �������� �� ���������
int Deceleration(RR * rr)
{
//     double D;	
//     double Kx;
//     DWORD Count;
//     WORD j;        
//     WORD FreeData = BUF_SIZE - rr->DataCount;
//     ARR_TYPE Tdest;        
//     ARR_TYPE T;
//     // V = B*T/(1-K*T)
//     // T = V/(B+K*V)
//     rr->X = rr->Xpos*rr->dx;
//     Tdest = (ARR_TYPE)(rr->TargetSpeed/((rr->B + rr->TargetSpeed * rr->K) * rr->TimerStep));
// 
//     for(WORD i = 0; i < FreeData; i++) {        
//         j = rr->NextWriteTo + i;
//         if(j >= BUF_SIZE) j -= BUF_SIZE;
//         Kx = rr->X * rr->K;
//         D = Kx * Kx + 4.0 * rr->X * rr->B;
//         if(D >= 0.0){
//             T = (ARR_TYPE)((-Kx + sqrt(D))/(2.0 * rr->TimerStep * rr->B ));
//             if(rr->BeginT == 0){
//                 rr->BeginT = T;
//             }
//             if(T <= Tdest){
//                 FreeData = i;
//                 rr->State = ST_FREE;
//                 rr->dwX = 0;
//                 //rr->X = 0.0;
//                 rr->BeginT = 0;
//                 break;
//             }
//             rr->IntervalArray[j] = 2* rr->BeginT - T; 
//         }        
//         rr->X -= rr->dx;   
//         if(rr->X <= 0.0) {
//             rr->X = 0.0;
//             rr->State = ST_STOP;
//             rr->BeginT = 0;
//             break;
//         }
//     }
//     rr->Xpos -= FreeData; 
//     rr->DataCount += FreeData;
//     rr->NextWriteTo += FreeData;
//     if(rr->NextWriteTo >= BUF_SIZE) rr->NextWriteTo -= BUF_SIZE;
    return 0;
}
