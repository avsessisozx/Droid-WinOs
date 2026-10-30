# Droid-WinOs devir notu (başka bir yapay zekâ için)

Durum: **geliştirme 2026-10-30'da kullanıcı isteğiyle durduruldu.** Kod `main` son commit `716fa3f2` (v0.1.18); sonuçlar DURUM.md'de. Devam edilecekse önce DURUM.md'yi oku, sonra aşağıdaki "Sıradaki işler" listesinden başla.

Amaç: Android telefonda taşınabilir Windows ortamı. Windows programları (oyun değil) kurulup çalışsın, masaüstü gibi kullanılsın. Kullanıcı programcı değil, Türkçe konuşuyor. Kısa, numaralı, sade adımlar ister. Onu test kullanıcısı yapma: uygulamayı sen telefonda dene, çalışan hâlini ver.

## Depo ve dallar
- https://github.com/avsessisozx/Droid-WinOs (DroidDeck 0.6.0 çatalı, GPL-3)
- main: kod. durum: bu dosyalar (DURUM.md, DEVIR.md). Sürüm notlarına sohbet bağlantısı koyma, sadece yeni sürümün notu olsun.
- Ana dosyalar:
  - app/src/main/java/com/droiddeck/launcher/WinOsActivity.kt — program listesi, oturum başlatma, masaüstü/görev çubuğu düğmesi, DLL kopyalama.
  - app/src/main/java/com/droiddeck/launcher/SessionActivity.kt — oturum ekranı; klavye düğmesi (keyboardFab) ve OSC/HUD gizleme.
  - tools/winos-bar/WinOsBar.c — Windows görev çubuğu (Win32, x86); derlenmiş hâli app/src/main/assets/winos/WinOsBar.exe.
  - .github/workflows/winos-release.yml — sürüm notu metni.

## Derleme ve yayın (bulutta, GitHub Actions)
1. main'e push edince "Build APK" çalışır (~20 dk). Gerekli işler: "Prepare verified components", "Compile APK and Android tests", "Compile and test APK". sign/packs/announce/catalog hataları bu çatalda beklenen.
2. Compile işi bitince "Droid-WinOs release" iş akışını (id 400542643) o run_id ve tag (örn. v0.1.18-winos) ile çalıştır. APK sürümde app-release.apk olarak çıkar.
3. gh release view/download çalışmıyor olabilir (GraphQL); REST ile indir: gh api -H "Accept: application/octet-stream" repos/avsessisozx/Droid-WinOs/releases/assets/<id>
4. Sürümü düzenleme/silme izni yok.
5. Yeni push, çalışan derlemeyi iptal eder (concurrency). Sırayla çalış.

## Görev çubuğu (WinOsBar)
- Win32, x86 derlenir (telefon ARM64 + FEX; 32 bit Notepad++ ile doğrulandı). Derleme komutu tools/winos-bar/README.md içinde.
- Alt şeritte "Başlat" + açık pencerelerin düğmeleri. Başlat menüsü kendi penceresinde 56 px satırlarla çizilir; Z:\storage\emulated\0\Download altındaki .exe'leri listeler, seçileni ShellExecuteW ile başlatır.
- APK içinden çıkarılır: WinOsActivity.ensureBar() → /sdcard/Download/WinOs-Bar/WinOsBar.exe.
- Z-sırası: SetWindowPos(HWND_TOP) + BringWindowToTop her 800 ms'de bir yinelenir. HWND_TOPMOST Wine'da işe yaramadı. Yine de bazı tam ekran pencereler çubuğu örtebiliyor; kalıcı çözüm aranıyor.
- SPI_SETWORKAREA denendi, Wine uygulamıyor.

## Telefon (ADB, USB bağlı)
- Xiaomi Mi 10T Pro, Android 12, ADB seri no 4d738514, paket com.droiddeck.launcher
- Kurulum: install_adb_package (replace=true). Ekran: capture_adb_screen (output_directory her seferinde yeni klasör olmalı).
- Telefonun kilidi AÇIK olmalı; `adb shell svc power stayon usb`. Ekran kapanırsa `input keyevent 224` + yukarı kaydırma ile açılır.
- Uzaktan dokunma/yazma için MIUI'de "USB hata ayıklama (Güvenlik ayarları)" AÇIK olmalı; kapalıysa `input tap` / `input text` "INJECT_EVENTS permission" hatası verir.
- Uygulamayı uzaktan sür: start_adb_activity, action android.intent.action.MAIN, component com.droiddeck.launcher/.WinOsActivity, ekstralar:
  - winos_run (string): çalıştırılacak .exe yolu
  - winos_args (string): programa boşlukla ayrılmış argümanlar
  - winos_guest_exe (string): Linux içi exe yolu (test), winos_shell (boolean, "true" metni), winos_desktop
  - winos_report=true: raporu /sdcard/Android/data/com.droiddeck.launcher/files/winos-report.txt dosyasına yazar.
- Çalışan örneğe gönderilen intent bazen oturum açmıyor: önce stop_adb_app, sonra soğuk başlat.
- Program açılması ~60-90 sn sürer. Ekran görüntüsünü aldıktan sonra kendin oku.

## Önemli teknik bilgiler
- Çalışma ortamı DroidDeck runtime r9 (Arch ARM64, proot, Proton Experimental ARM64, FEX). Steam istemcisi yok, gerek de yok.
- Oturum ayarları: OSC_NEVER (oyun tuşları yok), "hud" ve "hudDeck" false (fps kutusu yok), TOUCH_DIRECT (dokunulan yere tıkla). WinOsActivity bunları oturum başlamadan yazar.
- HUD anahtarı program oturumunda "hud"; Steam'de "hudDeck". setHudEnabled oturum başlamadan yanlış anahtara yazabiliyordu; bu yüzden iki anahtar da doğrudan kapatılıyor.
- Klavye düğmesi SessionActivity.keyboardFab: sağ alt, 56 dp; "PC klavyesi" panelini açar (Esc/F1..., Türkçe tuş adları, "Android klavyesi", "Gizle").
- explorer /desktop=shell ve /desktop=WinOs,1600x720 pencere AÇMIYOR (explorer.exe çalışıyor, masaüstü gelmiyor). Bu yüzden kendi görev çubuğumuz yazıldı.
- Teşhis için cmd.exe: winos_run=<herhangi exe> + winos_guest_exe=/root/.local/share/Steam/steamapps/compatdata/2564280011/pfx/drive_c/windows/system32/cmd.exe. Konsola `input text` ile komut yazılır (boşluk %s), Enter: input keyevent 66. tasklist, dir, explorer çalışıyor.
- Programların kendi DLL'leri yoksa açılmaz; İndirilenler/WinOs-DLL klasöründeki DLL'ler system32'ye kopyalanır.

## Sıradaki işler
1. Görev çubuğunun her zaman üstte kalması (appbar/WS_EX_TOPMOST alternatifi; Wine'da zor).
2. Sesin hoparlörden duyulduğunu doğrula.
3. Office benzeri büyük programları dene (kurulum + açılış), hata raporunu oku.
4. Kablosuz hata ayıklama eşleştirme sorununu incele.
5. Her sürümde DURUM.md'yi güncelle (sürüm, tarih, doğrulandı/doğrulanmadı).
