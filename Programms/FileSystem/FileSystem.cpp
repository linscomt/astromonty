// FileSystem.cpp: ���������� ����� ����� ��� ����������� ����������.
//
#include "stdafx.h"
#include "FileSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ������������ ������ ����������

CWinApp theApp;

using namespace std;



typedef struct _SUPERBLOCK 
{
    char Name[16];          // = "uFS";
    WORD VersionMajor;      //0
    WORD VersionMinor;      //1
    DWORD SectorSize;       // 256*1024
    DWORD SectorsCount;     //64
    DWORD PageSize;         // 256
    DWORD PageCount;        //65536
    DWORD InodeTable;       // 256
} SUPERBLOCK;

typedef enum _ADDRESSING_TYPE
{
    uFS_AT_DIRECT, uFS_AT_INDIRECT_1, uFS_AT_INDIRECT_2, uFS_AT_INDIRECT_3, uFS_AT_INVALID = 0xFF
} ADDRESSING_TYPE;

typedef struct _IdTable{
    ADDRESSING_TYPE AD_Type;
    WORD IdTable[3];            // ����� �������
    WORD IdAdress[3];           // ����� ������ �������
} IdTable;



BYTE FS[256*SECTORS_COUNT];
WORD uFS_LastInodeID = 0;

//FS_FREE 
typedef struct _FREE_SECTORS{
    BYTE AvailableSectors;
    BYTE ReleasedSectors;
} FREE_SECTORS;

FREE_SECTORS    FreeSectors[SECTORS_COUNT/8];

typedef struct _uFS_FREE_SECTOR_CONTROL {
    FREE_SECTORS    FreeSectorsCache[16];
    uFS_FILE*       FreeFile;
} uFS_FREE_SECTOR_CONTROL;

uFS_FREE_SECTOR_CONTROL uFS_FreeSectors;

uFS_FILE OpenFileArray[uFS_MAX_OPENED_FILE + 3];

uFS_FILE *  uFS_FileOpen(WORD wInode, FILESTATE Flags);
int uFS_Read(DWORD Addr, BYTE * Dest, DWORD Count);
int uFS_Write(DWORD Addr, BYTE * Source, DWORD Count);
WORD GetIndirectSectorNumber(WORD SectorNumber, WORD WordPosition);
int SetIndirectSectorNumber(WORD SectorNumber, WORD WordPosition, WORD Number);
int uFS_GetSectorAddr( INODE_RECORD Inode, WORD Addr, WORD * SectorAddr, IdTable * Indirect);
int uFS_GetAddr(INODE_RECORD Inode, DWORD Addr, DWORD * DataAddr );
int uFS_AddSector(INODE_RECORD * Inode, WORD * SectorAddr);
int uFS_AddSectorFromRange(INODE_RECORD * Inode, WORD * SectorAddr, WORD BeginSector, WORD EndSector);
int uFS_ReadData(DWORD Addr, BYTE * Dest, int Count);
int uFS_WriteData(DWORD Addr, BYTE * Source, int Count);
int uFS_ReadFile(INODE_RECORD Inode, DWORD Address, BYTE * Dest, int Count);
int uFS_WriteFile(INODE_RECORD * Inode, DWORD Address, BYTE * Source, int Count);
int uFS_ClaimFreeSectorFromRange(WORD * ClaimedSector, WORD BeginSector, WORD EndSector);
int uFS_ClaimFreeSector(WORD * ClaimedSector);
int uFS_ReleaseSector(WORD ReleasedSector);
int uFS_ReleaseFileSector(INODE_RECORD Inode, WORD SectorIndex);
int uFS_ReleaseFileSectors(INODE_RECORD Inode);
int uFS_ExpandFile(INODE_RECORD * Inode, DWORD NewSize);
int uFS_RemoveFile(INODE_RECORD Inode);
int uFS_DeleteInode(WORD wInode, DWORD SearchFromOffset);
int uFS_GetFileInode(WORD wInode, INODE_RECORD * Inode, DWORD * SearchFromOffset);

BYTE bBitCount(BYTE x);
BYTE wBitCount(WORD x);
WORD uFS_GetFreeSectorCount();
WORD uFS_GetFreeSectorCountInRange(WORD BeginSector, WORD EndSector);
WORD uFS_GetReleasedSectorCount();
WORD uFS_GetReleasedSectorCountInRange(WORD BeginSector, WORD EndSector);
// �����

WORD uFS_SearchInodeInDir(WORD DirInode, const char * FileName, BYTE FileNameLen);
WORD uFS_GetInodeByPath(const char * Path, WORD * ParentID, char ** ParentPath );

WORD uFS_FindLastInodeID();

// ***************************************************************************
int uFS_Read(DWORD Addr, BYTE * Dest, DWORD Count)
{
    memcpy(Dest, &FS[Addr],Count);
    return 0;
}

// ***************************************************************************
int uFS_Write(DWORD Addr, BYTE * Source, DWORD Count)
{
    memcpy(&FS[Addr], Source, Count);
    return 0;
}

// ***************************************************************************
WORD GetIndirectSectorNumber(WORD SectorNumber, WORD WordPosition)
{   
    DWORD AddrInSector = (SectorNumber << DATA_SECTOR_SIZE_BIT) | ((WordPosition << 1) & SECTOR_MASK);
    WORD DataAddr;
    uFS_Read(AddrInSector, (BYTE*)&DataAddr, sizeof(WORD));
    return DataAddr;
}

// ***************************************************************************
int SetIndirectSectorNumber(WORD SectorNumber, WORD WordPosition, WORD Number)
{   
    DWORD AddrInSector = (SectorNumber << DATA_SECTOR_SIZE_BIT) | ((WordPosition << 1) & SECTOR_MASK);
    return uFS_Write(AddrInSector, (BYTE*)&Number, sizeof(WORD));
}

