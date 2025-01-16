# adcproject1
Proje Özeti

Bu proje, TM4C123G mikrodenetleyicisinde bir ADC, UART ve Timer kullanarak sıcaklık izleme ve iletişim uygulamasını gerçekleştirmektedir. Program, bir ADC'ye bağlı NTC termistörden sıcaklık verilerini okur, Steinhart-Hart denklemini kullanarak sıcaklığı hesaplar ve UART üzerinden DWIN ekranına iletir. Kod ayrıca bir zamanlayıcı, ADC ve UART kesintilerini yönetme işlevselliğini içerir.

Özellikler

Sıcaklık Ölçümü:

ADC pinine bağlı bir NTC termistörden analog veri okur.

ADC değerini Steinhart-Hart denklemini kullanarak Celsius cinsinden sıcaklığa dönüştürür.

İşlenmiş sıcaklık verilerini DWIN ekranına gönderir.

UART İletişimi:

DWIN ekranıyla sayfa kontrolü ve veri güncellemeleri için veri iletişimini yönetir.

Gelen verileri dinler ve sayfa geçişlerini tetiklemek veya komutları onaylamak için kullanır.

Kesme Yönetimi:

Zamanlayıcı Kesmesi: Belirli aralıklarla ADC dönüşüm sürecini tetikler (1 saniye).

ADC Kesmesi: ADC değerlerini işler ve sıcaklığı hesaplar.

UART Kesmesi: DWIN ekranından gelen verileri yönetir ve sayfa geçişlerini gerçekleştirir.

DWIN Ekran Entegrasyonu:

Dinamik sayfa geçişlerini destekler ve UART komutları kullanarak belirli veri alanlarını günceller.

Donanım Gereksinimleri

TM4C123G LaunchPad

DWIN Ekranı

NTC Termistör

Dirençler (örneğin, voltaj bölücü için 10kΩ)

Harici güç kaynağı (gerekirse çevre birimler için)

Yazılım Kurulumu

Gereksinimler

Code Composer Studio (CCS)

TivaWare Kütüphanesi (Projedeki driverlib ve inc dosyalarının doğru şekilde dahil edildiğinden emin olun)

Dosya Yapısı

main.c: Ana uygulama mantığını içerir.

TivaWare Kütüphaneleri:

driverlib/: Tiva C Serisi için çevre birimi sürücü kütüphanesi.

inc/: TM4C123G için donanım başlık dosyaları.

Kütüphane Yapılandırması

Derleyiciye aşağıdaki dizinleri dahil ettiğinizden emin olun:

C:/ti/TivaWare_C_Series-2.2.0.295/driverlib

C:/ti/TivaWare_C_Series-2.2.0.295/inc

Kod Açıklaması

Anahtar Değişkenler

ntc_deger: Ortalama almak için biriken ADC değerlerini tutar.

ntcCnt: Alınan ADC okumalarının sayısını takip eden sayaç.

ntcLastValue: Ortalama ADC değerini tutar.

Temp_C: Hesaplanan sıcaklık değerini (Celsius cinsinden) tutar.

uartBuffer: Gelen UART verilerini tutan tampon.

sendData: Verilerin DWIN ekranına gönderilmeye hazır olduğunu belirten bayrak.

Önemli Fonksiyonlar

timerInterrupt

Zamanlayıcı kesme bayrağını temizler.

ADC'nin yeni bir dönüşüm başlatmasını tetikler.

adcInterrupt

ADC değerini okur ve üç ardışık okumanın ortalamasını alır.

NTC direncini ve Steinhart-Hart denklemini kullanarak sıcaklığı hesaplar.

Sıcaklığı Celsius'a dönüştürür.

sendData bayrağını 1 olarak ayarlar, bu da verinin gönderilmeye hazır olduğunu belirtir.

uartinterrupt

DWIN ekranından gelen verileri işler.

Sayfa geçişlerini kontrol etmek için belirli komut desenlerini kontrol eder.

Komutları işledikten sonra UART tamponunu sıfırlar.

dwinPageControl(uint8_t PageID)

DWIN ekranına sayfalar arasında geçiş yapmak için bir komut gönderir.

dwinPtext(uint16_t add, uint16_t value)

Belirli bir adrese sıcaklık verilerini göndermek için kullanılır.

Ana Döngü

Sürekli olarak sendData bayrağını kontrol eder.

sendData ayarlandığında sıcaklık verilerini DWIN ekranına gönderir.

Kullanım Talimatları

Kurulum

NTC termistörü bir voltaj bölücü devresi kullanarak bir ADC pinine (örneğin, PE3) bağlayın.

DWIN ekranını UART1'e bağlayın (PB0 RX için, PB1 TX için).

Mikrodenetleyicinin çevre birimleriyle doğru şekilde bağlandığından ve güç verildiğinden emin olun.

Programın Çalıştırılması

Programı Code Composer Studio ile TM4C123G'ye yükleyin.

Zamanlayıcı, periyodik olarak sıcaklık ölçümleri tetikleyecektir.

Sıcaklık verileri DWIN ekranına gönderilecektir.

UART komutları ile ekran sayfaları arasında geçiş yapabilirsiniz.

Notlar

Steinhart-Hart Katsayıları:

Kullanılan katsayılar (c1, c2, c3), NTC termistöre özgüdür. Termistörünüzün datasheet'ine göre bu değerleri ayarlayın.

UART Baud Hızı:

UART, 115200 baud hızıyla yapılandırılmıştır. DWIN ekranının aynı hızda ayarlandığından emin olun.

Kesme Önceliği:

Kesintilerin NVIC'de doğru şekilde yapılandırıldığından ve etkinleştirildiğinden emin olun.

Hata Ayıklama:

ADC değerlerini ve hesaplanan sıcaklıkları doğrulamak için UART çıktısını veya bir hata ayıklayıcıyı kullanın.

Örnek Çıktı

DWIN Ekranı:

Gerçek zamanlı olarak gösterilen sıcaklık değeri.

Gelen komutlara bağlı olarak dinamik sayfa geçişleri.

UART Komutları:

0x5A 0xA5 0x07 0x82 ...: Sayfa geçiş komutu.

0x5A 0xA5 0x05 0x82 ...: Veri güncelleme komutu.
