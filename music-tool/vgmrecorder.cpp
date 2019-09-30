#include "vgmrecorder.h"
#include <cstdio>
#include <cstddef>
#include <cstring>

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
    uint32_t delay;
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

bool VGMrecorder::openOutputFile(const char *path)
{
    FILE *fh = fopen(path, "wb");
    if (!fh)
        return false;
    file_.reset(fh);
    return true;
}

bool VGMrecorder::writeReg(uint32_t delay, uint8_t reg, uint8_t val)
{
    WriteRegMessage msg;
    msg.delay = delay;
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

                terminateFile();

                quit = true;
                break;
            }
            case MsgCodeWriteReg:
            {
                WriteRegMessage msg;
                if (!extractMessage(msg))
                    break;

                //fprintf(stderr, "WriteOPL: %02X %02X @ %f\n", msg.reg, msg.val, msg.timestamp);
                writeFileRegister(msg.reg, msg.val, msg.delay);
                break;
            }
            }
        }
    }
}

void VGMrecorder::ensureWriteFileHeader()
{
    if (fileHeaderWritten_)
        return;

    fileHeaderWritten_ = true;

    FILE *f = file_.get();

    memset(&vgmHead_, 0, sizeof(VgmHead));
    memcpy(vgmHead_.magic, "Vgm ", 4);
    vgmHead_.version = 0x00000151;

    vgmHead_.clock_ym3812 = 3579545;

    if (f) fwrite(&vgmHead_, 1, sizeof(VgmHead), f);
}

void VGMrecorder::terminateFile()
{
    ensureWriteFileHeader();

    FILE *f = file_.get();

    uint8_t out[1];
    out[0] = 0x66;// end of sound data
    if (f) fwrite(&out, 1, 1, f);
    bytesWritten_ += 1;

    if (f) fseek(f, 0x00, SEEK_SET);
    vgmHead_.total_samples = samplesWritten_;
    vgmHead_.offset_loop = sizeof(VgmHead) - offsetof(VgmHead, offset_loop);
    vgmHead_.loop_samples = samplesWritten_ - 1;
    vgmHead_.eof_offset = (sizeof(VgmHead) + bytesWritten_ - offsetof(VgmHead, eof_offset));
    vgmHead_.offset_data = sizeof(VgmHead) - offsetof(VgmHead, offset_data);
    //! FIXME: Make proper endianess suporrt
    if (f) {
        fwrite(&vgmHead_, 1, sizeof(VgmHead), f);
        fflush(f);
    }
}

void VGMrecorder::writeFileRegister(uint8_t reg, uint8_t val, uint32_t delay)
{
    ensureWriteFileHeader();

    FILE *f = file_.get();

    // write delay
    while(delay > 0)
    {
        uint16_t to_copy;
        if(delay > 65535)
        {
            to_copy = 65535;
            delay -= 65535;
        }
        else
        {
            to_copy = static_cast<uint16_t>(delay);
            delay = 0;
        }

        uint8_t delaycmd[3];
        unsigned delaycmdlen;
        if(to_copy == 735)
        {
            delaycmd[0] = 0x62;
            delaycmdlen = 1;
        }
        else if(to_copy == 882)
        {
            delaycmd[0] = 0x63;
            delaycmdlen = 1;
        }
        else
        {
            delaycmd[0] = 0x61;
            delaycmd[1] = to_copy & 0xFF;
            delaycmd[2] = (to_copy >> 8) & 0xFF;
            delaycmdlen = 3;
        }

        if (f) fwrite(&delaycmd, 1, delaycmdlen, f);
        bytesWritten_ += delaycmdlen;
        samplesWritten_ += to_copy;
    }

    // write register (OPL2)
    uint8_t opl2reg[] = {0x5A, reg, val};
    if (f) fwrite(&opl2reg, 1, sizeof(opl2reg), f);
    bytesWritten_ += sizeof(opl2reg);
}
