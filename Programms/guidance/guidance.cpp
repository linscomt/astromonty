// guidance.cpp: ���������� ����� ����� ��� ����������.
//

#include "stdafx.h"
#include "guidance.h"
#include <math.h>

#define MAX_LOADSTRING 100
#define PI 3.1415926535897932384626433832795
// ���������� ����������:
HINSTANCE hInst;								// ������� ���������
TCHAR szTitle[MAX_LOADSTRING];					// ����� ������ ���������
TCHAR szWindowClass[MAX_LOADSTRING];			// ��� ������ �������� ����

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
 

void Calc(HDC hdc);
double M(double F);
int SolvQuadratic(double A, double B, double C, double* X1, double* X2);
int Calculate_dT(double Xbeg, double Xend, double V, double A, double* T);
int Calculate_A(double V, double L, double *A);

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
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
                //LineTo(hdc, 100,100);
		// TODO: �������� ����� ��� ���������...
                Calc(hdc);
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
void Calc(HDC hdc)
{
    {
        static double Mass = 100.0f;
        static double Radius = 2.0f;
        double A = 0.0; //���������
        double dt = 0.0; // ��������� �������
        double V = 0.0;  // ���������� ��������
        static double dX =1.0/(200.0*16.0);// ��� ����������� � �������� (� 1 ������� 3200 �����)
        double X = 0;    // ������ ����������� � ��������
        double T = 0;    // ������ �����
        int K = 0;       // ����� ��� ����������� ������
        int K1 = 0;
        int K2 = 0;
        int K3 = 0;
        double timer1 = 0;  // �������� �������
        MoveToEx(hdc, 100,0, NULL);
        LineTo(hdc, 100,10*20);
        MoveToEx(hdc, 0, 10*20, NULL);
        LineTo(hdc, 1000,10*20);
        do{
            Calculate_A(V, 2/(Mass*Radius*Radius),&A);
            Calculate_dT(0, dX, V, A, &dt);
            V = dX/dt;
            X += dX;
            T += dt;
            timer1 = T/0.0000002; // ��������� ����������
            K = (int)(T*500.0);
            K2 = (int)(V*20.0);
            if((K != K1)||(K2!=K3)) {
                K1 = K;
                K3 = K2;
                 SetPixel(hdc, (int)(T*500), (int)(X*20), RGB(0,0,255));
                 SetPixel(hdc, (int)(T*500), (int)(V*20), RGB(0,255,0));
                 SetPixel(hdc, (int)(T*500), (int)((A)*10), RGB(255,0,0));
                 SetPixel(hdc, (int)(T*500), (int)(timer1/100), RGB(255,0,255));
            }
            if(X>180.0) break;
        }while ( V < 10);
        //T = 0;
        do{
            Calculate_A(V, 2/(Mass*Radius*Radius),&A);
            A = -A;
            if(Calculate_dT(0, dX, V, A, &dt)!=0){
                V=0;
            } else
            V = dX/dt;
            X += dX;
            T += dt;
            timer1 = T/0.0000002; // ��������� ����������
            K = (int)(T*500.0);
            K2 = (int)(V*20.0);
            if((K != K1)||(K2!=K3)) {
                K1 = K;
                K3 = K2;
                SetPixel(hdc, (int)(T*500), (int)(X*20), RGB(0,0,255));
                SetPixel(hdc, (int)(T*500), (int)(V*20), RGB(0,255,0));
                SetPixel(hdc, (int)(T*500), (int)((-A)*10), RGB(255,0,0));
                SetPixel(hdc, (int)(T*500), (int)(timer1/100), RGB(255,0,255));
            }
            if(X<0.0) break;
        }while ( V > 0);
    }
}
// ������ ���������� �������� ��������� � ����������� �� ��������
// V - �������� � �������� � ���
// L - ������ ������� �������
int Calculate_A(double V, double L, double *A)
{
    double F; // ������� ������ �����
    double R_G = 180/PI;
    //double dX = 1.0/200; // ���� � �������� ������� ����
    double Lm = 0.0;
    // ������ �� ����
    static double MPower[] = {
        0.85, 0.764642857, 0.67, 0.6, 0.53, 0.46, 0.4, 0.33, 0.22, 0.1, 0.01
    };
    // ������� � ��
    static double MaxF[] = {
        0.0, 100.0, 250.0, 500.0, 750.0, 1000.0, 1250.0, 1500.0, 1750.0, 2000.0, 2250.0
    };
    int sizeMaxF = sizeof(MaxF)/sizeof(double);
    F = V * 200;
    if(F > MaxF[sizeMaxF-1]){
        Lm = MPower[sizeMaxF-1];
    } else {
        for(int i = 0; i< sizeMaxF-1; i++){
            if((F >= MaxF[i]) && (F < MaxF[i+1])){
                Lm = LinInt(MaxF[i],MPower[i],MaxF[i+1],MPower[i+1], F);
                break;
            }
        }
    }
    *A = Lm * 360 * R_G * L;
    return 0;
}

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
        *T = T1;
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
    if(D<0.0) return 0; // ������������ 0 ������ ���

    *X1 = (-B+sqrt(D))/(2*A);
    *X2 = (-B-sqrt(D))/(2*A);
    return 2; // ��� ����� ���������
}

