# Droid-WinOs durum

Sürüm: v0.1.18-winos (görev çubuğu pencerelerin üstünde kalıyor)
Tarih: 2026-10-30

## Doğrulandı (gerçek cihazda: Xiaomi Mi 10T Pro, Android 12, ADB ile)
- APK kuruluyor, uygulama açılıyor, program listesi çalışıyor.
- Kendi test .exe'si (MessageBox) açılıyor; ekrana dokununca OK kapanıyor (dokunma = tıklama).
- Dokunma doğrudan: parmağınla dokunduğun yere tıklanır (oturumlar TOUCH_DIRECT ile başlıyor).
- Oyun tuşları ve fps kutusu (HUD) gizli; oturumda sadece program penceresi görünüyor.
- Ekran klavyesi düğmesi sağ altta; dokununca "PC klavyesi" açılıyor, tuşlar programa yazı yazıyor.
- Yazma: wine notepad ve Notepad++ 8.9.8.1 (32 bit) içine yazıldı.
- 32 bit program: Notepad++ 8.9.8.1 (x86) ARM64 Proton + FEX üzerinde açıldı.
- Görev çubuğu (WinOsBar): "Başlat" menüsü büyük satırlarla açılıyor, seçilen program başlıyor, açık pencereler için düğme çıkıyor, düğme pencereyi öne getiriyor. Notepad++ gibi tam ekran bir pencere açıkken çubuk görünür kalıyor.
- Ses: WinOsBeep.exe (winmm waveOut, 440 Hz, 8 sn) çalındı; waveOutOpen başarıyla açıldı (open=0) ve çalma tamamlandı; bu sırada Android'de uygulamanın AAudio çıkışı (USAGE_GAME) "started" görünüyor.
- qBittorrent Portable kurucusu ve ana penceresi (Qt tabanlı, büyük program).
- Eksik DLL mesajı ve İndirilenler/WinOs-DLL otomatik yükleme.

## Doğrulanmadı
- Sesin hoparlörden gerçekten duyulması (kulakla kontrol gerekiyor; ses yolu açık).
- Görev çubuğu her zaman üstte değil: bazı tam ekran pencereler (özellikle ilk açılışta) çubuğu örtebiliyor; pencereyi biraz oynatınca/çubuk yenilenince düzeliyor. Kalıcı çözüm için Win32 "appbar" benzeri bir yol gerekiyor.
- Office benzeri büyük programlar (Notepad++ 32 bit ve qBittorrent denenmiş durumda; Word/Excel türü bir program kurulamadı, indirme adresleri kapalı).
- Kablosuz hata ayıklama eşleştirme sorunu.

## Bilinen sorunlar
- Telefonun kilidi açık olmalı; ekran uykuya dalarsa oturum siyah kalır. `adb shell svc power stayon usb` ekranı açık tutar.
- MIUI'de "USB hata ayıklama (Güvenlik ayarları)" kapalıysa bilgisayardan `input tap` / `input text` çalışmaz (INJECT_EVENTS hatası).
- Çalışan örneğe gönderilen intent bazen oturum açmıyor; önce uygulamayı durdurup soğuk başlatmak gerekiyor.
- Program pencereleri 1600x720 masaüstünde küçük; tam ekran açılan program çubuğu örtebiliyor.
- Programların kendi DLL'leri yoksa açılmaz; Windows DLL'leri İndirilenler/WinOs-DLL klasörüne konur.

## Test yöntemi (bilgisayar bağlı telefona uzaktan)
- Intent: com.droiddeck.launcher/.WinOsActivity; ekstralar: winos_run=<exe yolu>, winos_args, winos_guest_exe, winos_shell, winos_desktop, winos_report=true.
- Masaüstü: "Windows masaüstü (görev çubuğu)" düğmesi; WinOsBar.exe APK içinden Download/WinOs-Bar klasörüne çıkarılır.
- Yazma testi: `adb shell input text 'Merhaba%sDroid'` (boşluk %s), Enter: `input keyevent 66`.
- cmd.exe ile teşhis: winos_guest_exe=/root/.local/share/Steam/steamapps/compatdata/2564280011/pfx/drive_c/windows/system32/cmd.exe
- Sürüm: "Build APK" bitince "Droid-WinOs release" iş akışı run_id + tag ile çalıştırılır (~20 dakika).

## Kalanlar
- Görev çubuğunun her zaman üstte kalması (kalıcı çözüm).
- Sesin duyulduğunu doğrula.
- Office benzeri büyük programları dene.
- Uygulama adı/simgesi Droid-WinOs.
