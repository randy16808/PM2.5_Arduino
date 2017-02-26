//---------------------------------------------------------------------------
#ifndef uSiEditerH
#define uSiEditerH
//---------------------------------------------------------------------------
#include "Comm.h"
//---------------------------------------------------------------------------
class TfmSiEditer : public TForm
{
__published:	// IDE-managed Components
	TComm *Comm1;
	TTimer *Timer1;
	TEdit *ed485ID;

	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall Comm1RxChar(TObject *Sender, DWORD Count);

private:	// User declarations
public:		// User declarations
    String DataPath;
    int ReadBytes,WriteBytes;
    int ReadTimes,WriteTimes;
    bool isBusy;
    int CurBits;
    int PrnPosX,PrnPosY;
    int PrnResolX,PrnResolY;
	BYTE ReadBuffer[2048];
	BYTE WriteBuffer[2048];
    
	__fastcall TfmSiEditer(TComponent* Owner);

    void __fastcall ReadAllStatus(void);
    bool __fastcall OpenComPort(void);
    int __fastcall WriteComPort(char *,int bytes);
    int __fastcall ReadComPort(char *,int bytes);
    int __fastcall ReadComPortA(char *,int bytes,int id);
};
//---------------------------------------------------------------------------
extern PACKAGE TfmSiEditer *fmSiEditer;
//---------------------------------------------------------------------------
#endif
