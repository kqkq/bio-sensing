/*
 * WinbondFlash.cpp
 *
 *  Created on: Feb. 25th, 2016
 *      Author: KuangQi
 */
#include "WinbondFlash.h"

// CONSTRUCTOR
WinbondFlash::WinbondFlash(PinName mosi, PinName miso, PinName sclk, PinName cs) : SPI(mosi, miso, sclk), _cs(cs) {
    this->format(SPI_NBIT, SPI_MODE);
    this->frequency(SPI_FREQ);
    chipDisable();
}

// READING
int WinbondFlash::readByte(uint32_t addr) {
    waitForReady();
    chipEnable();
    this->write(READ_INST);
    this->write((addr >> 16) & 0xff);
    this->write((addr >> 8)  & 0xff);
    this->write((addr >> 0)  & 0xff);
    int response = this->write(DUMMY_ADDR);
    chipDisable();
    return response;
}

void WinbondFlash::readStream(uint32_t addr, uint8_t* buf, int count) {
    if (count < 1)
        return;
    waitForReady();
    chipEnable();
    this->write(READ_INST);
    this->write((addr >> 16) & 0xff);
    this->write((addr >> 8)  & 0xff);
    this->write((addr >> 0)  & 0xff);
    for (int i = 0; i < count; i++)
        buf[i] =  this->write(DUMMY_ADDR);
    chipDisable();
}

// WRITING
void WinbondFlash::writeByte(uint32_t addr, uint8_t data) {
    waitForReady();
    writeEnable();
    chipEnable();
    this->write(WRITE_INST);
    this->write((addr >> 16) & 0xff);
    this->write((addr >> 8)  & 0xff);
    this->write((addr >> 0)  & 0xff);
    this->write(data);
    chipDisable();
    //writeDisable();
}

void WinbondFlash::writeStream(uint32_t addr, uint8_t* buf, int count) {
    bool nextPage = true;
    int bufIndex = 0;
    while(count)
    {
        if(nextPage)
        {
            nextPage = false;
            waitForReady();
            writeEnable();
            chipEnable();
            this->write(WRITE_INST);
            this->write((addr >> 16) & 0xff);
            this->write((addr >> 8)  & 0xff);
            this->write((addr >> 0)  & 0xff);
        }
        this->write(buf[bufIndex++]);
        addr++;
        count--;
        if((addr & 0xff) == 0)
        {
            nextPage = true;
            if(count) chipDisable();
        }
    }
    chipDisable();
}

//ERASING
void WinbondFlash::eraseChip() {
    waitForReady();
    writeEnable();
    chipEnable();
    this->write(ERASE_CHIP_INST);
    chipDisable();
    waitForReady();
}

void WinbondFlash::erase4K(uint32_t addr)
{
    if(addr & 0xfff) return;
    waitForReady();
    writeEnable();
    chipEnable();
    this->write(ERASE_4K_INST);
    this->write((addr >> 16) & 0xff);
    this->write((addr >> 8)  & 0xff);
    this->write((addr >> 0)  & 0xff);
    chipDisable();
    waitForReady();
}

void WinbondFlash::erase32K(uint32_t addr)
{
    if(addr & 0x7fff) return;
    waitForReady();
    writeEnable();
    chipEnable();
    this->write(ERASE_32K_INST);
    this->write((addr >> 16) & 0xff);
    this->write((addr >> 8)  & 0xff);
    this->write((addr >> 0)  & 0xff);
    chipDisable();
    waitForReady();
}

void WinbondFlash::erase64K(uint32_t addr)
{
    if(addr & 0xfff) return;
    waitForReady();
    writeEnable();
    chipEnable();
    this->write(ERASE_64K_INST);
    this->write((addr >> 16) & 0xff);
    this->write((addr >> 8)  & 0xff);
    this->write((addr >> 0)  & 0xff);
    chipDisable();
    waitForReady();
}

void WinbondFlash::waitForReady()
{
    chipEnable();
    this->write(READ_SR1_INST);
    while(this->write(DUMMY_ADDR) & 0x01);
    chipDisable();
}

//ENABLE (private functions)
void WinbondFlash::writeEnable() {
    chipEnable();
    this->write(WE_INST);
    chipDisable();
}

void WinbondFlash::chipEnable() {
    _cs = 0;
}

void WinbondFlash::chipDisable() {
    _cs = 1;
}
