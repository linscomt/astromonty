unit RatFracForm;
interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons, ExtCtrls, ComCtrls, Menus,
  ToolWin, RatFracClass, AboutForm, LogForm;
type
  TForm1 = class(TForm)
    MainMenu1:      TMainMenu;
    N1:             TMenuItem;
    N2:             TMenuItem;
    N3:             TMenuItem;
    N4:             TMenuItem;
    N5:             TMenuItem;
    N6:             TMenuItem;

    StatusBar1:     TStatusBar;
    BitBtn1:        TBitBtn;

    F1:             TGroupBox;
    F2:             TGroupBox;
    F3:             TGroupBox;
    F4:             TGroupBox;
    F5:             TGroupBox;
    Operand1:       TGroupBox;
    Operand2:       TGroupBox;
    ResultGroup:    TGroupBox;

    ResDivider:     TBevel;
    Bevel1:         TBevel;
    Bevel26:        TBevel;

    EnterNum:       TEdit;
    EnterDenom:     TEdit;
    EnterNum2:      TEdit;
    EnterDenom2:    TEdit;

    CmpRes1:        TLabel;
    Label1:         TLabel;
    ResNum:         TLabel;
    ResDenom:       TLabel;
    OperatorSymbol: TLabel;

    Bevel2:         TBevel;
    Bevel3:         TBevel;
    Bevel4:         TBevel;
    Bevel5:         TBevel;
    Bevel6:         TBevel;
    Bevel7:         TBevel;
    Bevel8:         TBevel;

    Label2:         TLabel;
    Label3:         TLabel;
    Label4:         TLabel;
    Label6:         TLabel;
    Label7:         TLabel;
    Label5:         TLabel;
    Label8:         TLabel;
    Label9:         TLabel;
    Label10:        TLabel;
    Label11:        TLabel;
    Label12:        TLabel;
    Label13:        TLabel;
    Label14:        TLabel;
    Label15:        TLabel;
    Label16:        TLabel;
    Label17:        TLabel;
    Label18:        TLabel;
    Label19:        TLabel;
    Label20:        TLabel;
    Label21:        TLabel;
    Operators1:     TRadioGroup;
    Functions1:     TRadioGroup;
    ModeSelect:     TRadioGroup;

    procedure FormCreate(Sender: TObject);
    procedure Calculate(Sender: TObject);
    procedure OperatorSelectClick(Sender: TObject);
    procedure ModeSelectClick(Sender: TObject);
    procedure N2Click(Sender: TObject);
    procedure N3Click(Sender: TObject);
    function CheckStringOper(InStr : string; var LogStr:string) : boolean;
    function IntFromString(Line: string; var PosFrom: integer): integer;
    function SetFromString(Line: string; var PosFrom: integer; var Res1 : TRationalFraction): boolean;
    procedure N5Click(Sender: TObject);
    procedure FormShow(Sender: TObject);
  private
    Operation : short;     // ����� ����������
    { Private declarations }
  public
    { Public declarations }
  end;
var
  Form1: TForm1;

  Number : integer;
implementation

{$R *.dfm}


// ������� ��������� ���������� ���� �������� �
// ����������� �� ���������� Operation
// ������� ������ �������������� �� ��������� �����
procedure TForm1.Calculate(Sender: TObject);
var
    a, b : integer;
    CompResult : boolean;
    X, X1, Res, N, Res1, Res2 : TRationalFraction;
