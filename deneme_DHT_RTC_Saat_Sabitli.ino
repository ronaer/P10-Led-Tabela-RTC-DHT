/*
  https://github.com/ronaer/P10-Led-Tabela-RTC-DHT
  YouTube >>> TR.ALP.18
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


float temp;
float hum ;

#include "RTClib.h" //Saat, takvim ve ısı ölçüm içeren DS3231 modul kütüphanesi
RTC_DS3231 rtc; //Modul ismimiz

char gun_isimleri[7][10] = {"PAZAR", "PAZARTESi", "SALI", "CARSAMBA", "PERSEMBE", "CUMA", "CUMARTESi"};
char ay_isimleri[13][10] = {" ", "OCAK", "SUBAT", "MART", "NiSAN", "MAYIS", "HAZiRAN", "TEMMUZ" , "AGUSTOS" , "EYLUL", "EKiM", "KASIM", "ARALIK"};
// Gün olarak 0-6, 0=PAZAR ; ay olarak ise 1-12 , 1=OCAK döndürülüyor...
    
#define DISPLAYS_ACROSS 1  //Satırda ve sütunda yeralacak P10 panel sayısı
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


void ScanDMD() //P10 led tabela için tarama fonksiyonu
{
  dmd.scanDisplayBySPI();
}


void setup() {

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
  //rtc.adjust(DateTime(2020, 04, 5, 12, 33, 0));
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  dht.begin(); //DHT başlasın
  Serial.begin(9600); //Seri portan bi bakalım her şey yolunda mı diye seri port başlasın...

  Timer1.initialize( 3000 ); //DMD kütüphanesi ile yazdırılacak yazıların işlem görebilmesi için Timer kütüphanesi başlasın
  Timer1.attachInterrupt( ScanDMD );
  dmd.clearScreen( true ); //P10 ledlerin hepsini bir söndürelim

}


//_____________________Nihayet Void Loop göründü
void loop() {



  DateTime now = rtc.now(); //Bu özelliği loop başına yazmakta fayda var, yoksa doğal olarak bu satıra ihtiyaç duyan varsa üzerindeki satırlar hata verecektir...

  saat1 = now.hour() / 10; //saatin ilk hanesini yazmak için
  saat2 = now.hour() % 10; //saatin  ikinci hanesini  yazmak için

  dakika1 = now.minute() / 10;  //Dakikanın ilk hanesini  yazmak için
  dakika2 = now.minute() % 10;  //Dakikanın  ikinci hanesini  yazmak için

  saniye = now.second(), DEC;

  char *yaz ;
  yaz = "youtube TR.ALP.18";

  int gun ;
  gun =  now.day();
  String strgun;
  if (gun < 10) strgun = '0' + String(gun);   else strgun = gun ;

  /*  buraya kayan yazıyı -String textToScroll- yaz, void drawText olmadan çalışmaz, tek satırda da yazılabilir
       araya (;) koymadan alt alta da yazılabilir...
      içine string değişkeni olacak şekilde 255 karektere kadar veri yazılabilir...
  */

  String textToScroll =

    "BUGUN:"  +
    strgun +
    //String(now.day()) +
    '/' +
    String(ay_isimleri[now.month()]) + '/' +
    String(now.year()) + '/' +
    String(gun_isimleri[now.dayOfTheWeek()]) + ',' +
    "Saat:" + saat1 + saat2 + ':' + dakika1 + dakika2 + "," +
    "Derece:" + String(temp, 1) + "'C," + "Nem:%" + String(hum, 1) + ", " +
    //String(rtc.getTemperature()) + "'C," + //RTC3231 içindeki sıcaklığa gerek yok şimdilik
    yaz ;
  // String textToScroll (;) ile sonlandı...

  //Serial.print(now.day());

  if (saniye == 8) //ayarlanan saniye  değişkeninde kayar yazıya geçer...
  {
    temp = dht.readTemperature();
    hum = dht.readHumidity();
  }
  if (saniye ==10)
  {

    drawText(textToScroll);//Yazıyı kaydırmak için drawtext() şart, void drawtext sonrasında çalışır...
  }
  else {

    dmd.selectFont(angka6x13);


    ///______Sabit saate geçelim
    dmd.drawChar(0, 1, 48 + saat1, GRAPHICS_NORMAL );
    dmd.drawChar(7, 1, 48 + saat2, GRAPHICS_NORMAL );

    //________saniye efekti_________

    dmd.selectFont(SystemFont5x7); // diğer fontlar ya asimetrik ya büyük ya da küçük kaldı...
    if (millis() / 1000 % 2 == 1)
    {
      dmd.drawChar(14, 5 , ':', GRAPHICS_NOR );
    }
    else
    {
      dmd.drawChar(14, 5 , ':', GRAPHICS_OR );
    }

    //__________dakika yazdırma__________
    dmd.selectFont(angka6x13);
    dmd.drawChar(19, 1, 48 + dakika1, GRAPHICS_NORMAL );
    dmd.drawChar(26, 1, 48 + dakika2, GRAPHICS_NORMAL );

  }

}
//______________Void loop'un bittiği an...

// ______________Kayan yazı için  şart olan fonksiyon!  void drawText( String dispString )
void drawText( String dispString )
{
  //dmd.clearScreen( true );
  dmd.selectFont( SystemFont5x7 ); // fontu buradan tanımlayabiliriz...
  char newString[256];
  int sLength = dispString.length();
  dispString.toCharArray( newString, sLength + 1 );
  dmd.drawMarquee( newString , sLength , ( 32 * DISPLAYS_ACROSS ) + 1 , 5);
  long start = millis();
  long timer = start;
  long timer2 = start;
  boolean ret = false;
  while ( !ret ) {
    if ( ( timer + 60 ) < millis() ) { //kayma hızı < hızlı, > yavaş
      ret = dmd.stepMarquee( -1 , 0 );
      timer = millis();

    }
  }
}
//_____________________Kayan yazı fonksiyon sonu

// YouTube--> TR.ALP.18  abone olursanız sevinirim...
