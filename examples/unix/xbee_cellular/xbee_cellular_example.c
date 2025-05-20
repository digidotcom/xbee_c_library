/**
 * @file xbee_cellular_example.c
 * @brief Example application demonstrating the use of the XBeeCellular driver on Unix.
 *
 * This file contains a sample application that demonstrates how to use the XBeeCellular library 
 * to communicate with XBee 3 Cellular modules in a Unix environment. It showcases basic operations 
 * such as initializing the module, configuring network settings, attaching to the cellular network, 
 * and sending IPv4 data packets.
 *
 * @version 1.0
 * @date 2025-05-14
 *
 * @license MIT
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @author Felix Galindo
 * @contact felix.galindo@digi.com
 */

#include "xbee_cellular.h"
#include "port.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @brief Callback function triggered when data is received from the XBee module.
 *
 * This function is called when the XBeeCellular module receives incoming data.
 * 
 * @param[in] self Pointer to the XBee instance.
 * @param[in] data Pointer to the received data packet (not yet parsed).
 */
void OnReceiveCallback(XBee* self, void* data) {
    portDebugPrintf("[RX] (Cellular) Data received\n");
}

/**
 * @brief Callback function triggered after data is sent from the XBee module.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] data Pointer to the data structure sent.
 */
void OnSendCallback(XBee* self, void* data) {
    portDebugPrintf("[TX] (Cellular) Send callback\n");
}

/**
 * @brief Main entry point for the XBeeCellular example application.
 *
 * Initializes the UART, applies cellular config, connects to network,
 * and sends a UDP IPv4 packet every 10 seconds.
 */
int main() {
    const XBeeHTable hw = {
        .PortUartRead = portUartRead,
        .PortUartWrite = portUartWrite,
        .PortMillis = portMillis,
        .PortFlushRx = portFlushRx,
        .PortUartInit = portUartInit,
        .PortDelay = portDelay,
    };

    const XBeeCTable cb = {
        .OnReceiveCallback = OnReceiveCallback,
        .OnSendCallback = OnSendCallback
    };

    portDebugPrintf("XBee Cellular Example\n");

    // Create instance
    XBeeCellular* xbee = XBeeCellularCreate(&cb, &hw);
    

    // Set configuration for APN, SIM PIN, and optional carrier
    XBeeCellularConfig_t cfg = {
        .apn = "hologram",
        .simPin = "1234",
        .carrier = "standard"
    };
    XBeeConfigure(xbee, &cfg);

    // Initialize serial interface
    if (!XBeeInit((XBee*)xbee, 9600, "/dev/cu.usbserial-1120")) {
        portDebugPrintf("[ERR] Failed to initialize XBee UART\n");
        return -1;
    }


    uint16_t fwVersion;
    if (XBeeGetFirmwareVersion(xbee, &fwVersion)) {
        portDebugPrintf("Firmware Version: 0x%04X\n", fwVersion);
    }


    // // Attempt cellular network attach
    // if (!XBeeConnect((XBee*)xbee)) {
    //     portDebugPrintf("[ERR] Cellular registration failed\n");
    //     return -1;
    // }

    // // Construct UDP IPv4 data packet
    // XBeeCellularPacket_t packet = {
    //     .protocol = 0x02, // UDP
    //     .port = 5000,
    //     .ip = {192, 168, 1, 100},
    //     .payload = (const uint8_t*)"XBeeCellular",
    //     .payloadSize = strlen("XBeeCellular")
    // };

    // // Loop to send every 10 seconds
    // time_t lastSent, now;
    // time(&lastSent);

    // while (1) {
    //     XBeeProcess((XBee*)xbee);
    //     time(&now);

    //     if (difftime(now, lastSent) >= 10) {
    //         portDebugPrintf("Sending IPv4 Packet\n");
    //         if (XBeeSendData((XBee*)xbee, &packet) != 0x00) {
    //             portDebugPrintf("[ERR] Send failed\n");
    //         } else {
    //             portDebugPrintf("[OK] Packet sent successfully\n");
    //         }
    //         time(&lastSent);
    //     }
    //     sleep(1);
    // }

    return 0;
}