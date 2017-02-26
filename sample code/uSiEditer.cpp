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

#define SL_DEFAULT_ID		1               //接收元件預設編號
#define SL_MIN_ID			1               //接收元件最小編號
#define SL_MAX_ID			127             //接收元件最大編號
#define SL_MAX_MODEL		255             //可定義最多元件屬性表
#define SL_ADDR_NUMS		1 				//Start in 20150427
#define SL_ADDR_START		2               //Start in 20150427
#define SL_TIMER_INTERVAL	100				//10爛20可30佳，30算出來的延遲最準
#define SL_CMD_NEXT			1
#define SL_READ_WAIT		1				//目前最佳值50,10也不錯
#define SL_DEFAULT_BAUD		"9600"          //預設值
#define SL_DEFAULT_DATA		"E,8,1"         //預設值
//---------------------------------------------------------------------------
TBaudRate aComBaud[]={br9600,br19200};		//定義有用到的傳輸率，與cbBaud的字串相呼應
typedef struct { TParity Parity;TDataBits DataBits;TStopBits StopBits; } STCOMDATA;
const STCOMDATA stComData[]={{paEven,da8,sb10},{paNone,da8,sb10}};	//定義傳輸協定，與cbData的字串相呼應
union { WORD *w; DWORD *dw; short *s; BYTE *b; byte *c;LONG *l; int *i; void *p;} PDATA; //定義多形變數指標
/*///========================================================================

//--偶而用到延遲的迴圈-------------------------------------------------------------------------
void __fastcall delay(int i)
{
  while(--i<=0) {}
}

TStrings *sList,*sItem; //常用的字串串列
TfmSiEditer *fmSiEditer;
//---------------------------------------------------------------------------
__fastcall TfmSiEditer::TfmSiEditer(TComponent* Owner)
	: TForm(Owner)
{
  //常用的字串串列
  sList=new TStringList();
  sItem=new TStringList();

  //取得版本編號並填入主畫面又上的版本顯示
  pnVersion->Caption="Ver:"+GetFileVersion();
  //指定系統根目錄
  DataPath=ExtractFilePath(Application->ExeName)+"Data";
  //準備程式環境
  TIniFile *pIni=new TIniFile(ChangeFileExt(Application->ExeName,".ini"));
  if (pIni) {
    //使用者經驗的環境還原
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
    //讀回字串表格的寬度
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
  //讀入待選元件清單的版本
  pnNodeVersion->Caption=NODE_VERSION;
  //讀入待選元件清單及資料
  lbNode->Clear();
  for(int i=0;StrLen(stNodeData[i].Name)>0;i++) {
    lbNode->Items->Add(stNodeData[i].Name);
  }
  //填入元件編輯的欄位名稱
  for(int i=0;i<StringGrid1->ColCount;i++) {
    StringGrid1->Cells[i][0]=lbStringGrid[i];
  }
  //建立主資料夾
  if (!DirectoryExists(DataPath)) CreateDir(DataPath);

  ReadTimes=0;
  WriteTimes=0;
  shReadLED->Brush->Color=clWhite;		//關掉讀取LED燈
  shWriteLED->Brush->Color=clWhite;     //關掉寫出LED燈
  Application->BringToFront();          //本程式移到最上層

  Application->ProcessMessages();

  Timer1->Enabled=false; 				//停止計數器
  Timer1->Interval=SL_TIMER_INTERVAL;	//5ms為單位
  Timer1->Enabled=true; 				//啟動計數器每5ms一次
  //首次計算
  cbWireStatus(this);
}



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfmSiEditer::ReadAllStatus(void)
{ //由計時器定時執行所有元件讀取的作業


  char cmd[20]={0xAA,0x03,0xAF,0xC8,0x00,0x01,0x3D,0x3B};
    int bytes=8;
    //準備送出請求資料的指令
    cmd[0]=ed485ID->Text.ToInt();	//使用正確ID
    cmd[1]=0x03;					//Read Command
    if (r==0) {						// check number of devices are connected to SI-RX
      *((WORD*)&cmd[2])=SL_ADDR_NUMS; 	//詢問可用的元件數量
      *((WORD*)&cmd[2])=W2I(*((WORD*)&cmd[2]));
    } else {
      *((int *)(&cmd[2]))=W2I(StringGrid1->Cells[SG_ADDR][r].ToInt()); //位址編碼
    }
    cmd[4]=0;
    //20150417 以資料的Bits數決定，需要接收多少個WORD資料。
    cmd[5]=(GetNode(StringGrid1->Cells[SG_MODEL][r]).Bits+15)/16;

    PrepareCRC16(cmd,bytes);                            //計算ModBus的CRC16

    if (WriteComPort(cmd,bytes)!=bytes) {	//送出0x03讀取指定位址的指令
				//error handling
      }
    } else { 											//完成送出指令
      ReadBytes=ReadComPortA(ReadBuffer,5+cmd[5]*2,ed485ID->Text.ToInt()); //讀回0x03指令的RS485資料
      //檢查寫出、讀回資料
      if (ReadBytes<6 || ((BYTE)ReadBuffer[0])!=(BYTE)cmd[0] ||
		  !CheckCRC16(ReadBuffer,ReadBytes)) { 	//檢查CRC16查核受否正確
        //交換讀回資料兩個WORD的LowByte、HiByte
        char c;
        c=ReadBuffer[3]; ReadBuffer[3]=ReadBuffer[4]; ReadBuffer[4]=c;
        c=ReadBuffer[5]; ReadBuffer[5]=ReadBuffer[6]; ReadBuffer[6]=c;
        PDATA.b=&ReadBuffer[3]; 					//取得資料指標
        //====以下可直接以指標 PDATA 讀取相對位置之讀回數據=====================
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
  //接收資料燈號開啟，由計時器關閉
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
    //傳送計數器累加
    ReadBytes=0;
    WriteTimes++;
    Comm1->PurgeOut();
    Comm1->PurgeIn();
    if (bytes==Comm1->Write(cmd,bytes)) {
      return bytes;
    } else if (OpenComPort()) {
      Comm1->Tag=max(1,SL_CMD_NEXT/Timer1->Interval);	//傳輸失敗
      while(Comm1->Tag>0) {}                    //等待10毫秒，再傳一次
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
  Comm1->Tag=max(3,SL_READ_WAIT/Timer1->Interval);	//讀取資料，必須等待至資料收足或逾時
  while(Comm1->Tag>0) {
    if (ReadBytes>=bytes) {
      ReadTimes++;
      if (ReadBytes>=6 && CheckCRC16(buffer,bytes)) {
        if (((BYTE)buffer[0])==id) { //讀入指定ID的回覆資料，不需等待
    	  Comm1->Tag=0;
          return ReadBytes;
        }
      }
    }
    Application->ProcessMessages();
  }
  return -1; //等候超時
}
//---------------------------------------------------------------------------


