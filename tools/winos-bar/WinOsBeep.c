#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <math.h>

int WINAPI wWinMain(HINSTANCE h, HINSTANCE p, LPWSTR c, int s) {
    (void)h; (void)p; (void)c; (void)s;
    const int rate = 44100;
    const int secs = 8;
    const int n = rate * secs;
    short *buf = (short *)malloc((size_t)n * sizeof(short));
    if (!buf) return 1;
    for (int i = 0; i < n; i++) {
        double t = (double)i / rate;
        buf[i] = (short)(9000 * sin(2 * 3.14159265358979 * 440.0 * t));
    }
    WAVEFORMATEX wf;
    ZeroMemory(&wf, sizeof(wf));
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 1;
    wf.nSamplesPerSec = rate;
    wf.wBitsPerSample = 16;
    wf.nBlockAlign = 2;
    wf.nAvgBytesPerSec = rate * 2;

    FILE *f = NULL;
    if (fopen_s(&f, "Z:\\storage\\emulated\\0\\Download\\WinOs-Bar\\beep.log", "a") == 0 && f) {
        fprintf(f, "start\n");
        fclose(f);
    }

    HWAVEOUT out = NULL;
    MMRESULT r = waveOutOpen(&out, WAVE_MAPPER, &wf, 0, 0, 0);
    if (fopen_s(&f, "Z:\\storage\\emulated\\0\\Download\\WinOs-Bar\\beep.log", "a") == 0 && f) {
        fprintf(f, "open=%u\n", (unsigned)r);
        fclose(f);
    }
    if (r != MMSYSERR_NOERROR) return 2;

    WAVEHDR hdr;
    ZeroMemory(&hdr, sizeof(hdr));
    hdr.lpData = (LPSTR)buf;
    hdr.dwBufferLength = n * 2;
    waveOutPrepareHeader(out, &hdr, sizeof(hdr));
    waveOutWrite(out, &hdr, sizeof(hdr));
    Sleep(secs * 1000);
    waveOutUnprepareHeader(out, &hdr, sizeof(hdr));
    waveOutClose(out);
    free(buf);
    if (fopen_s(&f, "Z:\\storage\\emulated\\0\\Download\\WinOs-Bar\\beep.log", "a") == 0 && f) {
        fprintf(f, "done\n");
        fclose(f);
    }
    return 0;
}
