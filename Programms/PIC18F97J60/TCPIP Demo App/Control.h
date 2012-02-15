#ifndef _CONTROL_H
#define _CONTROL_H

#ifdef _WINDOWS
#   include "..\..\guidance\stdafx.h"
#else
#   include "GenericTypeDefs.h"
#endif


typedef struct AXIS_PARAM
{
    BYTE  NeedToUpdate;     // ���������� ��������� � ������� ��������� ���������
    BYTE  NeedToCommit;     // ���������� ��������� �� ������ ��������� ���������

    double Angle;           // ������� ���������� � �������� (���� ������������ ��������� �������������) �� ���� ��������� ������    
    DWORD AbsSteps;         // ������� ����� ����    

} AXIS_PARAM;

typedef struct NETWORK_SETTINGS
{
    BYTE  NeedToUpdate;     // ���������� ��������� � ������� ��������� ���������
    BYTE  NeedToCommit;     // ���������� ��������� �� ������ ��������� ���������
    DWORD IP;
    DWORD Mask;
    DWORD Gate;
    DWORD DNS1;
    DWORD DNS2;    
    char  Name[16];    
}NETWORK_SETTINGS;

typedef struct All_PARAMS
{
    // ������� ����� -����� 
    BYTE NeedToUpdate;   // ���������� ��������� � ������� ��������� ���������
    BYTE NeedToCommit;   // ���������� ��������� �� ������ ��������� ���������

    AXIS_PARAM Alpha;
    AXIS_PARAM Delta;
    AXIS_PARAM Gamma;

    NETWORK_SETTINGS Local;
    NETWORK_SETTINGS Remote;

} ALL_PARAMS;

typedef enum MENU_ID {
    MAIN_WINDOW, MENU, SETTINGS, OBSERV, O_GOTO, S_OBSERV, S_NETWORK, S_MONTY,S_DISPLAY, SM_TYPESELECT, SM_ALPHA, SM_DELTA, SM_GAMMA
} MENU_ID;

void ProcessMenu( BYTE * KeyPressed);
#endif