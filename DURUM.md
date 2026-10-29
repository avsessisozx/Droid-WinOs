# Droid-WinOs durum

Sürüm: v0.1.8-winos
Tarih: 2026-10-29

## Doğrulandı (gerçek cihazda)
- Cihaz: Xiaomi Mi 10T Pro, Android 12
- APK kuruluyor, uygulama açılıyor, program listesi çalışıyor
- Küçük bir Windows .exe (MessageBox) açıldı ve ekranda göründü (Wine/Proton ARM64 + FEX)
- Hata raporu düğmesi çalışıyor (raporu panoya kopyalar ve dosyaya yazar)

## Doğrulanmadı
- Gerçek masaüstü programları (ör. kendi DLL'ini isteyen programlar)
- Klavye/fare kullanımı, ses, dosya erişimi
- Kablosuz hata ayıklama eşleştirme sorunu (DroidDeck içinde) incelenmedi

## Bilinen sorunlar
- Ekran kilitliyken pencere oluşmuyor (siyah ekran)
- Ekranda DroidDeck'in oyun kumandası ve HUD katmanı görünüyor
- Pencere 1600x720 sanal masaüstünde küçük görünüyor

## Kalanlar
- Kumanda/HUD katmanını gizle, program için uygun ekran boyutu
- Uygulama adı/simgesi Droid-WinOs olarak değiştirilecek