begin
    CompResult := false;
    // ������ ������� ������� 
    Res := TRationalFraction.Create;
    X := TRationalFraction.Create;
    X1 := TRationalFraction.Create;
    // ������ ������ � ���������� ������
    try
        a := StrToInt(EnterNum.Text);
        b := StrToInt(EnterDenom.Text);
        X.SetValue(a, b);
        if(Operand2.Visible) then begin
            a := StrToInt(EnterNum2.Text);
            b := StrToInt(EnterDenom2.Text);
            X1.SetValue(a, b);
        end else X1.SetValue(1,1);
    except
        on Exception : EConvertError do
        begin // �������� �� ������ � ������ �������
            StatusBar1.Panels[0].Text := '������:';
            StatusBar1.Panels[1].Text := '��� ����� �����';
            StatusBar1.Panels[2].Text := Exception.Message;
            exit;
        end
        //ShowMessage(Exception.Message);
    end;
    if(X.ErrorFlag) then
    begin // �������� �� ������ � ������ �������
        StatusBar1.Panels[0].Text := '������';
        StatusBar1.Panels[1].Text := '� ����� X';
        StatusBar1.Panels[2].Text := X.ErrorStr;
        ResNum.Caption := '������';
        ResDenom.Caption := '������';
        CmpRes1.Caption := '������';
        exit;
    end;
    if(X1.ErrorFlag) then
    begin // �������� �� ������ � ������ �������
        StatusBar1.Panels[0].Text := '������';
        StatusBar1.Panels[1].Text := '� ����� X1';
        StatusBar1.Panels[2].Text := X1.ErrorStr;
        ResNum.Caption := '������';
        ResDenom.Caption := '������';
        CmpRes1.Caption := '������';
        exit;
    end;
    case Operation of
        0 : begin // +
            Res.SetValue(X);
            Res.Add(X1);
        end;
        1 : begin // -
            Res.SetValue(X);
            Res.Sub(X1);
        end;
        2 : begin // *
            Res.SetValue(X);
            Res.Multiply(X1);
        end;
        3 : begin // /
            Res.SetValue(X);
            Res.Divide(X1);
        end;
        4 : begin // =
            CompResult := X.Eq(X1);
        end;
        5 : begin // >
            CompResult := X.Gt(X1);
        end;
        6 : begin // <
            CompResult := X.Lt(X1);
        end;
        7 : begin // <>
            CompResult := X.Ne(X1);
        end;
        8 : begin // >=
            CompResult := X.Ge(X1);
        end;
        9 : begin // <=
            CompResult := X.Le(X1);
        end;
        10 : begin  // F(x) = 2x+1/x
            Res.SetValue(X);
            N := TRationalFraction.Create(2,1);
            Res.Multiply(N);
            Res1 := TRationalFraction.Create(1,1);
            Res1.Divide(X);
            Res.Add(Res1);
            N.Free;
            Res1.Free;
        end;
        11 : begin   // F(x) = (x-1)/(x+2)
            Res1 := TRationalFraction.Create(X);
            N := TRationalFraction.Create(2,1);
            Res1.Add(N);
            Res.SetValue(X);
            N.SetValue(1,1);
            Res.Sub(N);
            Res.Divide(Res1);
            N.Free;
            Res1.Free;
        end;
        12 : begin    // F(x) = x*x - 1/x
            Res1 := TRationalFraction.Create(1,1);
            Res.SetValue(X);
            Res.Multiply(X);
            Res1.Divide(X);
            Res.Sub(Res1);
            Res1.Free;
        end;
        13 : begin   // F(x) = x/3 - 1/(1+x)
            Res1 := TRationalFraction.Create(X);
            Res2 := TRationalFraction.Create(1,1);
            Res.SetValue(X);
            N := TRationalFraction.Create(3,1);
            Res.Divide(N);
            N.SetValue(1,1);
            Res1.Add(N);
            Res2.Divide(Res1);
            Res.Sub(Res2);
            Res1.Free;
            Res2.Free;
            N.Free;
        end;      //F(x) = 3/x + 5 * x/11
        14 : begin
            Res1 := TRationalFraction.Create(X);
            Res.SetValue(3,1);
            Res.Divide(X);
            N := TRationalFraction.Create(11,1);
            Res1.Divide(N);
            N.SetValue(5,1);
            Res1.Multiply(N);
            Res.Add(Res1);
            Res1.Free;
            N.Free;
        end;
    end;

    if(Operation <= 3) or (Operation >= 10 ) then // ��������� ����������
    begin
        ResNum.Caption := Res.StrNumerator;
        ResDenom.Caption := Res.StrDenominator;
        if(Res.ErrorFlag) then
        begin // �������� �� ������ � ������ �������
            StatusBar1.Panels[0].Text := '������';
            StatusBar1.Panels[1].Text := '��������� ��������: ';
            StatusBar1.Panels[2].Text := Res.ErrorStr;
            exit;
        end else
        begin  // �������� �� ������ � ������ �������
            StatusBar1.Panels[0].Text := '�������';
            StatusBar1.Panels[1].Text := '��������� ��������: ';
            StatusBar1.Panels[2].Text := Res.ErrorStr;
        end;
    end else
    if(Operation < 10) then
    begin
        if(CompResult) then
            CmpRes1.Caption := '������'
        else
            CmpRes1.Caption := '����';
    end;
    // ����������� �������
    X.Free;
    X1.Free;
    Res.Free;
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
    Operation := 0; // +
