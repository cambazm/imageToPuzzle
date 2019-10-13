//---------------------------------------------------------------------------

#include <vcl.h>
#include <jpeg.hpp>
#pragma hdrstop

#include "islem.h"
#include "proje3.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TResimAc *ResimAc;
TRect Rect1;
int x1,y1,basla = 0;
bool ufak = true;
//---------------------------------------------------------------------------
__fastcall TResimAc::TResimAc(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TResimAc::resimacClick(TObject *Sender)
{
        basla = 0;
        lblHata->Caption = "";

        opd->Execute();

        /* buradaki dosya a�ma algoritmas� Sam's Borland C++ Builder 6's Developer's Guide
           adl� kitab�n 602 ve 603 sayfas�ndaki dosya a�ma algoritmas� baz al�narak ger�eklenmi�tir */
        if(!FileExists(opd->FileName)) {
                lblHata->Caption = "Resim y�klenemedi!";
                return;         //dosya sorunlu
        }
        AnsiString fileName = ExtractFileName(opd->FileName);
        AnsiString fileExt = ExtractFileExt(opd->FileName);
        AnsiString Ext = fileExt.LowerCase();

        if((Ext.AnsiPos("jpg") > 0) || (Ext.AnsiPos("jpeg") > 0)) { //jpg se�ilmi�
                resim = new TJPEGImage();
                resim->LoadFromFile(opd->FileName);     // a�
                resim->DIBNeeded();                     // jpg bmp e �evriliyor
                img->Picture->Bitmap->Assign(resim);    // img a resmi al

                //e�er verilen resmin eni veya/ve boyu sabit resim boyutlar�ndan ufak de�ilse stretch yap�lacak
                if( !(img->Picture->Bitmap->Height<450 || img->Picture->Bitmap->Width<600) ) {
                        TRect Rect1 = Rect(0,0,600,450);
                        img->Picture->Bitmap->Canvas->StretchDraw(Rect1,img->Picture->Bitmap);  //600*450 sabit boyuna ayarla
                        ufak = false;
                }


                resimAcildi = true;     //yapboz yap�lacak resim a��ld� bayra�� ayarland�
        }
        else if(Ext.AnsiPos("bmp") > 0) {
                img->Picture->Bitmap->LoadFromFile(opd->FileName);
                resimAcildi = true;     //yapboz yap�lacak resim a��ld� bayra�� ayarland�
        }

}
//---------------------------------------------------------------------------
void __fastcall TResimAc::FormCreate(TObject *Sender)
{
        for(int i=2;i<21;i++) {
            sutunSayisi->AddItem(i,Sender);
            satirSayisi->AddItem(i,Sender);
        }
        sutunSayisi->ItemIndex = 1;
        satirSayisi->ItemIndex = 1;
        resimAcildi = false;
}
//---------------------------------------------------------------------------

void __fastcall TResimAc::bolClick(TObject *Sender)
{
        lblHata->Caption = "";
        /* kullan�c�n�n se�ti�i kareler kald�r�l�yor*/
        for(unsigned int i=0; i< rect.size(); i++)
            img->Canvas->DrawFocusRect(rect[i]);

        if(resimAcildi==true) {
                Yapboz->saniyeTimer->Enabled = true;
                Yapboz->dakikaTimer->Enabled = true;
                Yapboz->saatTimer->Enabled = true;
                Yapboz->ShowModal();
        }
        else
                lblHata->Caption = "Resim y�klenmedi!";
}
//---------------------------------------------------------------------------

void __fastcall TResimAc::onClose(TObject *Sender, TCloseAction &Action)
{
        img->Free();
}
//---------------------------------------------------------------------------

void __fastcall TResimAc::cikisClick(TObject *Sender)
{
        delete resim;
        ResimAc->Close();
}
//---------------------------------------------------------------------------

void __fastcall TResimAc::imgMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((X>=0 && X<=600 && Y>=0 && Y<=450 && ufak == false)||
    (X>=0 && X<=img->Picture->Bitmap->Width && Y>=0 && Y<=img->Picture->Bitmap->Height && ufak == true)){


        x1 = X;
        y1 = Y;
        if(basla == 0){
            basla = 1;
            rect.clear();
        }
    }

}
//---------------------------------------------------------------------------

void __fastcall TResimAc::imgMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
       if(Button == 0 && basla == 1){
           if((X>=0 && X<=600 && Y>=0 && Y<=450 && ufak == false)||(X>=0 && X<=img->Picture->Bitmap->Width && Y>=0 && Y<=img->Picture->Bitmap->Height && ufak == true)){
               Rect1 = Rect(x1,y1,X,Y);
               rect.push_back(Rect1);
               img->Canvas->DrawFocusRect(Rect1);
           }
       }
       else if(Button == 1 && basla == 1){
           Rect1 = rect.back();
           rect.pop_back();
           img->Canvas->DrawFocusRect(Rect1);
       }
}
//---------------------------------------------------------------------------

