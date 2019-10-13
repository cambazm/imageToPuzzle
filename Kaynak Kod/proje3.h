//---------------------------------------------------------------------------

#ifndef proje3H
#define proje3H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <ExtDlgs.hpp>
#include <Menus.hpp>
#include <jpeg.hpp>
#include <vector>
using namespace std;
//---------------------------------------------------------------------------
class TResimAc : public TForm
{
__published:	// IDE-managed Components
        TImage *img;
        TOpenPictureDialog *opd;
        TMainMenu *MainMenu1;
        TMenuItem *Resim1;
        TMenuItem *resimac;
        TComboBox *sutunSayisi;
        TLabel *Label1;
        TButton *bol;
        TLabel *lblHata;
        TComboBox *satirSayisi;
        TLabel *Label2;
        TMenuItem *cikis;
        void __fastcall resimacClick(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall bolClick(TObject *Sender);
        void __fastcall onClose(TObject *Sender, TCloseAction &Action);
        void __fastcall cikisClick(TObject *Sender);
    void __fastcall imgMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall imgMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
private:	// User declarations
public:		// User declarations
        __fastcall TResimAc(TComponent* Owner);
        bool resimAcildi;         //yapboz yapýlacak resim açýldý bayraðý
        TJPEGImage *resim;
        vector<TRect> rect;
};
//---------------------------------------------------------------------------
extern PACKAGE TResimAc *ResimAc;
//---------------------------------------------------------------------------
#endif