end;

// �������-���������� ��������� � ������ �������� �� �����
procedure TForm1.OperatorSelectClick(Sender: TObject);
var
    isCompare : bool;
begin
    isCompare := false;
    if (ModeSelect.ItemIndex = 0) then
    begin
        OperatorSymbol.Caption := Operators1.Items.Strings[Operators1.ItemIndex];
        Operation := Operators1.ItemIndex;
        if(Operation >= 4) then
        begin
			CmpRes1.Visible := true;
			ResDenom.Visible := false;
			ResNum.Visible := false;
			ResDivider.Visible := false;
		end else
		begin
			CmpRes1.Visible := false;
			ResDenom.Visible := true;
			ResNum.Visible := true;
			ResDivider.Visible := true;
		end;
    end else
    begin
        Operation := Functions1.ItemIndex + 10;
        F1.Visible := false;
        F2.Visible := false;
        F3.Visible := false;
        F4.Visible := false;
        F5.Visible := false;
        case Functions1.ItemIndex of
            0: F1.Visible := true;
            1: F2.Visible := true;
            2: F3.Visible := true;
            3: F4.Visible := true;
            4: F5.Visible := true;
        end;
	end;
    Calculate(Sender);
 end;
// ������� - ���������� ������ ������ ������(������� ��������/�������)
procedure TForm1.ModeSelectClick(Sender: TObject);
begin
    if ModeSelect.ItemIndex = 0 then
    begin
        Operand2.Visible := true;
        Operators1.Visible := true;
        Functions1.Visible := false;
        OperatorSymbol.Visible := true;
        Operation := 0;             // ���������� �� �������� '+'
        Operators1.ItemIndex := 0;
        F1.Visible := false;
        F2.Visible := false;
        F3.Visible := false;
        F4.Visible := false;
        F5.Visible := false;
    end else
    begin
        Operand2.Visible := false;
        Operators1.Visible := false;
        Functions1.Visible := true;
        OperatorSymbol.Visible := false;
        Operation := 10;
        Functions1.ItemIndex := 0;
        ResNum.Visible := true;
        Resdenom.Visible := true;
        ResDivider.Visible := true;
        CmpRes1.Visible := false;
    end;
    OperatorSelectClick(Sender);
end;

// ������� �������� �������������� ������� ��� ����������
// (������������� �������� ���������� Operation)
// � ����������� ��������� � ��������� ����� � ����������� �� ��������� �������
procedure TForm1.N2Click(Sender: TObject);
begin
    Form2.ShowModal;
end;
// ������� ������ �� ���������� ����� ������, ������������ �� � ������� �� ����� Form3
procedure TForm1.N3Click(Sender: TObject);
var
    s, log : string;
    f: Textfile;
begin
    if(Form3.OpenDialog1.Execute) then
    begin
        Form3.ClearLog;
        Form3.Show;
        AssignFile(f, Form3.OpenDialog1.FileName ); {Assigns the Filename}
        Reset(f); {Opens the file for reading}
        Form3.AddLogString('������ ���� : ' + Form3.OpenDialog1.FileName);
        Form3.AddLogString('��������� ��������� ������: ');
        repeat
            Readln(f, s);
            CheckStringOper(s, log);
            if(log <> '') then
                Form3.AddLogString(log);
        until(Eof(f));
        CloseFile(f);
    end;
end;

