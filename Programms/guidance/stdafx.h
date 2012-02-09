// stdafx.h: ���������� ���� ��� ����������� ��������� ���������� ������
// ��� ���������� ������ ��� ����������� �������, ������� ����� ������������, ��
// �� ����� ����������
//

//#pragma once
#ifdef _WINDOWS
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

#define UNUSED(x) x

#ifndef _DWORD_VAL_
#define _DWORD_VAL_

typedef struct _WORD
{
    WORD LW;
    WORD HW;
} _WORD;
typedef struct DWBYTE
{
    BYTE LB;
    BYTE HB;
    BYTE UB;
    BYTE MB;
} DWBYTE;
typedef union 
{
    DWORD Val;
    _WORD word;
    DWBYTE byte;

} DWORD_VAL;
#endif
#ifndef _WORD_VAL_
#define _WORD_VAL_
typedef struct WBYTE
{
    BYTE LB;
    BYTE HB;
} WBYTE;
typedef union 
{
    WORD Val;    
    WBYTE byte;

} WORD_VAL;
#endif
#endif

// TODO. ���������� ����� ������ �� �������������� ���������, ����������� ��� ���������
