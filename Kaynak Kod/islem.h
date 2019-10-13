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
YAPBOZ PARÇALARINI TEMSÝL EDECEK SINIF
*/

class tekParca : public TImage
{
private:
        bool bagli;     //herhangi bir toplu parçaya baðlý mý ? true : false
public:
        int satir;                              //kaçýncý satýrda
        int sutun;                              //kaçýncý sütunda
        unsigned int kimlik;                    //baðlý olduðu toplu parça kimlik no
        void bagla(unsigned int);               //baðlanacaðý toplu parçayý belirle
        bool getBagli() { return bagli; }       //herhangi bir toplu parçaya baðlý mý öðren
        unsigned int getKimlik() { return kimlik; }
        tekParca( TComponent *, int, int );
        tekParca( TComponent *, tekParca * );                 //copy constructor
};
//-----------------------------------------------------------------
/*
TOPLU YAPBOZ PARÇALARI TEMSÝLÝ ÝÇÝN KULLANILACAK SINIF
*/

class cokParca
{
private:
        unsigned int kimlik;            //parçanýn kimlik nosu
public:
        vector<tekParca*> icerik;       //içerdiði tek parçalarýn tutulacaðý vektör
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
        unsigned int formEn, formBoy;           //formun resize edilme öncesi en ve boyu
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
