/*
 * WinbondFlash.h
 *
 *  Created on: Feb. 25th, 2016
 *      Author: KuangQi
 */

#ifndef WINBOND_FLASH_H_
#define WINBOND_FLASH_H_

#include "mbed.h"

#define SPI_FREQ        8000000
#define SPI_MODE        0
#define SPI_NBIT        8

#define WE_INST         0x06
#define WD_INST         0x04

#define READ_INST          0x03
#define WRITE_INST          0x02

#define READ_SR1_INST        0x05
#define ERASE_4K_INST   0x20
#define ERASE_32K_INST  0x52
#define ERASE_64K_INST  0xd8
#define ERASE_CHIP_INST    0x60

#define DUMMY_ADDR      0x00

class WinbondFlash: public SPI {
public:
    WinbondFlash(PinName mosi, PinName miso, PinName sclk, PinName cs);

    int readByte(uint32_t addr);                                 // takes a 24-bit (3 bytes) address and returns the data (1 byte) at that location
    void readStream(uint32_t addr, uint8_t* buf, int count);        // takes a 24-bit address, reads count bytes, and stores results in buf

    void writeByte(uint32_t addr, uint8_t data);                     // takes a 24-bit (3 bytes) address and a byte of data to write at that location
    void writeStream(uint32_t addr, uint8_t* buf, int count);       // write count bytes of data from buf to memory, starting at addr

    void eraseChip();                                       // erase all data on chip
    void erase4K(uint32_t addr);
    void erase32K(uint32_t addr);
    void erase64K(uint32_t addr);

private:
    void writeEnable();                                     // write enable
    void chipEnable();                                      // chip enable
    void chipDisable();                                     // chip disable
    void waitForReady();

   // SPI _spi;
    DigitalOut _cs;
};

#endif  // WINBOND_FLASH_H_
