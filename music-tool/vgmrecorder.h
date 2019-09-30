#pragma once
#include "realtime/ring_buffer.h"
#include "realtime/semaphore.h"
#include <thread>

class VGMrecorder {
public:
    VGMrecorder();
    ~VGMrecorder();

    bool writeReg(double timestamp, uint8_t reg, uint8_t val);

private:
    template <class T> bool postMessage(const T &msg);
    template <class T> bool extractMessage(T &msg);
    void threadRun();

private:
    std::thread thread_;
    Ring_Buffer ringbuff_;
    Semaphore sem_;
};
