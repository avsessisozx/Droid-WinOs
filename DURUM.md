# Droid-WinOs durum

Sürüm: v0.1.15-winos
Tarih: 2026-10-29

## Doğrulandı (gerçek cihazda: Xiaomi Mi 10T Pro, Android 12, ADB ile)
- APK kuruluyor, uygulama açılıyor, program listesi çalışıyor.
- Kendi test .exe'si (MessageBox) açılıyor; ekrana dokununca OK kapanıyor (dokunma = tıklama).
- Oyun tuşları ve fps kutusu (HUD) gizli: oturumda sadece program penceresi görünüyor.
- Ekran klavyesi düğmesi sol altta duruyor; dokununca "PC klavyesi" açılıyor ve tuşlar programa yazı yazıyor.
- Yazma: wine notepad ve Notepad++ 8.9.8.1 (32 bit) içine "Merhaba Droid 123ab" yazıldı.
- 32 bit program: Notepad++ 8.9.8.1 (x86) ARM64 Proton + FEX üzerinde açıldı.
- qBittorrent Portable kurucusu açılıyor; kurulu qBittorrent ana penceresi açılıyor (menü, araç çubuğu, liste).
- Eksik DLL mesajı ana ekranda görünüyor; İndirilenler/WinOs-DLL klasöründeki DLL'ler otomatik yükleniyor.
- Hata raporu düğmesi çalışıyor.
- Ses kanalı açık: oturum sırasında uygulamanın AAudio çıkışı (USAGE_GAME) "started" durumda.

## Doğrulanmadı
- Sesin hoparlörden gerçekten duyulması (ses çıkaran bir programla denenmeli).
- Görev çubuğu/başlat menülü tam masaüstü: Wine explorer `explorer /desktop=shell` ve `/desktop=WinOs,1600x720` pencere açmıyor (explorer.exe çalışıyor, masaüstü görünmüyor). "Windows masaüstünü aç" şimdilik dosya yöneticisi penceresi açıyor.
- Office gibi büyük programlar (yalnızca Notepad++ denendi).
- Kablosuz hata ayıklama eşleştirme sorunu (DroidDeck içinde) incelenmedi.

## Bilinen sorunlar
- Telefonun kilidi açık olmalı; ekran uykuya dalarsa oturum siyah kalır.
- MIUI'de "USB hata ayıklama (Güvenlik ayarları)" kapalıysa bilgisayardan `input tap` / `input text` çalışmaz (INJECT_EVENTS hatası). Uzaktan test için bu ayar açık olmalı.
- Çalışan örneğe gönderilen intent bazen oturum açmıyor; önce uygulamayı durdurup soğuk başlatmak gerekiyor.
- Pencere 1600x720 sanal masaüstünde küçük.
- Programların kendi DLL'leri (ör. GetUName.dll) yoksa açılmaz; Windows DLL'leri kullanıcı C:\Windows\System32'den kopyalar.

## Test yöntemi (bilgisayar bağlı telefona uzaktan)
- Intent: com.droiddeck.launcher/.WinOsActivity, ekstralar: winos_run=<exe yolu>, winos_args, winos_guest_exe, winos_shell, winos_desktop, winos_report=true (rapor: /sdcard/Android/data/com.droiddeck.launcher/files/winos-report.txt).
- Yazma testi: `adb shell input text 'Merhaba%sDroid'` (boşluk için %s); Enter için `input keyevent 66`.
- cmd.exe ile teşhis: winos_run=<herhangi bir exe> + winos_guest_exe=/root/.local/share/Steam/steamapps/compatdata/2564280011/pfx/drive_c/windows/system32/cmd.exe
- Sürüm: GitHub Actions "Build APK" bitince "Droid-WinOs release" iş akışını run_id ve tag ile çalıştır (derleme ~20 dakika).

## Kalanlar
- Görev çubuğu için alternatif (üçüncü taraf kabuk ya da kendi görev çubuğu).
- Sesin duyulduğunu doğrula.
- Office benzeri büyük programları dene.
- Uygulama adı/simgesi Droid-WinOs olarak değiştirilecek.
