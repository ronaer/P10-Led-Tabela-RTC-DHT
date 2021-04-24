/*
  https://github.com/ronaer/P10-Led-Tabela-RTC-DHT/blob/master/Ertan_Hocam_1.ino
  YouTube >>> Dr.TRonik...
  Açık kaynak lisansı altında kullanılabilir, paylaşılabilir...
*/
//Eklemeler ve global değişken tanımlamaları...

#include <DHT.h>  //DHT kütüphanesi https://github.com/adafruit/DHT-sensor-library
#include <Wire.h> //Tek kablo üzerinden bilgi aktarma için gerekli kütüphane https://github.com/PaulStoffregen/Wire
#include "SPI.h"  //DMD kütüphanesi için gerekli https://github.com/PaulStoffregen/SPI
#include "DMD.h"  //Bu proje için ekleme yapılmış olarak:https://github.com/ronaer/P10-Led-Tabela-RTC-DHT/blob/master/DMD-master.zip;
#include "TimerOne.h" https://github.com/PaulStoffregen/TimerOne
#include "SystemFont5x7.h" //DMD kütüphanesine dahil edilen font , kayan yazı fontu
#include "angka6x13.h" //DMD kütüphanesine dahil edilen font,sabit saat sayı fontu 
#include "TimesNewRoman16b.h"


float t, h;
boolean ilk_calisma = true ;

#include "RTClib.h" //Saat, takvim ve ısı ölçüm içeren DS3231 modul kütüphanesi
RTC_DS3231 rtc; //Modul ismimiz

//char gun_isimleri[7][10] = {"PAZAR", "PAZARTESi", "SALI", "CARSAMBA", "PERSEMBE", "CUMA", "CUMARTESi"};
//char gun_isimleri_1[7][5] = {"PAZAR", "PZRTS", "SALI", "CARSB", "PERSB", "CUMA", "CMRTS"};
char ay_isimleri[13][10] = {" ", "OCAK", "SUBAT", "MART", "NiSAN", "MAYIS", "HAZiRAN", "TEMMUZ" , "AGUSTOS" , "EYLUL", "EKiM", "KASIM", "ARALIK"};
// Gün olarak 0-6, 0=PAZAR ; ay olarak ise 1-12 , 1=OCAK döndürülüyor...

#define DISPLAYS_ACROSS 3  //Satırda ve sütunda yeralacak P10 panel sayısı
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN); // Bir satır, bir sütun

#define DHTPIN 2 //DHT 2.pine bağlı
#define DHTTYPE DHT22 // ve DHT22 bağlanıyor
DHT dht(DHTPIN, DHTTYPE); //DHT'yi kütüphaneye tanıtalım

int saniye;
int saat1; // saat ilk hanesi değişkeni tanımlayalım
int saat2; // saat ikinci hanesi değişkeni tanımlayalım

int dakika1; // dakika ilk hanesi değişkeni tanımlayalım
int dakika2; // dakika ikinci hanesi değişkeni tanımlayalım

int gun1, gun2;
int ay1, ay2;



void ScanDMD() //P10 led tabela için tarama fonksiyonu
{
  dmd.scanDisplayBySPI();
}


void setup() {
  dht.begin(); //DHT başlasın

#ifndef ESP8266 //ESP kullanılıyor ise ayarları, 
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  ///Saat modülü çalışmıyorsa uyar!
  if (! rtc.begin()) {
    Serial.println("Saat Modulu Yok!");
    while (1); //RTC çalışmıyorsa algoritma durur...
  }
  /// Saat modülünde pil bitmişse veya güç kaybı varsa programı yüklerken saati de otomatik ayarla!
  if (rtc.lostPower()) {
    Serial.println("RTC Yeniden Ayarlanacak!");
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2014, 1, 1, 3, 0, 0));
  }
  //rtc.adjust(DateTime(2021, 04, 24, 11, 17, 0));
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.begin(9600); //Seri portan bi bakalım her şey yolunda mı diye seri port başlasın...

  Timer1.initialize( 2000 );
  Timer1.attachInterrupt( ScanDMD );
  dmd.clearScreen( true );
  dmd.selectFont(TimesNewRoman16b);
  dmd.drawChar(5, 1 , '?', GRAPHICS_OR );
}


