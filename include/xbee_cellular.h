/***************************************************************************//**
 * @file xbee_cellular.h
 * @brief Header for XBee 3 Cellular API.
 *
 * Declares the registration and high-level management functions for
 * XBee 3 Cellular LTE/NB-IoT modems.
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
 ******************************************************************************/

#ifndef XBEE_CELLULAR_H
#define XBEE_CELLULAR_H

#include "xbee.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure for sending IPv4 data via XBee Cellular.
 */
typedef struct {
    uint8_t protocol;       ///< 0x01 = TCP, 0x02 = UDP
    uint16_t port;          ///< Destination port
    uint8_t ip[4];          ///< Destination IPv4 address
    const uint8_t* payload; ///< Pointer to payload buffer
    uint8_t payloadSize;    ///< Size of payload
} XBeeCellularPacket_t;

/**
 * @brief User configuration parameters for cellular operation.
 */
typedef struct {
    const char* apn;        ///< APN string (e.g. "hologram")
    const char* simPin;     ///< SIM unlock PIN (optional)
    const char* carrier;    ///< Carrier profile (optional)
} XBeeCellularConfig_t;

/**
 * @brief XBeeCellular instance derived from base XBee class.
 */
typedef struct {
    XBee base;
    XBeeCellularConfig_t config;
} XBeeCellular;

/**
 * @brief Applies the given APN/SIM/carrier configuration to a cellular instance.
 */
void XBeeConfigure(XBeeCellular* self, const XBeeCellularConfig_t* config);

/**
 * @brief Initializes the UART interface for the module.
 */
bool XBeeCellularInit(XBee* self, uint32_t baudRate, void* device);

/**
 * @brief Starts LTE network registration and waits until attached.
 */
bool XBeeCellularConnect(XBee* self);

/**
 * @brief Gracefully disconnects from the cellular network.
 */
bool XBeeCellularDisconnect(XBee* self);

/**
 * @brief Returns true if the module is currently connected.
 */
bool XBeeCellularConnected(XBee* self);

/**
 * @brief Sends a payload using UDP or TCP over cellular.
 */
uint8_t XBeeCellularSendData(XBee* self, const void* packet);

/**
 * @brief Issues a soft reset using AT commands.
 */
bool XBeeCellularSoftReset(XBee* self);

/**
 * @brief Invokes platform-specific hard reset logic.
 */
void XBeeCellularHardReset(XBee* self);

/**
 * @brief Processes incoming frames and dispatches handlers.
 */
void XBeeCellularProcess(XBee* self);

/**
 * @brief Allocates and initializes an XBeeCellular instance.
 */
XBeeCellular* XBeeCellularCreate(const XBeeCTable* cTable, const XBeeHTable* hTable);

/**
 * @brief Deallocates the XBeeCellular instance.
 */
void XBeeCellularDestroy(XBeeCellular* self);

#ifdef __cplusplus
}
#endif

#endif // XBEE_CELLULAR_H