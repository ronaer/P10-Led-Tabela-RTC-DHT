/*
  ---------------------------------------------------------------------
 | https://github.com/ronaer/P10-Led-Tabela-RTC-DHT                    |
 | YouTube >>> TR.ALP.18                                               |
 | Açık kaynak lisansı altında kullanılabilir, paylaşılabilir...       |
  ---------------------------------------------------------------------
*/
//Eklemeler ve tanımlamalar...

#include "SoftwareSerial.h" //ses çalıcımız için gerekli kütüphane...
SoftwareSerial mySoftwareSerial(A1, A2); // RX, TX Pinlerimizi tanımlayalım...
//Arduino A1 >>> dfrobot 3.pin , Arduino A2 >>> dfrobot 2.pin 1K direnç ile

#include "DFRobotDFPlayerMini.h" //Ses çalıcımızın kütüphanesi...
DFRobotDFPlayerMini myDFPlayer; // ses çalıcımızın ismi "myDFPlayer" oldu

#include "RTClib.h" //Saat, takvim (ve kullanmasak da ısı ölçüm) içeren DS3231 modul kütüphanesi
RTC_DS3231 rtc; //Modul ismimiz: "rtc"

#include <DHT.h>  //DHT kütüphanesi https://github.com/adafruit/DHT-sensor-library
#define DHTPIN 2 //DHT 2.pine bağlı, input, pull_up vs tanımlamalar DHT kütüphanesi tarafından yapılmakta...
#define DHTTYPE DHT11 // ve DHT11 tipinde sensör bağlanıyor
DHT dht(DHTPIN, DHTTYPE); //DHT'yi pin ve tip olarak kütüphaneye tanıtalım
int temp, hum; //sıcaklık ve nem değerlerini tutacağımız değişkenlerimiz...

#include "SPI.h"  //DMD kütüphanesi ile P10 panel sürmek için gerekli https://github.com/PaulStoffregen/SPI

#include "DMD.h"  //Bu proje için ekleme yapılmış olarak:https://github.com/ronaer/P10-Led-Tabela-RTC-DHT/blob/master/DMD-master.zip;
#define DISPLAYS_ACROSS 1  //Satırda, 
#define DISPLAYS_DOWN 2// ve sütunda yeralacak P10 panel sayısı
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN); // İki satır, bir sütun
#include "SystemFont5x7.h" //DMD kütüphanesine dahil edilen font , derece ve nem fontu
#include "angka6x13.h" //DMD kütüphanesine dahil edilen font,sabit saat sayı fontu 

#include "TimerOne.h" https://github.com/PaulStoffregen/TimerOne

long ref = 0, ref1 = 0 ; // kayar yazı ve derece nem okuma için zamanlama long tipinde değişkenleri tanımlayalım ve 0 a eşitleyelim...

#define role_1 A0  //Röle A0 pininde...
//#define role_2 3  //Röle A0 pininde...

// RTC3231 Modülü, Gün olarak 0-6, 0=PAZAR ; ay olarak ise 1-12 , 1=OCAK döndürüyor...
char gun_isimleri[7][5] = {"PAZAR", "PZRTS", "SALI", "CARSB", "PERSB", "CUMA", "CMRTS"};
char ay_isimleri[13][5] = {" ", "OCAK", "SUBAT", "MART", "NiSAN", "MAYIS", "HAZRN", "TEMMZ" , "AGSTS" , "EYLuL", "EKiM", "KASIM", "ARALK"};

int gun, saat, dakika, saniye, gun_isim;//Röleyi tetiklemek amaçlı  tamsayı değişkenlerimizi tanımlayalım...
int t_max, t_min ;//max, min derece verilerini tutacağımız tamsayı değişkenlerimizi tanımlayalım...

//P10 tabelaya yazdırma amaçlı saat ve dakika tamsayı sabitlerimizi tanımlayalım...
int saat1; // saat ilk hanesi değişkeni tanımlayalım
int saat2; // saat ikinci hanesi değişkeni tanımlayalım

int dakika1; // dakika ilk hanesi değişkeni tanımlayalım
int dakika2; // dakika ikinci hanesi değişkeni tanımlayalım

int gun1,gun2; //günü iki hanede yazdırmak amaçlı iki adet değişkenimizi tanımlayalım...
int gun_hiza, ay_hiza; //gün ve ay değerlerini ortalı yazdırabilimek için...

