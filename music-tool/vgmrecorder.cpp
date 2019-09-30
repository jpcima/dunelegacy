#include "vgmrecorder.h"
#include <cstdio>

static constexpr unsigned BufferCapacity = 512 * 1024;

///
enum {
    MsgCodeQuit,
    MsgCodeWriteReg,
};

template <unsigned Tag>
struct BufferMessage {
    const unsigned tag = Tag;
};

struct QuitMessage : public BufferMessage<MsgCodeQuit> {
};

struct WriteRegMessage : public BufferMessage<MsgCodeWriteReg> {
    double timestamp;
    uint8_t reg;
    uint8_t val;
};

///
VGMrecorder::VGMrecorder()
    : ringbuff_(BufferCapacity)
{
    thread_ = std::thread([this]() { threadRun(); });
}

VGMrecorder::~VGMrecorder()
{
    while (!postMessage(QuitMessage()))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    thread_.join();
}

bool VGMrecorder::writeReg(double timestamp, uint8_t reg, uint8_t val)
{
    WriteRegMessage msg;
    msg.timestamp = timestamp;
    msg.reg = reg;
    msg.val = val;
    return postMessage(msg);
}

template <class T> bool VGMrecorder::postMessage(const T &msg)
{
   if (ringbuff_.size_free() < sizeof(msg))
       return false;
    ringbuff_.put(msg);
    sem_.post();
    return true;
}

template <class T> bool VGMrecorder::extractMessage(T &msg)
{
    if (!ringbuff_.peek(msg))
        return false;
    ringbuff_.discard(sizeof(msg));
    return true;
}

void VGMrecorder::threadRun()
{
    BufferMessage<0> header;

    bool quit = false;
    while (!quit) {
        sem_.wait();

        while (ringbuff_.peek(header)) {
            switch (header.tag) {
            case MsgCodeQuit:
            {
                QuitMessage msg;
                if (!extractMessage(msg))
                    break;

                quit = true;
                break;
            }
            case MsgCodeWriteReg:
            {
                WriteRegMessage msg;
                if (!extractMessage(msg))
                    break;

                // TODO vgm write register
                //fprintf(stderr, "WriteOPL: %02X %02X @ %f\n", msg.reg, msg.val, msg.timestamp);

                break;
            }
            }
        }
    }
}