// ***************************************************************************
// INPUT
// Inode        ���� �����
// Addr         ����� ������� � �����
// 
// OUTPUT
// SectorAddr   ���������� ����� �������
// Indirect     ������ ���������� ������� ������, �������������� ��� ���������
// ���������� � ���������� SectorAddr ���������� ����� ������� ������ ��� ����������� ������ ������� �����, � � Indirect ������ �������� ������, ������������ ��� ��������� ����� �������
int uFS_GetSectorAddr( INODE_RECORD Inode, WORD Addr, WORD * SectorAddr, IdTable * Indirect)
{
    if(Addr > Inode.dwSize) 
        return -1; // EOF

    if(Indirect != NULL){
        Indirect->IdTable[0] = INVALID_SECTOR_NUMBER;
        Indirect->IdTable[1] = INVALID_SECTOR_NUMBER;
        Indirect->IdTable[2] = INVALID_SECTOR_NUMBER;
        Indirect->IdAdress[0] = 0;
        Indirect->IdAdress[1] = 0;
        Indirect->IdAdress[2] = 0;
        Indirect->AD_Type = uFS_AT_INVALID;
    }
    if(Addr < 8){
        // ������ ���������
        if(Inode.pwTable[Addr] == INVALID_SECTOR_NUMBER)
            return -1;
        *SectorAddr = (Inode.pwTable[Addr]);
        if(*SectorAddr == INVALID_SECTOR_NUMBER)
            return -1;
        if(Indirect != NULL){
            Indirect->AD_Type = uFS_AT_DIRECT;
        }
        return 0;
    } else Addr -= 8;

    if(Addr < INDERECT_ADDR_x1_SIZE){

        // ��������� ���������
        // Inode.pwTable[8]-->table1[128]-->data[256]

        if(Inode.pwTable[8] == INVALID_SECTOR_NUMBER)
            return -1;
        *SectorAddr = (GetIndirectSectorNumber(Inode.pwTable[8], Addr));
        if(*SectorAddr == INVALID_SECTOR_NUMBER)
            return -1;
        if(Indirect != NULL){
            Indirect->IdTable[0] = Inode.pwTable[8];
            Indirect->IdAdress[0] = Addr;
            Indirect->AD_Type = uFS_AT_INDIRECT_1;
        }
        return 0;

    } else Addr -= INDERECT_ADDR_x1_SIZE;

    if(Addr < INDERECT_ADDR_x2_SIZE){ 

        // ������� ��������� ���������
        // Inode.pwTable[9]-->table1[128]-->table2[128]-->data[256]

        if(Inode.pwTable[9] == INVALID_SECTOR_NUMBER)
            return -1;
        WORD Table1 = Addr >> TABLE_SECTOR_SIZE_BIT;
        WORD Table2 = Addr & TABLE_SECTOR_MASK;
        WORD Addr2 = GetIndirectSectorNumber(Inode.pwTable[9], Table1);
        if(Addr2 == INVALID_SECTOR_NUMBER)
            return -1;
        *SectorAddr = GetIndirectSectorNumber(Addr2, Table2);
        if(*SectorAddr == INVALID_SECTOR_NUMBER)
            return -1;
        if(Indirect != NULL){
            Indirect->IdTable[0] = Inode.pwTable[9];
            Indirect->IdTable[1] = Addr2;
            Indirect->IdAdress[0] = Table1;
            Indirect->IdAdress[1] = Table2;
            Indirect->AD_Type = uFS_AT_INDIRECT_2;
        }
        return 0;
    } else Addr -= INDERECT_ADDR_x2_SIZE;

    if(Addr < INDERECT_ADDR_x3_SIZE){

        // ������� ��������� ���������
        // Inode.pwTable[10]-->table1[]-->table2[]-->table3[]-->data
        //               table1[128]  table2[128]  table3[128]  data[256]
        // Addr: 0000000      0000000      0000000      00000000
        //               0x7F         0x7F         0x7F         0xFF

        WORD Table1 = Addr >> (TABLE_SECTOR_SIZE_BIT * 2);
        WORD Table2 = (Addr >> TABLE_SECTOR_SIZE_BIT) & TABLE_SECTOR_MASK;
        WORD Table3 = Addr & TABLE_SECTOR_MASK;
        if(Inode.pwTable[10] == INVALID_SECTOR_NUMBER)
            return -1;
        WORD Addr2 = GetIndirectSectorNumber(Inode.pwTable[10], Table1);
        if(Addr2 == INVALID_SECTOR_NUMBER)
            return -1;
        WORD Addr3 = GetIndirectSectorNumber(Addr2, Table2);
        if(Addr3 == INVALID_SECTOR_NUMBER)
            return -1;
        *SectorAddr = GetIndirectSectorNumber(Addr3, Table3);
        if(*SectorAddr == INVALID_SECTOR_NUMBER)
            return -1;
        if(Indirect != NULL){
            Indirect->IdTable[0] = Inode.pwTable[10];
            Indirect->IdTable[1] = Addr2;
            Indirect->IdTable[2] = Addr3;
            Indirect->IdAdress[0] = Table1;
            Indirect->IdAdress[1] = Table2;
            Indirect->IdAdress[2] = Table3;
            Indirect->AD_Type = uFS_AT_INDIRECT_3;
        }
        return 0;
    }
    return -1;
}


// ***************************************************************************
int uFS_GetAddr(
    INODE_RECORD Inode,             // Inode �����
    DWORD Addr, DWORD * DataAddr )
{
    WORD SectorAddr;
    int Err = uFS_GetSectorAddr(Inode, (WORD)(Addr >> DATA_SECTOR_SIZE_BIT), &SectorAddr, NULL);
    if(Err == 0){
        *DataAddr = (SectorAddr  << DATA_SECTOR_SIZE_BIT) | (Addr & SECTOR_MASK);
    }
    return Err;
}

// ***************************************************************************
int uFS_ExpandFile(INODE_RECORD * Inode, DWORD NewSize)
{
    WORD PhysicalAddr = 0;
    int Err = 0;
    
    if(NewSize <= Inode->dwSize){
        return -1;
    }

    DWORD Count = NewSize - Inode->dwSize;
    DWORD SectorCount = (Count >> DATA_SECTOR_SIZE_BIT);
    if((Count & SECTOR_MASK) > 0){
        SectorCount++;
    }

    for(DWORD i = 0; i < SectorCount; i++){
        Err = uFS_AddSector(Inode, &PhysicalAddr);
        if(Err != 0) 
            return Err;
        Inode->dwSize += SECTOR_SIZE;
    }
    Inode->dwSize = NewSize;
    return 0;
}

// ***************************************************************************
// ��������� ������ � �����
int uFS_AddSector(INODE_RECORD * Inode, WORD * SectorAddr)
{
    return uFS_AddSectorFromRange(Inode, SectorAddr, 0, SECTORS_COUNT - 1 );
}