void ScanDMD() //Timer1.attachInterrupt( ScanDMD );Timer1 kesmesi için bağlanan / attach edilen fonksiyon
{
  dmd.scanDisplayBySPI();
}

//-----------------Ve setup fonksiyonu--------------------------
void setup() {
//Serial.begin(9600); //seri iletişimi başlatalım, seri monitörden takip etmek gerekirse
  mySoftwareSerial.begin(9600); // yazılımsal serialimiz başlasın
  myDFPlayer.begin(mySoftwareSerial);// mp3 çalıcımız, software serialimiz dahilinde başlasın
  // myDFPlayer.EQ(DFPLAYER_EQ_POP);
  // myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
  // myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
  // myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
  // myDFPlayer.EQ(DFPLAYER_EQ_BASS);
  myDFPlayer.volume(22); // mp3 çalarımız ilk başta volüm seviyesi x/30
  myDFPlayer.setTimeOut(500);
  
  dht.begin(); //DHT başlasın
  delay(100);
  temp = dht.readTemperature(); // Sistem ilk çalıştığında veri alabilmek için...
  hum = dht.readHumidity();
    
  //tmp = rtc.getTemperature(); // 3231 saat modülününde yerleşik bulunan ısı sensöründen veri almak istersek...

  //RTC lib. kendi kütüphane ayarları...
#ifndef ESP8266  
  while (!Serial); // for Leonardo/Micro/Zero
#endif
  ///Saat modülü çalışmıyorsa uyar!
  if (! rtc.begin()) {
    Serial.println("Saat Modulu Yok!");
    while (1); //RTC çalışmıyorsa algoritma durur...
  }
  // Saat modülünde pil bitmişse veya güç kaybı varsa programı yüklerken saati de otomatik ayarla!
  if (rtc.lostPower()) {
    Serial.println("RTC Yeniden Ayarlanacak!");
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2020, 6, 13, 19, 35, 0));
  }
  //El ile ayarlanan değeri yüklemek>>>8.Temmuz.2020 18:25.00 gibi.Yükledikten sonra yorumu açarak tekrar yüklemeyi unutma!
  //rtc.adjust(DateTime(2020, 8, 5, 18, 25, 0)); 
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Bilgisayar saatine eşitleme (10-14 saniye yükleme gecikmesini de dikkate alın!)

  t_min = temp;// ilk ölçümde t_min ile temp aynı değerde olsun... yoksa t_min hep 0 olur...
  
 
  pinMode(role_1, OUTPUT); //Röle1 pinimiz çıkış olarak tanımlandı
  //pinMode(role_2, OUTPUT); //Röle2 pinimiz çıkış olarak tanımlandı
  digitalWrite(role_1 ,LOW);// ve açılışta LOW değeri aldı...
  //digitalWrite(role_2 ,HIGH);// ve açılışta LOW değeri aldı...

//DMD kütüphanesi ile yazdırılacak yazıların işlem görebilmesi için Timer kesmesi başlasın 
//Sonraki iki satır, RTC kütüphanesinin önünde ise çalışmaz!
  Timer1.initialize( 3000 ); 
  Timer1.attachInterrupt( ScanDMD ); //Timer kesmesini ScanDMD fonksiyonu ile attach edelim...
  dmd.clearScreen( true ); //P10 ledlerin hepsini bir söndürelim

   myDFPlayer.play(13); delay(3000); //Açılışta her şey yolunda ise 13.sıradaki ses dosyasını 3 saniye çalsın...

//LEdlerimizi kontrol amaçlı ufak bir animasyon örneği

  for(int z = 0;  z<32; z++){ dmd.drawLine(  0, z, 31, z, GRAPHICS_NORMAL ); delay(30); }
  for(int z = 0;  z<32; z++){ dmd.drawLine(  0, z, 31, z, GRAPHICS_NOR ); delay(30);  }
  for(int z = 31; z>-1; z--){ dmd.drawLine(  0, z, 31, z, GRAPHICS_NORMAL ); delay(30);  }
  for(int z = 31; z>-1; z--){ dmd.drawLine(  0, z, 31, z, GRAPHICS_NOR ); delay(30);  }
}

//_____________________Nihayet Void Loop göründü
void loop() {

  DateTime now = rtc.now(); //Bu özelliği loop başına yazmakta fayda var, yoksa doğal olarak bu satıra ihtiyaç duyan varsa üzerindeki satırlar hata verecektir...
  saat = now.hour(); // saat değişkenimiz saat modülünün saatine,
  dakika = now.minute();// dakika değişkenimiz saat modülünün dakikasına,
  saniye = now.second();// saniye değişkenimiz saat modülünün saniyesine atansın,

  saat1 = now.hour() / 10; //saatin ilk hanesini yazmak için (13/10=1 gibi)
  saat2 = now.hour() % 10; //saatin  ikinci hanesini  yazmak için (13 mod 10 >>> artan 3 gibi)

  dakika1 = now.minute() / 10;  //Dakikanın ilk hanesini  yazmak için
  dakika2 = now.minute() % 10;  //Dakikanın  ikinci hanesini  yazmak için

//Derece ve nem bilgisini yazdırmak için...
    dmd.selectFont(SystemFont5x7); //Font seçimi
    char buffer[10]; //dmd kütüphanesi ile drawString fonksiyonunda temp ve hum değerlerini yazdırabilmek için ...
    dmd.drawString(5, 0, (dtostrf(temp, 2, 0, buffer)), 2, GRAPHICS_NORMAL ); // int--> stringe çevirelim ve drawString fonksiyonunda kullanalım
    dmd.drawCircle( 18,  1,  1, GRAPHICS_NORMAL );//derece simgesi...
    dmd.drawString(21, 0, "C", 1, GRAPHICS_NORMAL ); // C harfi...
    dmd.drawString(1, 25, "Nem", 3, GRAPHICS_NORMAL ); // Nem yazalım...
    dmd.drawString(20, 25, (dtostrf(hum, 2, 0, buffer)), 2, GRAPHICS_NORMAL ); // int--> stringe çevirelim ve drawString fonksiyonunda kullanalım

//SAATi YAZDIRMAK İÇİN_____
    dmd.selectFont(angka6x13);//Font seçimi
    dmd.drawChar(0, 9, 48 + saat1, GRAPHICS_NORMAL ); //48 ekleyerek ASCII karekterini elde ediyoruz...
    dmd.drawChar(7, 9, 48 + saat2, GRAPHICS_NORMAL );
    //________saniye efekti_________
    dmd.selectFont(SystemFont5x7); // diğer fontlar ya asimetrik ya büyük ya da küçük kaldı...
    if (millis() / 1000 % 2 == 0) // her 1 saniye için
    {
      dmd.drawChar(14, 13 , ':', GRAPHICS_OR ); //iki noktayı göster
       }
    else
    {
      dmd.drawChar(14, 13 , ':', GRAPHICS_NOR ); // gösterme
        }
    //__________dakika yazdırma__________
    dmd.selectFont(angka6x13);
    dmd.drawChar(19, 9, 48 + dakika1, GRAPHICS_NORMAL );
    dmd.drawChar(26, 9, 48 + dakika2, GRAPHICS_NORMAL );
////SAATi YAZDIRMAK İÇİN_____Son___
   
////Belirlenen süre dahilinde sıcaklık ve nem bilgisini sensörden alalım...
   if (millis() - ref >= 60000) //dakikada 1 defa / 60 saniyede bir derece ve nem değerlerini alalım...
  {
    ref = millis();
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    delay(1);

////_____Ölçülen en üst ve en alt dereceleri belirlemek için______
   if (temp >= t_max) {t_max = temp;} else {t_max = t_max;} //Max değer
   if(temp >=1){ if (temp <= t_min) {t_min = temp;} else {t_min = t_min;}} else {t_min = t_min;} //Min değer
  }

  //Bazı okumaları kaçırabiliyor, bu durumda yeniden değerleri okutalım...
  if (temp == 0) {
    temp = dht.readTemperature();
    hum = dht.readHumidity();    
     delay(1);
     if (temp >= t_max) {t_max = temp;} else {t_max = t_max;} //Max değer
     if(temp >=1){ if (temp <= t_min) {t_min = temp;} else {t_min = t_min;}} else {t_min = t_min;} //Min değer
         } 

 //Ölçülen derece 18-27 aralığında ise        
if ((temp >= 18)&&(temp <= 27))
{   
  dmd.writePixel( 31, 0, GRAPHICS_NORMAL, 0 );dmd.writePixel( 31, 6, GRAPHICS_NORMAL, 0 );
  if (millis() / 2000 % 2 == 0){dmd.writePixel( 31, 3, GRAPHICS_NORMAL, 1 );}
    else {dmd.writePixel( 31, 3, GRAPHICS_NORMAL, 0 );}
    }
    

 //Ölçülen derece 28 ve daha büyük ise   
 if ((temp >= 28))
 {  
  dmd.writePixel( 31, 3, GRAPHICS_NORMAL, 0 );dmd.writePixel( 31, 6, GRAPHICS_NORMAL, 0 );
  if (millis() / 2000 % 2 == 0) {dmd.writePixel( 31, 0, GRAPHICS_NORMAL, 1 );}
    else {dmd.writePixel( 31, 0, GRAPHICS_NORMAL, 0 );}  
}

//Ölçülen derece 17 ve daha küçük ise  
if ((temp <= 17))
 {  
  dmd.writePixel( 31, 0, GRAPHICS_NORMAL, 0 );dmd.writePixel( 31, 3, GRAPHICS_NORMAL, 0 );
  if (millis() / 2000 % 2 == 0) {dmd.writePixel( 31, 6, GRAPHICS_NORMAL, 1 );}
    else {dmd.writePixel( 31, 6, GRAPHICS_NORMAL, 0 );}  
}


 gun_isim = now.dayOfTheWeek(); //

//Pazartesi ve Cuma arası çalışacak komutlar,
  if((gun_isim >=1)&&(gun_isim <= 5)) {
//Rölemiz için belirlediğimiz saat / mesai ayarlamaları;
//Rölenin istenilen zamanlarda tetiklenmesi...Cumartesi Pazar hariç; 07-12  13-17 arası>> if((gun_isim >=1)&&(gun_isim <= 5))bloğu
  if (((saat >= 7)&& (saat <= 11)) || ((saat >= 13)&& (saat <= 16))) 
  {
//Çalışma saatleri içinde röle kontağını çekelim...
    digitalWrite(role_1, HIGH); 
//Belirlediğimiz dakikalardaki anonslar... Bu anonslar da çalışma saatleri içinde olsun...
  if(( dakika== 20 || dakika== 40) && (saniye==0) ) {anons_1();}//20. ve 40. dakikalarda anons_1 fonksiyonu
  if(( dakika== 10 || dakika== 50) && (saniye==0) ) {anons_2();}
  if(( dakika== 30 ) && (saniye==0) ) {half_();}
  } 
  else {digitalWrite(role_1, LOW);}// Öğle tatilli :)
  // if ((saat >= 7)&& (saat <= 16)) {digitalWrite(role_1, HIGH);} else {digitalWrite(role_1, LOW);}//Öğle tatilsiz 7 - 17 arası kesintisiz...
 
// Saat başı mp3 çalarımızı aktive etmek ve çalma sırası ile gecikme süresini "saat_basi(int parca, int gecikme)" fonksiyonuna göndermek için... 
  if((saat==7 && dakika ==0) && (saniye==0) ) {saat_basi(1 , 10000);} //Saat 07:00:00 ise 1. parçayı 10 saniye çal bilgisini saat_basi fonksiyonuna gönderir
  if((saat==8 && dakika== 0) && (saniye==0) ) {saat_basi(2 , 8000); }
  if((saat==9 && dakika== 0) && (saniye==0) ) {saat_basi(3 , 5000); }
  if((saat==10&& dakika== 0) && (saniye==0) ) {saat_basi(4 , 8000); }
  if((saat==11&& dakika== 0) && (saniye==0) ) {saat_basi(5 , 5000); }
  if((saat==12&& dakika== 0) && (saniye==0) ) {saat_basi(6 , 8000); }
  if((saat==13&& dakika== 0) && (saniye==0) ) {saat_basi(7 , 8000); }
  if((saat==14&& dakika== 0) && (saniye==0) ) {saat_basi(8 , 4000); }
  if((saat==15&& dakika== 0) && (saniye==0) ) {saat_basi(9 , 5000); }
  if((saat==16&& dakika== 0) && (saniye==0) ) {saat_basi(10 ,7000); }
  if((saat==17&& dakika== 0) && (saniye==0) ) {saat_basi(11 ,10000);}
  if((saat==18&& dakika== 0) && (saniye==0) ) {saat_basi(12 ,30000);}
  }
//Pazartesi ve Cuma arası çalışacak komutlar sonu...

  gun1 = now.day() / 10; //günün ilk hanesini yazmak için
  gun2 = now.day() % 10; //günün  ikinci hanesini  yazmak için
  

  if ((millis() - ref1 >= 180000)&&(saniye < 54))  //180 saniye (3 dak.) de bir ...
  {
    ref1 = millis() ;
     dmd.clearScreen( true );
     dmd.selectFont(angka6x13);
     dmd.drawChar(10, -1, 48 + gun1, GRAPHICS_NORMAL );
     dmd.drawChar(17, -1, 48 + gun2, GRAPHICS_NORMAL );
     dmd.selectFont(SystemFont5x7); 
   
     if((now.month() ==1)||(now.month() == 3)||(now.month() ==10)){ay_hiza=5;} else {ay_hiza=2;}
     dmd.drawString( ay_hiza, 15, ay_isimleri[now.month()], 5, GRAPHICS_NORMAL );
     if(now.month() == 9) {dmd.writePixel( 24, 15, GRAPHICS_NORMAL, 1);dmd.writePixel( 20, 15, GRAPHICS_NORMAL, 1);} 
     else {dmd.writePixel( 24, 15, GRAPHICS_NORMAL, 0);dmd.writePixel( 20, 15, GRAPHICS_NORMAL, 0);} 
     
//Eğer haftanın günü yerine yıl bilgisi yazdırmak istenirse sonraki iki satırı aktive ederek, sonraki gelen 3. ve 4. satırı inaktive edin
     //char buffer[10];
     //dmd.drawString( 5, 25, dtostrf(now.year(), 4, 0, buffer), 4, GRAPHICS_NORMAL );
     if((now.dayOfTheWeek() ==2)||(now.dayOfTheWeek()== 5)){gun_hiza=5;}else{gun_hiza=2;}     
     dmd.drawString( gun_hiza, 24, gun_isimleri[now.dayOfTheWeek()], 5, GRAPHICS_NORMAL );
     if((now.dayOfTheWeek() ==4)||(now.dayOfTheWeek() ==3)){dmd.writePixel( 22, 31, GRAPHICS_NORMAL, 1);} else {dmd.writePixel( 22, 31, GRAPHICS_NORMAL, 0);}
     if((now.dayOfTheWeek() ==3)){dmd.writePixel( 4, 31, GRAPHICS_NORMAL, 1);} else {dmd.writePixel( 4, 31, GRAPHICS_NORMAL, 0);}

     if(now.month() == 8) {dmd.writePixel( 9, 14, GRAPHICS_NORMAL, 1);dmd.writePixel( 10, 14, GRAPHICS_NORMAL, 1);dmd.writePixel( 11, 14, GRAPHICS_NORMAL, 1);} 
      else {dmd.writePixel( 9, 14, GRAPHICS_NORMAL, 0);dmd.writePixel( 10, 14, GRAPHICS_NORMAL, 0);dmd.writePixel( 11, 14, GRAPHICS_NORMAL, 0);} 
     if(now.month() == 2){dmd.writePixel( 4, 22, GRAPHICS_NORMAL, 1);}else {dmd.writePixel( 4, 22, GRAPHICS_NORMAL, 0);}
     
     delay(3000);
     
     t_max_min_yaz (); // arkasına bir de max ve min değerleri t_max_min_yaz fonksiyonu ile yazdıralım...
   }
 }
