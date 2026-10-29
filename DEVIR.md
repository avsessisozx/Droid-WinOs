# Droid-WinOs devir notu (başka bir yapay zekâ için)

Amaç: Android telefonda taşınabilir Windows ortamı. Windows programları (oyun değil) kurulup çalışsın, masaüstü gibi kullanılsın. Kullanıcı programcı değil, Türkçe konuşuyor. Kısa, numaralı, sade adımlar ister. Onu test kullanıcısı yapma: uygulamayı sen telefonda dene, çalışan hâlini ver.

## Depo ve dallar
- https://github.com/avsessisozx/Droid-WinOs (DroidDeck 0.6.0 çatalı, GPL-3)
- main: kod. durum: bu dosyalar (DURUM.md, DEVIR.md). Sürüm notlarına sohbet bağlantısı koyma, sadece yeni sürümün notu olsun.
- Ana dosyalar: app/src/main/java/com/droiddeck/launcher/WinOsActivity.kt (program listesi, oturum başlatma), app/src/main/java/com/droiddeck/launcher/SessionActivity.kt (oturum ekranı, klavye düğmesi).
- Sürüm notu metni: .github/workflows/winos-release.yml

## Derleme ve yayın (bulutta, GitHub Actions)
1. main'e push edince "Build APK" çalışır (~20 dk). Gerekli işler: "Prepare verified components", "Compile APK and Android tests", "Compile and test APK". sign/packs/announce/catalog hataları bu çatalda beklenen.
2. Compile işi bitince "Droid-WinOs release" iş akışını (id 400542643) o run_id ve tag (örn. v0.1.15-winos) ile çalıştır. APK sürümde app-release.apk olarak çıkar (boyut hep ~75,4 MB görünür; sürümler farklı, boyut tesadüfen aynı).
3. gh release view/download çalışmıyor olabilir (GraphQL); REST ile indir: gh api -H "Accept: application/octet-stream" repos/avsessisozx/Droid-WinOs/releases/assets/<id>
4. Sürümü düzenleme/silme izni yok.
5. Yeni push, çalışan derlemeyi iptal eder (concurrency). Sırayla çalış.

## Telefon (ADB, USB bağlı)
- Xiaomi Mi 10T Pro, Android 12, ADB seri no 4d738514, paket com.droiddeck.launcher
- Kurulum: install_adb_package (replace=true). Ekran: capture_adb_screen (output_directory her seferinde yeni klasör olmalı).
- Telefonun kilidi AÇIK olmalı, yoksa oturum siyah kalır. `adb shell svc power stayon usb` ile ekranı açık tut.
- Uzaktan dokunma/yazma için MIUI'de "USB hata ayıklama (Güvenlik ayarları)" AÇIK olmalı; kapalıysa `input tap` / `input text` "INJECT_EVENTS permission" hatası verir. Ayarlar > Ek ayarlar > Geliştirici seçenekleri.
- Uygulamayı uzaktan sür: start_adb_activity, action android.intent.action.MAIN, component com.droiddeck.launcher/.WinOsActivity, ekstralar:
  - winos_run (string): çalıştırılacak .exe yolu, örn. /storage/emulated/0/Download/qBittorrentPortable/App/qBittorrent/qbittorrent.exe
  - winos_args (string): programa boşlukla ayrılmış argümanlar, örn. --confirm-legal-notice
  - winos_guest_exe (string): Linux içi exe yolu (test), winos_shell (boolean, değeri "true" metni), winos_desktop
  - winos_report=true: raporu /sdcard/Android/data/com.droiddeck.launcher/files/winos-report.txt dosyasına yazar. Rapor için önce stop_adb_app, sonra bu ekstrayla soğuk başlat.
- Çalışan örneğe gönderilen intent bazen oturum açmıyor: önce stop_adb_app, sonra soğuk başlat.
- Program açılması ~60-90 sn sürer. Ekran görüntüsünü aldıktan sonra kendin oku.

## Önemli teknik bilgiler
- Çalışma ortamı DroidDeck runtime r9 (Arch ARM64, proot, Proton Experimental ARM64, FEX). Steam istemcisi yok, gerek de yok.
- Eksik Windows DLL'leri (icuuc.dll, icuin.dll, icu.dll gibi) kullanıcı C:\Windows\System32'den kopyalar, telefonda İndirilenler/WinOs-DLL klasörüne koyar; uygulama otomatik yükler. Ana ekranda eksik DLL adı yazar.
- v0.1.14: oyun tuşları (OSC_NEVER) ve fps kutusu gizlendi. HUD anahtarı "hud" (program oturumu), Steam'de "hudDeck"; WinOsActivity ikisini de kapatıyor.
- v0.1.15: klavye düğmesi ayrı bir yüzen düğme (SessionActivity.keyboardFab); OSC_NEVER'da pad tamamen gizli, dokunuşlar programa geçiyor. Düğme "PC klavyesi" panelini açar (Esc/F1..., Türkçe tuş adları, "Android klavyesi" ve "Gizle").
- explorer /desktop=shell ve /desktop=WinOs,1600x720 pencere AÇMIYOR: explorer.exe çalışıyor (tasklist'te görünüyor) ama masaüstü/görev çubuğu gelmiyor. "Windows masaüstünü aç" dosya yöneticisi penceresi açıyor. Görev çubuğu için üçüncü taraf kabuk veya kendi çubuğumuz gerekiyor.
- Teşhis için cmd.exe çalıştırılabilir: winos_run=<herhangi exe> + winos_guest_exe=/root/.local/share/Steam/steamapps/compatdata/2564280011/pfx/drive_c/windows/system32/cmd.exe. Konsola `input text` ile komut yazılır (boşluk %s), Enter: input keyevent 66. tasklist, dir, explorer gibi komutlar çalışıyor.
- 32 bit programlar FEX ile açılıyor (Notepad++ 8.9.8.1 x86 denendi).

## Sıradaki işler
1. Görev çubuğu/başlat menüsü için alternatif: üçüncü taraf Windows kabuğu (ör. bb4win/Blackbox) veya uygulama içi basit bir görev çubuğu.
2. Sesin hoparlörden duyulduğunu doğrula (ses çıkaran bir Windows programıyla); AAudio çıkışı "started" görünüyor.
3. Office benzeri büyük programları dene (kurulum + açılış), hata raporunu oku, düzelt.
4. Kablosuz hata ayıklama eşleştirme sorununu incele.
5. Her sürümde DURUM.md'yi güncelle (sürüm, tarih, doğrulandı/doğrulanmadı).
