#pragma once

#include "resource.h"

#define MAX_FILE_NAME_LENGTH 32

/*
 *        �������, ���� ���
 *        | ������� ����, ���� ����/�� ������� ����
 *        | | ���������(0)/��������(1)
 *        | | | ������ � �����/������ �� ���������
 *        | | | | ������ ���������
 *        | | | | | ������ ���������
 *        | | | | | |
 *   |7|6|5|4|3|2|1|0|
 */
typedef union {
    BYTE Val;
    //BYTE_VAL VAL;
    struct {
        unsigned RE:1;
        unsigned WE:1;
        unsigned WrToEnd:1;
        unsigned Binary:1;
        unsigned Renew:1;
        unsigned New:1;
        unsigned b6:1;
        unsigned b7:1;
    } bits;
}FILESTATE;

typedef struct _INODE_RECORD
{
    WORD wInodeID;                  // ������ �����        
    WORD wFlags;                    // �����
    DWORD dwSize;                   // ������ �����
    WORD wSectorsCount;             // ���������� ��������
    WORD pwTable[11];               // ������� ���������� �������� 
                                    // [0..7] ������ ���������
                                    // [8] ���������
                                    // [9] ������� ���������
                                    // [10] ������� ���������
} INODE_RECORD;

typedef struct _DIR_RECORD
{
    WORD wInodeID;                          // ������ �����
    BYTE bNameLength;                       // ������ ����� �����
    char pcFileName[MAX_FILE_NAME_LENGTH];  // ��� �����
} DIR_RECORD;

// ������ �������� �����
typedef struct _uFS_FILE
{
    INODE_RECORD Inode;
    DIR_RECORD Dir;
    DWORD DataPointer;
    FILESTATE Flags;
    BYTE ErrorState;
} uFS_FILE;


int uFS_Init();
int CreateFS();
uFS_FILE *  FS_fopen ( const char * filename, const char * mode );
int uFS_fwrite ( BYTE * ptr, DWORD size, DWORD count, uFS_FILE * stream );
int uFS_fread ( BYTE * ptr, DWORD size, DWORD count, uFS_FILE * stream );
int uFS_fseek ( uFS_FILE * stream, int offset, int origin );