//______________Void loop'un bittiği an...

////////////////FONKSİYONLAR ve TANIMLARI


//____En üst ve en alt sıcaklık değerleri yazdırmak için_______________
void t_max_min_yaz () {
  //t_max yazdırmak için
  dmd.clearScreen( true );
  dmd.drawBox(  0,  0, (32 * DISPLAYS_ACROSS) - 1, (16 * DISPLAYS_DOWN) - 1, GRAPHICS_NORMAL );
  dmd.selectFont( SystemFont5x7 );
  dmd.drawString( 4, 3, "Max:", 4, GRAPHICS_NORMAL );
  dmd.selectFont(angka6x13);//Font seçimi
  char buffer[10];
  dmd.drawString(5, 13, (dtostrf(t_max, 2, 0, buffer)), 2, GRAPHICS_NORMAL );
  dmd.selectFont(SystemFont5x7); //Font seçimi
  dmd.drawCircle( 21,  15,  1, GRAPHICS_NORMAL );//derece simgesi...
  dmd.drawString(24, 14, "C", 1, GRAPHICS_NORMAL ); // C harfi...
  delay(2000);
  dmd.clearScreen( true );
  //t_min yazdırmak için
  dmd.drawBox(  0,  0, (32 * DISPLAYS_ACROSS) - 1, (16 * DISPLAYS_DOWN) - 1, GRAPHICS_NORMAL );
  dmd.selectFont( SystemFont5x7 );
  dmd.drawString( 4, 3, "Min:", 4, GRAPHICS_NORMAL );
  dmd.selectFont(angka6x13);//Font seçimi  
  dmd.drawString(5, 13, (dtostrf(t_min, 2, 0, buffer)), 2, GRAPHICS_NORMAL );
  dmd.selectFont(SystemFont5x7); //Font seçimi
  dmd.drawCircle( 21,  21,  1, GRAPHICS_NORMAL );//derece simgesi...
  dmd.drawString(24, 20, "C", 1, GRAPHICS_NORMAL ); // C harfi...
  delay(2000);
  dmd.clearScreen( true );
  }
  // Fonksiyon sonu...

