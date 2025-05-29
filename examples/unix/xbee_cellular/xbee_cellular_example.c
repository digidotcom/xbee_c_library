/***************************************************************************//**
 * @file xbee_cellular_example.c
 * @brief Example application demonstrating the use of the XBeeCellular driver on Unix.
 *
 * This file contains a sample application that demonstrates how to use the XBeeCellular library 
 * to communicate with XBee 3 Cellular modules in a Unix environment. It showcases basic operations 
 * such as initializing the module, configuring network settings, attaching to the cellular network, 
 * creating a TCP socket, sending an HTTP GET request, and printing the response.
 *
 * @version 1.2
 * @date 2025-05-20
 * @author Felix Galindo
 * @contact felix.galindo@digi.com
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
 ******************************************************************************/

#include "xbee_cellular.h"
#include "port.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @brief Callback triggered when data is received via SOCKET_RECEIVE frame (0xCD).
 *
 * This function prints out the payload from the remote server in plain text format.
 * Typically used to handle HTTP response content received over TCP.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] data Pointer to the received XBeeCellularPacket_t struct.
 */
void OnReceiveCallback(XBee* self, void* data) {
    XBeeCellularPacket_t* packet = (XBeeCellularPacket_t*)data;

    portDebugPrintf("\n[RX] Received %u bytes from %u.%u.%u.%u:\n",
        packet->payloadSize, packet->ip[0], packet->ip[1], packet->ip[2], packet->ip[3]);

    // Full hex dump
    portDebugPrintf("[Payload HEX Dump]:\n");
    for (int i = 0; i < packet->payloadSize; i++) {
        portDebugPrintf("%02X ", packet->payload[i]);
        if ((i + 1) % 16 == 0) {
            portDebugPrintf("\n");
        }
    }
    if (packet->payloadSize % 16 != 0) {
        portDebugPrintf("\n");
    }

    // Full ASCII dump
    portDebugPrintf("[Payload ASCII Dump]:\n");
    for (int i = 0; i < packet->payloadSize; i++) {
        char c = packet->payload[i];
        if (c >= 32 && c <= 126) {
            portDebugPrintf("%c", c); // printable ASCII
        } else {
            portDebugPrintf(".");     // non-printable, print dot
        }
    }
    portDebugPrintf("\n");
}


/**
 * @brief Callback triggered after a frame has been transmitted successfully.
 *
 * This example logs the send event. The application may be extended to
 * log timestamps or measure round-trip latency.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] data Pointer to the original sent packet (optional).
 */
void OnSendCallback(XBee* self, void* data) {
    portDebugPrintf("[TX] Send callback invoked.\n");
}

/**
 * @brief Main entry point for the cellular socket example.
 *
 * This function:
 * - Initializes the UART and the XBeeCellular instance.
 * - Configures the cellular network (APN).
 * - Connects to the network.
 * - Opens a TCP socket to numbersapi.com.
 * - Sends a GET /random/trivia HTTP request.
 * - Waits for incoming responses over 15 seconds.
 */
int main() {
    // Hardware abstraction setup
    const XBeeHTable hw = {
        .PortUartRead = portUartRead,
        .PortUartWrite = portUartWrite,
        .PortMillis = portMillis,
        .PortFlushRx = portFlushRx,
        .PortUartInit = portUartInit,
        .PortDelay = portDelay,
    };

    // User callbacks for RX/TX events
    const XBeeCTable cb = {
        .OnReceiveCallback = OnReceiveCallback,
        .OnSendCallback = OnSendCallback
    };

    portDebugPrintf("XBee 3 Cellular - HTTP GET Example\n");

    // Allocate instance
    XBeeCellular* xbee = XBeeCellularCreate(&cb, &hw);

    // Set SIM configuration: APN is required, others optional
    XBeeCellularConfig_t cfg = {
        .apn = "broadband",  // example: AT&T
        .simPin = "",
        .carrier = ""
    };
    XBeeConfigure(xbee, &cfg);

    // Initialize serial port
    if (!XBeeInit((XBee*)xbee, 9600, "/dev/cu.usbserial-1110")) {
        portDebugPrintf("[ERR] Failed to initialize UART\n");
        return -1;
    }

    // Attach to cellular network
    portDebugPrintf("Connecting to LTE network...\n");
    XBeeConnect((XBee*)xbee, false);

    while(1) {
        // Check if the module is connected
        if (XBeeCellularConnected((XBee*)xbee)) {
            portDebugPrintf("[OK] Connected to cellular network.\n");
            break;
        }
        portDebugPrintf("Waiting for network attach...\n");
        sleep(1);
    }

    // Create a TCP socket
    uint8_t socketId = 0;
    if (!XBeeCellularSocketCreate((XBee*)xbee, 0x01 /* TCP */, &socketId)) {
        portDebugPrintf("[ERR] Socket create failed\n");
        return -1;
    }

    // Connect socket to numbersapi.com
    if (!XBeeCellularSocketConnect((XBee*)xbee, socketId, "numbersapi.com", 80, true)) {
        portDebugPrintf("[ERR] Socket connect failed\n");
        return -1;
    }

    // Send HTTP GET /random/trivia
    const char* httpRequest =
        "GET /random/trivia HTTP/1.1\r\n"
        "Host: numbersapi.com\r\n"
        "Connection: close\r\n\r\n";

    if (!XBeeCellularSocketSend((XBee*)xbee, socketId,
            (const uint8_t*)httpRequest, strlen(httpRequest))) {
        portDebugPrintf("[ERR] Socket send failed\n");
        return -1;
    }

    portDebugPrintf("[OK] HTTP GET request sent. Awaiting response...\n");

    // Process incoming response frames for ~15 seconds
    time_t start, now;
    time(&start);
    do {
        XBeeProcess((XBee*)xbee);
        time(&now);
        sleep(1);
    } while (difftime(now, start) < 15);

    portDebugPrintf("HTTP transaction complete. Exiting.\n");
    XBeeCellularSocketClose((XBee*)xbee, socketId);
    XBeeCellularDestroy(xbee);
    return 0;
}