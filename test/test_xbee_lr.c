#include "unity.h"
#include "xbee_lr.h"
#include "xbee_api_frames.h"
#include <string.h>
#include <stdlib.h>

// --- MOCKS AND STUBS ---

static int dummyStatus = API_SEND_SUCCESS;
static uint8_t dummyResponse[33] = {0};
static uint8_t dummyResponseLength = 0;

int apiSendAtCommandAndGetResponse(XBee* self, at_command_t cmd, const uint8_t *param, uint8_t paramLen,
                                    uint8_t *respBuf, uint8_t *respLen, uint32_t timeout, size_t respBufSize) {
    if (respBuf && respBufSize > 0 && dummyResponseLength > 0) {
        memcpy(respBuf, dummyResponse, dummyResponseLength);
        *respLen = dummyResponseLength;
    }
    return dummyStatus;
}

uint32_t portMillis(void) {
    static uint32_t fake_time = 0;
    return fake_time += 500; // Simulate time progression
}

void portDelay(uint32_t ms) {
    (void)ms; // Simulated delay
}

int asciiToHexArray(const char *ascii, uint8_t *hex, size_t hexLen) {
    if (strlen(ascii) != hexLen * 2) return -1;
    memset(hex, 0xAB, hexLen); // Dummy hex fill
    return 0;
}

// --- TEST SETUP ---

void setUp(void) {
    dummyStatus = API_SEND_SUCCESS;
    dummyResponseLength = 1;
    dummyResponse[0] = 1;
}

void tearDown(void) {}

// --- BASIC API TESTS ---

void test_XBeeLRInit_should_return_true_on_uart_success(void) {
    XBee xbee = {0};
    XBeeHTable htable = {.PortUartInit = [](uint32_t b, void* d){ return UART_SUCCESS; }};
    xbee.htable = &htable;
    TEST_ASSERT_TRUE(XBeeLRInit(&xbee, 9600, NULL));
}

void test_XBeeLRConnected_should_return_true_when_response_is_1(void) {
    XBee xbee = {0};
    dummyResponse[0] = 1;
    dummyResponseLength = 1;
    TEST_ASSERT_TRUE(XBeeLRConnected(&xbee));
}

void test_XBeeLRConnected_should_return_false_on_error(void) {
    dummyStatus = -1;
    TEST_ASSERT_FALSE(XBeeLRConnected(NULL));
}

void test_XBeeLRSetAppKey_should_fail_on_invalid_length(void) {
    TEST_ASSERT_FALSE(XBeeLRSetAppKey(NULL, "ABCD")); // too short
}

void test_XBeeLRSetAppKey_should_fail_on_conversion_error(void) {
    dummyStatus = -1;
    TEST_ASSERT_FALSE(XBeeLRSetAppKey(NULL, "0000000000000000000000000000000G")); // invalid hex
}

void test_XBeeLRSetAppKey_should_pass_on_valid_input(void) {
    const char* appKey = "00000000000000000000000000000000";
    TEST_ASSERT_TRUE(XBeeLRSetAppKey(NULL, appKey));
}

void test_XBeeLRSetNwkKey_should_fail_on_bad_hex(void) {
    TEST_ASSERT_FALSE(XBeeLRSetNwkKey(NULL, "Z234567890ABCDEF1234567890ABCDEF")); // bad char
}

void test_XBeeLRSetJoinRX1Delay_should_succeed_with_valid_value(void) {
    uint32_t delay = 5000;
    TEST_ASSERT_TRUE(XBeeLRSetJoinRX1Delay(NULL, delay));
}

void test_XBeeLRSetRX2Frequency_should_succeed(void) {
    uint32_t freq = 869525000;
    TEST_ASSERT_TRUE(XBeeLRSetRX2Frequency(NULL, freq));
}

void test_XBeeLRSetChannelsMask_should_fail_on_odd_length(void) {
    TEST_ASSERT_FALSE(XBeeLRSetChannelsMask(NULL, "ABC")); // Invalid hex string length
}

void test_XBeeLRGetDevEUI_should_return_string_on_valid_response(void) {
    char buffer[17] = {0};
    dummyResponseLength = 8;
    for (int i = 0; i < 8; i++) dummyResponse[i] = i;
    TEST_ASSERT_TRUE(XBeeLRGetDevEUI(NULL, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("0001020304050607", buffer);
}

// Add more unit tests for:
// - XBeeLRSetAppEUI
// - XBeeLRSetClass
// - XBeeLRSendData (with mocks)
// - XBeeLRHandleTransmitStatus and XBeeLRHandleRxPacket (mock frame parsing)

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_XBeeLRInit_should_return_true_on_uart_success);
    RUN_TEST(test_XBeeLRConnected_should_return_true_when_response_is_1);
    RUN_TEST(test_XBeeLRConnected_should_return_false_on_error);
    RUN_TEST(test_XBeeLRSetAppKey_should_fail_on_invalid_length);
    RUN_TEST(test_XBeeLRSetAppKey_should_fail_on_conversion_error);
    RUN_TEST(test_XBeeLRSetAppKey_should_pass_on_valid_input);
    RUN_TEST(test_XBeeLRSetNwkKey_should_fail_on_bad_hex);
    RUN_TEST(test_XBeeLRSetJoinRX1Delay_should_succeed_with_valid_value);
    RUN_TEST(test_XBeeLRSetRX2Frequency_should_succeed);
    RUN_TEST(test_XBeeLRSetChannelsMask_should_fail_on_odd_length);
    RUN_TEST(test_XBeeLRGetDevEUI_should_return_string_on_valid_response);

    return UNITY_END();
}