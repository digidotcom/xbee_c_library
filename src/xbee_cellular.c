/***************************************************************************//**
 * @file xbee_cellular.c
 * @brief Implementation of XBee 3 Cellular subclass.
 *
 * This file contains the implementation of functions specific to the XBee 3 Cellular module.
 * It includes methods for initializing, connecting to LTE networks, sending data,
 * and handling module configuration and runtime events.
 *
 * @version 1.2
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

#include "xbee_cellular.h"
#include "xbee_api_frames.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*****************************************************************************/
/**
 * @brief Initializes the XBee Cellular device with given UART settings.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] baudRate UART baud rate.
 * @param[in] device Pointer to serial device identifier.
 *
 * @return true if UART was initialized successfully, false otherwise.
 ******************************************************************************/
bool XBeeCellularInit(XBee* self, uint32_t baudRate, void* device) {
    return (self->htable->PortUartInit(baudRate, device)) == UART_SUCCESS ? true : false;
}

/*****************************************************************************/
/**
 * @brief Determines if the XBee Cellular module is connected to a network.
 *
 * Sends AT command `AI` to check for network registration.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if connected, false otherwise.
 ******************************************************************************/
bool XBeeCellularConnected(XBee* self) {
    uint8_t response = 0;
    uint8_t responseLength;
    int status = apiSendAtCommandAndGetResponse(self, AT_AI, NULL, 0, &response, &responseLength, 5000);
    return (status == API_SEND_SUCCESS && response == 0);
}

/*****************************************************************************/
/**
 * @brief Attempts to connect the XBee Cellular module to the LTE network.
 *
 * Applies SIM PIN, APN, and carrier profile settings. Then polls `AI` for attach success.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if registration succeeded, false otherwise.
 ******************************************************************************/
bool XBeeCellularConnect(XBee* self) {
    XBeeCellular* cell = (XBeeCellular*) self;
    XBEEDebugPrint("Applying cellular config and attempting attach...\n");

    if (cell->config.simPin && strlen(cell->config.simPin) > 0) {
        apiSendAtCommand(self, AT_PN, (const uint8_t*)cell->config.simPin, strlen(cell->config.simPin));
    }

    if (cell->config.apn && strlen(cell->config.apn) > 0) {
        apiSendAtCommand(self, AT_AN, (const uint8_t*)cell->config.apn, strlen(cell->config.apn));
    }

    if (cell->config.carrier && strlen(cell->config.carrier) > 0) {
        apiSendAtCommand(self, AT_CP, (const uint8_t*)cell->config.carrier, strlen(cell->config.carrier));
    }

    for (int i = 0; i < 20; ++i) {
        if (XBeeCellularConnected(self)) {
            XBEEDebugPrint("Successfully attached to cellular network.\n");
            return true;
        }
        self->htable->PortDelay(1000);
    }

    XBEEDebugPrint("Network attach failed.\n");
    return false;
}

/*****************************************************************************/
/**
 * @brief Gracefully disconnects the XBee Cellular module using AT command.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if AT_SD was accepted successfully.
 ******************************************************************************/
bool XBeeCellularDisconnect(XBee* self) {
    int status = apiSendAtCommand(self, AT_SD, NULL, 0);
    return (status == API_SEND_SUCCESS);
}

/*****************************************************************************/
/**
 * @brief Sends a UDP/TCP data packet over the cellular interface.
 *
 * @param[in] self Pointer to the XBee instance.
 * @param[in] data Pointer to a XBeeCellularPacket_t structure.
 *
 * @return 0 if sent successfully, 0xFF on error.
 ******************************************************************************/
uint8_t XBeeCellularSendData(XBee* self, const void* data) {
    XBeeCellularPacket_t* packet = (XBeeCellularPacket_t*) data;
    uint8_t frame[128];
    uint8_t frameId = self->frameIdCntr;
    uint8_t offset = 0;

    frame[offset++] = frameId;
    frame[offset++] = packet->protocol;
    frame[offset++] = packet->port >> 8;
    frame[offset++] = packet->port & 0xFF;
    memcpy(&frame[offset], packet->ip, 4); offset += 4;
    memcpy(&frame[offset], packet->payload, packet->payloadSize); offset += packet->payloadSize;

    int status = apiSendFrame(self, XBEE_API_TYPE_CELLULAR_TX_IPV4, frame, offset);
    return (status == API_SEND_SUCCESS) ? 0x00 : 0xFF;
}

