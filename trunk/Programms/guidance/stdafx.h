// stdafx.h: ���������� ���� ��� ����������� ��������� ���������� ������
// ��� ���������� ������ ��� ����������� �������, ������� ����� ������������, ��
// �� ����� ����������
//

#pragma once
#ifndef __C30__
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ��������� ����� ������������ ���������� �� ���������� Windows
// ����� ���������� Windows:
#include <windows.h>

// ����� ���������� C RunTime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define USE_PROTOCOL_SERVER
#define USE_PROTOCOL_CLIENT


typedef struct _WORD
{
    WORD LW;
    WORD HW;
} _WORD;

typedef union 
{
    DWORD Val;
    _WORD word;

} DWORD_VAL;

#endif

// TODO. ���������� ����� ������ �� �������������� ���������, ����������� ��� ���������
