# WinOsBar

Droid-WinOs oturumları için küçük bir Windows görev çubuğu. Windows programları
Wine'ın sanal masaüstünde açıldığı için explorer'ın görev çubuğu gelmiyor; bu
program alt şeritte "Başlat" düğmesi ve açık pencerelerin düğmelerini gösterir.

- "Başlat": Z:\storage\emulated\0\Download altındaki .exe'leri listeler, seçileni başlatır.
- Pencere düğmeleri: o pencereyi öne getirir.
- "Görev çubuğunu kapat": oturumu bitirir.

## Derleme (Windows, Visual Studio)

```
cmd /c ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 && cl /nologo /O2 /W3 /utf-8 WinOsBar.c /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib shell32.lib shlwapi.lib"
```

Derleme sonrası `WinOsBar.exe` dosyasını `app/src/main/assets/winos/WinOsBar.exe`
konumuna kopyala; uygulama ilk açılışta bunu `Download/WinOs-Bar/` klasörüne çıkarır.
