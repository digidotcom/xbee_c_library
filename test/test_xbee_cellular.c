#include "unity.h"
#include "xbee_cellular.h"
#include "mock_xbee_api_frames.h"
#include "mock_port.h"
#include <string.h>

// Mock XBee and associated tables
static XBeeCellular mockCellular;
static XBee* self = (XBee*)&mockCellular;
static XBeeHTable htable;
static XBeeCTable ctable;

// Setup and teardown
void setUp(void) {
    memset(&mockCellular, 0, sizeof(mockCellular));
    mockCellular.base.htable = &htable;
    mockCellular.base.ctable = &ctable;
}

void tearDown(void) {}

// Test XBeeCellularInit
void test_XBeeCellularInit_should_return_true_when_uart_init_succeeds(void) {
    htable.PortUartInit_ExpectAndReturn(9600, "COM1", UART_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularInit(self, 9600, "COM1"));
}

// Test XBeeCellularConnected
void test_XBeeCellularConnected_should_return_true_when_AI_is_zero(void) {
    uint8_t dummyResponse = 0;
    uint8_t dummyResponseLength = 1;
    apiSendAtCommandAndGetResponse_ExpectAndReturn(
        self, AT_AI, NULL, 0, &dummyResponse, &dummyResponseLength, 5000, sizeof(dummyResponse), API_SEND_SUCCESS
    );
    TEST_ASSERT_TRUE(XBeeCellularConnected(self));
}

// Test XBeeCellularConnect
void test_XBeeCellularConnect_should_return_false_when_attach_fails(void) {
    mockCellular.config.simPin = "";
    mockCellular.config.apn = "";
    mockCellular.config.carrier = "";
    htable.PortDelay_Ignore();
    htable.PortMillis_ExpectAndReturn(0);
    htable.PortMillis_ExpectAndReturn(60000);
    TEST_ASSERT_FALSE(XBeeCellularConnect(self, true));
}

// Test Disconnect
void test_XBeeCellularDisconnect_should_send_AT_SD_and_return_true(void) {
    apiSendAtCommand_ExpectAndReturn(self, AT_SD, NULL, 0, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularDisconnect(self));
}

// Test SendData
void test_XBeeCellularSendData_should_return_success_when_frame_sent(void) {
    XBeeCellularPacket_t pkt = {
        .protocol = 1,
        .port = 80,
        .payload = (uint8_t*)"test",
        .payloadSize = 4
    };
    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_TX_IPV4, NULL, 11, API_SEND_SUCCESS);
    TEST_ASSERT_EQUAL_HEX8(0x00, XBeeCellularSendData(self, &pkt));
}

// Test SoftReset
void test_XBeeCellularSoftReset_should_send_AT_SD(void) {
    apiSendAtCommand_ExpectAndReturn(self, AT_SD, NULL, 0, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularSoftReset(self));
}

// Test HardReset (no-op)
void test_XBeeCellularHardReset_should_not_crash(void) {
    XBeeCellularHardReset(self);
}

// Test Configure
void test_XBeeCellularConfigure_should_copy_config(void) {
    XBeeCellularConfig_t config = {
        .simPin = "1234",
        .apn = "internet",
        .carrier = "verizon"
    };
    TEST_ASSERT_TRUE(XBeeCellularConfigure(self, &config));
    TEST_ASSERT_EQUAL_STRING("internet", mockCellular.config.apn);
}

// Test SocketCreate
void test_XBeeCellularSocketCreate_should_return_socket_id_on_success(void) {
    uint8_t socketId;
    xbee_api_frame_t response = {
        .type = XBEE_API_TYPE_CELLULAR_SOCKET_CREATE_RESPONSE,
        .data = {0, 1, 0x12, 0x00}
    };
    htable.PortMillis_ExpectAndReturn(0);
    htable.PortMillis_ExpectAndReturn(1000);
    htable.PortDelay_Ignore();
    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_SOCKET_CREATE, NULL, 2, API_SEND_SUCCESS);
    apiReceiveApiFrame_ExpectAndReturn(self, NULL, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularSocketCreate(self, 0x01, &socketId));
}

// Test SocketSend
void test_XBeeCellularSocketSend_should_return_false_on_null_payload(void) {
    TEST_ASSERT_FALSE(XBeeCellularSocketSend(self, 1, NULL, 0));
}

// Test SocketSetOption
void test_XBeeCellularSocketSetOption_should_send_option(void) {
    uint8_t value[2] = {0x01, 0x02};
    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_SOCKET_OPTION, NULL, 5, API_SEND_SUCCESS);
    TEST_ASSERT_TRUE(XBeeCellularSocketSetOption(self, 1, 2, value, sizeof(value)));
}

// Test SocketClose
void test_XBeeCellularSocketClose_should_return_false_on_send_failure(void) {
    apiSendFrame_ExpectAndReturn(self, XBEE_API_TYPE_CELLULAR_SOCKET_CLOSE, NULL, 2, API_SEND_ERROR_UART_FAILURE);
    TEST_ASSERT_FALSE(XBeeCellularSocketClose(self, 2));
}

// // Entry point
// int main(void) {
//     UNITY_BEGIN();
//     RUN_TEST(test_XBeeCellularInit_should_return_true_when_uart_init_succeeds);
//     RUN_TEST(test_XBeeCellularConnected_should_return_true_when_AI_is_zero);
//     RUN_TEST(test_XBeeCellularConnect_should_return_false_when_attach_fails);
//     RUN_TEST(test_XBeeCellularDisconnect_should_send_AT_SD_and_return_true);
//     RUN_TEST(test_XBeeCellularSendData_should_return_success_when_frame_sent);
//     RUN_TEST(test_XBeeCellularSoftReset_should_send_AT_SD);
//     RUN_TEST(test_XBeeCellularHardReset_should_not_crash);
//     RUN_TEST(test_XBeeCellularConfigure_should_copy_config);
//     RUN_TEST(test_XBeeCellularSocketCreate_should_return_socket_id_on_success);
//     RUN_TEST(test_XBeeCellularSocketSend_should_return_false_on_null_payload);
//     RUN_TEST(test_XBeeCellularSocketSetOption_should_send_option);
//     RUN_TEST(test_XBeeCellularSocketClose_should_return_false_on_send_failure);
//     return UNITY_END();
// }