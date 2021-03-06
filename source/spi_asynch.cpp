
/* mbed Microcontroller Library
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed-drivers/mbed.h"
#include <stdio.h>
#include "minar/minar.h"
#include "core-util/Event.h"

#if DEVICE_SPI_ASYNCH

#define SHORT_XFR 3
#define LONG_XFR 16
#define TEST_BYTE0 0x00
#define TEST_BYTE1 0x11
#define TEST_BYTE2 0xFF
#define TEST_BYTE3 0xAA
#define TEST_BYTE4 0x55
#define TEST_BYTE5 0x50

#define TEST_BYTE_RX TEST_BYTE3
#define TEST_BYTE_TX_BASE TEST_BYTE5

#if !defined(YOTTA_CFG_HARDWARE_TEST_PINS_SPI_MISO) || !defined(YOTTA_CFG_HARDWARE_TEST_PINS_SPI_MOSI) || \
    !defined(YOTTA_CFG_HARDWARE_TEST_PINS_SPI_SCLK) || !defined(YOTTA_CFG_HARDWARE_TEST_PINS_SPI_SSEL)
#error This example requires SPI test pins to be defined. Please define the hardware.test-pins.spi.miso/ \
    mosi/sclk/ssel yotta confing values
#endif

using namespace minar;

class SPITest {

public:
    SPITest(): spi(YOTTA_CFG_HARDWARE_TEST_PINS_SPI_MOSI, YOTTA_CFG_HARDWARE_TEST_PINS_SPI_MISO,
        YOTTA_CFG_HARDWARE_TEST_PINS_SPI_SCLK), cs(YOTTA_CFG_HARDWARE_TEST_PINS_SPI_SSEL) {
        for (uint32_t i = 0; i < sizeof(tx_buf); i++) {
            tx_buf[i] = i + TEST_BYTE_TX_BASE;
        }
        cs = 1;
    }

    void start() {
        printf("Starting short transfer test\r\n");
        init_rx_buffer();
        cs = 0;
        printf("Res is %d\r\n", spi.transfer()
            .tx(tx_buf, SHORT_XFR)
            .rx(rx_buf, SHORT_XFR)
            .callback(SPI::event_callback_t(this, &SPITest::short_transfer_complete_cb), SPI_EVENT_COMPLETE)
            .apply());
    }

private:
    void init_rx_buffer() {
        for (uint32_t i = 0; i < sizeof(rx_buf); i ++) {
            rx_buf[i] = 0;
        }
    }

    void compare_buffers(uint32_t len) {
         for (uint32_t i = 0; i < len; i ++) {
            if (tx_buf[i] != rx_buf[i]) {
                printf("MISMATCH at position %lu: expected %d, got %d\r\n", i, (int)tx_buf[i], (int)rx_buf[i]);
            }
        }
    }

    void short_transfer_complete_cb(Buffer tx_buffer, Buffer rx_buffer, int narg) {
        (void)tx_buffer;
        (void)rx_buffer;
        cs = 1;
        printf("Short transfer DONE, event is %d\r\n", narg);
        compare_buffers(SHORT_XFR);
        printf("Starting long transfer test\r\n");
        init_rx_buffer();
        cs = 0;
        printf("Res is %d\r\n", spi.transfer()
            .tx(tx_buf, LONG_XFR)
            .rx(rx_buf, LONG_XFR)
            .callback(SPI::event_callback_t(this, &SPITest::long_transfer_complete_cb), SPI_EVENT_COMPLETE)
            .apply());
    }

    void long_transfer_complete_cb(Buffer tx_buffer, Buffer rx_buffer, int narg) {
        (void)tx_buffer;
        (void)rx_buffer;
        cs = 1;
        printf("Long transfer DONE, event is %d\r\n", narg);
        compare_buffers(LONG_XFR);
        printf("**** Test done ****\r\n");
    }

private:
    SPI spi;
    DigitalOut cs;
    uint8_t tx_buf[LONG_XFR];
    uint8_t rx_buf[LONG_XFR];
};

void app_start(int, char*[]) {
    static SPITest test;
    // set 115200 baud rate for stdout
    get_stdio_serial().baud(115200);
    Scheduler::postCallback(mbed::util::FunctionPointer0<void>(&test, &SPITest::start).bind());
}

#else
void app_start(int, char*[]) {
    // set 115200 baud rate for stdout
    get_stdio_serial().baud(115200);
    printf("The target does not support SPI asynch API.\r\n");
}
#endif

