#ifndef _CONTROL_H
#define _CONTROL_H

#ifdef _WINDOWS
#   include "..\..\guidance\stdafx.h"
#else
#   include "GenericTypeDefs.h"
#endif
// ����� ��� NeedToUpdate � NeedToCommit


typedef union {
    BYTE Val;
    struct __PACKED
    {        
        BYTE Angle:1;
        BYTE AbsStep:1;
        BYTE TargetAngle:1;
        BYTE TargetAbsStep:1;
        BYTE Flag:1;
        BYTE b5:1;
        BYTE b6:1;
        BYTE b7:1;
    } bits;
} AXIS_FLAG_STRUCT; 
typedef union {
    BYTE Val;
    struct __PACKED
    {        
        BYTE Enable:1;
        BYTE Run:1;
        BYTE b2:1;
        BYTE b3:1;
        BYTE b4:1;
        BYTE b5:1;
        BYTE b6:1;
        BYTE b7:1;
    } bits;
} AXIS_STATUS_FLAG_STRUCT; 
typedef struct AXIS_PARAM
{
    AXIS_FLAG_STRUCT  NeedToUpdate;     // ���������� ��������� � ������� ��������� ���������
    AXIS_FLAG_STRUCT  NeedToCommit;     // ���������� ��������� �� ������ ��������� ���������
    AXIS_FLAG_STRUCT  IsModified;

    float Angle;           // ������� ���������� � �������� (���� ������������ ��������� �������������) �� ���� ��������� ������    
    DWORD  AbsSteps;         // ������� ����� ����    
    float TargetAngle;           // ������� ���������� � �������� (���� ������������ ��������� �������������) �� ���� ��������� ������    
    DWORD  TargetAbsSteps;         // ������� ����� ����    
    AXIS_STATUS_FLAG_STRUCT  StatusFlag;       // ���� ��������� ���
} AXIS_PARAM;
// ����� ��� NeedToUpdate � NeedToCommit

typedef union {
    BYTE Val;
    struct __PACKED
    {        
        BYTE IP:1;
        BYTE Mask:1;
        BYTE Gate:1;
        BYTE DNS1:1;
        BYTE DNS2:1;
        BYTE Name:1;
        BYTE NTP:1;
        BYTE Flag:1;
    } bits;
} NET_FLAG_STRUCT;
typedef struct NETWORK_SETTINGS
{
    NET_FLAG_STRUCT  NeedToUpdate;     // ���������� ��������� � ������� ��������� ���������
    NET_FLAG_STRUCT  NeedToCommit;     // ���������� ��������� �� ������ ��������� ���������
    NET_FLAG_STRUCT  IsModified;
    DWORD IP;
    DWORD Mask;
    DWORD Gate;
    DWORD DNS1;
    DWORD DNS2;    
    char  Name[16];    
    DWORD NTP;
    BYTE ConnectFlag;
}NETWORK_SETTINGS;
// ����� ��� NeedToUpdate � NeedToCommit

typedef union SYSTEM_FLAG_STRUCT {
    BYTE Val;
    struct __PACKED
    {        
        BYTE Alpha:1;
        BYTE Delta:1;
        BYTE Gamma:1;
        BYTE Local:1;
        BYTE Remote:1;
        BYTE Common:1;
        BYTE b6:1;
        BYTE b7:1;
    } bits;
} SYSTEM_FLAG_STRUCT;

typedef union COMMON_FLAG_STRUCT {
    BYTE Val;
    struct __PACKED
    {        
        BYTE Flags:1;
        BYTE Speed:1;
        BYTE b2:1;
        BYTE b3:1;
        BYTE b4:1;
        BYTE b5:1;
        BYTE b6:1;
        BYTE b7:1;
    } bits;
} COMMON_FLAG_STRUCT ;

typedef union COMMON_SETTINGS_FLAG_STRUCT{
    BYTE Val;
    struct __PACKED
    {        
        BYTE Man_Auto:1;        // ������ = 1/ �������������� = 0 ���������
        BYTE b1:1;
        BYTE b2:1;
        BYTE b3:1;
        BYTE b4:1;
        BYTE b5:1;
        BYTE b6:1;
        BYTE b7:1;
    } bits;
} COMMON_SETTINGS_FLAG_STRUCT;
typedef struct COMMON_SETTINGS
{
    COMMON_FLAG_STRUCT  NeedToUpdate;     // ���������� ��������� � ������� ��������� ���������
    COMMON_FLAG_STRUCT  NeedToCommit;     // ���������� ��������� �� ������ ��������� ���������
    COMMON_FLAG_STRUCT  IsModified;

    COMMON_SETTINGS_FLAG_STRUCT Flags;
    double MaxGuidanceSpeed;              // ������������ �������� ��������� � ������/�������������� �������
    
} COMMON_SETTINGS;
typedef struct All_PARAMS
{
    // ������� ����� -����� 
    SYSTEM_FLAG_STRUCT NeedToUpdate;   // ���������� ��������� � ������� ��������� ���������
    SYSTEM_FLAG_STRUCT NeedToCommit;   // ���������� ��������� �� ������ ��������� ���������    
    SYSTEM_FLAG_STRUCT IsModified;

    AXIS_PARAM Alpha;
    AXIS_PARAM Delta;
    AXIS_PARAM Gamma;

    NETWORK_SETTINGS Local;
    NETWORK_SETTINGS Remote;

    COMMON_SETTINGS Common;

} ALL_PARAMS;

#define NO_SELECT 0x00
#define SELECT_LINE 0x01
#define SELECT_COLUMN 0x02
#define FONT_TYPE_B     0x04

typedef union KEYS_STR{
    BYTE Val;
    struct __PACKED
        {        
            BYTE up:1;
            BYTE down:1;
            BYTE left:1;
            BYTE right:1;
            BYTE esc:1;
            BYTE enter:1;
            BYTE b6:1;
            BYTE b7:1;
    } keys;
} KEYS_STR; 

#ifndef DATE_TIME_STRUCT
#define DATE_TIME_STRUCT
typedef struct DateTimeStruct {
    WORD Year;
    BYTE Month;
    BYTE Day;
    BYTE DayOfWeak;
    BYTE Hour;
    BYTE Min;
    BYTE Sec;
    double uSec;
} DateTime;
#endif
    
void ProcessMenu( KEYS_STR * KeyPressed);
void ExecuteCommands( void );
void SecondsToTime(DWORD Seconds, DateTime * Date);
#endif
