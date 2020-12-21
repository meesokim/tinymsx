/**
 * SUZUKI PLAN - TinyMSX
 * -----------------------------------------------------------------------------
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Yoji Suzuki.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */
#ifndef INCLUDE_TINYMSX_H
#define INCLUDE_TINYMSX_H
#include <stdio.h>
#include "z80.hpp"
#include "tinymsx_def.h"
#include "msxslot.hpp"

class TinyMSX {
    private:
        struct MsxBIOS {
            unsigned char main[0x8000];
            unsigned char logo[0x4000];
        } bios;
        int type;
        unsigned char pad[2];
        unsigned char specialKeyX[2];
        unsigned char specialKeyY[2];
        unsigned char* rom;
        size_t romSize;
        short soundBuffer[65536];
        unsigned short soundBufferCursor;
        unsigned char tmpBuffer[1024 * 1024];
    public:
        unsigned short display[256 * 192];
        unsigned short palette[16];
        struct VideoDisplayProcessor {
            unsigned char ram[0x4000];
            unsigned char reg[8];
            unsigned char tmpAddr[2];
            unsigned short addr;
            unsigned char stat;
            unsigned char latch;
            unsigned char readBuffer;
        } vdp;
        struct SN76489 {
            int b;
            int i;
            unsigned int r[8];
            unsigned int c[4];
            unsigned int e[4];
            unsigned int np;
            unsigned int ns;
            unsigned int nx;
        } sn76489;
        struct AY8910 {
            int clocks;
            unsigned char latch;
            unsigned char tmask[3];
            unsigned char nmask[3];
            unsigned char reserved[5];
            unsigned char reg[0x20];
            unsigned int count[3];
            unsigned int volume[3];
            unsigned int freq[3];
            unsigned int edge[3];
            struct Envelope {
                unsigned int volume;
                unsigned int ptr;
                unsigned int face;
                unsigned int cont;
                unsigned int attack;
                unsigned int alternate;
                unsigned int hold;
                unsigned int pause;
                unsigned int reset;
                unsigned int freq;
                unsigned int count;
                unsigned int reserved;
            } env;
            struct Noise {
                unsigned int seed;
                unsigned int count;
                unsigned int freq;
                unsigned int reserved;
            } noise;
            short ch_out[4];
        } ay8910;
        unsigned int psgClock;
        unsigned char psgLevels[16];
        struct MemoryRegister {
            unsigned char page[4];
            unsigned char slot[4]; // E * * * - B B E E
            unsigned char portAA; // keyboard position
        } mem;
        struct InternalRegister {
            int frameClock;
            int lineClock;
            int lineNumber;
        } ir;
        unsigned char ram[0x4000];
        MsxSlot slots;
        Z80* cpu;
        TinyMSX(int type, const void* rom, size_t romSize, int colorMode);
        ~TinyMSX();
        bool loadBiosFromFile(const char* path);
        bool loadBiosFromMemory(void* bios, size_t size);
        bool loadLogoFromFile(const char* path);
        bool loadLogoFromMemory(void* logo, size_t size);
        void setupSpecialKey1(unsigned char ascii, bool isTenKey = false);
        void setupSpecialKey2(unsigned char ascii, bool isTenKey = false);
        void reset();
        void tick(unsigned char pad1, unsigned char pad2);
        void* getSoundBuffer(size_t* size);
        const void* saveState(size_t* size);
        void loadState(const void* data, size_t size);
        inline int getVideoMode() { return (vdp.reg[0] & 0b000000010) | (vdp.reg[1] & 0b00010000) >> 4 | (vdp.reg[1] & 0b000001000) >> 1; }
        inline unsigned short getBackdropColor() { return palette[vdp.reg[7] & 0b00001111]; }
        inline int getSlotNumber(int page) { return mem.slot[mem.page[page]] & 0b11; }
        inline int getExtSlotNumber(int page) { return (mem.slot[mem.page[page]] & 0b1100) >> 2; }

    private:
        inline void setupSpecialKeyV(int n, int x, int y) {
            this->specialKeyX[n] = x;
            this->specialKeyY[n] = y;
        }
        void setupSpecialKey(int n, unsigned char ascii, bool isTenKey);
        inline void initBIOS();
        inline bool isSG1000() { return this->type == TINYMSX_TYPE_SG1000; }
        inline bool isMSX1() { return this->type == TINYMSX_TYPE_MSX1; }
        inline unsigned short getInitAddr();
        inline unsigned char readMemory(unsigned short addr);
        inline void setExtraPage(int slot, int extra);
        inline void writeMemory(unsigned short addr, unsigned char value);
        inline unsigned char inPort(unsigned char port);
        inline void outPort(unsigned char port, unsigned char value);
        inline unsigned char vdpReadData();
        inline unsigned char vdpReadStatus();
        inline void vdpWriteData(unsigned char value);
        inline void vdpWriteAddress(unsigned char value);
        inline void updateVdpAddress();
        inline void readVideoMemory();
        inline void updateVdpRegister();
        inline void psgLatch(unsigned char value);
        inline void psgWrite(unsigned char value);
        inline unsigned char psgRead();
        inline void sn76489Calc(short* left, short* right);
        inline short ay8910Calc();
        inline void psgExec(int clocks);
        inline void changeMemoryMap(int page, unsigned char map);
        inline void consumeClock(int clocks);
        inline void checkUpdateScanline();
        inline void drawScanline(int lineNumber);
        inline void drawScanlineMode0(int lineNumber);
        inline void drawScanlineMode2(int lineNumber);
        inline void drawScanlineMode3(int lineNumber);
        inline void drawEmptyScanline(int lineNumber);
        inline void drawSprites(int lineNumber);
        inline bool loadSpecificSizeFile(const char* path, void* buffer, size_t size);
        size_t calcAvairableRamSize();
};

#endif
