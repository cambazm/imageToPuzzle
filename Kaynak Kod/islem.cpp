//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <vector>         //vector i�in
#include <list>           //list i�in
#include "islem.h"
#include "proje3.h"
#include <mmsystem.h>     //PlaySound fonksiyonu i�in (wav dosyas�n� �almak i�in)
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
using namespace std;

TYapboz *Yapboz;
vector <tekParca*> parcaVector;         //yapboz par�alar�n� i�eren vekt�r
list <cokParca*> cokParcaList;          //toplu yapboz par�alar�n� i�eren nesneleri i�eren vekt�r
vector<int> parcaTopVector, parcaLeftVector;      //otomatik ��z�mdeki par�a yerlerini i�eren vekt�rler
//---------------------------------------------------------------------------
__fastcall TYapboz::TYapboz(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
//form kapan�rken yap�lacak rutinler
void __fastcall TYapboz::onClose(TObject *Sender, TCloseAction &Action)
{
        //ResimAc formunda y�klenmi� resim kald�r�lacak
        ResimAc->resimAcildi = false;
        ResimAc->img->Picture->Bitmap->Assign(NULL);

        for(unsigned int i=0; i<parcaVector.size(); i++)
                parcaVector[i]->Free();         //mevcut yapboz par�alar�n� temsil eden tekParca nesnelerini yok ediyoruz
        parcaVector.clear();
        cokParcaList.clear();
        parcaTopVector.clear();
        parcaLeftVector.clear();
        ResimAc->rect.clear();

        saniyeTimer->Enabled = false;
        dakikaTimer->Enabled = false;
        saatTimer->Enabled = false;
        saat->Caption = "00";
        dakika->Caption = "00";
        saniye->Caption = "00";

        yeniKimlik = 0;
        timer_i = 0;
}
//---------------------------------------------------------------------------
//form olu�turulurken yap�lacaklar
void __fastcall TYapboz::onLoad(TObject *Sender)
{
        formEn = Yapboz->Width;
        formBoy = Yapboz->Height;

        timer_i = 0;

        yeniKimlik = 0;
        //Alan �zerinde s�r�kleme i�lemlerinin yap�laca�� panel
        Alan->Height = GetSystemMetrics(SM_CYSCREEN)-20;        //bilgisayar�n mevcut ��z�n�rl���n�n boyu - 20 (-20 g�rev �ubu�u i�in)
        Alan->Width = GetSystemMetrics(SM_CXSCREEN);            //bilgisayar�n mevcut ��z�n�rl���n�n eni
        //s�r�kleme i�lemleri i�in kendi yazd���m�z methodlar� kullanaca��z
        Alan->OnDragOver = myDragOver;
        Alan->OnDragDrop = myDragDrop;
        //kaynakResim yapbozu yap�lan resmin ufak olarak sa� �st k��ede g�sterilen halidir
        kaynakResim->Parent = Alan;
        kaynakResim->Top = 10;
        kaynakResim->Left = GetSystemMetrics(SM_CXSCREEN) - kaynakResim->Width - 10;

        saat->Parent = Alan;
        Label1->Parent = Alan;
        dakika->Parent = Alan;
        Label2->Parent = Alan;
        saniye->Parent = Alan;
}
//---------------------------------------------------------------------------
//form g�sterildi�inde yap�lacaklar
void __fastcall TYapboz::onShow(TObject *Sender)
{
        formEn = Yapboz->Width;
        formBoy = Yapboz->Height;

        kaynakResim->Picture->Assign(ResimAc->resim);
        //b�l�necek sat�r ve s�tun say�lar� al�n�yor
        int satir = ResimAc->satirSayisi->ItemIndex + 2;
        int sutun = ResimAc->sutunSayisi->ItemIndex + 2;
        toplamParcaSayisi = satir*sutun;
        int sutunNo, satirNo, sutunNo2, satirNo2, satirBoyu, sutunBoyu;
        tekParca *image;
        cokParca *cp;


        /* yapboz i�in verilen resmin kesilecek halinin bir kopyas� ��kar�ld� (src)
           yapboz par�alar�n�n i�lerine bu resimden al�nan yerler kopyalanacak     */

        /* buradaki bitmap kopyalama algoritmas� Sam's Borland C++ Builder 6 Developer's Guide
           adl� kitab�n 600 ve 601. sayfas�ndaki algoritma baz al�narak ger�eklenmi�tir */
        Graphics::TBitmap *src = new Graphics::TBitmap;
        src->Assign(ResimAc->resim);
        src->PixelFormat = ResimAc->img->Picture->Bitmap->PixelFormat;
        TRect Rect1;
        bool ufak = false;

        //e�er yapboz yap�lacak resmin eni ve/veya boyu sabit yapboz boyundan k���kse
        if(src->Height<450 || src->Width<600) {
                ufak = true;
                Rect1 = Rect(0,0,src->Width,src->Height);
                //resim orijinal boyu ile yapboz yap�lacak
                src->Canvas->StretchDraw(Rect1,src);
        }
        else {
                //yapboz i�in verilen resmin ��z�n�rl��� sabit yapboz boyuna indirgeniyor
                Rect1 = Rect(0,0,600,450);
                src->Canvas->StretchDraw(Rect1,src);
                //��z�n�rl�k ayarland�
        }

                /*(satirNo, sutunNo) ile (satirNo2, sutunNo2) aras�ndaki t�m tekParcalar toplu par�a olacak*/
        //her yapboz par�as�n� sat�r ve s�tun say�lar�na g�re tek tek olu�turup gerekli de�erleri verece�iz
        for(int i=0; i<satir; i++){

            for(int j=0; j<sutun; j++){

                //her image o anda olu�turulan yapboz par�as�n� temsil ediyor
                image = new tekParca(this,i,j);
                //i - ka��nc� sat�rda onu belirtir
                //j - ka��nc� s�tunda onu belirtir

                image->Parent = Alan;           //Alan �zerinde bulunacak
                image->Stretch = true;

                //e�er sabit yapboz boyutundan ufaksa kendi boyutuyla �izilecek yapboz
                if(ufak == true) {
                        image->Width = src->Width/sutun;
                        image->Height = src->Height/satir;
                }
                else {
                        image->Width = ResimAc->img->Width/sutun;
                        image->Height = ResimAc->img->Height/satir;
                }
                //otomatik ��z�m i�in gerekli koordinatlar.resim bir b�t�n halinde g�r�necek
                image->Top = 50 + i*image->Height;
                image->Left = 50 + j*image->Width;

                image->Visible = true;

                //otomatik ��z�mde yerle�mesi gereken yere ait bilgiler vekt�rlere al�n�yor
                parcaTopVector.push_back(image->Top);
                parcaLeftVector.push_back(image->Left);


                image->DragMode = dmAutomatic;
                //yapboz par�alar�n�n s�r�klenme �zelli�i otomatik olacak


                Graphics::TBitmap *dest = new Graphics::TBitmap;
                //dest her yapboz par�as� i�in resimden al�nan yeri temsil ediyor
                dest->Width = image->Width;
                dest->Height = image->Height;

                TRect Eski = Rect(image->Left-50,image->Top-50,image->Left+image->Width-50,image->Top+image->Height-50);
                TRect Yeni = Rect(0, 0, dest->Width, dest->Height);

                dest->PixelFormat = src->PixelFormat;
                dest->Canvas->CopyRect(Yeni, src->Canvas, Eski);
                //resimden belirlenmi� yer al�narak yapboz par�as�na kopyalan�yor
                image->Picture->Bitmap->Assign(dest);
                //resimden kopyalanm�� resim par�as� art�k yapboz par�as�na resim olarak veriliyor


                parcaVector.push_back(image);
                //yapboz par�alar�n� i�eren vekt�re olu�turulmu� yapboz par�as� ekleniyor


                //yapboz par�alar�n� panelde d��ar� ta�mayacak �ekilde rastgele yerle�tiriyoruz
                image->Top = rand() % (Alan->Height - image->Width);
                image->Left = rand() % (Alan->Width - image->Height);

                delete dest;
            }
        }
        delete src;


        int sizeRect = ResimAc->rect.size();
        int sizeParca = parcaVector.size();
        int parcaIndex1,parcaIndex2;
        vector <tekParca*> secilen;
        unsigned int kimlik;
        bool bagli = false;
        list<cokParca*>::iterator iterator;
        int indexBagli;
        for(int i=0; i<sizeRect; i++){
            bagli = false;
            secilen.clear();

            for(int j=0; j<sizeParca; j++){
                //kesilmemesi gereken d�rtgenlerin koordinatlar� hangi par�alar �zerinde oldu�u belirleniyor.
                if((parcaVector[j]->sutun*parcaVector[j]->Width <= ResimAc->rect[i].Left) && ((parcaVector[j]->sutun+1)*parcaVector[j]->Width > ResimAc->rect[i].Left) &&
                    (parcaVector[j]->satir*parcaVector[j]->Height <= ResimAc->rect[i].Top) && ((parcaVector[j]->satir+1)*parcaVector[j]->Height > ResimAc->rect[i].Top)){

                    parcaIndex1 = j;
                    break;
                }
            }

            for(int j=parcaIndex1; j<sizeParca; j++){
                if((parcaVector[j]->sutun*parcaVector[j]->Width <= ResimAc->rect[i].Right) && ((parcaVector[j]->sutun+1)*parcaVector[j]->Width > ResimAc->rect[i].Right) &&
                    (parcaVector[j]->satir*parcaVector[j]->Height <= ResimAc->rect[i].Bottom) && ((parcaVector[j]->satir+1)*parcaVector[j]->Height > ResimAc->rect[i].Bottom)){

                    parcaIndex2 = j;
                    break;

                }
            }


            for(int j=parcaIndex1; j<=parcaIndex2; j++){
                if((parcaVector[j]->satir >= parcaVector[parcaIndex1]->satir) && (parcaVector[j]->satir <= parcaVector[parcaIndex2]->satir)&&
                   (parcaVector[j]->sutun >= parcaVector[parcaIndex1]->sutun) && (parcaVector[j]->sutun <= parcaVector[parcaIndex2]->sutun)){

                    secilen.push_back(parcaVector[j]);
                    if(parcaVector[j]->getBagli() == true){
                        kimlik = parcaVector[j]->getKimlik();
                        bagli = true;
                        indexBagli = j;
                    }
                }
            }
            //parcalardan daha �nce ba�lanm�� olan yok
            if(bagli == false){
                cokParca *tp = new cokParca(yeniKimlikVer());
                //yeni bir toplu par�a yarat�larak birle�tirilen tek par�alar ona dahil edilecek

                //birle�en par�alar art�k yeni bir toplu par�aya ba�l� olacaklar
                for(unsigned int j=0; j<secilen.size(); j++){
                    secilen[j]->bagla(tp->getKimlik());
                    //yeni olu�an toplu par�an�n i�erik vekt�r�ne i�erdi�i tek par�alar ekleniyor
                    tp->ekle(secilen[j]);
                }
                cokParcaList.push_back(tp);

                //parcalanmamas� istenen parcalar ekranda birle�ik olarak g�steriliyor.
                unsigned int index;
                int sutunSayisi = -1;


                for(index=0; index<secilen.size()-1; index++){
                    //sat�r olarak yanyana olan par�alar birle�tiriliyor.
                    if(secilen[index]->satir == secilen[index+1]->satir){
                        secilen[index+1]->Left = secilen[index]->Left + secilen[index]->Width;
                        secilen[index+1]->Top = secilen[index]->Top;
                        continue;
                    }


                    //s�t�n olarak yanyana olan par�alar birle�tiriliyor.
                    if(secilen[0]->sutun == secilen[index+1]->sutun){
                        if(sutunSayisi == -1)
                            sutunSayisi = index;
                        secilen[index+1]->Left = secilen[index - sutunSayisi]->Left;
                        secilen[index+1]->Top = secilen[index - sutunSayisi]->Top + secilen[index - sutunSayisi]->Height;
                    }
                }
                continue;

            }
            int XLeft1,YTop1,XFark,YFark;
            if(bagli == true){
                for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); iterator++){
                    if((*iterator)->getKimlik() == kimlik)
                        break;
                }
                for(unsigned int j=0; j<secilen.size(); j++){
                    if(secilen[j]->getBagli() == false){
                        secilen[j]->bagla(kimlik);
                        //yeni olu�an toplu par�an�n i�erik vekt�r�ne i�erdi�i tek par�alar ekleniyor
                        (*iterator)->ekle(secilen[j]);
                    }
                }
                XLeft1 = parcaVector[indexBagli]->Left;
                YTop1 = parcaVector[indexBagli]->Top;

                unsigned int index;
                int sutunSayisi = -1;

                for(index=0; index<secilen.size()-1; index++){
                    //sat�r olarak yanyana olan par�alar birle�tiriliyor.
                    if(secilen[index]->satir == secilen[index+1]->satir){
                        secilen[index+1]->Left = secilen[index]->Left + secilen[index]->Width;
                        secilen[index+1]->Top = secilen[index]->Top;
                        continue;
                    }
                    //s�t�n olarak yanyana olan par�alar birle�tiriliyor.
                    if(secilen[0]->sutun == secilen[index+1]->sutun){
                        if(sutunSayisi == -1)
                            sutunSayisi = index;
                        secilen[index+1]->Left = secilen[index - sutunSayisi]->Left;
                        secilen[index+1]->Top = secilen[index - sutunSayisi]->Top + secilen[index - sutunSayisi]->Height;
                    }
                }

                XFark = parcaVector[indexBagli]->Left - XLeft1;
                YFark = parcaVector[indexBagli]->Top - YTop1;

               if(XFark != 0 || YFark != 0){
                    for(index=0; index<secilen.size(); index++){
                        secilen[index]->Left  = secilen[index]->Left - XFark;
                        secilen[index]->Top  = secilen[index]->Top - YFark;
                    }
                }

            }


        }

}
//---------------------------------------------------------------------------
//s�r�klenen nesnenin nerelere s�r�klenebilece�i
void __fastcall TYapboz::myDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
        Accept=true;    //Alan paneli �zerinde s�r�kleme i�lemi yap�labilecek
}
//---------------------------------------------------------------------------
//s�r�klenen nesne b�rak�ld�
void __fastcall TYapboz::myDragDrop(TObject *Sender, TObject *Source, int X, int Y)
{
        tekParca *suruklenen = (tekParca*)Source;    //s�r�klenme i�leminin ger�eklendi�i nesne (yapboz par�as�)
        tekParca *kontrol = NULL;
        TPanel *kaynak = (TPanel*)Sender;              //�zerinde s�r�kleme i�lemi yap�lan nesne (Alan (panel))
        TPoint temp, solUst, sagAlt;
        /* solUst: arama yap�lacak dikd�rtgenin sol �st k��e noktas�
           sagAlt: arama yap�lacak dikd�rtgenin sa� alt k��e noktas� */
        bool tamam = false;
        int bakma_araligi = 20;
        /* bakma_araligi: ne kadar yak�n b�rak�lan par�alar otomatik olarak yerle�ecek
           varsay�lan 20 dir, iste�e g�re de�i�tirilebilir */
        list<cokParca*>::iterator iterator;

        int degisimX = X - suruklenen->Left;
        int degisimY = Y - suruklenen->Top;

        //e�er s�r�klenen par�a toplu bir par�aya ba�l� ise ba�l� t�m elemanlar� da s�r�klemeliyiz
        if(suruklenen->getBagli() == true)
        {
                for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                {
                        if((*iterator)->getKimlik() == suruklenen->kimlik)
                                break;
                }
                //(*iterator) s�r�klenen par�an�n ba�l� oldu�u toplu par�a nesnesini g�steriyor
                for(unsigned int i=0; i<(*iterator)->getParcaSayisi(); i++)
                {
                        //ba�l� t�m par�alar s�r�klenen par�a ile birlikte s�r�kleniyorlar
                        (*iterator)->icerik[i]->Left += degisimX;
                        (*iterator)->icerik[i]->Top += degisimY;
                }
        }
        //s�r�klenen par�aya b�rak�ld��� yer koordinatlar� atan�yor
        suruklenen->Left=X;
        suruklenen->Top=Y;


        temp.x = X;
        temp.y = Y;

        solUst.x = X - bakma_araligi;
        solUst.y = Y;
        sagAlt.x = X;
        sagAlt.y = Y + suruklenen->Height;

        //sol tarafta bakma_araligi uzakl�ktan suruklenen kontrolun boyu s�resince olu�an dikd�rtgen i�erisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bak�lan noktada bir kontrol var (yapboz par�as�)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* s�r�klenen par�an�n ve varsa kendisine ba�l� par�alar�n otomatik olarak yerle�ebilmesi i�in
                           yan�na gelmesi gereken par�an�n <=20 piksel yak�n�na b�rak�lm�� olmal,
                           bulunan kontrol�n zaten s�r�klenen par�an�n ba�l� oldu�u toplu par�aya ba�l� olmamas� gerekli */
                        if( (kontrol->satir == suruklenen->satir) && (kontrol->sutun == suruklenen->sutun-1) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //soluna gelmesi gereken par�a ise yerle�tir
                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yan�na koyulacak par�a tek par�a

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu par�a yarat�larak birle�tirilen tek par�alar ona dahil edilecek

                                        //birle�en par�alar art�k yeni bir toplu par�aya ba�l� olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni olu�an toplu par�an�n i�erik vekt�r�ne i�erdi�i tek par�alar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);

                                        suruklenen->Left = kontrol->Left + kontrol->Width;
                                        suruklenen->Top = kontrol->Top;
                                        tamam = true;
                                        sesCikar();     //yerle�ince ses ��kar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yan�na koyulacak par�a daha �nce bir toplu par�aya ba�lanm��
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yan�na koyulacak par�an�n ba�l� oldu�u toplu par�ay� g�steriyor

                                        /* s�r�klenen par�a e�er herhangi bir toplu par�ayan ba�l� de�ilse
                                           yan�na koyulaca�� par�an�n ba�l� oldu�u toplu par�aya ba�lan�r */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left + kontrol->Width;
                                                suruklenen->Top = kontrol->Top;
                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //s�r�klenen par�a daha �nce bir toplu par�aya ba�lanm��sa o zaman iki toplu par�a birle�tirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 s�r�klenen par�an�n ba�l� oldu�u toplu par�ay� g�sterecek
                                                        if((*iterator2)->getKimlik() == suruklenen->kimlik)
                                                                break;
                                                }

                                                suruklenen->Left = kontrol->Left + kontrol->Width;
                                                suruklenen->Top = kontrol->Top;

                                                degisimX = suruklenen->Left - X;
                                                degisimY = suruklenen->Top - Y;

                                                for(unsigned int i=0; i<(*iterator2)->getParcaSayisi(); i++)
                                                {
                                                        (*iterator2)->icerik[i]->bagla((*iterator)->getKimlik());
                                                        //s�r�klenen hari� ba�l� di�er par�alar da yerle�tirilecek ��nk� s�r�klenen daha �nce yerle�tirildi
                                                        if((*iterator2)->icerik[i] != suruklenen)
                                                        {
                                                                (*iterator2)->icerik[i]->Left += degisimX;
                                                                (*iterator2)->icerik[i]->Top += degisimY;
                                                        }
                                                        //yerle�tirilen her par�a kontrol�n ba�l� oldu�u toplu par�aya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //art�k s�r�klenenin ba�l� oldu�u toplu par�a nesnesine ihtiya� yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                }

                                break;
                        }
                }
            }
            if(tamam)
                break;
        }
        if(tamam)
                return;         //gerekli kontrol bulundu geri d�n


        tamam = false;
        solUst.x = X + suruklenen->Width;
        solUst.y = Y;
        sagAlt.x = solUst.x + bakma_araligi;
        sagAlt.y = Y + suruklenen->Height;

        //sa� tarafta bakma_araligi uzakl�ktan suruklenen kontrolun boyu s�resince olu�an dikd�rtgen i�erisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bak�lan noktada bir kontrol (yapboz par�as�)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* s�r�klenen par�an�n ve varsa kendisine ba�l� par�alar�n otomatik olarak yerle�ebilmesi i�in
                           yan�na gelmesi gereken par�an�n <=20 piksel yak�n�na b�rak�lm�� olmal,
                           bulunan kontrol�n zaten s�r�klenen par�an�n ba�l� oldu�u toplu par�aya ba�l� olmamas� gerekli */
                        if( (kontrol->satir == suruklenen->satir) && (kontrol->sutun == suruklenen->sutun+1) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //sa��na gelmesi gereken par�a ise yerle�tir

                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yan�na koyulacak par�a tek par�a

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu par�a yarat�larak birle�tirilen tek par�alar ona dahil edilecek

                                        //birle�en par�alar art�k yeni bir toplu par�aya ba�l� olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni olu�an toplu par�an�n i�erik vekt�r�ne i�erdi�i tek par�alar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);
                                        suruklenen->Left = kontrol->Left - kontrol->Width;
                                        suruklenen->Top = kontrol->Top;
                                        tamam = true;
                                        sesCikar();     //yerle�ince ses ��kar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yan�na koyulacak par�a daha �nce bir toplu par�aya ba�lanm��
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yan�na koyulacak par�an�n ba�l� oldu�u toplu par�ay� g�steriyor

                                        /* s�r�klenen par�a e�er herhangi bir toplu par�ayan ba�l� de�ilse
                                           yan�na koyulaca�� par�an�n ba�l� oldu�u toplu par�aya ba�lan�r */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left - kontrol->Width;
                                                suruklenen->Top = kontrol->Top;
                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //s�r�klenen par�a daha �nce bir toplu par�aya ba�lanm��sa o zaman iki toplu par�a birle�tirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 s�r�klenen par�an�n ba�l� oldu�u toplu par�ay� g�sterecek
                                                        if((*iterator2)->getKimlik() == suruklenen->kimlik)
                                                                break;
                                                }
                                                suruklenen->Left = kontrol->Left - kontrol->Width;
                                                suruklenen->Top = kontrol->Top;

                                                degisimX = suruklenen->Left - X;
                                                degisimY = suruklenen->Top - Y;

                                                for(unsigned int i=0; i<(*iterator2)->getParcaSayisi(); i++)
                                                {
                                                        (*iterator2)->icerik[i]->bagla((*iterator)->getKimlik());
                                                        //s�r�klenen hari� ba�l� di�er par�alar da yerle�tirilecek ��nk� s�r�klenen daha �nce yerle�tirildi
                                                        if((*iterator2)->icerik[i] != suruklenen)
                                                        {
                                                                (*iterator2)->icerik[i]->Left += degisimX;
                                                                (*iterator2)->icerik[i]->Top += degisimY;
                                                        }
                                                        //yerle�tirilen her par�a kontrol�n ba�l� oldu�u toplu par�aya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //art�k s�r�klenenin ba�l� oldu�u toplu par�a nesnesine ihtiya� yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                }

                                break;
                        }
                }
            }
            if(tamam)
                break;
        }
        if(tamam)
                return;         //gerekli kontrol bulundu geri d�n


        tamam = false;
        solUst.x = X;
        solUst.y = Y - bakma_araligi;
        sagAlt.x = X + suruklenen->Width;
        sagAlt.y = Y;

        //�st tarafta bakma_araligi uzakl�ktan suruklenen kontrolun eni s�resince olu�an dikd�rtgen i�erisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bak�lan noktada bir kontrol (yapboz par�as�)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* s�r�klenen par�an�n ve varsa kendisine ba�l� par�alar�n otomatik olarak yerle�ebilmesi i�in
                           yan�na gelmesi gereken par�an�n <=20 piksel yak�n�na b�rak�lm�� olmal,
                           bulunan kontrol�n zaten s�r�klenen par�an�n ba�l� oldu�u toplu par�aya ba�l� olmamas� gerekli */
                        if( (kontrol->satir == suruklenen->satir-1) && (kontrol->sutun == suruklenen->sutun) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //�st�ne gelmesi gereken par�a ise yerle�tir

                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yan�na koyulacak par�a tek par�a

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu par�a yarat�larak birle�tirilen tek par�alar ona dahil edilecek

                                        //birle�en par�alar art�k yeni bir toplu par�aya ba�l� olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni olu�an toplu par�an�n i�erik vekt�r�ne i�erdi�i tek par�alar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);
                                        suruklenen->Left = kontrol->Left;
                                        suruklenen->Top = kontrol->Top + kontrol->Height;
                                        tamam = true;
                                        sesCikar();     //yerle�ince ses ��kar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yan�na koyulacak par�a daha �nce bir toplu par�aya ba�lanm��
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yan�na koyulacak par�an�n ba�l� oldu�u toplu par�ay� g�steriyor

                                        /* s�r�klenen par�a e�er herhangi bir toplu par�ayan ba�l� de�ilse
                                           yan�na koyulaca�� par�an�n ba�l� oldu�u toplu par�aya ba�lan�r */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left;
                                                suruklenen->Top = kontrol->Top + kontrol->Height;
                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //s�r�klenen par�a daha �nce bir toplu par�aya ba�lanm��sa o zaman iki toplu par�a birle�tirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 s�r�klenen par�an�n ba�l� oldu�u toplu par�ay� g�sterecek
                                                        if((*iterator2)->getKimlik() == suruklenen->kimlik)
                                                                break;
                                                }

                                                suruklenen->Left = kontrol->Left;
                                                suruklenen->Top = kontrol->Top + kontrol->Height;

                                                degisimX = suruklenen->Left - X;
                                                degisimY = suruklenen->Top - Y;

                                                for(unsigned int i=0; i<(*iterator2)->getParcaSayisi(); i++)
                                                {
                                                        (*iterator2)->icerik[i]->bagla((*iterator)->getKimlik());
                                                        if((*iterator2)->icerik[i] != suruklenen)
                                                        {
                                                                (*iterator2)->icerik[i]->Left += degisimX;
                                                                (*iterator2)->icerik[i]->Top += degisimY;
                                                        }
                                                        //yerle�tirilen her par�a kontrol�n ba�l� oldu�u toplu par�aya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //art�k s�r�klenenin ba�l� oldu�u toplu par�a nesnesine ihtiya� yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                }

                                break;
                        }
                }
            }
            if(tamam)
                break;
        }
        if(tamam)
                return;         //gerekli kontrol bulundu geri d�n


        tamam = false;
        solUst.x = X;
        solUst.y = Y + suruklenen->Height;
        sagAlt.x = X + suruklenen->Width;
        sagAlt.y = Y + suruklenen->Height + bakma_araligi;

        //alt tarafta bakma_araligi uzakl�ktan suruklenen kontrolun eni s�resince olu�an dikd�rtgen i�erisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bak�lan noktada bir kontrol (yapboz par�as�)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* s�r�klenen par�an�n ve varsa kendisine ba�l� par�alar�n otomatik olarak yerle�ebilmesi i�in
                           yan�na gelmesi gereken par�an�n <=20 piksel yak�n�na b�rak�lm�� olmal,
                           bulunan kontrol�n zaten s�r�klenen par�an�n ba�l� oldu�u toplu par�aya ba�l� olmamas� gerekli */
                        if( (kontrol->satir == suruklenen->satir+1) && (kontrol->sutun == suruklenen->sutun) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //alt�na gelmesi gereken par�a ise yerle�tir

                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yan�na koyulacak par�a tek par�a

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu par�a yarat�larak birle�tirilen tek par�alar ona dahil edilecek

                                        //birle�en par�alar art�k yeni bir toplu par�aya ba�l� olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni olu�an toplu par�an�n i�erik vekt�r�ne i�erdi�i tek par�alar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);

                                        suruklenen->Left = kontrol->Left;
                                        suruklenen->Top = kontrol->Top - kontrol->Height;
                                        tamam = true;
                                        sesCikar();     //yerle�ince ses ��kar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yan�na koyulacak par�a daha �nce bir toplu par�aya ba�lanm��
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yan�na koyulacak par�an�n ba�l� oldu�u toplu par�ay� g�steriyor

                                        /* s�r�klenen par�a e�er herhangi bir toplu par�ayan ba�l� de�ilse
                                           yan�na koyulaca�� par�an�n ba�l� oldu�u toplu par�aya ba�lan�r */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left;
                                                suruklenen->Top = kontrol->Top - kontrol->Height;
                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //s�r�klenen par�a daha �nce bir toplu par�aya ba�lanm��sa o zaman iki toplu par�a birle�tirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 s�r�klenen par�an�n ba�l� oldu�u toplu par�ay� g�sterecek
                                                        if((*iterator2)->getKimlik() == suruklenen->kimlik)
                                                                break;
                                                }
                                                
                                                suruklenen->Left = kontrol->Left;
                                                suruklenen->Top = kontrol->Top - kontrol->Height;

                                                degisimX = suruklenen->Left - X;
                                                degisimY = suruklenen->Top - Y;

                                                for(unsigned int i=0; i<(*iterator2)->getParcaSayisi(); i++)
                                                {
                                                        (*iterator2)->icerik[i]->bagla((*iterator)->getKimlik());
                                                        //s�r�klenen hari� ba�l� di�er par�alar da yerle�tirilecek ��nk� s�r�klenen daha �nce yerle�tirildi
                                                        if((*iterator2)->icerik[i] != suruklenen)
                                                        {
                                                                (*iterator2)->icerik[i]->Left += degisimX;
                                                                (*iterator2)->icerik[i]->Top += degisimY;
                                                        }
                                                        //yerle�tirilen her par�a kontrol�n ba�l� oldu�u toplu par�aya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //art�k s�r�klenenin ba�l� oldu�u toplu par�a nesnesine ihtiya� yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerle�ince ses ��kar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                }

                                break;
                        }
                }
            }
            if(tamam)
                break;
        }
        if(tamam)
                return;         //gerekli kontrol bulundu geri d�n
}
//---------------------------------------------------------------------------
//par�a otomatik yerle�ince yerle�ti�ine dair ses ��karacak
void TYapboz::sesCikar()
{
        PlaySound("click.wav", NULL, SND_ASYNC);   //yerle�ince ses ��kar
}
//---------------------------------------------------------------------------
//her saniyede saniye label � bir artacak mod 60 a g�re
void __fastcall TYapboz::saniyeTimerOnTimer(TObject *Sender)
{
        unsigned int s = StrToInt(saniye->Caption);
        s = (s+1) % 60;                                 //saniye 0-59 aras�nda de�erler alabilir
        saniye->Caption = IntToStr(s);
}
//---------------------------------------------------------------------------
//her dakikada dakika label � bir artacak mod 60 a g�re
void __fastcall TYapboz::dakikaTimeronTimer(TObject *Sender)
{
        unsigned int d = StrToInt(dakika->Caption);
        d = (d+1) % 60;                                 //dakika 0-59 aras�nda de�erler alabilir
        dakika->Caption = IntToStr(d);
}
//---------------------------------------------------------------------------
//her saat ge�ti�inde saat label � bir artacak mod 24 e g�re
void __fastcall TYapboz::saatTimeronTimer(TObject *Sender)
{
        unsigned int sa = StrToInt(saat->Caption);
        sa = (sa+1) % 24;                               //saat 0-23 aras�nda de�erler alabilir
        saat->Caption = IntToStr(sa);
}
//---------------------------------------------------------------------------
//her toplu par�aya tek e�siz bir kimlik numaras� verir
unsigned int TYapboz::yeniKimlikVer()
{
        yeniKimlik++;
        return yeniKimlik-1;
}
//---------------------------------------------------------------------------
//e�er yapboz ba�ar� ile tamamlan�rsa kullan�c�y� tebrik et
void TYapboz::tebrikler()
{
        saniyeTimer->Enabled = false;
        dakikaTimer->Enabled = false;
        saatTimer->Enabled = false;
        PlaySound("applause.wav", NULL, SND_ASYNC);
        AnsiString mesaj = "Tebrikler, "+IntToStr(toplamParcaSayisi)+" par�adan olu�an yapbozu toplamda "+saat->Caption+" saat, "+dakika->Caption+" dakika, "+saniye->Caption+" saniyede ��zd�n�z";
        Application->MessageBox(mesaj.c_str(), "Bravo", MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
//formun boyutu de�i�tirildi�inde par�alar�n enleri ve boylar� b�y�me/k���lme oran�na g�re de�i�ecek
void __fastcall TYapboz::onResize(TObject *Sender)
{
        float olcekEn = (float) Yapboz->Width / formEn;
        float olcekBoy = (float) Yapboz->Height / formBoy;

        for(unsigned int i=0; i<parcaVector.size(); i++)
        {
                parcaVector[i]->Width *= olcekEn;
                parcaVector[i]->Height *= olcekBoy;
        }
        formEn = Yapboz->Width;
        formBoy = Yapboz->Height;

        kaynakResim->Left = Yapboz->Width - kaynakResim->Width - 30;
}
//---------------------------------------------------------------------------
//verilen X koordinat� satir kadar b�l�nm�� yapbozda hangi sat�rda bul
int TYapboz::hangiSatirda(int X, int satir, int satirBoyu)
{
        int satirNo;
        for(satirNo=0; satirNo<satir; satirNo++)
        {
                if( (satirNo*satirBoyu <= X) && ((satirNo+1)*satirBoyu >= X) )
                        break;
        }
        return satirNo;
}
//---------------------------------------------------------------------------
//verilen Y koordinat� sutun kadar b�l�nm�� yapbozda hangi sutunda bul
int TYapboz::hangiSutunda(int Y, int sutun, int sutunBoyu)
{
        int sutunNo;
        for(sutunNo=0; sutunNo<sutun; sutunNo++)
        {
                if( (sutunNo*sutunBoyu <= Y) && ((sutunNo+1)*sutunBoyu >= Y) )
                        break;
        }
        return sutunNo;
}
//---------------------------------------------------------------------------
//yapbozu otomatik ��z denildi
void __fastcall TYapboz::otomatikCozClick(TObject *Sender)
{
        if(otomatikCozTimer->Enabled == false) {
                otomatikCozTimer->Enabled = true;
                otomatikCoz->Caption = "Otomatik ��z�m� Durdur";
        }
        else {
                otomatikCozTimer->Enabled = false;
                //otomatik ��zerken bir daha otomatik ��ze bas�l�rsa otomatik ��zme olay� durdurulur
                otomatikCoz->Caption = "Otomatik ��z";
        }
}
//---------------------------------------------------------------------------
//her 400ms de bir bir par�a olmas� gereken yere s�r�klenecek
void __fastcall TYapboz::otomatikCozTimerOnTimer(TObject *Sender)
{
        if(timer_i < parcaVector.size()){
                Yapboz->myDragDrop(Alan, parcaVector[timer_i], parcaLeftVector[timer_i], parcaTopVector[timer_i]);
                /*
                parcaVector[timer_i]->Left = parcaLeftVector[timer_i];
                parcaVector[timer_i]->Top = parcaTopVector[timer_i];
                yap�lm�� oldu esas�nda.
                yani bir par�a olmas� gereken yere s�r�klendi
                */
        }
        else {
                otomatikCozTimer->Enabled = false;
                otomatikCoz->Caption = "Otomatik ��z";
                saniyeTimer->Enabled = false;
                dakikaTimer->Enabled = false;
                saatTimer->Enabled = false;
        }
        timer_i++;
}
//---------------------------------------------------------------------------
//tek yapboz par�as� olu�turulurken yap�lacaklar (constructor)
tekParca::tekParca( TComponent *Owner, int satir1, int sutun1 ) : TImage ( Owner )
{
        kimlik = -1;
        satir = satir1;
        sutun = sutun1;
        bagli = false;
}
//---------------------------------------------------------------------------
//tek yapboz par�as� kopyalanmak isterse yap�lacaklar (copy constructor)
tekParca::tekParca( TComponent *Owner, tekParca *right ) : TImage ( Owner )
{
        satir = right->satir;
        sutun = right->sutun;
        bagli = right->bagli;
        kimlik = right->kimlik;
}
//---------------------------------------------------------------------------
//tek yapboz par�as� verilen kimlik nolu toplu par�a s�n�f�na ba�lanmas�
void tekParca::bagla( unsigned int kimlik1 )
{
        kimlik = kimlik1;
        bagli = true;
}
//---------------------------------------------------------------------------
//toplu yapboz par�as� olu�tururken yap�lacaklar (constructor)
cokParca::cokParca(unsigned int kimlik1)
{
        kimlik = kimlik1;
        /* yeni olu�turulacak toplu par�an�n kimlik nosu e�siz olacak
        bunun i�in Yapboz formunda bu i� i�in �zel bir de�i�ken ve fonksiyon kullan�lacak */
}
//---------------------------------------------------------------------------
//toplu yapboz par�as� kopyalanmak isterse yap�lacaklar (copy constructor)
cokParca::cokParca( cokParca *right )
{
        kimlik = right->kimlik;
        for(unsigned int i=0; i<icerik.size(); i++)
                icerik[i] = right->icerik[i];
}
//---------------------------------------------------------------------------
//toplu yapboz par�as�na bir tek par�a eklenmesi
void cokParca::ekle(tekParca *parca)
{
        icerik.push_back(parca);
}
//---------------------------------------------------------------------------
//toplu yapboz par�as� yokedilirken yap�lacaklar (deconstructor)
cokParca::~cokParca()
{
        //bir toplu par�a yok edilirken art�k hi� tek par�a i�ermeyecek
        icerik.clear();
}
//---------------------------------------------------------------------------