// Saat başlarında, istenilen gecikme süresi ayarlanabilen ses çalma fonksiyonumuz...
void saat_basi(int parca , int gecikme){
  myDFPlayer.play(parca);
  dmd.clearScreen( true );
  dmd.drawBox(  0,  0, (32 * DISPLAYS_ACROSS) - 1, (16 * DISPLAYS_DOWN) - 1, GRAPHICS_NORMAL );
  dmd.selectFont( SystemFont5x7 );
  dmd.drawString( 3, 3, "Saat", 4, GRAPHICS_NORMAL );
  dmd.selectFont(angka6x13);//Font seçimi
  dmd.drawChar(3, 13, 48 + saat1, GRAPHICS_NORMAL ); //48 ekleyerek ASCII karekterini elde ediyoruz...
  dmd.drawChar(10, 13, 48 + saat2, GRAPHICS_NORMAL );
  dmd.selectFont( SystemFont5x7 );
  dmd.drawChar(18, 13, 48 + dakika1, GRAPHICS_NORMAL );
  dmd.drawChar(24, 13, 48 + dakika2, GRAPHICS_NORMAL );
  delay(gecikme);
  dmd.clearScreen( true );
}

// belirlediğimiz dakikalarda, anons fonksiyonu 1...
void anons_1() 
{
  myDFPlayer.play(14);
  dmd.clearScreen( true );
  dmd.selectFont( SystemFont5x7 );
  dmd.drawString( 1, 1, "MASKE", 5, GRAPHICS_NORMAL );
  dmd.drawString( 1, 13, "MESFE", 5, GRAPHICS_NORMAL );
  dmd.drawString( 1, 24, "HiJYN", 5, GRAPHICS_NORMAL );
  delay(5000);
  dmd.clearScreen( true );
}
// belirlediğimiz dakikalarda, anons fonksiyonu 2...
void anons_2() 
{
  myDFPlayer.play(15);
  dmd.clearScreen( true );
  dmd.selectFont( SystemFont5x7 );
  dmd.drawString( 1, 1, "TC NO", 5, GRAPHICS_NORMAL );
  dmd.drawString( 5, 13, "ILAC", 4, GRAPHICS_NORMAL );
  dmd.drawString( 1, 24, "RAPOR", 5, GRAPHICS_NORMAL );
  dmd.writePixel( 7, 11, GRAPHICS_NORMAL, 1);
  dmd.writePixel( 25, 20, GRAPHICS_NORMAL, 1);
  delay(5000);
  dmd.clearScreen( true );
}
//Buçuklu saatlerdeki anons fonksiyonu...
void half_() 
{
  myDFPlayer.play(16);
  dmd.clearScreen( true );
  dmd.selectFont( SystemFont5x7 );
  dmd.drawString( 5, 1, "SAAT", 4, GRAPHICS_NORMAL );
  dmd.drawString( 1, 13, "BUCUK", 5, GRAPHICS_NORMAL );
  dmd.drawString( 5, 24, "OLDU", 4, GRAPHICS_NORMAL );
  dmd.writePixel( 15, 20, GRAPHICS_NORMAL, 1);
  delay(5000);
  dmd.clearScreen( true );
}
// YouTube--> TR.ALP.18 -->bilgi@ronaer.com
