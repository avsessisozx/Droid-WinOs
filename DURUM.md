# Droid-WinOs durum

Sürüm: v0.1.12-winos (v0.1.13 derleniyor: oyun tuşları ve fps kutusu gizleme)
Tarih: 2026-10-29

## Doğrulandı (gerçek cihazda: Xiaomi Mi 10T Pro, Android 12, ADB ile)
- APK kuruluyor, uygulama açılıyor, program listesi çalışıyor
- Kendi test .exe'si (MessageBox) açılıyor
- qBittorrent Portable kurucusu açılıyor; kurulu qBittorrent ana penceresi açılıyor (menü, araç çubuğu, liste)
- "Windows masaüstünü aç" düğmesi: Windows dosya yöneticisi penceresi açılıyor
- Eksik DLL mesajı ana ekranda görünüyor; İndirilenler/WinOs-DLL klasöründeki DLL'ler otomatik yükleniyor
- Hata raporu düğmesi çalışıyor

## Doğrulanmadı
- Dokunma ile tıklama, yazı yazma (ekran klavyesi), ses
- Görev çubuğu/başlat menülü tam masaüstü (explorer /desktop=shell telefonda çöküyor)
- Office gibi büyük programlar, 32 bit programlar
- Kablosuz hata ayıklama eşleştirme sorunu (DroidDeck içinde) incelenmedi

## Bilinen sorunlar
- Ekran kilitliyken oturum siyah kalıyor (telefon kilidi açık olmalı)
- Gizlenen oyun tuşlarıyla birlikte klavye düğmesi de gitmiş olabilir: kontrol et, gerekirse sadece klavye düğmesini geri koy
- Pencere 1600x720 sanal masaüstünde küçük
- Programların kendi DLL'leri (ör. GetUName.dll) yoksa açılmaz; Windows DLL'leri kullanıcı C:\Windows\System32'den kopyalar

## Test yöntemi (bilgisayar bağlı telefona uzaktan)
- Intent: com.droiddeck.launcher/.WinOsActivity, ekstralar: winos_run=<exe yolu>, winos_args, winos_guest_exe, winos_shell, winos_desktop, winos_report=true (rapor: /sdcard/Android/data/com.droiddeck.launcher/files/winos-report.txt)
- Sürüm: GitHub Actions "Build APK" bitince "Droid-WinOs release" iş akışını run_id ve tag ile çalıştır

## Kalanlar
- Dokunma ve klavye doğrulaması, sonra kalıcı çözüm
- Görev çubuklu masaüstü için başka yol (şu an dosya yöneticisi)
- Uygulama adı/simgesi Droid-WinOs olarak değiştirilecek
- Kurulum sonrası çöken başka programları dene (Office türü)
