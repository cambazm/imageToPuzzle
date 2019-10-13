//---------------------------------------------------------------------------

#ifndef islemH
#define islemH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <vector>
using namespace std;
//---------------------------------------------------------------------------
/*
YAPBOZ PAR�ALARINI TEMS�L EDECEK SINIF
*/

class tekParca : public TImage
{
private:
        bool bagli;     //herhangi bir toplu par�aya ba�l� m� ? true : false
public:
        int satir;                              //ka��nc� sat�rda
        int sutun;                              //ka��nc� s�tunda
        unsigned int kimlik;                    //ba�l� oldu�u toplu par�a kimlik no
        void bagla(unsigned int);               //ba�lanaca�� toplu par�ay� belirle
        bool getBagli() { return bagli; }       //herhangi bir toplu par�aya ba�l� m� ��ren
        unsigned int getKimlik() { return kimlik; }
        tekParca( TComponent *, int, int );
        tekParca( TComponent *, tekParca * );                 //copy constructor
};
//-----------------------------------------------------------------
/*
TOPLU YAPBOZ PAR�ALARI TEMS�L� ���N KULLANILACAK SINIF
*/

class cokParca
{
private:
        unsigned int kimlik;            //par�an�n kimlik nosu
public:
        vector<tekParca*> icerik;       //i�erdi�i tek par�alar�n tutulaca�� vekt�r
        void ekle(tekParca*);
        unsigned int getParcaSayisi() { return icerik.size(); }
        unsigned int getKimlik() { return kimlik; }
        cokParca( unsigned int );
        cokParca( cokParca * );         //copy constructor
        ~cokParca();

};

//-----------------------------------------------------------------
class TYapboz : public TForm
{
__published:	// IDE-managed Components
        TImage *kaynakResim;
        TPanel *Alan;
        TTimer *saniyeTimer;
        TLabel *saat;
        TLabel *dakika;
        TLabel *saniye;
        TLabel *Label1;
        TLabel *Label2;
        TTimer *dakikaTimer;
        TTimer *saatTimer;
        TMainMenu *MainMenu1;
        TMenuItem *otomatikCoz;
        TTimer *otomatikCozTimer;
        void __fastcall onClose(TObject *Sender, TCloseAction &Action);
        void __fastcall onLoad(TObject *Sender);
        void __fastcall onShow(TObject *Sender);
        void __fastcall saniyeTimerOnTimer(TObject *Sender);
        void __fastcall dakikaTimeronTimer(TObject *Sender);
        void __fastcall saatTimeronTimer(TObject *Sender);
        void __fastcall onResize(TObject *Sender);
        void __fastcall otomatikCozClick(TObject *Sender);
        void __fastcall otomatikCozTimerOnTimer(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TYapboz(TComponent* Owner);
        virtual void __fastcall myDragOver(TObject *Sender,
                                           TObject *Source,
                                           int X,
                                           int Y,
                                           TDragState State,
                                           bool &Accept);
        virtual void __fastcall myDragDrop(TObject *Sender,
                                           TObject *Source,
                                           int X,
                                           int Y);
        unsigned int toplamParcaSayisi;
        unsigned int formEn, formBoy;           //formun resize edilme �ncesi en ve boyu
        unsigned int yeniKimlik;
        unsigned int yeniKimlikVer();
        unsigned int timer_i;
        void sesCikar();
        void tebrikler();
        int hangiSatirda(int, int, int);
        int hangiSutunda(int, int, int);
};
//---------------------------------------------------------------------------
extern PACKAGE TYapboz *Yapboz;
//---------------------------------------------------------------------------
#endif