// ������� ���������� ��������� �������� ����������� �� ������ s
// ������� ����� '#' ������������
// ������ ������:
// 1. �������� �������������� ��������:
//      X1/Y1 S X2/Y2 = X3/Y3 ��� 'e' (��� ������),
// ��� S ����� ���� '+', '-', '*', '/'
// 2. �������� �������� ���������:
//      X1/Y1 S X2/Y2 = R , ��� R ����� ���� 't', 'f',
// ��� S : '>', '<', '=', '<>', '>=', '<=' 
// X1,Y1, X2, Y2 ��������
// X3, Y3, R  ��������� ��������� ���������
function TForm1.CheckStringOper(InStr : string; var LogStr :string) : boolean;
var
    T1, T2, T3, Res1 : TRationalFraction;
    i, o, k : integer;
    b, eq, br : boolean;
    er : boolean; // �������� ������
    Sym, Comment : string;
begin
    // ������� �������
    T1 := TRationalFraction.Create;
    T2 := TRationalFraction.Create;
    T3 := TRationalFraction.Create;
    Res1 := TRationalFraction.Create;

    // �������������� ����������
    b:=false;
    br := false;
    er := false;
    eq := false;
    o := 0;
    k := 1;
    i := 0;
    // ��������� ���������� ������
    while (i <= Length(InStr)) do
    begin
        i := i + 1;
        if(InStr[i] = ' ') then continue else
        if(InStr[i] = '#') then
        begin  // ������ ���� ����������
            Comment := Copy(InStr, i, Length(InStr) - i + 1);
            break;
        end;
        case k of
            1: begin    // ������ ��������
                SetFromString(InStr, i, T1);
                k := 2;
            end;
            2: begin    // ������ ��������
                case InStr[i] of
                    '+': o := 1;
                    '-': o := 2;
                    '*': o := 3;
                    '/': o := 4;
                    '<': begin
                        if(InStr[i+1]=' ') then o := 5 else     // <
                        begin
                            if(InStr[i+1]='=') then o := 9 else // <=
                            if(InStr[i+1]='>') then o := 7;// <>
                            i:= i + 1;
                        end;
                        eq := true;
                    end;
                    '>': begin
                        if(InStr[i+1]=' ') then o := 6 else     // >
                        begin
                            if(InStr[i+1]='=') then o := 10; // >=
                            i:= i + 1;
                        end;
                        eq := true;
                    end;
                    '=': begin o := 8; eq:= true end;
                    else break; // ����������� ������
                end;
                k := 3;
            end;
            3: begin    // ������ ��������
                SetFromString(InStr, i, T2);
                k := 4;
            end;
            4: begin    // ������ ���������
                if InStr[i] <> '=' then break; // ���� �� ����� - ������
                if(not eq) then k := 5 else k := 6;
            end;
            5: begin    // ��������� ��������� - �����
                if InStr[i] = 'e' then begin er := true; end else // ��������� ��������� - ������
                    SetFromString(InStr, i , T3);
                k:=7;  // ���� ������ ��������
            end;
            6: begin    // ��������� ��������� ������
                if InStr[i] = 't' then br := true else
                if InStr[i] = 'f' then br := false;
                k:=7;  // ���� ������ ��������
            end;
        end;
    end;
    // ��������� ���������� ���������
    Res1.SetValue(T1);
    case o of
        1:  begin Res1.Add(T2);     Sym:='+'; end;
        2:  begin Res1.Sub(T2);     Sym:='-'; end;
        3:  begin Res1.Multiply(T2);Sym:='*'; end;
        4:  begin Res1.Divide(T2);  Sym:='/'; end;
        5:  begin b:=T1.Lt(T2);     Sym:='<'; end;
        6:  begin b:=T1.Gt(T2);     Sym:='>'; end;
        7:  begin b:=T1.Ne(T2);     Sym:='<>'; end;
        8:  begin b:=T1.Eq(T2);     Sym:='='; end;
        9:  begin b:=T1.Le(T2);     Sym:='<='; end;
        10: begin b:=T1.Ge(T2);     Sym:='>='; end;
    else
        Result := true;
        exit;
    end;
    if(not Result) then
    Begin
		// ��������� ������ � �������
		LogStr := T1.StrNumerator + '/' + T1.StrDenominator + ' ' + Sym + ' '
				+ T2.StrNumerator + '/' + T2.StrDenominator  + ' = ';
		if( not eq) then  // �������� + - * /
		begin
			b := Res1.Eq(T3);
			LogStr := LogStr + #$9 + Res1.StrNumerator + '/' + Res1.StrDenominator +
				#$9 + '(������� : ';
			if(er) then
			begin
				LogStr := LogStr + '������ ) ';
				if(Res1.ErrorFlag) then
					LogStr:=LogStr + #$9 + '- (�����)' + #$9 + Comment
				else
					LogStr:=LogStr + #$9 + '- (������)' + #$9 + Comment;

			end else
			begin
				LogStr := LogStr + T3.StrNumerator + '/' + T3.StrDenominator + ')';
				if(b) then
					LogStr:=LogStr + #$9 + #$9 + '- (�����)' + #$9 + Comment
				else
					LogStr:=LogStr + #$9 + #$9 + '- (������)' + #$9 + Comment;
			end;
			Result := b;
		end else
		begin   // �������� ���������
			if(b) then
				LogStr := LogStr + #$9 + '������ '
			else
				LogStr := LogStr + #$9 + '���� ';
			LogStr := LogStr + #$9 + '(������� : ';
			if(br) then
			begin
				LogStr := LogStr + '������)';
			end else
			begin
				LogStr := LogStr + '����)  ';
			end;
			if(b = br) then
				LogStr:=LogStr + #$9 + '- (�����)' + #$9 + Comment
			else
				LogStr:=LogStr + #$9 + '- (������)' + #$9 + Comment;
			Result := (b = br);
		end;
    End else LogStr:= '������ ������� ������.';
    // ����������� �������
    T1.Free;
    T2.Free;
    T3.Free;
    Res1.Free;
