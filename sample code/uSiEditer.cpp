/*///---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SysUtils.hpp"
#include "algorithm.h"
#include "math.hpp"
#include "uCommUtils.h"
#include "uReciverSetting.h"
#include "uSiEditer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Comm"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))
//---------------------------------------------------------------------------

#define SL_DEFAULT_ID		1               //��������w�]�s��
#define SL_MIN_ID			1               //��������̤p�s��
#define SL_MAX_ID			127             //��������̤j�s��
#define SL_MAX_MODEL		255             //�i�w�q�̦h�����ݩʪ�
#define SL_ADDR_NUMS		1 				//Start in 20150427
#define SL_ADDR_START		2               //Start in 20150427
#define SL_TIMER_INTERVAL	100				//10��20�i30�ΡA30��X�Ӫ�����̷�
#define SL_CMD_NEXT			1
#define SL_READ_WAIT		1				//�ثe�̨έ�50,10�]����
#define SL_DEFAULT_BAUD		"9600"          //�w�]��
#define SL_DEFAULT_DATA		"E,8,1"         //�w�]��
//---------------------------------------------------------------------------
TBaudRate aComBaud[]={br9600,br19200};		//�w�q���Ψ쪺�ǿ�v�A�PcbBaud���r��۩I��
typedef struct { TParity Parity;TDataBits DataBits;TStopBits StopBits; } STCOMDATA;
const STCOMDATA stComData[]={{paEven,da8,sb10},{paNone,da8,sb10}};	//�w�q�ǿ��w�A�PcbData���r��۩I��
union { WORD *w; DWORD *dw; short *s; BYTE *b; byte *c;LONG *l; int *i; void *p;} PDATA; //�w�q�h���ܼƫ���
/*///========================================================================

//--���ӥΨ쩵�𪺰j��-------------------------------------------------------------------------
void __fastcall delay(int i)
{
  while(--i<=0) {}
}

