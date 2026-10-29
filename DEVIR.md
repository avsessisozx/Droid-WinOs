# Droid-WinOs devir notu (başka bir yapay zekâ için)

Amaç: Android telefonda taşınabilir Windows ortamı. Windows programları (oyun değil) kurulup çalışsın, masaüstü gibi kullanılsın. Kullanıcı programcı değil, Türkçe konuşuyor. Kısa, numaralı, sade adımlar ister. Onu test kullanıcısı yapma: uygulamayı sen telefonda dene, çalışan hâlini ver.

## Depo ve dallar
- https://github.com/avsessisozx/Droid-WinOs (DroidDeck 0.6.0 çatalı, GPL-3)
- main: kod. durum: bu dosyalar (DURUM.md, DEVIR.md). Sürüm notlarına sohbet bağlantısı koyma, sadece yeni sürümün notu olsun.
- Ana dosya: app/src/main/java/com/droiddeck/launcher/WinOsActivity.kt
- Sürüm notu metni: .github/workflows/winos-release.yml

## Derleme ve yayın (bulutta, GitHub Actions)
1. main'e push edince "Build APK" çalışır (~10 dk). Gerekli işler: "Prepare verified components", "Compile APK and Android tests", "Compile and test APK". sign/packs/announce/catalog hataları bu çatalda beklenen.
2. Başarılıysa "Droid-WinOs release" iş akışını (id 400542643) run_id ve tag (örn. v0.1.14-winos) ile çalıştır. APK sürümde app-release.apk olarak çıkar.
3. gh release view/download çalışmıyor olabilir (GraphQL); REST ile indir: gh api -H "Accept: application/octet-stream" repos/avsessisozx/Droid-WinOs/releases/assets/<id>
4. Sürümü düzenleme/silme izni yok.

## Telefon (ADB, USB bağlı)
- Xiaomi Mi 10T Pro, Android 12, ADB seri no 4d738514, paket com.droiddeck.launcher
- Kurulum: install_adb_package (replace=true). Ekran: capture_adb_screen (output_directory her seferinde yeni klasör olmalı).
- Telefonun kilidi AÇIK olmalı, yoksa oturum siyah kalır. Kullanıcıdan "Stay awake" açık tutmasını iste.
- Uygulamayı uzaktan sür: start_adb_activity, action android.intent.action.MAIN, component com.droiddeck.launcher/.WinOsActivity, ekstralar:
  - winos_run (string): çalıştırılacak .exe yolu, örn. /storage/emulated/0/Download/qBittorrentPortable/App/qBittorrent/qbittorrent.exe
  - winos_args (string): programa boşlukla ayrılmış argümanlar, örn. --confirm-legal-notice
  - winos_guest_exe (string): Linux içi exe yolu (test), winos_shell (boolean, değeri "true" metni), winos_desktop
  - winos_report=true: raporu /sdcard/Android/data/com.droiddeck.launcher/files/winos-report.txt dosyasına yazar. Rapor için önce stop_adb_app, sonra bu ekstrayla soğuk başlat.
- Program açılması ~60-90 sn sürer. Ekran görüntüsünü aldıktan sonra kendin oku.

## Önemli teknik bilgiler
- Çalışma ortamı DroidDeck runtime r9 (Arch ARM64, proot, Proton Experimental ARM64, FEX). Steam istemcisi yok, gerek de yok.
- Eksik Windows DLL'leri (icuuc.dll, icuin.dll, icu.dll gibi) kullanıcı C:\Windows\System32'den kopyalar, telefonda İndirilenler/WinOs-DLL klasörüne koyar; uygulama otomatik yükler. Ana ekranda eksik DLL adı yazar.
- explorer /desktop=shell (görev çubuğu) telefonda çöküyor; dosya yöneticisi penceresi açılıyor.
- Oyun tuşları ve fps kutusu SessionPrefs.setOscMode(NEVER) ve setHudEnabled(false) ile gizleniyor (v0.1.13). Klavye düğmesi de gitmiş olabilir: kontrol et.

## Sıradaki işler
1. v0.1.13'ü kur, gizlemenin çalıştığını ve ekran klavyesine/dokunmaya ulaşılabildiğini doğrula; klavye düğmesi gittiyse geri getir.
2. Dokunma = tıklama, yazı yazma, ses doğrula.
3. Görev çubuklu masaüstü için başka yol bul.
4. Kurulum sonrası çöken diğer programları (Office türü) dene; hata raporunu oku, düzelt.
5. Her sürümde DURUM.md'yi güncelle (sürüm, tarih, doğrulandı/doğrulanmadı).
