//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <vector>         //vector için
#include <list>           //list için
#include "islem.h"
#include "proje3.h"
#include <mmsystem.h>     //PlaySound fonksiyonu için (wav dosyasýný çalmak için)
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
using namespace std;

TYapboz *Yapboz;
vector <tekParca*> parcaVector;         //yapboz parçalarýný içeren vektör
list <cokParca*> cokParcaList;          //toplu yapboz parçalarýný içeren nesneleri içeren vektör
vector<int> parcaTopVector, parcaLeftVector;      //otomatik çözümdeki parça yerlerini içeren vektörler
//---------------------------------------------------------------------------
__fastcall TYapboz::TYapboz(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
//form kapanýrken yapýlacak rutinler
void __fastcall TYapboz::onClose(TObject *Sender, TCloseAction &Action)
{
        //ResimAc formunda yüklenmiþ resim kaldýrýlacak
        ResimAc->resimAcildi = false;
        ResimAc->img->Picture->Bitmap->Assign(NULL);

        for(unsigned int i=0; i<parcaVector.size(); i++)
                parcaVector[i]->Free();         //mevcut yapboz parçalarýný temsil eden tekParca nesnelerini yok ediyoruz
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
//form oluþturulurken yapýlacaklar
void __fastcall TYapboz::onLoad(TObject *Sender)
{
        formEn = Yapboz->Width;
        formBoy = Yapboz->Height;

        timer_i = 0;

        yeniKimlik = 0;
        //Alan üzerinde sürükleme iþlemlerinin yapýlacaðý panel
        Alan->Height = GetSystemMetrics(SM_CYSCREEN)-20;        //bilgisayarýn mevcut çözünürlüðünün boyu - 20 (-20 görev çubuðu için)
        Alan->Width = GetSystemMetrics(SM_CXSCREEN);            //bilgisayarýn mevcut çözünürlüðünün eni
        //sürükleme iþlemleri için kendi yazdýðýmýz methodlarý kullanacaðýz
        Alan->OnDragOver = myDragOver;
        Alan->OnDragDrop = myDragDrop;
        //kaynakResim yapbozu yapýlan resmin ufak olarak sað üst köþede gösterilen halidir
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
//form gösterildiðinde yapýlacaklar
void __fastcall TYapboz::onShow(TObject *Sender)
{
        formEn = Yapboz->Width;
        formBoy = Yapboz->Height;

        kaynakResim->Picture->Assign(ResimAc->resim);
        //bölünecek satýr ve sütun sayýlarý alýnýyor
        int satir = ResimAc->satirSayisi->ItemIndex + 2;
        int sutun = ResimAc->sutunSayisi->ItemIndex + 2;
        toplamParcaSayisi = satir*sutun;
        int sutunNo, satirNo, sutunNo2, satirNo2, satirBoyu, sutunBoyu;
        tekParca *image;
        cokParca *cp;


        /* yapboz için verilen resmin kesilecek halinin bir kopyasý çýkarýldý (src)
           yapboz parçalarýnýn içlerine bu resimden alýnan yerler kopyalanacak     */

        /* buradaki bitmap kopyalama algoritmasý Sam's Borland C++ Builder 6 Developer's Guide
           adlý kitabýn 600 ve 601. sayfasýndaki algoritma baz alýnarak gerçeklenmiþtir */
        Graphics::TBitmap *src = new Graphics::TBitmap;
        src->Assign(ResimAc->resim);
        src->PixelFormat = ResimAc->img->Picture->Bitmap->PixelFormat;
        TRect Rect1;
        bool ufak = false;

        //eðer yapboz yapýlacak resmin eni ve/veya boyu sabit yapboz boyundan küçükse
        if(src->Height<450 || src->Width<600) {
                ufak = true;
                Rect1 = Rect(0,0,src->Width,src->Height);
                //resim orijinal boyu ile yapboz yapýlacak
                src->Canvas->StretchDraw(Rect1,src);
        }
        else {
                //yapboz için verilen resmin çözünürlüðü sabit yapboz boyuna indirgeniyor
                Rect1 = Rect(0,0,600,450);
                src->Canvas->StretchDraw(Rect1,src);
                //çözünürlük ayarlandý
        }

                /*(satirNo, sutunNo) ile (satirNo2, sutunNo2) arasýndaki tüm tekParcalar toplu parça olacak*/
        //her yapboz parçasýný satýr ve sütun sayýlarýna göre tek tek oluþturup gerekli deðerleri vereceðiz
        for(int i=0; i<satir; i++){

            for(int j=0; j<sutun; j++){

                //her image o anda oluþturulan yapboz parçasýný temsil ediyor
                image = new tekParca(this,i,j);
                //i - kaçýncý satýrda onu belirtir
                //j - kaçýncý sütunda onu belirtir

                image->Parent = Alan;           //Alan üzerinde bulunacak
                image->Stretch = true;

                //eðer sabit yapboz boyutundan ufaksa kendi boyutuyla çizilecek yapboz
                if(ufak == true) {
                        image->Width = src->Width/sutun;
                        image->Height = src->Height/satir;
                }
                else {
                        image->Width = ResimAc->img->Width/sutun;
                        image->Height = ResimAc->img->Height/satir;
                }
                //otomatik çözüm için gerekli koordinatlar.resim bir bütün halinde görünecek
                image->Top = 50 + i*image->Height;
                image->Left = 50 + j*image->Width;

                image->Visible = true;

                //otomatik çözümde yerleþmesi gereken yere ait bilgiler vektörlere alýnýyor
                parcaTopVector.push_back(image->Top);
                parcaLeftVector.push_back(image->Left);


                image->DragMode = dmAutomatic;
                //yapboz parçalarýnýn sürüklenme özelliði otomatik olacak


                Graphics::TBitmap *dest = new Graphics::TBitmap;
                //dest her yapboz parçasý için resimden alýnan yeri temsil ediyor
                dest->Width = image->Width;
                dest->Height = image->Height;

                TRect Eski = Rect(image->Left-50,image->Top-50,image->Left+image->Width-50,image->Top+image->Height-50);
                TRect Yeni = Rect(0, 0, dest->Width, dest->Height);

                dest->PixelFormat = src->PixelFormat;
                dest->Canvas->CopyRect(Yeni, src->Canvas, Eski);
                //resimden belirlenmiþ yer alýnarak yapboz parçasýna kopyalanýyor
                image->Picture->Bitmap->Assign(dest);
                //resimden kopyalanmýþ resim parçasý artýk yapboz parçasýna resim olarak veriliyor


                parcaVector.push_back(image);
                //yapboz parçalarýný içeren vektöre oluþturulmuþ yapboz parçasý ekleniyor


                //yapboz parçalarýný panelde dýþarý taþmayacak þekilde rastgele yerleþtiriyoruz
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
                //kesilmemesi gereken dörtgenlerin koordinatlarý hangi parçalar üzerinde olduðu belirleniyor.
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
            //parcalardan daha önce baðlanmýþ olan yok
            if(bagli == false){
                cokParca *tp = new cokParca(yeniKimlikVer());
                //yeni bir toplu parça yaratýlarak birleþtirilen tek parçalar ona dahil edilecek

                //birleþen parçalar artýk yeni bir toplu parçaya baðlý olacaklar
                for(unsigned int j=0; j<secilen.size(); j++){
                    secilen[j]->bagla(tp->getKimlik());
                    //yeni oluþan toplu parçanýn içerik vektörüne içerdiði tek parçalar ekleniyor
                    tp->ekle(secilen[j]);
                }
                cokParcaList.push_back(tp);

                //parcalanmamasý istenen parcalar ekranda birleþik olarak gösteriliyor.
                unsigned int index;
                int sutunSayisi = -1;


                for(index=0; index<secilen.size()-1; index++){
                    //satýr olarak yanyana olan parçalar birleþtiriliyor.
                    if(secilen[index]->satir == secilen[index+1]->satir){
                        secilen[index+1]->Left = secilen[index]->Left + secilen[index]->Width;
                        secilen[index+1]->Top = secilen[index]->Top;
                        continue;
                    }


                    //sütün olarak yanyana olan parçalar birleþtiriliyor.
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
                        //yeni oluþan toplu parçanýn içerik vektörüne içerdiði tek parçalar ekleniyor
                        (*iterator)->ekle(secilen[j]);
                    }
                }
                XLeft1 = parcaVector[indexBagli]->Left;
                YTop1 = parcaVector[indexBagli]->Top;

                unsigned int index;
                int sutunSayisi = -1;

                for(index=0; index<secilen.size()-1; index++){
                    //satýr olarak yanyana olan parçalar birleþtiriliyor.
                    if(secilen[index]->satir == secilen[index+1]->satir){
                        secilen[index+1]->Left = secilen[index]->Left + secilen[index]->Width;
                        secilen[index+1]->Top = secilen[index]->Top;
                        continue;
                    }
                    //sütün olarak yanyana olan parçalar birleþtiriliyor.
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
//sürüklenen nesnenin nerelere sürüklenebileceði
void __fastcall TYapboz::myDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
        Accept=true;    //Alan paneli üzerinde sürükleme iþlemi yapýlabilecek
}
//---------------------------------------------------------------------------
//sürüklenen nesne býrakýldý
void __fastcall TYapboz::myDragDrop(TObject *Sender, TObject *Source, int X, int Y)
{
        tekParca *suruklenen = (tekParca*)Source;    //sürüklenme iþleminin gerçeklendiði nesne (yapboz parçasý)
        tekParca *kontrol = NULL;
        TPanel *kaynak = (TPanel*)Sender;              //üzerinde sürükleme iþlemi yapýlan nesne (Alan (panel))
        TPoint temp, solUst, sagAlt;
        /* solUst: arama yapýlacak dikdörtgenin sol üst köþe noktasý
           sagAlt: arama yapýlacak dikdörtgenin sað alt köþe noktasý */
        bool tamam = false;
        int bakma_araligi = 20;
        /* bakma_araligi: ne kadar yakýn býrakýlan parçalar otomatik olarak yerleþecek
           varsayýlan 20 dir, isteðe göre deðiþtirilebilir */
        list<cokParca*>::iterator iterator;

        int degisimX = X - suruklenen->Left;
        int degisimY = Y - suruklenen->Top;

        //eðer sürüklenen parça toplu bir parçaya baðlý ise baðlý tüm elemanlarý da sürüklemeliyiz
        if(suruklenen->getBagli() == true)
        {
                for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                {
                        if((*iterator)->getKimlik() == suruklenen->kimlik)
                                break;
                }
                //(*iterator) sürüklenen parçanýn baðlý olduðu toplu parça nesnesini gösteriyor
                for(unsigned int i=0; i<(*iterator)->getParcaSayisi(); i++)
                {
                        //baðlý tüm parçalar sürüklenen parça ile birlikte sürükleniyorlar
                        (*iterator)->icerik[i]->Left += degisimX;
                        (*iterator)->icerik[i]->Top += degisimY;
                }
        }
        //sürüklenen parçaya býrakýldýðý yer koordinatlarý atanýyor
        suruklenen->Left=X;
        suruklenen->Top=Y;


        temp.x = X;
        temp.y = Y;

        solUst.x = X - bakma_araligi;
        solUst.y = Y;
        sagAlt.x = X;
        sagAlt.y = Y + suruklenen->Height;

        //sol tarafta bakma_araligi uzaklýktan suruklenen kontrolun boyu süresince oluþan dikdörtgen içerisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bakýlan noktada bir kontrol var (yapboz parçasý)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* sürüklenen parçanýn ve varsa kendisine baðlý parçalarýn otomatik olarak yerleþebilmesi için
                           yanýna gelmesi gereken parçanýn <=20 piksel yakýnýna býrakýlmýþ olmal,
                           bulunan kontrolün zaten sürüklenen parçanýn baðlý olduðu toplu parçaya baðlý olmamasý gerekli */
                        if( (kontrol->satir == suruklenen->satir) && (kontrol->sutun == suruklenen->sutun-1) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //soluna gelmesi gereken parça ise yerleþtir
                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yanýna koyulacak parça tek parça

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu parça yaratýlarak birleþtirilen tek parçalar ona dahil edilecek

                                        //birleþen parçalar artýk yeni bir toplu parçaya baðlý olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni oluþan toplu parçanýn içerik vektörüne içerdiði tek parçalar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);

                                        suruklenen->Left = kontrol->Left + kontrol->Width;
                                        suruklenen->Top = kontrol->Top;
                                        tamam = true;
                                        sesCikar();     //yerleþince ses çýkar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yanýna koyulacak parça daha önce bir toplu parçaya baðlanmýþ
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yanýna koyulacak parçanýn baðlý olduðu toplu parçayý gösteriyor

                                        /* sürüklenen parça eðer herhangi bir toplu parçayan baðlý deðilse
                                           yanýna koyulacaðý parçanýn baðlý olduðu toplu parçaya baðlanýr */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left + kontrol->Width;
                                                suruklenen->Top = kontrol->Top;
                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //sürüklenen parça daha önce bir toplu parçaya baðlanmýþsa o zaman iki toplu parça birleþtirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 sürüklenen parçanýn baðlý olduðu toplu parçayý gösterecek
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
                                                        //sürüklenen hariç baðlý diðer parçalar da yerleþtirilecek çünkü sürüklenen daha önce yerleþtirildi
                                                        if((*iterator2)->icerik[i] != suruklenen)
                                                        {
                                                                (*iterator2)->icerik[i]->Left += degisimX;
                                                                (*iterator2)->icerik[i]->Top += degisimY;
                                                        }
                                                        //yerleþtirilen her parça kontrolün baðlý olduðu toplu parçaya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //artýk sürüklenenin baðlý olduðu toplu parça nesnesine ihtiyaç yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

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
                return;         //gerekli kontrol bulundu geri dön


        tamam = false;
        solUst.x = X + suruklenen->Width;
        solUst.y = Y;
        sagAlt.x = solUst.x + bakma_araligi;
        sagAlt.y = Y + suruklenen->Height;

        //sað tarafta bakma_araligi uzaklýktan suruklenen kontrolun boyu süresince oluþan dikdörtgen içerisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bakýlan noktada bir kontrol (yapboz parçasý)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* sürüklenen parçanýn ve varsa kendisine baðlý parçalarýn otomatik olarak yerleþebilmesi için
                           yanýna gelmesi gereken parçanýn <=20 piksel yakýnýna býrakýlmýþ olmal,
                           bulunan kontrolün zaten sürüklenen parçanýn baðlý olduðu toplu parçaya baðlý olmamasý gerekli */
                        if( (kontrol->satir == suruklenen->satir) && (kontrol->sutun == suruklenen->sutun+1) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //saðýna gelmesi gereken parça ise yerleþtir

                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yanýna koyulacak parça tek parça

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu parça yaratýlarak birleþtirilen tek parçalar ona dahil edilecek

                                        //birleþen parçalar artýk yeni bir toplu parçaya baðlý olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni oluþan toplu parçanýn içerik vektörüne içerdiði tek parçalar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);
                                        suruklenen->Left = kontrol->Left - kontrol->Width;
                                        suruklenen->Top = kontrol->Top;
                                        tamam = true;
                                        sesCikar();     //yerleþince ses çýkar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yanýna koyulacak parça daha önce bir toplu parçaya baðlanmýþ
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yanýna koyulacak parçanýn baðlý olduðu toplu parçayý gösteriyor

                                        /* sürüklenen parça eðer herhangi bir toplu parçayan baðlý deðilse
                                           yanýna koyulacaðý parçanýn baðlý olduðu toplu parçaya baðlanýr */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left - kontrol->Width;
                                                suruklenen->Top = kontrol->Top;
                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //sürüklenen parça daha önce bir toplu parçaya baðlanmýþsa o zaman iki toplu parça birleþtirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 sürüklenen parçanýn baðlý olduðu toplu parçayý gösterecek
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
                                                        //sürüklenen hariç baðlý diðer parçalar da yerleþtirilecek çünkü sürüklenen daha önce yerleþtirildi
                                                        if((*iterator2)->icerik[i] != suruklenen)
                                                        {
                                                                (*iterator2)->icerik[i]->Left += degisimX;
                                                                (*iterator2)->icerik[i]->Top += degisimY;
                                                        }
                                                        //yerleþtirilen her parça kontrolün baðlý olduðu toplu parçaya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //artýk sürüklenenin baðlý olduðu toplu parça nesnesine ihtiyaç yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

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
                return;         //gerekli kontrol bulundu geri dön


        tamam = false;
        solUst.x = X;
        solUst.y = Y - bakma_araligi;
        sagAlt.x = X + suruklenen->Width;
        sagAlt.y = Y;

        //üst tarafta bakma_araligi uzaklýktan suruklenen kontrolun eni süresince oluþan dikdörtgen içerisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bakýlan noktada bir kontrol (yapboz parçasý)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* sürüklenen parçanýn ve varsa kendisine baðlý parçalarýn otomatik olarak yerleþebilmesi için
                           yanýna gelmesi gereken parçanýn <=20 piksel yakýnýna býrakýlmýþ olmal,
                           bulunan kontrolün zaten sürüklenen parçanýn baðlý olduðu toplu parçaya baðlý olmamasý gerekli */
                        if( (kontrol->satir == suruklenen->satir-1) && (kontrol->sutun == suruklenen->sutun) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //üstüne gelmesi gereken parça ise yerleþtir

                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yanýna koyulacak parça tek parça

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu parça yaratýlarak birleþtirilen tek parçalar ona dahil edilecek

                                        //birleþen parçalar artýk yeni bir toplu parçaya baðlý olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni oluþan toplu parçanýn içerik vektörüne içerdiði tek parçalar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);
                                        suruklenen->Left = kontrol->Left;
                                        suruklenen->Top = kontrol->Top + kontrol->Height;
                                        tamam = true;
                                        sesCikar();     //yerleþince ses çýkar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yanýna koyulacak parça daha önce bir toplu parçaya baðlanmýþ
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yanýna koyulacak parçanýn baðlý olduðu toplu parçayý gösteriyor

                                        /* sürüklenen parça eðer herhangi bir toplu parçayan baðlý deðilse
                                           yanýna koyulacaðý parçanýn baðlý olduðu toplu parçaya baðlanýr */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left;
                                                suruklenen->Top = kontrol->Top + kontrol->Height;
                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //sürüklenen parça daha önce bir toplu parçaya baðlanmýþsa o zaman iki toplu parça birleþtirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 sürüklenen parçanýn baðlý olduðu toplu parçayý gösterecek
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
                                                        //yerleþtirilen her parça kontrolün baðlý olduðu toplu parçaya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //artýk sürüklenenin baðlý olduðu toplu parça nesnesine ihtiyaç yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

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
                return;         //gerekli kontrol bulundu geri dön


        tamam = false;
        solUst.x = X;
        solUst.y = Y + suruklenen->Height;
        sagAlt.x = X + suruklenen->Width;
        sagAlt.y = Y + suruklenen->Height + bakma_araligi;

        //alt tarafta bakma_araligi uzaklýktan suruklenen kontrolun eni süresince oluþan dikdörtgen içerisinde kontrol ara
        for(int j=solUst.y; j<=sagAlt.y; j++)
        {
            temp.y=j;
            for(int i=solUst.x; i<=sagAlt.x; i++)
            {
                temp.x=i;
                if(kaynak->ControlAtPos(temp, false, true)!=NULL)
                {   //bakýlan noktada bir kontrol (yapboz parçasý)
                        kontrol = (tekParca*) kaynak->ControlAtPos(temp, false, true);

                        /* sürüklenen parçanýn ve varsa kendisine baðlý parçalarýn otomatik olarak yerleþebilmesi için
                           yanýna gelmesi gereken parçanýn <=20 piksel yakýnýna býrakýlmýþ olmal,
                           bulunan kontrolün zaten sürüklenen parçanýn baðlý olduðu toplu parçaya baðlý olmamasý gerekli */
                        if( (kontrol->satir == suruklenen->satir+1) && (kontrol->sutun == suruklenen->sutun) && !(kontrol->getBagli() && suruklenen->getBagli() && suruklenen->kimlik == kontrol->kimlik) )
                        {
                                //altýna gelmesi gereken parça ise yerleþtir

                                if( (kontrol->getBagli() == false) && (suruklenen->getBagli() == false) )
                                {      //yanýna koyulacak parça tek parça

                                        cokParca *tp = new cokParca(yeniKimlikVer());
                                        //yeni bir toplu parça yaratýlarak birleþtirilen tek parçalar ona dahil edilecek

                                        //birleþen parçalar artýk yeni bir toplu parçaya baðlý olacaklar
                                        kontrol->bagla(tp->getKimlik());
                                        suruklenen->bagla(tp->getKimlik());

                                        //yeni oluþan toplu parçanýn içerik vektörüne içerdiði tek parçalar ekleniyor
                                        tp->ekle(kontrol);
                                        tp->ekle(suruklenen);

                                        cokParcaList.push_back(tp);

                                        suruklenen->Left = kontrol->Left;
                                        suruklenen->Top = kontrol->Top - kontrol->Height;
                                        tamam = true;
                                        sesCikar();     //yerleþince ses çýkar
                                }
                                else if(kontrol->getBagli() == true)
                                {  //yanýna koyulacak parça daha önce bir toplu parçaya baðlanmýþ
                                        for(iterator=cokParcaList.begin(); iterator != cokParcaList.end(); ++iterator)
                                        {
                                                if((*iterator)->getKimlik() == kontrol->kimlik)
                                                        break;
                                        }
                                        //(*iterator) yanýna koyulacak parçanýn baðlý olduðu toplu parçayý gösteriyor

                                        /* sürüklenen parça eðer herhangi bir toplu parçayan baðlý deðilse
                                           yanýna koyulacaðý parçanýn baðlý olduðu toplu parçaya baðlanýr */
                                        if(suruklenen->getBagli() == false)
                                        {
                                                suruklenen->bagla(kontrol->kimlik);

                                                (*iterator)->ekle(suruklenen);

                                                suruklenen->Left = kontrol->Left;
                                                suruklenen->Top = kontrol->Top - kontrol->Height;
                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

                                                if((*iterator)->getParcaSayisi() == toplamParcaSayisi)
                                                        tebrikler();
                                        }
                                        //sürüklenen parça daha önce bir toplu parçaya baðlanmýþsa o zaman iki toplu parça birleþtirilir
                                        else
                                        {
                                                list<cokParca*>::iterator iterator2;
                                                for(iterator2=cokParcaList.begin(); iterator2 != cokParcaList.end(); ++iterator2)
                                                {
                                                        //iterator2 sürüklenen parçanýn baðlý olduðu toplu parçayý gösterecek
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
                                                        //sürüklenen hariç baðlý diðer parçalar da yerleþtirilecek çünkü sürüklenen daha önce yerleþtirildi
                                                        if((*iterator2)->icerik[i] != suruklenen)
                                                        {
                                                                (*iterator2)->icerik[i]->Left += degisimX;
                                                                (*iterator2)->icerik[i]->Top += degisimY;
                                                        }
                                                        //yerleþtirilen her parça kontrolün baðlý olduðu toplu parçaya eklenecek
                                                        (*iterator)->ekle((*iterator2)->icerik[i]);
                                                }
                                                //artýk sürüklenenin baðlý olduðu toplu parça nesnesine ihtiyaç yok
                                                cokParcaList.erase(iterator2);

                                                tamam = true;
                                                sesCikar();     //yerleþince ses çýkar

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
                return;         //gerekli kontrol bulundu geri dön
}
//---------------------------------------------------------------------------
//parça otomatik yerleþince yerleþtiðine dair ses çýkaracak
void TYapboz::sesCikar()
{
        PlaySound("click.wav", NULL, SND_ASYNC);   //yerleþince ses çýkar
}
//---------------------------------------------------------------------------
//her saniyede saniye label ý bir artacak mod 60 a göre
void __fastcall TYapboz::saniyeTimerOnTimer(TObject *Sender)
{
        unsigned int s = StrToInt(saniye->Caption);
        s = (s+1) % 60;                                 //saniye 0-59 arasýnda deðerler alabilir
        saniye->Caption = IntToStr(s);
}
//---------------------------------------------------------------------------
//her dakikada dakika label ý bir artacak mod 60 a göre
void __fastcall TYapboz::dakikaTimeronTimer(TObject *Sender)
{
        unsigned int d = StrToInt(dakika->Caption);
        d = (d+1) % 60;                                 //dakika 0-59 arasýnda deðerler alabilir
        dakika->Caption = IntToStr(d);
}
//---------------------------------------------------------------------------
//her saat geçtiðinde saat label ý bir artacak mod 24 e göre
void __fastcall TYapboz::saatTimeronTimer(TObject *Sender)
{
        unsigned int sa = StrToInt(saat->Caption);
        sa = (sa+1) % 24;                               //saat 0-23 arasýnda deðerler alabilir
        saat->Caption = IntToStr(sa);
}
//---------------------------------------------------------------------------
//her toplu parçaya tek eþsiz bir kimlik numarasý verir
unsigned int TYapboz::yeniKimlikVer()
{
        yeniKimlik++;
        return yeniKimlik-1;
}
//---------------------------------------------------------------------------
//eðer yapboz baþarý ile tamamlanýrsa kullanýcýyý tebrik et
void TYapboz::tebrikler()
{
        saniyeTimer->Enabled = false;
        dakikaTimer->Enabled = false;
        saatTimer->Enabled = false;
        PlaySound("applause.wav", NULL, SND_ASYNC);
        AnsiString mesaj = "Tebrikler, "+IntToStr(toplamParcaSayisi)+" parçadan oluþan yapbozu toplamda "+saat->Caption+" saat, "+dakika->Caption+" dakika, "+saniye->Caption+" saniyede çözdünüz";
        Application->MessageBox(mesaj.c_str(), "Bravo", MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
//formun boyutu deðiþtirildiðinde parçalarýn enleri ve boylarý büyüme/küçülme oranýna göre deðiþecek
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
//verilen X koordinatý satir kadar bölünmüþ yapbozda hangi satýrda bul
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
//verilen Y koordinatý sutun kadar bölünmüþ yapbozda hangi sutunda bul
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
//yapbozu otomatik çöz denildi
void __fastcall TYapboz::otomatikCozClick(TObject *Sender)
{
        if(otomatikCozTimer->Enabled == false) {
                otomatikCozTimer->Enabled = true;
                otomatikCoz->Caption = "Otomatik Çözümü Durdur";
        }
        else {
                otomatikCozTimer->Enabled = false;
                //otomatik çözerken bir daha otomatik çöze basýlýrsa otomatik çözme olayý durdurulur
                otomatikCoz->Caption = "Otomatik Çöz";
        }
}
//---------------------------------------------------------------------------
//her 400ms de bir bir parça olmasý gereken yere sürüklenecek
void __fastcall TYapboz::otomatikCozTimerOnTimer(TObject *Sender)
{
        if(timer_i < parcaVector.size()){
                Yapboz->myDragDrop(Alan, parcaVector[timer_i], parcaLeftVector[timer_i], parcaTopVector[timer_i]);
                /*
                parcaVector[timer_i]->Left = parcaLeftVector[timer_i];
                parcaVector[timer_i]->Top = parcaTopVector[timer_i];
                yapýlmýþ oldu esasýnda.
                yani bir parça olmasý gereken yere sürüklendi
                */
        }
        else {
                otomatikCozTimer->Enabled = false;
                otomatikCoz->Caption = "Otomatik Çöz";
                saniyeTimer->Enabled = false;
                dakikaTimer->Enabled = false;
                saatTimer->Enabled = false;
        }
        timer_i++;
}
//---------------------------------------------------------------------------
//tek yapboz parçasý oluþturulurken yapýlacaklar (constructor)
tekParca::tekParca( TComponent *Owner, int satir1, int sutun1 ) : TImage ( Owner )
{
        kimlik = -1;
        satir = satir1;
        sutun = sutun1;
        bagli = false;
}
//---------------------------------------------------------------------------
//tek yapboz parçasý kopyalanmak isterse yapýlacaklar (copy constructor)
tekParca::tekParca( TComponent *Owner, tekParca *right ) : TImage ( Owner )
{
        satir = right->satir;
        sutun = right->sutun;
        bagli = right->bagli;
        kimlik = right->kimlik;
}
//---------------------------------------------------------------------------
//tek yapboz parçasý verilen kimlik nolu toplu parça sýnýfýna baðlanmasý
void tekParca::bagla( unsigned int kimlik1 )
{
        kimlik = kimlik1;
        bagli = true;
}
//---------------------------------------------------------------------------
//toplu yapboz parçasý oluþtururken yapýlacaklar (constructor)
cokParca::cokParca(unsigned int kimlik1)
{
        kimlik = kimlik1;
        /* yeni oluþturulacak toplu parçanýn kimlik nosu eþsiz olacak
        bunun için Yapboz formunda bu iþ için özel bir deðiþken ve fonksiyon kullanýlacak */
}
//---------------------------------------------------------------------------
//toplu yapboz parçasý kopyalanmak isterse yapýlacaklar (copy constructor)
cokParca::cokParca( cokParca *right )
{
        kimlik = right->kimlik;
        for(unsigned int i=0; i<icerik.size(); i++)
                icerik[i] = right->icerik[i];
}
//---------------------------------------------------------------------------
//toplu yapboz parçasýna bir tek parça eklenmesi
void cokParca::ekle(tekParca *parca)
{
        icerik.push_back(parca);
}
//---------------------------------------------------------------------------
//toplu yapboz parçasý yokedilirken yapýlacaklar (deconstructor)
cokParca::~cokParca()
{
        //bir toplu parça yok edilirken artýk hiç tek parça içermeyecek
        icerik.clear();
}
//---------------------------------------------------------------------------

