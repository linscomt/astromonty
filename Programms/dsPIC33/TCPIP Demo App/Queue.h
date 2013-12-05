#ifndef __QUEUE_H_
#define __QUEUE_H_

#ifdef __C30__
#   include "GenericTypeDefs.h"
#else
#   include "stdafx.h"
#endif

//
//
//  -------*******--------
//  ^                    |
//  |____________________|

typedef struct QUEUE_ELEMENT {
    BYTE Key;
    BYTE * Value;
} QUEUE_ELEMENT;

typedef struct PRIORITY_QUEUE {
    QUEUE_ELEMENT * Queue;
    BYTE * Values;
    QUEUE_ELEMENT * Start;
    QUEUE_ELEMENT * End;
    //BYTE QueueFull;
    BYTE Count;
    BYTE Size;
    WORD ValueSize;
} PRIORITY_QUEUE;

int Queue_Insert(PRIORITY_QUEUE * Queue, BYTE Key, BYTE * Value);               // ��������� ���� (k,v) � ���������; �������� �� ������� Value ���������� �� ���������� ���������
int Queue_GetEndBuffer( PRIORITY_QUEUE * Queue, BYTE Key, BYTE ** Value );      // ��������� ���� (k,v) � ��������� � ���������� ��������� �� ����� ( �������� �������� ���������� �����������, �� ���� ���� ������� ��������� �����)
int Queue_Min(PRIORITY_QUEUE * Queue, BYTE * Key, BYTE ** Value);               // ���������� ���� (k,v) � ����������� ��������� ����� k. � Value ��������� ��������� �� ������ �� ���������� ���������
int Queue_Extract(PRIORITY_QUEUE * Queue, BYTE * Key, BYTE * Value);            // ���������� ���� (k,v) ������ � �� ���������. �������� �� ����������� ��������� ���������� � ������ Value
int Queue_ExtractMin( PRIORITY_QUEUE * Queue, BYTE * Key, BYTE * Value );       // ���������� ���� (k,v) � ����������� ��������� ����� k. ������ �� ��������� ��� ����, ����������� ������ ���� � ����������� ��������� k,
                                                                                // �������� �� ����������� ��������� ���������� � ������ Value  
int Queue_ExtractAllToMin(PRIORITY_QUEUE * Queue, BYTE * Key, BYTE ** Value);    // ���������� ���� (k,v) � ����������� ��������� ����� k. ������ �� ��������� ��� ����, ����������� ������ ���� � ����������� ��������� k
int Queue_Init(PRIORITY_QUEUE * Queue, QUEUE_ELEMENT * KeyBuf, BYTE KeyBufSize, BYTE * ValueBuf, WORD ValueSize );         // ���������� �������
int Queue_Delete(PRIORITY_QUEUE * Queue);                                       // ������� ���� �� ������ �������
int Queue_Revert( PRIORITY_QUEUE * Queue );                                     // ������� � ������ ������� ( �������� ���������� )
int Queue_Selftest ();
int Queue_First(PRIORITY_QUEUE * Queue, BYTE * Key, BYTE ** Value);             // ����������� ������ � ������� ������� ( ����� ������ )
int Queue_Last( PRIORITY_QUEUE * Queue, BYTE * Key, BYTE ** Value );            // ����������� ��������� � ������� ������� ( ����� ����� )
/*
// Example
QUEUE_ELEMENT q[5];
PRIORITY_QUEUE Q;
BYTE Values[5*6];
BYTE k1 = 128;
BYTE v1[] = {1,1,1,1,1,1};
BYTE K = 0;
BYTE V[] = {0,0,0,0,0,0};

// Init
Queue_Init(&Q, q, 5, Values, 6);
// add values
Queue_Insert( &Q, k1, v1);
Queue_Insert( &Q, k1, v1);
Queue_Insert( &Q, k1, v1);
// get min value
Queue_Min(&Q, &K, V );
// remove all values before high priority
Queue_ExtractAllToMin(&Q, &K, V);

*/

#endif //__QUEUE_H_
