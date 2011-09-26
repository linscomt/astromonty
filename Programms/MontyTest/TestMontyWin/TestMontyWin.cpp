// TestMontyWin.cpp: ���������� ����� ����� ��� ����������.
//

#include "stdafx.h"
#include "TestMontyWin.h"
#include <my_global.h>
//#include <mysql.h>
#include <mysql.h>



#define MAX_LOADSTRING 100

// ���������� ����������:
HINSTANCE hInst;								// ������� ���������
TCHAR szTitle[MAX_LOADSTRING];					// ����� ������ ���������
TCHAR szWindowClass[MAX_LOADSTRING];			// ��� ������ �������� ����
HWND MyHWND;

// ���������� ����������

MYSQL *conn;

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	        WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	        About(HWND, UINT, WPARAM, LPARAM);
void                            MyDraw(HWND hWnd);
BOOL NeedClose;
RECT CirclePos;

struct REQ_DATA_TYPE {
	UINT HD;
	float Alpha;
	float Alpha_m;
	float Delta;
	float Delta_m;
	float Size;
};
struct DATA_TYPE {
	UINT HD;
	double Alpha;
	double Delta;
	double Size;
};

static REQ_DATA_TYPE* pData;/*[] = {
	{23.0, 	44.7, 	+42.0, 	03.0,  8.834},
	{00.0, 	18.4, 	+44.0, 	01.0,  8.105},
	{19.0, 	31.2, 	+43.0, 	38.0,  9.914},
	{04.0, 	36.5, 	+44.0, 	59.0,  8.552},
	{11.0, 	05.5, 	+43.0, 	32.0,  8.765},
	{11.0, 	47.2, 	+43.0, 	28.0,  8.705},
	{12.0, 	32.0, 	+43.0, 	29.0,  9.536},
	{02.0, 	33.5, 	+45.0, 	39.0,  8.838},
	{02.0, 	38.2, 	+46.0, 	33.0,  9.102},
	{17.0, 	48.8, 	+45.0, 	42.0,  8.264},
	{19.0, 	28.8, 	+46.0, 	03.0,  8.569},
	{17.0, 	39.2, 	+46.0, 	09.0,  8.207},
	{02.0, 	49.8, 	+48.0, 	08.0,  9.487},
	{02.0, 	51.1, 	+48.0, 	06.0,  9.321},
	{07.0, 	24.5, 	+47.0, 	11.0, 10.179},
	{20.0, 	41.3, 	+48.0, 	09.0, 10.451},
	{17.0, 	35.6, 	+48.0, 	50.0,  8.682},
	{23.0, 	51.2, 	+48.0, 	59.0,  9.222},
	{10.0, 	20.7, 	+49.0, 	06.0, 10.046},
	{13.0, 	43.9, 	+49.0, 	08.0,  9.415},
	{09.0, 	05.2, 	+60.0, 	17.0,  8.869},
	{08.0, 	07.6, 	+60.0, 	41.0, 10.106},
	{02.0, 	46.7, 	+63.0, 	00.0,  9.534},
	{01.0, 	04.7, 	+74.0, 	50.0,  8.909},
	{22.0, 	02.9, 	+00.0, 	04.0,  9.079},
	{09.0, 	35.5, 	-01.0, 	47.0,  9.283},
	{14.0, 	42.0, 	-03.0, 	31.0,  8.974},
	{21.0, 	40.3, 	-01.0, 	47.0,  9.947},
	{23.0, 	23.8, 	-01.0, 	19.0,  8.991},
	{00.0, 	21.1, 	-02.0, 	21.0,  9.537},
};*/
DATA_TYPE* phData;
unsigned long DataLen = 0;
RECT DataMaxSize;
int StarMaxSize = -5;
RECT ViewRect = {
    50,50,800,600
};

// ViewRect.top = 5;
// ViewRect.left = 5;
// ViewRect.bottom = 605;
// ViewRect.right = 805;
   
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
	LoadString(hInstance, IDC_TESTMONTYWIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ��������� ������������� ����������:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTMONTYWIN));
	
	// ���� ��������� ���������:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	// ��������� ���������� � �������� ���� ������
	mysql_close(conn);
	delete [] phData;
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
	wcex.hIcon		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTMONTYWIN));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TESTMONTYWIN);
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
   MyHWND = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   // �������� ���������� ����������
   conn = mysql_init(NULL);
   // ���������� �������������� �������
   MYSQL_RES *res;
   // ���������� ������
   MYSQL_ROW row;

   if(conn == NULL)
   {
	   // ���� ���������� �� ������� - ������� ��������� �� ������
	   //fprintf(stderr, "Error: can't create MySQL-descriptor\n");
	   //MessageBox(hWnd,_T("Error: can't create MySQL-descriptor\n"),_T("Error"),0);
	   return FALSE;
   }
   // ������������ � �������
   if(!mysql_real_connect(conn,
	   "localhost",
	   "root",
	   "1qweDCVB",
	   "mybase",
	   NULL,
	   NULL,
	   0
	   ))
   {
	   // ���� ��� ����������� ���������� ���������� � �������� 
	   // ���� ������ ������� ��������� �� ������
	   //MessageBox(hWnd,_T("Error: can't connect to database %s\n"),_T("Error"),0);
	    return FALSE;
    }
    else  {
        // ���� ���������� ������� ����������� ������� ����� - "Success!"
        //fprintf(stdout, "Success!\n"); // ��������� SQL-������
        //if(mysql_query(conn, "SELECT HD, Alpha_grad, Alpha_Min, Delta_grad, Delta_Min, W FROM Stars ") != 0)
        if(mysql_query(conn, "SELECT HD, Alpha_grad, Alpha_Min, Delta_grad, Delta_Min, W FROM Stars WHERE W<8") != 0)
        //if(mysql_query(conn, "SELECT HD, Alpha_grad, Alpha_Min, Delta_grad, Delta_Min, W FROM Stars WHERE W<4 and Alpha_grad>2 AND Alpha_grad<8") != 0)
        //if(mysql_query(conn, "SELECT HD, Alpha_grad, Alpha_Min, Delta_grad, Delta_Min, W FROM Stars where HD=39801") != 0)
           //MessageBox(hWnd,_T("Error: can't execute SQL-query\n"),_T("Error"),0);
           return FALSE;

        // �������� ���������� �������������� �������
        res = mysql_store_result(conn);
        if(res == NULL) return FALSE;
            //MessageBox(hWnd,_T("Error: can't get the result description\n"),_T("Error"),0);

        // ���� ������� ���� �� ���� ������ - �������	  
        DataLen = mysql_num_rows(res);
        if(DataLen > 0) {
            pData = new REQ_DATA_TYPE[DataLen];
            // � ����� ���������� ��� ������
            // �������������� �������
            UINT i = 0;
            while((row = mysql_fetch_row(res)) != NULL) {
                // ������� ��������� � ����������� �����
                sscanf_s(row[0],"%lu", &pData[i].HD);
                sscanf_s(row[1],"%f", &pData[i].Alpha);
                sscanf_s(row[2],"%f", &pData[i].Alpha_m);
                sscanf_s(row[3],"%f", &pData[i].Delta);
                sscanf_s(row[4],"%f", &pData[i].Delta_m);
                sscanf_s(row[5],"%f", &pData[i].Size);
                i++;
            }
	    // ����������� ������, ������� �������������� ��������
	        mysql_free_result(res);
        }
   }   
   if(DataLen==0) return FALSE;
   DataMaxSize.top = 90;
   DataMaxSize.bottom = -90;
   DataMaxSize.left = 24*15;
   DataMaxSize.right = 0;

   phData = new DATA_TYPE[DataLen];
   memset(phData, 0, DataLen*sizeof(DATA_TYPE));

   for(UINT i = 0; i< DataLen; i++){
	   if(pData[i].Alpha>=0) phData[i].Alpha = (pData[i].Alpha + pData[i].Alpha_m/60)*15;
	   else phData[i].Alpha = (pData[i].Alpha - pData[i].Alpha_m/60)*15;
	   if(pData[i].Delta>=0) phData[i].Delta = (pData[i].Delta + pData[i].Delta_m/60);
	   else phData[i].Delta = (pData[i].Delta - pData[i].Delta_m/60);
	   phData[i].Size =  pData[i].Size;
	   phData[i].HD = pData[i].HD;

	   if(phData[i].Alpha< DataMaxSize.left)   DataMaxSize.left   = phData[i].Alpha;
	   if(phData[i].Alpha> DataMaxSize.right)  DataMaxSize.right  = phData[i].Alpha;
	   if(phData[i].Delta< DataMaxSize.top)    DataMaxSize.top    = phData[i].Delta;
	   if(phData[i].Delta> DataMaxSize.bottom) DataMaxSize.bottom = phData[i].Delta;
           if(phData[i].Size > StarMaxSize) StarMaxSize = phData[i].Size;
   }
   delete [] pData;
   CirclePos.top = ViewRect.top - 10;
   CirclePos.left = ViewRect.left - 10;
   CirclePos.right = ViewRect.right + 10;
   CirclePos.bottom=ViewRect.bottom + 10;
   //SetTimer(hWnd, 1,  1000 , NULL);
   InvalidateRect(hWnd, &CirclePos, TRUE);
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
	RECT DrawPos;

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
        MyDraw(hWnd);		
		break;
	case WM_DESTROY:		
		PostQuitMessage(0);
		break;
        case WM_TIMER:
			//DrawPos.top = CirclePos.top-2;
			//DrawPos.left = CirclePos.left-2;
			//DrawPos.right = CirclePos.right+2;
			//DrawPos.bottom=CirclePos.bottom+2;
			//CirclePos.left+=2;
			//CirclePos.right+=2;
				
            
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

void MyDraw(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc;   	
    RECT lpRect;
    UINT size;
    double aspectX = (ViewRect.right-ViewRect.left)/(DataMaxSize.right - DataMaxSize.left);
    double aspectY = (ViewRect.bottom-ViewRect.top)/(DataMaxSize.bottom - DataMaxSize.top);
    

    hdc = BeginPaint(hWnd, &ps);  	

    Rectangle(hdc, ViewRect.left-5,ViewRect.top-5,ViewRect.right+5,ViewRect.bottom+5);

    HPEN hPen = CreatePen (PS_SOLID, 1, RGB(255, 255, 255));
    HPEN hOldPen = (HPEN)SelectObject (hdc, hPen);    
    HBRUSH hBrush = CreateSolidBrush(0x000000); 
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    FillRect( hdc, &ViewRect, hBrush);
       
    SelectObject(hdc, hOldBrush);
    
    UINT X;
    UINT Y;
    for(UINT i = 0; i< DataLen; i++){
        size = StarMaxSize - phData[i].Size + 1;
        Y = ViewRect.top  + (DataMaxSize.bottom - phData[i].Delta)*aspectY;
        X = 20+ViewRect.left + (DataMaxSize.right  - phData[i].Alpha)*aspectX;
        
        /*lpRect.top    = Y - size/2;
        lpRect.bottom = Y + size/2;
        lpRect.left   = X - size/2;
        lpRect.right  = X + size/2;*/
        switch(size){
            case 3:
                SetPixel(hdc, X+1, Y-1, 0xFFFFFF);
                SetPixel(hdc, X-1, Y+1, 0xFFFFFF);
                SetPixel(hdc, X-1, Y-1, 0xFFFFFF);
                SetPixel(hdc, X+1, Y+1, 0xFFFFFF);
            case 2: 
                SetPixel(hdc, X, Y-1, 0xFFFFFF);
                SetPixel(hdc, X, Y+1, 0xFFFFFF);
                SetPixel(hdc, X-1, Y, 0xFFFFFF);
                SetPixel(hdc, X+1, Y, 0xFFFFFF);
            case 0:
            case 1: SetPixel(hdc, X, Y, 0xFFFFFF);
                break;
            default:
                Ellipse(hdc, X - size/2, Y - size/2, X + size/2, Y + size/2);
            
        }
            //if((phData[i].HD == 39801)||    //�����������
            //    //(phData[i].HD == 8890)||  // ����� ����� ���������
            //    (phData[i].HD == 37742)||   //�������
            //    (phData[i].HD == 35468)||   //���������
            //    (phData[i].HD == 37128)||   //�������
            //    (phData[i].HD == 36486)||   //�������	62509	
            //    (phData[i].HD == 29139)     //����������
            //    //(phData[i].HD == 148478)  //�������
            //    //(phData[i].HD == 48915)   // ������
            //    ){ 
            //    Rectangle(hdc, lpRect.left, lpRect.top, lpRect.right, lpRect.bottom); 
            //}            
            //    Ellipse(hdc, lpRect.left, lpRect.top, lpRect.right, lpRect.bottom);	
            //}
    
    }   
    SelectObject(hdc, hOldPen);
   

    // ������� Pen
    DeleteObject (hPen);
    EndPaint(hWnd, &ps);	
   
}