end;
//******************************************************************************
// ������� ��������� �� ������ Line ������������������ ���� ������������ ����� �
// ������ �������� Res1
// ���������� ���������� � ������� PosFrom
// � ������ ������ �������������� ������ ���� TRationalFraction
// ��������������� � ���������� ������ 
//******************************************************************************
function TForm1.SetFromString(Line: string; var PosFrom: integer; var Res1 : TRationalFraction): boolean;
var
    l, m : integer;
    c : char;
begin
    Result := false;
    l := 0;
    m := 0;
    //������� ���������� ������ ��������
    try
        l := IntFromString(Line,PosFrom);
    except
        on Exception : EConvertError do
            Result :=  Result or true;
    end;
    // ������ ������ "/"
    c := Line[PosFrom];
    PosFrom:= PosFrom + 1;
    //������� ���������� ������ ��������
    try
        m := IntFromString(Line,PosFrom);
    except
        on Exception : EConvertError do
            Result := Result or true;
    end;
    if(Result) then m := 0; // ���� ������, �� ���������� ������ ������� � ��������� ���������
    if(c = '/') then
    begin
        Res1.SetValue(l,m);
        Result := Result or false;
    end else
    begin
        Result := Result or true;
    end;
end;
//******************************************************************************
// ������� ��������� �� ������ �����, ������� � ������� PosFrom � ����������
// ������ ������, ������ �������� ��� �������� '/' ����� ������� PosFrom
// � ������ ������, ���������� ���������� EConvertError
//******************************************************************************
function TForm1.IntFromString(Line: string; var PosFrom: integer): integer;
var
    i, k, l : integer;
    b : boolean;
    r : string;
Begin
    b := false;
    k := 0;
    l := 0;
    for i := PosFrom to Length(Line) do
    begin
        if((Line[i] <> ' ') and (Line[i] <> '/')) then
        begin
            if(b=false) then
            begin
                l := i;
                b:=true; // ���������� �����
            end;
            k := k + 1;
        end else if(b) then break;
    end;
    if(b) then
    begin // ����� �����
        r := Copy(Line, l, k);
        PosFrom := i;
        Result := StrToInt(r);
        exit;
    end;
    Result := 0; // ����� �� ������� (������)
End;
// ����� �� ���������
procedure TForm1.N5Click(Sender: TObject);
begin
    Application.Terminate;
end;

procedure TForm1.FormShow(Sender: TObject);
begin
    EnterNum.Text := '1';
    EnterDenom.Text := '1';
    EnterNum2.Text := '1';
    EnterDenom2.Text := '1';
    OperatorSelectClick(Sender);
end;

end.
