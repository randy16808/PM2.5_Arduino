//---------------------------------------------------------------------------
#pragma hdrstop
#include "uCommUtils.h"
#define CRC16	_CRC16
//#define CRC16	_CRC16A
//---------------------------------------------------------------------------
#pragma package(smart_init)
typedef union {
  BYTE by[2];
  WORD wd;
} comboVaule;
//---------------------------------------------------------------------------
BYTE __fastcall LRC(BYTE *nData, WORD wLength)
{
  BYTE nLRC = 0 ; // LRC char initialized
  for (int i = 0; i < wLength; i++) {
    nLRC += *nData++;
  }
  return (BYTE)(-nLRC);
} // End: LRC
//---------------------------------------------------------------------------
static const WORD wCRCTable[] = {
   0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
   0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
   0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
   0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
   0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
   0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
   0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
   0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
   0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
   0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
   0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
   0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
   0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
   0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
   0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
   0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
   0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
   0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
   0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
   0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
   0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
   0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
   0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
   0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
   0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
   0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
   0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
   0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
   0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
   0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
   0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
   0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };
//---------------------------------------------------------------------------
WORD __fastcall _CRC16(BYTE *nData,WORD wLength)
{ //CRC16查表法
  BYTE nTemp;
  WORD wCRCWord = 0xFFFF;

  while (wLength--) {
    nTemp = *nData++ ^ wCRCWord;
    wCRCWord >>= 8;
    wCRCWord  ^= wCRCTable[nTemp];
  }
  return wCRCWord;
} // End: CRC16
//---------------------------------------------------------------------------
WORD __fastcall _CRC16A(unsigned char *data,WORD length)
{ //CRC16採取計算方式
  int j;
  WORD wCRC16=0xFFFF;
  while(length--) {
    wCRC16 ^= *data++;
    for(j=0;j<8;j++) {
  	  if(wCRC16 & 0x01) { /* LSB(b0)=1 */
	  	wCRC16=(wCRC16>>1) ^ 0xA001;
	  } else {
		wCRC16=wCRC16>>1;
      }
    }
  }
  return wCRC16;
}
//---------------------------------------------------------------------------
DWORD __fastcall DW2UL(BYTE *pdat)
{ //
  BYTE pb[4];
  pb[0]=pdat[1]; pb[1]=pdat[0];
  pb[2]=pdat[3]; pb[3]=pdat[2];
  return *((DWORD*)&pb[0]);
}
//---------------------------------------------------------------------------
WORD __fastcall W2I(BYTE *pdat)
{
  BYTE pb[2];
  pb[0]=pdat[1]; pb[1]=pdat[0];
  return *((WORD*)pb);
}
//---------------------------------------------------------------------------
WORD __fastcall W2I(WORD dat)
{
  return W2I((BYTE *)&dat);
}
//---------------------------------------------------------------------------
AnsiString __fastcall H2A(BYTE *buffer,int count)
{ //字元陣列內的字元轉換成16進位表示式的字串類別
  AnsiString hex;
  BYTE MSB,LSB;
  char Hex[4];
  hex="";
  if (count>0) {
    buffer[count]=0;
    for (int i=0;i<count;i++) {
      LSB=(buffer[i]   )&0xF;
      MSB=(buffer[i]>>4)&0xF;
      if (LSB>=0x0A) LSB=LSB+0x37; else LSB=LSB+0x30;
      if (MSB>=0x0A) MSB=MSB+0x37; else MSB=MSB+0x30;
      Hex[0]=MSB;Hex[1]=LSB; Hex[2]=' '; Hex[3]=0;
      hex=hex+Hex;
    }
  }
  return hex;
}
//---------------------------------------------------------------------------
int __fastcall A2H(AnsiString hex,BYTE *buffer)
{	//16進位表示式的字串類別轉換成字元陣列內的字元
  int Bytes=0; 	//轉換後的字元數量
  int nbytes=0; //nibble byte number
  int Hex=0;	//轉換過程的BYTE內容
  int len=hex.Length(); //輸入字串長度
  BYTE ch;      //per chatactor for hex
  hex=hex.UpperCase(); 	//轉換成大寫
  //------------------------------------------------
  for(int i=1;i<=len;i++) {
    ch=hex[i];
    if (isxdigit(ch)) {
      if (ch>=0x30 && ch<=0x39) {
        nbytes++;
        Hex=Hex*16+ch-48;
      } else if (ch>=0x41 && ch<=0x46) {
        nbytes++;
        Hex=Hex*16+(ch-55);
      }
    } else { //非HEX數字時
      if (nbytes>0) nbytes=2;
    }
    if (nbytes==2) {
      buffer[Bytes++]=Hex;
      nbytes=0;
      Hex=0;
    }
  }
  if (nbytes>0) buffer[Bytes++]=Hex;  //字串結尾為單位元組時
  return Bytes;
}
//---------------------------------------------------------------------------
bool __fastcall CheckCRC16(BYTE *buffer,int count)
{ //檢查字元陣列的CRC16是否正確
  WORD CRC=CRC16(buffer,count-2);
  if (CRC==*((WORD *)(buffer+count-2))) return true;
  return false;
}
//---------------------------------------------------------------------------
WORD __fastcall PrepareCRC16(BYTE *buffer,int count)
{ //寫入並取回CRC16
  int Bytes;
  WORD CRC=0;
  CRC=CRC16(buffer,count-2);
  *((WORD *)(buffer+count-2))=CRC;
  return CRC;
}
//-------------------------------------------------------------------------------
AnsiString __fastcall GetFileVersion()
{ //讀回目前執行檔的版本
  return GetFileVersion(Application->ExeName);
}
//-------------------------------------------------------------------------------
AnsiString __fastcall GetFileVersion(AnsiString AppName)
{ //取得執行檔案版本，傳回字串類別
  char AppVer[80];
  GetVersionOfFile(AppName.c_str(),AppVer,sizeof(AppVer),NULL,0);
  return AnsiString(AppVer);
}
//-------------------------------------------------------------------------------
void __fastcall GetVersionOfFile (char *pszAppName,char *pszVerBuff,int iVerBuffLen,char *pszLangBuff,int iLangBuffLen)
{ //取得執行檔案版本，傳回字串陣列
  DWORD dwScratch;
  DWORD *pdwLangChar;
  DWORD dwInfSize ;
  UINT  uSize;
  BYTE  *pbyInfBuff;
  char  szVersion [32];
  char  szResource [80];
  char  *pszVersion = szVersion;

  dwInfSize = GetFileVersionInfoSize (pszAppName, &dwScratch);
  if (dwInfSize) {
    pbyInfBuff = new BYTE [dwInfSize];
    memset (pbyInfBuff, 0, dwInfSize);
    if (pbyInfBuff) {
      if (GetFileVersionInfo (pszAppName, 0, dwInfSize, pbyInfBuff)) {
        if (VerQueryValue (pbyInfBuff,"\\VarFileInfo\\Translation",(void**)(&pdwLangChar),&uSize))  {
          if (VerLanguageName (LOWORD (*pdwLangChar),szResource,sizeof(szResource)))  {
            strncpy (pszLangBuff, szResource, iLangBuffLen);
          }
          wsprintf (szResource, "\\StringFileInfo\\%04X%04X\\FileVersion",LOWORD (*pdwLangChar), HIWORD (*pdwLangChar));
          if (VerQueryValue (pbyInfBuff,szResource,(void**)(&pszVersion),&uSize))  {
            strncpy (pszVerBuff, pszVersion, iVerBuffLen-1);
          }
        }
      }
      delete [] pbyInfBuff;
    }
  }
}
//-------------------------------------------------------------------------------
//---------------------------------------------------------------------------