TStrings *sList,*sItem; //�`�Ϊ��r���C
TfmSiEditer *fmSiEditer;
//---------------------------------------------------------------------------
__fastcall TfmSiEditer::TfmSiEditer(TComponent* Owner)
	: TForm(Owner)
{
  //�`�Ϊ��r���C
  sList=new TStringList();
  sItem=new TStringList();

  //���o�����s���ö�J�D�e���S�W���������
  pnVersion->Caption="Ver:"+GetFileVersion();
  //���w�t�ήڥؿ�
  DataPath=ExtractFilePath(Application->ExeName)+"Data";
  //�ǳƵ{������
  TIniFile *pIni=new TIniFile(ChangeFileExt(Application->ExeName,".ini"));
  if (pIni) {
    //�ϥΪ̸g�窺�����٭�
    Top=pIni->ReadInteger("Last","Top",1);
    Left=pIni->ReadInteger("Last","Left",1);
    Width=pIni->ReadInteger("Last","Width",Width);
    Height=pIni->ReadInteger("Last","Height",Height);
    edProject->Text=pIni->ReadString("Last","ProjectName","NEW");
    edFile->Text="";
    cbWireLen->ItemIndex=pIni->ReadInteger("Last","WireLen",0);
    cbWireDM->ItemIndex=pIni->ReadInteger("Last","WireDiameter",0);
    cbPort1->Text=pIni->ReadString("Last","Port1","COM1");
    cbBaud1->ItemIndex=pIni->ReadInteger("Last","Baud1",0);
    cbData1->ItemIndex=pIni->ReadInteger("Last","Data1",0);
    edTimer->Text=pIni->ReadString("Last","Timer","1");
    ed485ID->Text=pIni->ReadString("Last","485ID",IntToStr(SL_DEFAULT_ID));
    //Ū�^�r���檺�e��
    StringGrid1->ColCount=9;
    StringGrid1->RowCount=1;
    sList->CommaText=pIni->ReadString("Last","GridWidths","");
    if (sList->Count>0) {
      for(int c=0;c<StringGrid1->ColCount;c++) {
        StringGrid1->ColWidths[c]=StrToInt(sList->Strings[c]);
      }
    }
    delete pIni;
  }
  //Ū�J�ݿ露��M�檺����
  pnNodeVersion->Caption=NODE_VERSION;
  //Ū�J�ݿ露��M��θ��
  lbNode->Clear();
  for(int i=0;StrLen(stNodeData[i].Name)>0;i++) {
    lbNode->Items->Add(stNodeData[i].Name);
  }
  //��J����s�誺���W��
  for(int i=0;i<StringGrid1->ColCount;i++) {
    StringGrid1->Cells[i][0]=lbStringGrid[i];
  }
  //�إߥD��Ƨ�
  if (!DirectoryExists(DataPath)) CreateDir(DataPath);

  ReadTimes=0;
  WriteTimes=0;
  shReadLED->Brush->Color=clWhite;		//����Ū��LED�O
  shWriteLED->Brush->Color=clWhite;     //�����g�XLED�O
  Application->BringToFront();          //���{������̤W�h

  Application->ProcessMessages();

  Timer1->Enabled=false; 				//����p�ƾ�
  Timer1->Interval=SL_TIMER_INTERVAL;	//5ms�����
  Timer1->Enabled=true; 				//�Ұʭp�ƾ��C5ms�@��
  //�����p��
  cbWireStatus(this);
}



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfmSiEditer::ReadAllStatus(void)
{ //�ѭp�ɾ��w�ɰ���Ҧ�����Ū�����@�~


  char cmd[20]={0xAA,0x03,0xAF,0xC8,0x00,0x01,0x3D,0x3B};
    int bytes=8;
    //�ǳưe�X�ШD��ƪ����O
    cmd[0]=ed485ID->Text.ToInt();	//�ϥΥ��TID
    cmd[1]=0x03;					//Read Command
    if (r==0) {						// check number of devices are connected to SI-RX
      *((WORD*)&cmd[2])=SL_ADDR_NUMS; 	//�߰ݥi�Ϊ�����ƶq
      *((WORD*)&cmd[2])=W2I(*((WORD*)&cmd[2]));
    } else {
      *((int *)(&cmd[2]))=W2I(StringGrid1->Cells[SG_ADDR][r].ToInt()); //��}�s�X
    }
    cmd[4]=0;
    //20150417 �H��ƪ�Bits�ƨM�w�A�ݭn�����h�֭�WORD��ơC
    cmd[5]=(GetNode(StringGrid1->Cells[SG_MODEL][r]).Bits+15)/16;

    PrepareCRC16(cmd,bytes);                            //�p��ModBus��CRC16

    if (WriteComPort(cmd,bytes)!=bytes) {	//�e�X0x03Ū�����w��}�����O
				//error handling
      }
    } else { 											//�����e�X���O
      ReadBytes=ReadComPortA(ReadBuffer,5+cmd[5]*2,ed485ID->Text.ToInt()); //Ū�^0x03���O��RS485���
      //�ˬd�g�X�BŪ�^���
      if (ReadBytes<6 || ((BYTE)ReadBuffer[0])!=(BYTE)cmd[0] ||
		  !CheckCRC16(ReadBuffer,ReadBytes)) { 	//�ˬdCRC16�d�֨��_���T
        //�洫Ū�^��ƨ��WORD��LowByte�BHiByte
        char c;
        c=ReadBuffer[3]; ReadBuffer[3]=ReadBuffer[4]; ReadBuffer[4]=c;
        c=ReadBuffer[5]; ReadBuffer[5]=ReadBuffer[6]; ReadBuffer[6]=c;
        PDATA.b=&ReadBuffer[3]; 					//���o��ƫ���
        //====�H�U�i�����H���� PDATA Ū���۹��m��Ū�^�ƾ�=====================
        }
      }
      Application->ProcessMessages();
    }
  }
  StringGrid1->Tag=0;
}
//---------------------------------------------------------------------------
void __fastcall TfmSiEditer::Comm1RxChar(TObject *Sender, DWORD Count)
{
  //������ƿO���}�ҡA�ѭp�ɾ�����
  shReadLED->Tag=max(1,SL_LED_TIMES/Timer1->Interval);
  shReadLED->Brush->Color=clLime;
  if (Count>0 && ReadBytes<(sizeof(ReadBuffer)-10)) {
    Comm1->Read(&ReadBuffer[ReadBytes],Count);
    ReadBytes=ReadBytes+Count;
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool __fastcall TfmSiEditer::OpenComPort(void)
{
  try {
    if (Comm1->Enabled()) Comm1->Close();
    delay(1000);
    Comm1->Open();
    return Comm1->Enabled();
  } catch(...) {
    return false;
  }
}
//---------------------------------------------------------------------------
int __fastcall TfmSiEditer::WriteComPort(char *cmd,int bytes)
{
  if (!Comm1->Enabled()) {
    OpenComPort();
  }
  if (Comm1->Enabled()) {
    shWriteLED->Tag=max(1,SL_LED_TIMES/Timer1->Interval);
    shWriteLED->Brush->Color=clLime;
    //�ǰe�p�ƾ��֥[
    ReadBytes=0;
    WriteTimes++;
    Comm1->PurgeOut();
    Comm1->PurgeIn();
    if (bytes==Comm1->Write(cmd,bytes)) {
      return bytes;
    } else if (OpenComPort()) {
      Comm1->Tag=max(1,SL_CMD_NEXT/Timer1->Interval);	//�ǿ饢��
      while(Comm1->Tag>0) {}                    //����10�@��A�A�Ǥ@��
      ReadBytes=0;
      WriteTimes++;
      Comm1->PurgeOut();
      Comm1->PurgeIn();
      return Comm1->Write(cmd,bytes);
    }
  }
  return -1;
}
//---------------------------------------------------------------------------
int __fastcall TfmSiEditer::ReadComPort(char *buffer,int bytes)
{
  return ReadComPortA(buffer,bytes,ed485ID->Text.ToInt());
}
//---------------------------------------------------------------------------
int __fastcall TfmSiEditer::ReadComPortA(char *buffer,int bytes,int id)
{
  Comm1->Tag=max(3,SL_READ_WAIT/Timer1->Interval);	//Ū����ơA�������ݦܸ�Ʀ����ιO��
  while(Comm1->Tag>0) {
    if (ReadBytes>=bytes) {
      ReadTimes++;
      if (ReadBytes>=6 && CheckCRC16(buffer,bytes)) {
        if (((BYTE)buffer[0])==id) { //Ū�J���wID���^�и�ơA���ݵ���
    	  Comm1->Tag=0;
          return ReadBytes;
        }
      }
    }
    Application->ProcessMessages();
  }
  return -1; //���ԶW��
}
//---------------------------------------------------------------------------