//_____________________Nihayet Void Loop göründü
void loop() {
  if (ilk_calisma == true)
  {
    t = dht.readTemperature();
    h = dht.readHumidity();
  }
  ilk_calisma = false;
  //



  DateTime now = rtc.now(); //Bu özelliği loop başına yazmakta fayda var, yoksa doğal olarak bu satıra ihtiyaç duyan varsa üzerindeki satırlar hata verecektir...
  saat1 = now.hour() / 10; //saatin ilk hanesini yazmak için
  saat2 = now.hour() % 10; //saatin  ikinci hanesini  yazmak için
  dakika1 = now.minute() / 10;  //Dakikanın ilk hanesini  yazmak için
  dakika2 = now.minute() % 10;  //Dakikanın  ikinci hanesini  yazmak için
  saniye = now.second(), DEC;
  gun1 = now.day() / 10; //günün ilk hanesini yazmak için
  gun2 = now.day() % 10; //günün  ikinci hanesini  yazmak için
  ay1 = now.month() / 10;
  ay2 = now.month() % 10;

  char *yaz ;
  yaz = "MOTORLU ARACLAR TEKNOLOJiSi... ";

  int gun ;
  gun =  now.day();
  String strgun;
  if (gun < 10) strgun = '0' + String(gun);   else strgun = gun ;

  /*  buraya kayan yazıyı -String textToScroll- yaz, void drawText olmadan çalışmaz, tek satırda da yazılabilir
       araya (;) koymadan alt alta da yazılabilir...
      içine string değişkeni olacak şekilde 255 karektere kadar veri yazılabilir...
  */

  String textToScroll =

    yaz +
    strgun + '/' + String(ay_isimleri[now.month()]) + '/' + String(now.year()) //+ '/' +String(gun_isimleri[now.dayOfTheWeek()])
    + " "
    + saat1 + saat2 + ':' + dakika1 + dakika2
    + " "
    + String(t, 1) + "'C"
    + " "
    + "%" + String(h, 1) + "... " ;

  // String textToScroll (;) ile sonlandı...



  //
  if (saniye == 8) //ayarlanan saniye  değişkeninde kayar yazıya geçer...
  {

    t = dht.readTemperature();
    h = dht.readHumidity();
  }


  if (saniye == 10)
  {

    drawText(textToScroll);//Yazıyı kaydırmak için drawtext() şart, void drawtext sonrasında çalışır...
  }
  else {

    dmd.selectFont(angka6x13);


    ///______Sabit saate geçelim
    dmd.drawChar(28, 1, 48 + saat1, GRAPHICS_NORMAL );
    dmd.drawChar(35, 1, 48 + saat2, GRAPHICS_NORMAL );

    //________saniye efekti_________

    dmd.selectFont(SystemFont5x7); // diğer fontlar ya asimetrik ya büyük ya da küçük kaldı...
    if (millis() / 1000 % 2 == 1)
    {
      dmd.drawChar(42, 5 , ':', GRAPHICS_NOR );
    }
    else
    {
      dmd.drawChar(42, 5 , ':', GRAPHICS_OR );
    }

    //__________dakika yazdırma__________
    dmd.selectFont(angka6x13);
    dmd.drawChar(47, 1, 48 + dakika1, GRAPHICS_NORMAL );
    dmd.drawChar(55, 1, 48 + dakika2, GRAPHICS_NORMAL );

    dmd.selectFont(angka6x13);//Font seçimi
    char buffer[10];
    dmd.drawString(0, 1, (dtostrf(t, 2, 0, buffer)), 2, GRAPHICS_NORMAL );
    dmd.selectFont(SystemFont5x7); //Font seçimi
    dmd.drawCircle( 15,  1,  1, GRAPHICS_NORMAL );//derece simgesi...
    dmd.drawString(18, 1, "C", 1, GRAPHICS_NORMAL ); // C harfi...

    dmd.selectFont(SystemFont5x7);
    dmd.drawChar(67, 1, 48 + gun1, GRAPHICS_NORMAL );
    dmd.drawChar(73, 1, 48 + gun2, GRAPHICS_NORMAL );
    dmd.drawString(79, 1, "/", 1, GRAPHICS_NORMAL );
    dmd.drawChar(85, 1, 48 + ay1, GRAPHICS_NORMAL );
    dmd.drawChar(91, 1, 48 + ay2, GRAPHICS_NORMAL );
    //char buffer[10];
    //dmd.drawString( 68, 9, gun_isimleri_1[now.dayOfTheWeek()], 5, GRAPHICS_NORMAL );
    dmd.drawString( 70, 9, dtostrf(now.year(), 4, 0, buffer), 4, GRAPHICS_NORMAL );

  }

}
//______________Void loop'un bittiği an...

// ______________Kayan yazı için  şart olan fonksiyon!  void drawText( String dispString )
void drawText( String dispString )
{
  //dmd.clearScreen( true );
  dmd.selectFont( TimesNewRoman16b ); // fontu buradan tanımlayabiliriz...
  char newString[256];
  int sLength = dispString.length();
  dispString.toCharArray( newString, sLength + 1 );
  dmd.drawMarquee( newString , sLength , ( 32 * DISPLAYS_ACROSS ) + 1 , 2);
  long start = millis();
  long timer = start;
  long timer2 = start;
  boolean ret = false;
  while ( !ret ) {
    if ( ( timer + 55 ) < millis() ) { //kayma hızı < hızlı, > yavaş
      ret = dmd.stepMarquee( -1 , 0 );
      timer = millis();

    }
  }
}
//_____________________Kayan yazı fonksiyon sonu

// YouTube--> Dr.TRonik ...
