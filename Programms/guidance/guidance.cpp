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
static double Accelerate[ACCELERATE_SIZE];

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
 

void Calc(HWND hWnd, HDC hdc);
double M(double F);
int SolvQuadratic(double A, double B, double C, double* X1, double* X2);
int Calculate_dT(double Xbeg, double Xend, double V, double A, double* T);
DWORD MaxAcceleration(DWORD Xb, DWORD Xe,double dx, double K, double B, double * T, DWORD Len, DWORD * Xpos);
//int Calculate_A(double V, double *A);
//int Calculate_A(DWORD F, double *A);
//int Calculate_A(double V, double *A);
//int InitAccelerate(FREQ_POWER* FreqPower, WORD Len, double I);


int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���������� ��� �����.
// 		{
// 			// I = (m*r^2/4) + (m*l^2)/12
// 			static double Mass = 500.0f;
// 			static double Radius = 0.30f;
// 			static double Length = 2.0f;
// 			static double Reduction = 360.0f;
// 			static double Grad_to_Rad = 180.0/PI;
// 			double I = ((Mass*Radius*Radius/4) + (Mass*Length*Length/12))/Reduction; 
// 			//double L = (2 * Reduction * Grad_to_Rad)/(Mass*Radius*Radius);
// 			InitAccelerate(FreqPower, sizeof(FreqPower)/sizeof(FreqPower[0]), I);
// 		}
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
    static double B = 0.79962406 / I;
    static int SizeX = 400;
    static int SizeY = 10;
    static double Pi = PI;
    static double TT[65536];
    static DWORD TTLen = 65536;
    static DWORD Count = 0;
    DWORD XPos = 0;
    //double Vf = 180 * 200/PI;
    //char RRR[256];
    GetClientRect(hWnd, &rect);
    MoveToEx(hdc, rect.left+9, rect.bottom - 9, NULL);
    LineTo(hdc, rect.right - 9, rect.bottom - 9);
    MoveToEx(hdc, rect.left+9, rect.bottom - 9, NULL);
    LineTo(hdc, rect.left+9, rect.top);        
    MoveToEx(hdc, rect.left+10, rect.bottom - 10 - (rect.bottom/2), NULL);
    LineTo(hdc, rect.right-10, rect.bottom - 10 - (rect.bottom/2) );        
    MoveToEx(hdc, rect.left+10, rect.bottom - 10 - (rect.bottom/2) - (int)(1.0*SizeY), NULL);
    LineTo(hdc, rect.right-10, rect.bottom - 10 - (rect.bottom/2) - (int)(1.0*SizeY) );  
    dt = 0.0;
    T = 0.0;
    X = 0.0;

    do{		
            
//             
//             // KX = K*X;     // 1
//             // KXu2 = Kx*Kx; // 1
//             // B2 = 2.0 * B; //const 0
//             // B4 = 4.0 * B; //const 0
//             // B4X = B4 * X; // 1
//             // D = Kxu2 + B4X; // 
//             // T = (-KX + sqrt(D))/B2; // 2
//             // �����
//             D = X*X*K*K + 4.0*X*B;
//             if(D>=0.0){
//                 T = (-K*X + sqrt(D))/(2.0*B);
//                 timer1 = T/0.0000002; // ��������� ����������
//             }
//             V = X / T;
//             A = V / T;
// 
// 			//A2 = A / T;
// 			W = (int)(T1*SizeX);
// 			W1 = (int)(X1*SizeY);


	    //if((W != K1)||(W1!=K3)) 
        Count = MaxAcceleration(0, 20*3200, dX, K, B, TT, TTLen, &XPos);
        for( i = 0; i<Count; i++)
        {
            T = TT[i];
            X += dX;
            V = X / T;
            A = V / T;
            K1 = W;
            K3 = W1;
            SetPixel(hdc, rect.left + 10 + (int)(T*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(X*Rad_to_Grad*SizeY), RGB(0,0,255));
            SetPixel(hdc, rect.left + 10 + (int)(T*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(V*Rad_to_Grad*SizeY), RGB(0,255,0));
            SetPixel(hdc, rect.left + 10 + (int)(T*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(A*Rad_to_Grad*SizeY), RGB(255,0,0));                				
            SetPixel(hdc, rect.left + 10 + (int)(V*Rad_to_Grad*SizeY), rect.bottom - 10 - (rect.bottom/2) - (int)(A*Rad_to_Grad*SizeY), RGB(255,255,0));                				
            SetPixel(hdc, rect.left + 10 + (int)(T*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(A2*Rad_to_Grad*SizeY), RGB(200,0,200));                				

            SetPixel(hdc, rect.left + 10 + (int)(T1*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(X1*Rad_to_Grad*SizeY), RGB(0,0,200));
            SetPixel(hdc, rect.left + 10 + (int)(T1*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(V1*Rad_to_Grad*SizeY), RGB(0,200,0));
            SetPixel(hdc, rect.left + 10 + (int)(T1*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(A1*Rad_to_Grad*SizeY), RGB(200,0,0));                				

            SetPixel(hdc, rect.left + 10 + (int)(V1*Rad_to_Grad*SizeY), rect.bottom - 10 - (rect.bottom/2) - (int)(A1*Rad_to_Grad*SizeY), RGB(200,200,0));                				
            //SetPixel(hdc, rect.left + 10 + (int)(T*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(0.004166667*SizeY), RGB(0,128,0));
            SetPixel(hdc, rect.left + 10 + (int)(T*SizeX), rect.bottom - 10 - (rect.bottom/2) - (int)(10.0*SizeY), RGB(0,128,0));
            MoveToEx(hdc, rect.left + 9 + (int)(T)*SizeX, rect.bottom - 9, NULL);
            LineTo(  hdc, rect.left + 9 + (int)(T)*SizeX, rect.top); 
        }       
    }while ( X < 16.0 * Grad_to_Rad);       

}
// ������ ���������� �������� ��������� � ����������� �� ��������
// F - ������� ������ �����
// L - ������ ������� �������
// int Calculate_A(double V, double *A)
// {
// 	static double Mass = 500.0f;
// 	static double Radius = 0.30f;
// 	static double Length = 2.0f;
// 	static double Reduction = 360.0f;
// 	static double Grad_to_Rad = 180.0/PI;
// 	double I = ((Mass*Radius*Radius/4) + (Mass*Length*Length/12))/Reduction; 
//     static double K = (-0.000349812 * 180 * 200/PI)/I;
//     static double B = 0.79962406 / I;
// 
// 	// y=kx+b 
//     // -0.000349812	0.79962406
// 
// 	*A = K * V  + B;
// 	//*A = (-1.1 * V  + 0.287457143)/I;
// 	return 0;
// }
// int Calculate_A(DWORD F, double *A)
// {
//  	DWORD f = (DWORD)(F/FREQ_STEP);
//  	if(f < ACCELERATE_SIZE){
//  		*A = Accelerate[f];
//  	} else {
//  		*A = Accelerate[ACCELERATE_SIZE-1];
//  	}
// 	return 0;
// }

// ������� ���������� �����, ���������
// ��� �������� � ��������� ����������� �� ���������� (Xbeg,Xend), 
// ��� ��������� �������� V � ��������� A
int Calculate_dT(double Xbeg, double Xend, double V, double A, double* T)
{
	double T1 = 0.0;
	double T2 = 0.0;
	int res = 0;
	if((A==0.0)&&(V==0.0)){
		return -1; // ������
	}
	if((A == 0)&&(V != 0.0)){
		*T = (Xend-Xbeg)/V;
		return 0;  // ��� ������
	}
	// Xend = Xbeg + V*T + (A * T^2)/2
	// (A/2)T^2 + V*T + (Xbeg - Xend) = 0
	res = SolvQuadratic(A/2,V,(Xbeg-Xend),&T1, &T2); 
	if(res > 0) {
		*T = T1>=0?T1:T2;
		return 0;
	}
	else return 1;
}
// ��������� ������ ����������� ���������
int SolvQuadratic(double A, double B, double C, double* X1, double* X2)
{
	// Ax^2+Bx+C=0
	double D = 0.0;
	if((X1 == NULL) || (X2 == NULL)) return -1; // ������: �������� ���������
	*X1 = 0.0;
	*X2 = 0.0;
	if(A == 0.0){
		// Bx+C = 0
		// x = -C/B
		if(C!=0.0){
			*X1 = -B/C;
		} else {
			*X1 = 0.0;
		}
		return 1; // �� ���������� ���������, ���� ������
	}
	D = B*B - 4.0 * A * C;
	if(D<0.0) 
		return 0; // ������������ 0 ������ ���

	*X1 = (-B+sqrt(D))/(2*A);
	*X2 = (-B-sqrt(D))/(2*A);
	return 2; // ��� ����� ���������
}
/**************************************************************************
 * M = Ia =>   a = M/I
 * M - ������ ����
 * I - ������ �������
 * a - ������� ��������� � �������� � ������� �� �������
 * ��� �����: ( �������� ������� ������ l, ������� r � ����� m, ��� ��������������� �������� � �������� ����� ��� ��������)
 * I = (m*r^2/4) + (m*l^2)/12  
 * 
 * ��� ������: ( ����� ������������ ������� ������ l, ������� r � ����� m, ��� ��������������� �������� � �������� ����� ��� ��������)
 * I = (m*r^2/2)+ (m*l^2/12)
 **************************************************************************
 * ��������� �������
 * Power - ������ �������� ���� ���������
 * Freq  - ������ ������ ����� ���������
 * Len   - ������ ��������
 * I	 - ������ �������  
 * 
 ************************************************************************/
// int InitAccelerate(FREQ_POWER* FreqPower, WORD Len, double I)
// {    
// 	double Lm = 0.0;	
// 	int j;
//     int k = 0;
//     WORD Freq1;
//     WORD Freq2;
//     BYTE b = 0;
// 	WORD F = 0;
// 	for(j = 0; j < ACCELERATE_SIZE; j++){
// 		for(int i = k; i< Len-1; i++){
//             Freq1 = FreqPower[i].Freq;  
//             Freq2 = FreqPower[i+1].Freq;            
// 			if((F >= Freq1) && (F <= Freq2)){
// 				Lm = LinInt(Freq1,FreqPower[i].Power,Freq2,FreqPower[i+1].Power, F);
//                 b = 1;
//                 k = i;
// 				break;
// 			}
// 		}
// 		if(b == 0) 
//             Lm = FreqPower[Len - 1].Power;
// 		F += FREQ_STEP;
// 		Accelerate[j] = Lm / I;
// 	}
// 	return 0;
// }

// ��������� ������ T ������� Len ���������� ������� ������� �� �������� Vb � ���������� ��������� Ve
// K B dx - ��������� 
// ���������� ���������� ������������ �����
// 
//
DWORD MaxAcceleration(DWORD Xb, DWORD Xe,double dx, double K, double B, double * T, DWORD Len, DWORD * Xpos)
{
    double D;
    double X;
    DWORD StepCount;
    DWORD Count;
    double Kx;

    // ���������� ���������� 
    if(*Xpos<Xb) *Xpos = Xb;
    if(*Xpos>=Xe) {*Xpos = Xe; return 0;}
    Count = (DWORD)(Xe - *Xpos);
    if(Count > Len) StepCount = Len; 
    else StepCount = Count;
    X = *Xpos*dx;
    for(WORD i = 0; i < StepCount; i++) {        
        Kx = X * K;
        D = Kx * Kx + 4.0 * X * B;
        if(D >= 0.0){
            T[i] = (-Kx + sqrt(D))/(2.0 * B);        
        }
        X += dx;
    }
    *Xpos += StepCount; 
    return StepCount;
}
double Deceleration(double X, double K, double B)
{
    double D;

    D = X*X*K*K + 4.0*X*B;
    if(D>=0.0){
        return (-K*X + sqrt(D))/(2.0*B);        
    }
    return -1.0;

}