#pragma once
#include "realtime/ring_buffer.h"
#include "realtime/semaphore.h"
#include <thread>
#include <memory>
#include <cstdio>

class VGMrecorder {
public:
    VGMrecorder();
    ~VGMrecorder();

    bool openOutputFile(const char *path);
    bool writeReg(double timestamp, uint8_t reg, uint8_t val);

private:
    template <class T> bool postMessage(const T &msg);
    template <class T> bool extractMessage(T &msg);
    void threadRun();

    void ensureWriteFileHeader();
    void terminateFile();
    void writeFileRegister(uint8_t reg, uint8_t val, double timestamp);

private:
    std::thread thread_;
    Ring_Buffer ringbuff_;
    Semaphore sem_;
    struct FILE_delete {
        void operator()(FILE *x) const noexcept { fclose(x); }
    };
    std::unique_ptr<FILE, FILE_delete> file_;
    bool fileHeaderWritten_ = false;

    ///
    struct VgmHead {
        char magic[4];
        uint32_t eof_offset;
        uint32_t version;
        uint32_t clock_sn76489;
        uint32_t clock_ym2413;
        uint32_t offset_gd3;
        uint32_t total_samples;
        uint32_t offset_loop;
        uint32_t loop_samples;
        uint32_t rate;
        uint16_t feedback_sn76489;
        uint8_t  shift_register_width_sn76489;
        uint8_t  flags_sn76489;
        uint32_t clock_ym2612;
        uint32_t clock_ym2151;
        uint32_t offset_data;
        //
        uint32_t clock_segapcm;
        uint32_t interface_spcm;
        uint32_t clock_rf5c68;
        uint32_t clock_ym2203;
        uint32_t clock_ym2608;
        uint32_t clock_ym2610;
        uint32_t clock_ym3812;
    };
    VgmHead vgmHead_;

    unsigned bytesWritten_ = 0;
    unsigned samplesWritten_ = 0;
};