/*****************************************************************************/
/**
 * @brief Issues a soft reset via AT command to gracefully shut down the modem.
 *
 * @param[in] self Pointer to the XBee instance.
 *
 * @return true if shutdown command was sent successfully.
 ******************************************************************************/
bool XBeeCellularSoftReset(XBee* self) {
    return apiSendAtCommand(self, AT_SD, NULL, 0) == API_SEND_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Placeholder for hard reset logic (implementation platform-specific).
 *
 * @param[in] self Pointer to the XBee instance.
 ******************************************************************************/
void XBeeCellularHardReset(XBee* self) {
    // Optional stub
}

/*****************************************************************************/
/**
 * @brief Continuously polls for incoming frames from the XBee module.
 *
 * @param[in] self Pointer to the XBee instance.
 ******************************************************************************/
void XBeeCellularProcess(XBee* self) {
    xbee_api_frame_t frame;
    int status = apiReceiveApiFrame(self, &frame);
    if (status == API_SEND_SUCCESS) {
        apiHandleFrame(self, frame);
    }
}

/**
 * @brief Configures the XBee Cellular module with APN, SIM PIN, and carrier settings.
 *
 * This function casts the generic config pointer to an XBeeCellularConfig_t structure and applies
 * the settings to the cellular instance. This is typically called through the base class interface
 * `XBeeConfigure()` which dispatches to this function via the vtable.
 *
 * @param[in] self Pointer to the base XBee instance.
 * @param[in] config Pointer to a valid XBeeCellularConfig_t structure.
 *
 * @return bool Returns true if configuration was applied successfully, otherwise false.
 */
static bool XBeeCellularConfigure(XBee* self, const void* config) {
    if (!self || !config) return false;

    XBeeCellular* cell = (XBeeCellular*)self;
    const XBeeCellularConfig_t* cfg = (const XBeeCellularConfig_t*)config;

    // Save config to internal copy
    cell->config = *cfg;
    return true;
}

/*****************************************************************************/
/**
 * @brief Virtual function dispatch table for XBeeCellular methods.
 ******************************************************************************/
static const XBeeVTable XBeeCellularVTable = {
    .init = XBeeCellularInit,
    .process = XBeeCellularProcess,
    .connect = XBeeCellularConnect,
    .disconnect = XBeeCellularDisconnect,
    .sendData = XBeeCellularSendData,
    .softReset = XBeeCellularSoftReset,
    .hardReset = XBeeCellularHardReset,
    .connected = XBeeCellularConnected,
    .handleRxPacketFrame = NULL,
    .handleTransmitStatusFrame = NULL,
    .configure = XBeeCellularConfigure
};

/*****************************************************************************/
/**
 * @brief Allocates and initializes a new XBeeCellular instance.
 *
 * @param[in] cTable Callback table for RX/TX handlers.
 * @param[in] hTable Platform-specific HAL interface table.
 *
 * @return Pointer to the new XBeeCellular instance.
 ******************************************************************************/
XBeeCellular* XBeeCellularCreate(const XBeeCTable* cTable, const XBeeHTable* hTable) {
    XBeeCellular* instance = (XBeeCellular*)malloc(sizeof(XBeeCellular));
    instance->base.vtable = &XBeeCellularVTable;
    instance->base.ctable = cTable;
    instance->base.htable = hTable;
    return instance;
}

/*****************************************************************************/
/**
 * @brief Frees memory allocated to an XBeeCellular instance.
 *
 * @param[in] self Pointer to the instance to destroy.
 ******************************************************************************/
void XBeeCellularDestroy(XBeeCellular* self) {
    free(self);
}