// ***************************************************************************
// ��������� ������ � ����� �� ��������
int uFS_AddSectorFromRange(INODE_RECORD * Inode, WORD * SectorAddr, WORD BeginSector, WORD EndSector)             // Inode �����    
{
    
    // ���������� ������� ���������� �������� � �����
    WORD SectorNumber = Inode->wSectorsCount;

    // ���������� ��� ���������    
    if(SectorNumber < 8){

        // ������ ���������
        if(Inode->pwTable[SectorNumber] == INVALID_SECTOR_NUMBER){
            if(uFS_ClaimFreeSectorFromRange(&Inode->pwTable[SectorNumber], BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
            Inode->wSectorsCount++;
        }
        if(SectorAddr != NULL)
            *SectorAddr = Inode->pwTable[SectorNumber]; 
        return 0;

    } else SectorNumber -= 8;

    if(SectorNumber < INDERECT_ADDR_x1_SIZE){

        // ��������� ���������
        // Inode.pwTable[8]-->table1[]-->data
        //               table1[128]  data[256]
        // SectorNumber: 0000000      00000000
        // 
        if((SectorNumber == 0) && (Inode->pwTable[8] == INVALID_SECTOR_NUMBER )){
            if(uFS_ClaimFreeSectorFromRange(&Inode->pwTable[8], BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
        }

        WORD Table1 = (GetIndirectSectorNumber(Inode->pwTable[8], SectorNumber));
        if(Table1 == INVALID_SECTOR_NUMBER){

            if(uFS_ClaimFreeSectorFromRange(&Table1, BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������            
            SetIndirectSectorNumber(Inode->pwTable[8], SectorNumber, Table1);
            Inode->wSectorsCount++;
        } 
        if(SectorAddr != NULL)
            *SectorAddr = Table1;
        return 0;

    } else SectorNumber -= INDERECT_ADDR_x1_SIZE;

    if(SectorNumber < INDERECT_ADDR_x2_SIZE){ 

        // ������� ��������� ���������
        // Inode.pwTable[9]-->table1[]-->table2[]-->data
        //               table1[128]  table2[128]  data[256]
        // SectorNumber: 0000000      0000000      00000000

        if((SectorNumber == 0) && (Inode->pwTable[9] == INVALID_SECTOR_NUMBER )){
            if(uFS_ClaimFreeSectorFromRange(&Inode->pwTable[9], BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
        }

        WORD Table1 = SectorNumber >> TABLE_SECTOR_SIZE_BIT;
        WORD Table2 = SectorNumber & TABLE_SECTOR_MASK;

        WORD Addr1 = (GetIndirectSectorNumber(Inode->pwTable[9], Table1));

        if(Addr1 == INVALID_SECTOR_NUMBER){
            if(uFS_ClaimFreeSectorFromRange(&Addr1, BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
            SetIndirectSectorNumber(Inode->pwTable[9], Table1, Addr1);
        }

        WORD Addr2 = (GetIndirectSectorNumber(Addr1, Table2));

        if(Addr2 == INVALID_SECTOR_NUMBER){
            if(uFS_ClaimFreeSectorFromRange(&Addr2, BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
            SetIndirectSectorNumber(Addr1, Table2, Addr2);
            Inode->wSectorsCount++;
        }
        if(SectorAddr != NULL)
            *SectorAddr = Addr2;
        
        return 0;

    } else SectorNumber -= INDERECT_ADDR_x2_SIZE;

    if(SectorNumber < INDERECT_ADDR_x3_SIZE){

        // ������� ��������� ���������
        // Inode.pwTable[10]-->table1[]-->table2[]-->table3[]-->data
        //               table1[128]  table2[128]  table3[128]  data[256]
        // SectorNumber: 0000000      0000000      0000000      00000000

        if((SectorNumber == 0) && (Inode->pwTable[10] == INVALID_SECTOR_NUMBER )){
            if(uFS_ClaimFreeSectorFromRange(&Inode->pwTable[10], BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
        }

        WORD Table1 = SectorNumber >> (TABLE_SECTOR_SIZE_BIT * 2);
        WORD Table2 = (SectorNumber >> TABLE_SECTOR_SIZE_BIT) & TABLE_SECTOR_MASK;
        WORD Table3 = SectorNumber & TABLE_SECTOR_MASK;

        WORD Addr1 = (GetIndirectSectorNumber(Inode->pwTable[10], Table1));

        if(Addr1 == INVALID_SECTOR_NUMBER){
            if(uFS_ClaimFreeSectorFromRange(&Addr1, BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
            SetIndirectSectorNumber(Inode->pwTable[10], Table1, Addr1);
        }

        WORD Addr2 = (GetIndirectSectorNumber(Addr1, Table2));
        if(Addr2 == INVALID_SECTOR_NUMBER){
            if(uFS_ClaimFreeSectorFromRange(&Addr2, BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
            SetIndirectSectorNumber(Addr1, Table2, Addr2);
        }

        WORD Addr3 = (GetIndirectSectorNumber(Addr2, Table3));
        if(Addr3 == INVALID_SECTOR_NUMBER){
            if(uFS_ClaimFreeSectorFromRange(&Addr3, BeginSector, EndSector) != 0)
                return -1; // ��� ��������� ��������
            SetIndirectSectorNumber(Addr2, Table3, Addr3);
            Inode->wSectorsCount++;
        }
        if(SectorAddr != NULL)
            *SectorAddr = Addr3;

        return 0;
    }
    return -1;

}

// ***************************************************************************
// ������ ������ � ������������ �� ������� �������.
// ���������� ���������� ������� ����������� ����
int uFS_ReadData(DWORD Addr, BYTE * Dest, int Count)
{
    int CountToRead = SECTOR_SIZE - (Addr & SECTOR_MASK);
    if(Addr == ((INVALID_SECTOR_NUMBER << DATA_SECTOR_SIZE_BIT) | SECTOR_MASK)){
        return -1;
    }
    if(Count == 0) 
        return 0;
    if(Count >= CountToRead){
        uFS_Read(Addr, Dest, CountToRead);
        return CountToRead;
    } else {
        uFS_Read(Addr, Dest, Count);
        return Count;
    }
    //return 0;
}

// ***************************************************************************
// ���������� ������ � ������������ �� ������� �������.
// ���������� ���������� ������� ���������� ����
int uFS_WriteData(DWORD Addr, BYTE * Source, int Count)
{
    int CountToWrite = SECTOR_SIZE - (Addr & SECTOR_MASK);
    if(Addr == ((INVALID_SECTOR_NUMBER << DATA_SECTOR_SIZE_BIT) | SECTOR_MASK)){
        return -1;
    }
    if(Count == 0) 
        return 0;
    if(Count >= CountToWrite){
        uFS_Write(Addr, Source, CountToWrite);
        return CountToWrite;
    } else {
        uFS_Write(Addr, Source, Count);
        return Count;
    }
    //return 0;
}

// ***************************************************************************
// ������ ������ �� �����
// ���������� ���������� ����������� ���� � ������ ������ ��� ��� ������
int uFS_ReadFile(
    INODE_RECORD Inode,             // Inode �����
    DWORD Address,                  // ����� ������ �����
    BYTE * Dest,                    // ����� ������
    int Count                       // ���������� � ������
    )                               // ���������� ���������� ������� ��������� ���� ��� �������� < 0 ��� ������
{
    DWORD PhysicalAddr = 0;
    int Err = 0;
    int ReadedCnt = 0;
    
    if(Count + Address > Inode.dwSize){
        Count = Inode.dwSize - Address;
    }
    if (Count < 0)
        return -1;
    
    while(Count != ReadedCnt){

        Err = uFS_GetAddr(Inode, Address + ReadedCnt , &PhysicalAddr);
        if(Err != 0){
            return Err;
        }

        int Cnt = uFS_ReadData(PhysicalAddr, &Dest[ReadedCnt], Count - ReadedCnt);
        if(Cnt < 0 ){
            return Cnt;
        } 
        ReadedCnt += Cnt;
    }

    return ReadedCnt;
}

// ***************************************************************************
// ���������� ������ � ����
// ���������� ���������� ��������� ���� � ������ ������ ��� ��� ������
int uFS_WriteFile(
    INODE_RECORD * Inode,           // Inode �����
    DWORD Address,                  // ����� ������ �����
    BYTE * Source,                  // ����� ������
    int Count                       // ���������� � ������
    )                               // ���������� ���������� ������� ���������� ���� ��� �������� < 0 ��� ������
{
    DWORD PhysicalAddr = 0;
    WORD PhysicalSector = 0;
    int Err = 0;
    int WritedCnt = 0;
       
    if (Count < 0)
        return -1;

    while(Count != WritedCnt){

        if(uFS_GetSectorAddr(*Inode, (WORD)(Address >> DATA_SECTOR_SIZE_BIT), &PhysicalSector, NULL) != 0){
            Err = uFS_AddSector(Inode, &PhysicalSector);
            if(Err != 0){
                return Err;
            }
        }
        PhysicalAddr = (PhysicalSector << DATA_SECTOR_SIZE_BIT) | (Address & SECTOR_MASK);
        int Cnt = uFS_WriteData(PhysicalAddr, &Source[WritedCnt], Count - WritedCnt);
        if(Cnt < 0 ){
            return Cnt;
        } 
        WritedCnt += Cnt;
        //Inode->dwSize += Cnt;
    }

    return WritedCnt;
}

// ***************************************************************************
// ����� � �������� �������� � �������, ������� ������� Beg � ���������� �������� End � ������� � Pos
int DropOneInAvailableSectors(FREE_SECTORS * Buf, WORD Size, WORD Beg, WORD End, WORD* Pos)
{
    WORD MaxOnesCount = Size << 3;
    if((Beg > MaxOnesCount)||(Size == 0)||(End == Beg)||(Pos == NULL)||(Buf == NULL)||(Beg > End)){
        return -1;
    }

    if(End > MaxOnesCount){
        End = MaxOnesCount;    
    }

    WORD MinByte = Beg >> 3;
    WORD MaxByte = End >> 3;
    WORD MinBit = Beg & 0x07;
    WORD MaxBit = End & 0x07;

    WORD j = 0;
    for(WORD i = MinByte; i <= MaxByte; i++){
        if(Buf[i].AvailableSectors > 0){
            if(i == MinByte)
                j = MinBit;
            else 
                j = 0;
            while(!((Buf[i].AvailableSectors >> j) & 0x1) && (j < 8)){
                j++;
            }            
            if((i == MaxByte) && (j > MaxBit))
                return 1; // ��� ���������
            else if(j > 7) 
                continue;

            Buf[i].AvailableSectors &= (~(1 << j));
            *Pos = (i << 3) + j;

            return 0;
        }
    }
    return 1;
}

// ***************************************************************************
// ����������� ��������� ������ �� ��������� ���������
int uFS_ClaimFreeSectorFromRange(WORD * ClaimedSector, WORD BeginSector, WORD EndSector)
{
    WORD CurrentEnd = (WORD)(uFS_FreeSectors.FreeFile->DataPointer * 8 / sizeof(FREE_SECTORS));
    WORD CurrentBeg = CurrentEnd - 128;
    
    WORD Beg = BeginSector >> 3;
    //WORD End = EndSector >> 3;

    // ��� ����� �������� ������� �������� ���������
    // ��������� ����� �������� �� �����
    // ���� �������� ������ ������ �������� �����, ������� ����� ���
    
    // ��������� CurrentPos ��������� �� ���� 16 ���� 
    // 


    
    // 1. ���� ��� ���������
    //
    //                 BeginSector
    //                 |       EndSector
    //                 V       V
    //*****************+++++++++***************  
    //---++++----------------------------------
    //   ^  ^
    //   |  CurrentEnd
    //   CurrentBeg

    // 2. ���� ����������� ������ ���������
    //
    //                 BeginSector
    //                 |       EndSector
    //                 V       V
    //*****************+++++++++***************  
    // -------------++++++---------------------
    //              ^    ^
    //              |    CurrentEnd
    //              CurrentBeg

    // 3. ���� ������ ���������
    //
    //                 BeginSector
    //                 |       EndSector
    //                 V       V
    //*****************+++++++++***************  
    //-------------------++++------------------
    //                   ^  ^
    //                   |  CurrentEnd
    //                   CurrentBeg

    // 4.���� ����������� ����� ���������
    //
    //                 BeginSector
    //                 |       EndSector
    //                 V       V
    //*****************+++++++++***************  
    //------------------------++++-------------
    //                        ^  ^
    //                        |  CurrentEnd
    //                        CurrentBeg
    // 
    int Err = 1;

    if((CurrentBeg >= BeginSector) && ( CurrentEnd <= EndSector) && (uFS_FreeSectors.FreeFile->DataPointer > 0)){
        // �������� �� ����
        CurrentEnd = (WORD)(uFS_FreeSectors.FreeFile->DataPointer * 8 / sizeof(FREE_SECTORS));
        CurrentBeg = CurrentEnd - 128;
        WORD Count = CurrentEnd - CurrentBeg;
        WORD b = 0;
        if(CurrentBeg < BeginSector){
            b = BeginSector & 0x07;
        }
        if(CurrentBeg >= EndSector){
            return -1;
        }
        if(CurrentEnd > EndSector){
            Count = EndSector - CurrentBeg;
        }
        Err = DropOneInAvailableSectors(uFS_FreeSectors.FreeSectorsCache, 16, b, Count, ClaimedSector);
        if(Err == 0){
            *ClaimedSector += CurrentBeg;
            return 0;
        }
    } 

    {
        // ���� ���������� ����� ������
        if(uFS_FreeSectors.FreeFile->Inode.dwSize != 0){
            
            // �������� ���
            if(uFS_FreeSectors.FreeFile->DataPointer > 0){
                uFS_fseek(uFS_FreeSectors.FreeFile, -(int)sizeof(uFS_FreeSectors.FreeSectorsCache), uFS_SEEK_CUR);
                uFS_fwrite((BYTE*)uFS_FreeSectors.FreeSectorsCache, sizeof(uFS_FreeSectors.FreeSectorsCache), 1, uFS_FreeSectors.FreeFile);
            }
            // ��������� ���
            // ���� ���� �� � ���������, �����������
            if(!((CurrentBeg >= BeginSector) && ( CurrentEnd <= EndSector))){
                if(uFS_fseek(uFS_FreeSectors.FreeFile, Beg, uFS_SEEK_SET) != 0)
                    return -1;
            }

            while( Err > 0){

                if(uFS_fread((BYTE*)uFS_FreeSectors.FreeSectorsCache, sizeof(uFS_FreeSectors.FreeSectorsCache), 1, uFS_FreeSectors.FreeFile) < 0 )
                    return -1;
                CurrentEnd = (WORD)(uFS_FreeSectors.FreeFile->DataPointer * 8 / sizeof(FREE_SECTORS));
                CurrentBeg = CurrentEnd - 128;
                WORD Count = CurrentEnd - CurrentBeg;
                WORD b = 0;
                if(CurrentBeg < BeginSector){
                    b = BeginSector & 0x07;
                }
                if(CurrentBeg >= EndSector){
                    return -1;
                }
                if(CurrentEnd > EndSector){
                    Count = EndSector - CurrentBeg;
                }
                Err = DropOneInAvailableSectors(uFS_FreeSectors.FreeSectorsCache, 16, b, Count, ClaimedSector);
                if(Err == 0){
                    *ClaimedSector += CurrentBeg;
                    return 0;
                }
            }
        }
    }
    return -1;
}

// ***************************************************************************
// ����������� ��������� ������
int uFS_ClaimFreeSector(WORD * ClaimedSector)
{
    return uFS_ClaimFreeSectorFromRange(ClaimedSector, 0, sizeof(FreeSectors)/sizeof(FREE_SECTORS));
}

// ***************************************************************************
// ����������� ������
int uFS_ReleaseSector(WORD ReleasedSector)
{   
    int i = ReleasedSector >> 3;  
    int j = ReleasedSector & 0x07;
    if(FreeSectors[i].ReleasedSectors > 0){  
        FreeSectors[i].ReleasedSectors &= (~(1 << j));
        return 0;
    }
    return -1;
}


// ***************************************************************************

int uFS_ReleaseFileSector(INODE_RECORD Inode, WORD SectorIndex)
{   
    IdTable Indirect;
    WORD SectorAddr = 0;
        
    // ��������� ���������� � �������
    uFS_GetSectorAddr( Inode, SectorIndex, &SectorAddr, &Indirect );

    // ������������ ������� ������
    if(uFS_ReleaseSector(SectorAddr) != 0)
        return -1;  // ������ ������������ �������   

    // ������������ �������� �������

    switch (Indirect.AD_Type) {
    case uFS_AT_DIRECT:     // ������ ���������
        break;
    case uFS_AT_INDIRECT_1: // ��������� ���������
        if(Indirect.IdAdress[0] == 0){
            if(uFS_ReleaseSector(Indirect.IdTable[0]) != 0)
                return -1;  // ������ ������������ �������   
        }
        break;
    case uFS_AT_INDIRECT_2: // ������� ��������� ���������
        if(Indirect.IdAdress[1] == 0){
            if(uFS_ReleaseSector(Indirect.IdTable[1]) != 0)
                return -1;  // ������ ������������ �������   
            if(Indirect.IdAdress[0] == 0){
                if(uFS_ReleaseSector(Indirect.IdTable[0]) != 0)
                    return -1;  // ������ ������������ �������   
            }
        }
        break;
    case uFS_AT_INDIRECT_3: // ������� ��������� ���������
        if(Indirect.IdAdress[2] == 0){
            if(uFS_ReleaseSector(Indirect.IdTable[2]) != 0)
                return -1;  // ������ ������������ �������   
            if(Indirect.IdAdress[1] == 0){
                if(uFS_ReleaseSector(Indirect.IdTable[1]) != 0)
                    return -1;  // ������ ������������ �������   
                if(Indirect.IdAdress[0] == 0){
                    if(uFS_ReleaseSector(Indirect.IdTable[0]) != 0)
                        return -1;  // ������ ������������ �������   
                }
            }
        }
        break;
    default:
        return -2;          // ����������� ��� ���������
        break;
    }
   
    return 0;
}

int uFS_ReleaseFileSectors(INODE_RECORD Inode)
{
    int Result = 0;
    for(WORD i = 0; i < Inode.wSectorsCount; i++){
        Result |= uFS_ReleaseFileSector(Inode, i);
    }
    return Result;
}

BYTE bc[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
// ***************************************************************************
BYTE bBitCount(BYTE x)
{
    return  bc[x & 0x0F] + bc[(x >> 4) & 0x0F];
}

// ***************************************************************************
BYTE wBitCount(WORD x)
{
    return  bc[x & 0x000F] + bc[(x >> 4) & 0x000F] + bc[(x >> 8) & 0x000F] + bc[(x >> 12) & 0x000F];
}

// ***************************************************************************
WORD uFS_GetFreeSectorCount()
{
    return uFS_GetFreeSectorCountInRange(0, SECTORS_COUNT - 1);
}

// ***************************************************************************
WORD uFS_GetFreeSectorCountInRange(WORD BeginSector, WORD EndSector)
{
    WORD count = 0;
    WORD Beg = BeginSector >> 3;
    WORD End = EndSector >> 3;
    if(End > sizeof(FreeSectors)/sizeof(FREE_SECTORS)){
        End = sizeof(FreeSectors)/sizeof(FREE_SECTORS);
    }    
    for(int i = Beg; i <= End; i++){
        count += bBitCount(FreeSectors[i].AvailableSectors);
    }
    return count;
}

// ***************************************************************************
WORD uFS_GetReleasedSectorCount()
{
    return uFS_GetReleasedSectorCountInRange(0, SECTORS_COUNT - 1);
}

// ***************************************************************************
WORD uFS_GetReleasedSectorCountInRange(WORD BeginSector, WORD EndSector)
{
    WORD count = 0;
    WORD Beg = BeginSector >> 3;
    WORD End = EndSector >> 3;
    if(End > sizeof(FreeSectors)/sizeof(FREE_SECTORS)){
        End = sizeof(FreeSectors)/sizeof(FREE_SECTORS);
    }    
    for(int i = Beg; i < End; i++){
        count += bBitCount(FreeSectors[i].ReleasedSectors);
    }
    return count;
}

// ***************************************************************************
int uFS_RemoveFile(INODE_RECORD Inode)
{
    WORD SectorsCount = (WORD)(Inode.dwSize >> DATA_SECTOR_SIZE_BIT);
    if((Inode.dwSize & SECTOR_MASK) > 0){
        SectorsCount++;
    }
    // ���������� �������
    for(WORD i = 0; i < SectorsCount; i++){
        if(uFS_ReleaseFileSector(Inode, i) !=0 )
            return -1;

    }
    //TODO: ���������� ����
    return 0;
}

int uFS_remove ( const char * filename )
{
    // ����� ���� � ��������.
    // ����� ��� Inode
    // ������� Inode � ������� uFS_RemoveFile
    // ������� ������ �� ����� ����������
    
    return 0;
}
// ***************************************************************************
int uFS_Init()
{
    for ( int i = 0; i < uFS_MAX_OPENED_FILE + 3; i++){
        
        OpenFileArray[i].ErrorState = uFS_ERROR_FREE_RECORD;
        OpenFileArray[i].Flags.Val = 0;
        OpenFileArray[i].DataPointer = 0;

        OpenFileArray[i].Inode.dwSize = 0;
        OpenFileArray[i].Inode.wFlags = 0;
        OpenFileArray[i].Inode.wInodeID = 0;
        memset(OpenFileArray[i].Inode.pwTable, 0xFF, sizeof(OpenFileArray[i].Inode.pwTable));
        
        OpenFileArray[i].Dir.bNameLength = 0;
        memset(OpenFileArray[i].Dir.pcFileName, 0, MAX_FILE_NAME_LENGTH);
        OpenFileArray[i].Dir.wInodeID = 0;
    }
    uFS_FILE fs_root;
    SUPERBLOCK SuperBlock;
    fs_root.Inode.dwSize = sizeof(SuperBlock);
    fs_root.Inode.wInodeID = 1;
    fs_root.Inode.wFlags = 0;
    fs_root.Flags.Enums.Mode = uFS_MODE_READ;
    fs_root.DataPointer = 0;

    memset(fs_root.Inode.pwTable, INVALID_SECTOR_NUMBER, sizeof(fs_root.Inode.pwTable));
    fs_root.Inode.pwTable[0] = 0;

    if(uFS_fread ( (BYTE*)&SuperBlock, sizeof(SuperBlock), 1, &fs_root ) != sizeof(SuperBlock)){
        return -1;
    }
    
    fs_root.Inode.dwSize = sizeof(INODE_RECORD);
    fs_root.Inode.wInodeID = 1;
    fs_root.Inode.wFlags = 0;
    fs_root.Flags.Enums.Mode = uFS_MODE_READ;
    fs_root.DataPointer = 0;
    fs_root.Inode.pwTable[0] = (WORD)(SuperBlock.InodeTable >> DATA_SECTOR_SIZE_BIT);

    if(uFS_fread((BYTE*)&OpenFileArray[0].Inode, sizeof(INODE_RECORD), 1, &fs_root) < 0){
        return -1;
    } else {
        // ���� ������ ��������� ��� ������. ���� ���� ��������, �������� ��������� ��� ������ 
        OpenFileArray[0].ErrorState = uFS_ERROR_NO_ERROR;
        OpenFileArray[0].Flags.Enums.Binary = uFS_BINARY;
        OpenFileArray[0].Flags.Enums.Mode = uFS_MODE_READ;
        OpenFileArray[0].DataPointer = 0;        
    }
    FILESTATE Flags;
    Flags.Enums.Mode = uFS_MODE_READ;
    Flags.Enums.Binary = uFS_BINARY;
    Flags.Enums.Dir = uFS_RECORD_IS_FILE;

    //uFS_FileOpen(uFS_INODE_TABLE_ID, Flags);
    uFS_FileOpen(uFS_FREE_FILE_ID, Flags);
    Flags.Enums.Dir = uFS_RECORD_IS_DIR;
    uFS_FileOpen(uFS_ROOT_DIR_ID, Flags);

    uFS_FileOpen(0xFFFF, Flags);

    //uFS_SearchInodeInDir(uFS_ROOT_DIR_ID, ".free", 5);
    WORD Parent = 0;
    char * P = NULL;
    WORD Inode = uFS_GetInodeByPath("/.free", &Parent, &P);
    uFS_DeleteInode(Inode, NULL);

    {
        strcpy (OpenFileArray[0].Dir.pcFileName,".inodes");
        OpenFileArray[0].Dir.bNameLength = strlen(OpenFileArray[0].Dir.pcFileName);        

        strcpy (OpenFileArray[1].Dir.pcFileName,".free");
        OpenFileArray[1].Dir.bNameLength = strlen(OpenFileArray[1].Dir.pcFileName);        

        strcpy (OpenFileArray[2].Dir.pcFileName,"/");
        OpenFileArray[2].Dir.bNameLength = strlen(OpenFileArray[2].Dir.pcFileName);        
    }
    
    //uFS_RemoveFile(OpenFileArray[0].Inode);
    //uFS_ReleaseFileSector(&OpenFileArray[0].Inode, 0);
    //DWORD SectorAddr;
    //IdTable Indirect;

    //uFS_GetSectorAddr( OpenFileArray[0].Inode, 32762, &SectorAddr, Indirect );
    //uFS_GetSectorAddr( OpenFileArray[0].Inode, 29000, &SectorAddr, NULL );

    // ��������� ���������� ��������� ����� �����
    uFS_LastInodeID = uFS_FindLastInodeID();
 
    
    return 0;
}


WORD uFS_FindLastInodeID()
{
    WORD wInodeID = 0;
    WORD wMAXInodeID = 0;
    uFS_FILE * InodeFile = NULL;
    
    InodeFile = &OpenFileArray[0];

    // ������������� ��������� �� ������
    if(uFS_fseek(InodeFile, 0, uFS_SEEK_SET) == 0){

        while(uFS_fread((BYTE*)&wInodeID, sizeof(wInodeID), 1, InodeFile) == sizeof(wInodeID)){

            if ((wInodeID > wMAXInodeID) && (wInodeID != uFS_INVALID_HANDLE)) {   
                wMAXInodeID = wInodeID;
            } 

            if(uFS_fseek(InodeFile, sizeof(INODE_RECORD) - sizeof(WORD), uFS_SEEK_CUR) < 0){
                break;
            }
        }
    }
    uFS_fseek(InodeFile, 0, uFS_SEEK_SET);

    return ++wMAXInodeID;
}


// ����� �� ���������.
// ����:    ���� � �����
// �����:   InodeID �����, 
//          uFS_INVALID_HANDLE � ������ ���� ���� ��� ���� �� ������
//          ���� ParentID != NULL ���������� ID ���������� ���������� �������� �� ����
//          ���� ParentPath != NULL ���������� ����� � Path �������� �������� ���� ��� ������, 
//              ��� ����� ��������, �� ������� ��������� ������ 
//          
//    ParentID ��� ������������ ����������
//    | ParentPath
//    v v           
//  /dir/rete/yurt
WORD uFS_GetInodeByPath(const char * Path, WORD * ParentID, char ** ParentPath )
{
    char * BeginFName;
    char * EndFName;
    char * EndPath;
    int PathLen = strlen(Path);
    WORD DirInode = 4; // �������� ����������
    WORD TmpDirInode = 0;

    EndPath = (char*)&Path[PathLen];
    EndFName = (char*)memchr(Path, '/', PathLen); //strrchr 
    while (1) {        
        if(EndFName < EndPath){
            EndFName++;
            BeginFName = EndFName;
            EndFName = (char*)memchr(BeginFName, '/', PathLen);
            if(EndFName == 0) {
                EndFName = EndPath;
            }
            if(ParentID != NULL){
                *ParentID = DirInode;
            }
            if(ParentPath != NULL){
                *ParentPath = BeginFName;
            }
            DirInode = uFS_SearchInodeInDir(DirInode, BeginFName, (BYTE)(EndFName - BeginFName));
            if (DirInode == uFS_INVALID_HANDLE) {
                break;
            }
        } else 
            break;

    }
    return DirInode;
}

WORD uFS_SearchInodeInDir(WORD DirInode, const char * FileName, BYTE FileNameLen)
{
    uFS_FILE * DirFile;
    FILESTATE Flags;
    DIR_RECORD Dir;

    Flags.Enums.Binary = uFS_BINARY;
    Flags.Enums.Mode = uFS_MODE_READ;
    
    DirFile = uFS_FileOpen(DirInode, Flags);
    if(DirFile == NULL)
        return uFS_INVALID_HANDLE;

    while(uFS_fread((BYTE*)&Dir, 3, 1, DirFile) == 3){
        if (FileNameLen == (Dir.bNameLength - 1)) {   
            // ������ ��� �����
            if(uFS_fread((BYTE*)&Dir.pcFileName, Dir.bNameLength, 1, DirFile) < 0){
                // ������ ������ ����� ����������
                return uFS_INVALID_HANDLE;
            }
            // ���������� ��� ����� ������
            if(memcmp(FileName,Dir.pcFileName, FileNameLen) == 0){
                // ���� �����, ��������� ������� ���� ���������� � 
                // ���������� Inode ���������� ����� ��� ����������
                uFS_fclose(DirFile);
                return Dir.wInodeID;
            }
        } else {
            if(uFS_fseek(DirFile, Dir.bNameLength, uFS_SEEK_CUR) < 0){
                // ������ ������ ����� ����������
                return uFS_INVALID_HANDLE;
            }
        }
    }
    return uFS_INVALID_HANDLE;
}


int uFS_mkdir(const char * DirName)
{
    char * ParentDirName = NULL;
    WORD ParentID = 0;

    if(uFS_GetInodeByPath(DirName, &ParentID, &ParentDirName) != uFS_INVALID_HANDLE){
        // ���� � ����� ������ ����������
        return -1;
    }
    return 0;
}

// ***************************************************************************
// ������� ���������� ����������� ������ �����, ���� ������ � ����� ID �������.
// ���������� ��������� ������, ���� ����� Inode ����� uFS_INVALID_HANDLE
// 
// ����:
// wInode           ����� Inode
// Offset           ������ ������ � ���������� ���������
// �����:       
// Inode            ��������� �� ��������� Inode
// SearchFromOffset ��������� Inode � ����� ������
// 
// �������:          0 �����
//                  -1 �������
// ***************************************************************************
int uFS_GetFileInode(WORD wInode, INODE_RECORD * Inode, DWORD * SearchFromOffset)
{
    uFS_FILE * InodeFile = &OpenFileArray[0];
    BYTE RecordFound = 0;
    DWORD dwOffset = 0;

    if(wInode == 0)
        return -1;

    if(SearchFromOffset != NULL)
        dwOffset = *SearchFromOffset;

    // ���� � ����� ������ Inode � �������� ID
    uFS_fseek( InodeFile, dwOffset, uFS_SEEK_SET);

    while(uFS_fread((BYTE*)Inode, sizeof(INODE_RECORD), 1, InodeFile) == sizeof(INODE_RECORD)){

        if(Inode->wInodeID == wInode) {            
            if(SearchFromOffset != NULL){
                *SearchFromOffset = dwOffset;
            }
            return 0;            
        }
        dwOffset = InodeFile->DataPointer;
    }

    return -1;
}


int uFS_DeleteInode(WORD wInode, DWORD SearchFromOffset)
{
    INODE_RECORD Inode;
    uFS_FILE * InodeFile = &OpenFileArray[0];

    if(wInode == uFS_INVALID_HANDLE)
        return -1;

    memset(&Inode, 0xFF, sizeof(Inode));

    if(uFS_GetFileInode(wInode, &Inode, &SearchFromOffset)){
        return -1;
    }

    InodeFile->Flags.Enums.Mode = uFS_MODE_READ_PLUS;

    Inode.wInodeID = 0;

    uFS_fseek( InodeFile, SearchFromOffset, uFS_SEEK_SET);
    uFS_fwrite((BYTE*)&Inode, sizeof(Inode), 1, InodeFile);

    InodeFile->Flags.Enums.Mode = uFS_MODE_READ;

    return 0;
}


// ***************************************************************************
// �������� �����
int uFS_fclose( uFS_FILE * stream)
{
    // TODO: ����� ������ ����� � ����� ���������� �����, ���� ��� ��� ���� �� ������
    // TODO: ������ Inode �����, ���� �� ��� ������ ��� ����������, ���� ������
    stream->ErrorState = uFS_ERROR_FREE_RECORD;
    return 0;
}



// ***************************************************************************
// �������� ����� �� ������ Inode
uFS_FILE *  uFS_FileOpen(WORD wInode, FILESTATE Flags)
{
    INODE_RECORD Inode;
    BYTE bFreeRecordId = 0;
    WORD wFoundInode = 0;
    uFS_FILE * InodeFile = &OpenFileArray[0];
    BYTE RecordFound = 0;
    DWORD InodeOffset = 0;

    // ���� ��������� ����� � �������
    for ( int i = 0; i < uFS_MAX_OPENED_FILE + 3; i++){
        if(OpenFileArray[i].ErrorState == uFS_ERROR_FREE_RECORD){
            bFreeRecordId = i;
            break;
        }        
    }
    if(bFreeRecordId == 0)
        return NULL;

    // ���� � ����� ������ Inode � �������� ID
    if(uFS_GetFileInode(wInode, &Inode, &InodeOffset) == 0){
        RecordFound = 1;
    }
    
    switch(Flags.Enums.Mode){

    case uFS_MODE_READ: // ��������� ��� ������
    case uFS_MODE_READ_PLUS:
        if (!RecordFound){
            return NULL;
        }
        // ���� � ����� InodeID ����������
        memcpy(&OpenFileArray[bFreeRecordId].Inode, &Inode, sizeof(INODE_RECORD));
        OpenFileArray[bFreeRecordId].ErrorState = uFS_ERROR_NO_ERROR;
        OpenFileArray[bFreeRecordId].Flags.Val = Flags.Val;
        // ���������� ��������� � ������ �����
        OpenFileArray[bFreeRecordId].DataPointer = 0;
        OpenFileArray[bFreeRecordId].InodeOffset = InodeOffset;
        return &OpenFileArray[bFreeRecordId];

    case uFS_MODE_WRITE: // ��������� ��� ������            
    case uFS_MODE_WRITE_PLUS:        
        if (RecordFound){
            // ������� ����.
            uFS_ReleaseFileSectors(Inode);
            uFS_DeleteInode(wInode, InodeOffset);            
        }
        // ����� ��������� ������
        if(uFS_GetFileInode(wInode, &Inode, &InodeOffset) == 0) {
            wInode = uFS_LastInodeID++;
        }
        // ������� ����
        memcpy(&OpenFileArray[bFreeRecordId].Inode, &Inode, sizeof(INODE_RECORD));
        OpenFileArray[bFreeRecordId].ErrorState = uFS_ERROR_NO_ERROR;
        OpenFileArray[bFreeRecordId].Flags.Val = Flags.Val;
        OpenFileArray[bFreeRecordId].Inode.dwSize = 0;
        OpenFileArray[bFreeRecordId].Inode.wSectorsCount = 0;        
        
        // ���������� ��������� � ������ �����
        OpenFileArray[bFreeRecordId].DataPointer = 0;
        OpenFileArray[bFreeRecordId].InodeOffset = InodeOffset;
        return &OpenFileArray[bFreeRecordId];

    case uFS_MODE_APPEND:
    case uFS_MODE_APPEND_PLUS:
        if (RecordFound){
            // ������� ����
            memcpy(&OpenFileArray[bFreeRecordId].Inode, &Inode, sizeof(INODE_RECORD));
            OpenFileArray[bFreeRecordId].ErrorState = uFS_ERROR_NO_ERROR;
            OpenFileArray[bFreeRecordId].Flags.Val = Flags.Val;  
        } else {
            // ���� ��� ������� �����
            // ����� ��������� ������
            if(uFS_GetFileInode(wInode, &Inode, &InodeOffset) == 0) {
                wInode = uFS_LastInodeID++;
            }
            OpenFileArray[bFreeRecordId].ErrorState = uFS_ERROR_NO_ERROR;
            OpenFileArray[bFreeRecordId].Flags.Val = Flags.Val;
            OpenFileArray[bFreeRecordId].Inode.dwSize = 0;
            OpenFileArray[bFreeRecordId].Inode.wSectorsCount = 0;        
            // ���������� ��������� � ������ �����
            OpenFileArray[bFreeRecordId].DataPointer = 0;
            return &OpenFileArray[bFreeRecordId];
        }
        // ���������� ��������� � ����� �����
        OpenFileArray[bFreeRecordId].DataPointer = OpenFileArray[bFreeRecordId].Inode.dwSize;
        OpenFileArray[bFreeRecordId].InodeOffset = InodeOffset;
        return &OpenFileArray[bFreeRecordId];
    
    default:
        break;
    }        
    
   return NULL;
}


// ***************************************************************************
/*
"r"     read: Open file for input operations. The file must exist.
"w"     write: Create an empty file for output operations. If a file with the same name already exists, 
        its contents are discarded and the file is treated as a new empty file.
"a"     append: Open file for output at the end of a file. Output operations always 
        write data at the end of the file, expanding it. Repositioning operations (fseek, fsetpos, rewind) 
        are ignored. The file is created if it does not exist.
"r+"    read/update: Open a file for update (both for input and output). The file must exist.
"w+"    write/update: Create an empty file and open it for update (both for input and output). 
        If a file with the same name already exists its contents are discarded and the file is treated as a new empty file.
"a+"    append/update: Open a file for update (both for input and output) with all output 
        operations writing data at the end of the file. Repositioning operations 
        (fseek, fsetpos, rewind) affects the next input operations, but output operations 
        move the position back to the end of file. The file is created if it does not exist.
text:
r, r+, rt, rt+, r+t
w, w+, wt, wt+, w+t
a, a+, at, at+, a+t

binary:
rb, rb+, r+b
wb, wb+, w+b
ab, ab+, a+b

/folder/filename.bin
./filename.bin

���� ���� �� ������, �������� �����
*/
// ***************************************************************************
uFS_FILE *  uFS_fopen ( const char * filename, const char * mode )   //Open file
{
    int i;
    FILESTATE Flags;
    WORD wInode;
    uFS_FILE * Stream;

    Flags.Val = 0;
    
    if ((filename == NULL)||(mode == NULL)) {
        return NULL;
    }

    for (i = 0; i < 4; i++)
    {
        switch (mode[i])
        {
        // �����������������
        case 'r':
            Flags.Enums.Mode = uFS_MODE_READ;
            break;
        case 'w':
            Flags.Enums.Mode = uFS_MODE_WRITE;
            break;
        case 'a':
            Flags.Enums.Mode = uFS_MODE_APPEND;
            break;
        // ������������
        case '+':
            switch (Flags.Enums.Mode)
            {
            case uFS_MODE_READ: 
                Flags.Enums.Mode = uFS_MODE_READ_PLUS;
            	break;
            case uFS_MODE_WRITE: 
                Flags.Enums.Mode = uFS_MODE_WRITE_PLUS;
                break;
            case uFS_MODE_APPEND: 
                Flags.Enums.Mode = uFS_MODE_APPEND_PLUS;
                break;
            default:
                break;
            }
            break;
        case 'b':
            Flags.Enums.Binary = uFS_BINARY;
            break;
        case 't':
            Flags.Enums.Binary = uFS_TEXT;
            break;
        case 'd':
            Flags.Enums.Dir = uFS_RECORD_IS_DIR;
            break;
        // ����� ������
        case '\0':
            i = 4;
            break;
        default:
            return NULL;
        }
    }

    wInode = uFS_GetInodeByPath(filename, NULL, NULL);

    if (wInode == uFS_INVALID_HANDLE) {
        // ����� ��� 
    } else {
        // ���� ����
        
    }
       
    Stream = uFS_FileOpen( wInode, Flags);
    return Stream;
}


// ***************************************************************************
// Wite block of data from stream
int uFS_fwrite ( BYTE * ptr, DWORD size, DWORD count, uFS_FILE * stream )
{
    if(stream->Flags.Enums.Mode > uFS_MODE_READ) {
    
        // ���������� � �����
        if((stream->Flags.Enums.Mode == uFS_MODE_APPEND) || (stream->Flags.Enums.Mode == uFS_MODE_APPEND_PLUS)){
            stream->DataPointer = stream->Inode.dwSize;
        }

        int Cnt = uFS_WriteFile(&stream->Inode, stream->DataPointer, (BYTE*)ptr, count*size);
        if(Cnt > 0){
            stream->DataPointer += Cnt;
            if(stream->DataPointer > stream->Inode.dwSize){
                stream->Inode.dwSize = stream->DataPointer;
            }
            return Cnt;
        }
        return Cnt;
    }
    return -1;
}

// ***************************************************************************
// Read block of data from stream
int uFS_fread ( BYTE * ptr, DWORD size, DWORD count, uFS_FILE * stream )
{
    if(  (stream->Flags.Enums.Mode == uFS_MODE_READ) || 
         (stream->Flags.Enums.Mode == uFS_MODE_WRITE_PLUS) || 
         (stream->Flags.Enums.Mode == uFS_MODE_APPEND_PLUS))
    {


        int Cnt = uFS_ReadFile(stream->Inode, stream->DataPointer, (BYTE*)ptr, count*size);
        if(Cnt != 0){
            stream->DataPointer += Cnt;
            return Cnt;
        }
    }
    return -1;
}

// ***************************************************************************
// 
int uFS_fseek ( uFS_FILE * stream, int offset, int origin )
{
    if(stream->Flags.Enums.Mode == uFS_MODE_NONE) 
        return -1;

    switch (origin) {
    case uFS_SEEK_SET:
        if(offset <0)
            return -1;
        if(offset < (int)stream->Inode.dwSize){
            stream->DataPointer = offset;            
        } else {
            if(stream->Flags.Enums.Mode == uFS_MODE_READ) {
                stream->DataPointer = stream->Inode.dwSize;
                return -1;
            } else {
                if(uFS_ExpandFile(&stream->Inode, offset) != 0)
                    return -1;
                stream->DataPointer = offset;
            }

        }
    	break;
    case uFS_SEEK_CUR: 
        {
            int res = stream->DataPointer + offset;
            if(res <= 0){
                stream->DataPointer = 0;                
            } else {
                if(res > (int)stream->Inode.dwSize){
                    if(stream->Flags.Enums.Mode > uFS_MODE_READ) {
                        if(uFS_ExpandFile(&stream->Inode, res) != 0)
                            return -1;
                        stream->DataPointer = stream->Inode.dwSize;
                    } else {
                        stream->DataPointer = stream->Inode.dwSize;
                        return -1;
                    }
                } else {
                    stream->DataPointer = res;
                }
            }
            break;
        }
    case uFS_SEEK_END:
        if (offset < 0) {
            int res = stream->DataPointer + offset;
            if(res <= 0){
                stream->DataPointer = 0;
            } else {
                stream->DataPointer = res;
            }
        } else {
            if(stream->Flags.Enums.Mode > uFS_MODE_READ ) {
                if(uFS_ExpandFile(&stream->Inode, stream->Inode.dwSize + offset) != 0)
                    return -1;
                stream->DataPointer = stream->Inode.dwSize;
            } else {
                stream->DataPointer = stream->Inode.dwSize;
                return -1;
            } 
        }
        break;
    default:
        return -1;
    }
    return 0;
}
//*********************************************************************************
int CreateFS()
{
    uFS_FILE fs_root;
    uFS_FILE fs_inode;
    uFS_FILE fs_free;
    uFS_FILE fs_dir;
    SUPERBLOCK SuperBlock;
    //BYTE Buf[256];


    strcpy(SuperBlock.Name, "uFS Version 0.1");
    SuperBlock.VersionMajor = 0;      //0
    SuperBlock.VersionMinor = 1;      //1
    SuperBlock.SectorSize   = 256*1024;       // 256*1024
    SuperBlock.SectorsCount = 64;     //64
    SuperBlock.PageSize     = 256;         // 256
    SuperBlock.PageCount    = 65536;        //65536
    SuperBlock.InodeTable   = 0;       // 256


    fs_root.Inode.dwSize = 0;
    fs_root.Inode.wInodeID = 1;
    fs_root.Inode.wFlags = 0;
    fs_root.Flags.Enums.Mode = uFS_MODE_WRITE_PLUS;
    fs_root.Flags.Enums.Binary = uFS_BINARY;
    fs_root.Flags.Enums.Dir = uFS_RECORD_IS_FILE;
    fs_root.DataPointer = 0;
    fs_root.Inode.wSectorsCount = 0;
    memset(fs_root.Inode.pwTable, INVALID_SECTOR_NUMBER, sizeof(fs_root.Inode.pwTable));

    fs_inode.Inode.dwSize = 0;
    fs_inode.Inode.wInodeID = 2;
    fs_inode.Inode.wFlags = 0;
    fs_inode.Flags.Enums.Mode = uFS_MODE_WRITE_PLUS; 
    fs_inode.Flags.Enums.Binary = uFS_BINARY;
    fs_inode.Flags.Enums.Dir = uFS_RECORD_IS_FILE;
    fs_inode.DataPointer = 0;
    fs_inode.Inode.wSectorsCount = 0;
    memset(fs_inode.Inode.pwTable, INVALID_SECTOR_NUMBER, sizeof(fs_inode.Inode.pwTable));

    fs_free.Inode.dwSize = 0;
    fs_free.Inode.wInodeID = 3;
    fs_free.Inode.wFlags = 0;
    fs_free.Flags.Enums.Mode = uFS_MODE_WRITE_PLUS; 
    fs_free.Flags.Enums.Binary = uFS_BINARY;
    fs_free.Flags.Enums.Dir = uFS_RECORD_IS_FILE;
    fs_free.DataPointer = 0;
    fs_free.Inode.wSectorsCount = 0;
    memset(fs_free.Inode.pwTable, INVALID_SECTOR_NUMBER, sizeof(fs_free.Inode.pwTable));

    fs_dir.Inode.dwSize = 0;
    fs_dir.Inode.wInodeID = 4;
    fs_dir.Inode.wFlags = 0;
    fs_dir.Flags.Enums.Mode = uFS_MODE_WRITE_PLUS; 
    fs_dir.Flags.Enums.Binary = uFS_BINARY;
    fs_dir.Flags.Enums.Dir = uFS_RECORD_IS_DIR;
    fs_dir.DataPointer = 0;
    fs_dir.Inode.wSectorsCount = 0;
    memset(fs_dir.Inode.pwTable, INVALID_SECTOR_NUMBER, sizeof(fs_dir.Inode.pwTable));

    memset(uFS_FreeSectors.FreeSectorsCache, 0xFF, sizeof(uFS_FreeSectors.FreeSectorsCache));
    uFS_FreeSectors.FreeFile = &fs_free;
    FS[256] = 0xFD;         // ����� ������ ������ � ������ �����
    for(WORD i = 1; i < 64; i++){
        FS[1024*256*i] = 0xFE;    // ����� ������ ������ �� ������ �����
    }

    fs_free.Inode.pwTable[0] = 1;
    fs_free.Inode.pwTable[1] = 1024;
    fs_free.Inode.pwTable[2] = 1024 * 2;
    fs_free.Inode.pwTable[3] = 1024 * 3;
    fs_free.DataPointer = 0;
    fs_free.Inode.dwSize = 256*4;

    
    //fs_root.Inode.pwTable[0] = 0;
    //fs_root.Inode.dwSize = sizeof(SuperBlock);
    //fs_root.DataPointer = 0;
    
    // ������ ���� ���������� �����
    //uFS_AddSectorFromRange( &fs_free.Inode, NULL, 1, CLEAN_SECTOR_SIZE >> DATA_SECTOR_SIZE_BIT);

    //for(WORD i = 1; i < CLEAN_SECTOR_COUNT; i++){
    //    uFS_AddSectorFromRange( &fs_free.Inode, NULL, (i * CLEAN_SECTOR_SIZE) >> DATA_SECTOR_SIZE_BIT, ((i + 1) * CLEAN_SECTOR_SIZE) >> DATA_SECTOR_SIZE_BIT);
    //}

    // ������ ���� ������� �������    
    uFS_AddSectorFromRange( &fs_root.Inode, NULL, 0, 1);


    // ������ ���� ������
    uFS_fseek( &fs_inode, 32*1024, uFS_SEEK_SET); 
    uFS_fseek( &fs_inode, 0, uFS_SEEK_SET); 

    //���������� ���������
    SuperBlock.InodeTable = fs_inode.Inode.pwTable[0] << DATA_SECTOR_SIZE_BIT;
    uFS_fwrite ( (BYTE*)&SuperBlock, sizeof(SuperBlock), 1, &fs_root );
        
    // ������ ���� �������� ����������
    {
        DIR_RECORD Dir;
        Dir.wInodeID = 2;
        strcpy(Dir.pcFileName,".Inode");
        Dir.bNameLength = strlen(Dir.pcFileName) + 1;

        uFS_fwrite ( (BYTE*)&Dir, sizeof(WORD)+sizeof(BYTE), 1, &fs_dir );
        uFS_fwrite ( (BYTE*)Dir.pcFileName, Dir.bNameLength, 1, &fs_dir );

        Dir.wInodeID = 3;
        strcpy(Dir.pcFileName,".free");
        Dir.bNameLength = strlen(Dir.pcFileName) + 1;

        uFS_fwrite ( (BYTE*)&Dir, sizeof(WORD)+sizeof(BYTE), 1, &fs_dir );
        uFS_fwrite ( (BYTE*)Dir.pcFileName, Dir.bNameLength, 1, &fs_dir );

        //Dir.wInodeID = 4;
        //strcpy(Dir.pcFileName,"/");
        //Dir.bNameLength = strlen(Dir.pcFileName) + 1;

        //uFS_fwrite ( (BYTE*)&Dir, sizeof(WORD)+sizeof(BYTE), 1, &fs_dir );
        //uFS_fwrite ( (BYTE*)Dir.pcFileName, Dir.bNameLength, 1, &fs_dir );
    }

    // ������ � ���������� ���� ���������� �����
    //uFS_fwrite (  (BYTE*)uFS_FreeSectors.FreeSectorsCache, sizeof(uFS_FreeSectors.FreeSectorsCache), 1, &fs_free );    

    // ����� Inode ������� ������    
    uFS_fwrite (  (BYTE*)&fs_inode.Inode, 1, sizeof(fs_inode.Inode), &fs_inode );
    
    // ����� Inode ����� ���������� �����    
    uFS_fwrite (  (BYTE*)&fs_free.Inode, 1, sizeof(fs_free.Inode), &fs_inode );

    // ����� Inode ����� �������� ����������
    uFS_fwrite (  (BYTE*)&fs_dir.Inode, 1, sizeof(fs_dir.Inode), &fs_inode );
    

    return 0;
}

// ***************************************************************************
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;
    WORD Data[65536];
    WORD Data1[65536];
    int Count = 0;
    // ���������������� MFC, � ����� ������ � ��������� �� ������� ��� ����
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        // TODO. �������� ��� ������ �������������� ����� ������������
        _tprintf(_T("����������� ������: ���� ��� ������������� MFC\n"));
        nRetCode = 1;
    }
    else
    {


        uFS_FILE file;

        //INODE_RECORD Inode;
        file.Inode.dwSize = 0;
        file.Inode.wInodeID = 0;
        file.Inode.wFlags = 0;
        file.Flags.Enums.Mode = uFS_MODE_WRITE_PLUS; 
        file.DataPointer = 0;

        
        memset(FS,0xFF,sizeof(FS));
        memset(Data1,0x00,sizeof(Data1));
        memset((void*)FreeSectors, 0xFF, sizeof(FreeSectors));
        CreateFS();

        if(uFS_Init())
            return 1;



        

        /*Count = SECTORS_COUNT - uFS_GetFreeSectorCount();
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );
        file.Inode.wInodeID++;
        uFS_fwrite (  (BYTE*)&file.Inode, 1, sizeof(file.Inode), &file );*/

        //uFSReadFile(Inode, 0, (BYTE*)Data1, sizeof(Data1));
        Count = SECTORS_COUNT - uFS_GetFreeSectorCount();

       
//         while(Count < SECTORS_COUNT){
//             if(uFSWriteFile(&Inode, 0, (BYTE*)Data, sizeof(Data)) < 0)
//                 break;
//             if(uFSReadFile(Inode, 0, (BYTE*)Data1, sizeof(Data1)) < 0)
//                 break;
//             if(memcmp(Data, Data1, sizeof(Data))){
//                 break;
//             };
//             Count = SECTORS_COUNT - uFS_GetFreeSectorCount();
//         }
//         Count = SECTORS_COUNT - uFS_GetFreeSectorCount();
    }
return nRetCode;
}
