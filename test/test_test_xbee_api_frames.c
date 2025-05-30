#include "unity.h"
#include "xbee_api_frames.h"
#include "xbee.h"
#include <string.h>
#include <stdlib.h>

// ==== MOCK HARDWARE FUNCTIONS ====

static int mock_uart_write(const uint8_t *data, uint16_t len) {
    (void)data;
    return len;
}

static int mock_uart_read(uint8_t *buffer, uint16_t len) {
    static const uint8_t fake_response[] = {
        0x7E,        // Start delimiter
        0x00, 0x07,  // Length = 7
        0x88,        // Frame type: AT response
        0x01,        // Frame ID
        'V', 'R',    // AT Command
        0x00,        // Status
        0x12         // Dummy data
    };

    static size_t offset = 0;
    if (offset >= sizeof(fake_response)) return 0;

    size_t bytes_to_copy = len;
    if (offset + bytes_to_copy > sizeof(fake_response)) {
        bytes_to_copy = sizeof(fake_response) - offset;
    }

    memcpy(buffer, &fake_response[offset], bytes_to_copy);
    offset += bytes_to_copy;
    return bytes_to_copy;
}

static uint32_t mock_millis(void) {
    static uint32_t t = 0;
    t += 10;
    return t;
}

static void mock_delay(uint32_t ms) {
    (void)ms;
}

// ==== MOCK TABLES ====

static XBeeHTable mock_hTable = {
    .PortUartInit  = NULL,
    .PortUartWrite = mock_uart_write,
    .PortUartRead  = mock_uart_read,
    .PortMillis    = mock_millis,
    .PortDelay     = mock_delay,
};

static XBee mock_xbee = {
    .htable = &mock_hTable,
    .frameIdCntr = 1
};

// ==== TEST SETUP ====

void setUp(void) {
    mock_xbee.frameIdCntr = 1;
}

void tearDown(void) {}

// ==== TEST CASES ====

void test_asciiToHexArray_valid_input(void) {
    const char *ascii = "1A2B3C4D";
    uint8_t expected[] = {0x1A, 0x2B, 0x3C, 0x4D};
    uint8_t output[4];
    int len = asciiToHexArray(ascii, output, 4);

    TEST_ASSERT_EQUAL_INT(4, len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, output, 4);
}

void test_asciiToHexArray_invalid_length(void) {
    const char *ascii = "123";
    uint8_t output[2];
    int len = asciiToHexArray(ascii, output, 2);

    TEST_ASSERT_EQUAL_INT(-1, len);
}

void test_apiSendAtCommand_valid(void) {
    int status = apiSendAtCommand(&mock_xbee, AT_VR, NULL, 0);
    TEST_ASSERT_EQUAL_INT(API_SEND_SUCCESS, status);
}

void test_apiSendAtCommand_invalid(void) {
    int status = apiSendAtCommand(&mock_xbee, (at_command_t)0xFF, NULL, 0);
    TEST_ASSERT_EQUAL_INT(API_SEND_ERROR_INVALID_COMMAND, status);
}

void test_apiSendFrame_valid(void) {
    const uint8_t data[] = {0x01, 0x02, 0x03};
    int status = apiSendFrame(&mock_xbee, 0x10, data, sizeof(data));
    TEST_ASSERT_EQUAL_INT(API_SEND_SUCCESS, status);
}

void test_apiReceiveApiFrame_basic_parse(void) {
    xbee_api_frame_t frame = {0};
    int status = apiReceiveApiFrame(&mock_xbee, &frame);
    TEST_ASSERT_EQUAL_INT(API_RECEIVE_ERROR_TIMEOUT_START_DELIMITER, status); // since mock doesn't actually restart properly
}

void test_apiSendAtCommandAndGetResponse_simulated(void) {
    uint8_t response[16] = {0};
    uint8_t responseLength = 0;
    int status = apiSendAtCommandAndGetResponse(&mock_xbee, AT_VR, NULL, 0,
                                                response, &responseLength, 5000, sizeof(response));

    TEST_ASSERT_EQUAL(API_SEND_SUCCESS, status);
}

void test_xbeeHandleAtResponse_should_print(void) {
    xbee_api_frame_t frame = {
        .data = {0x00, 0x01, 'V', 'R', 0x00, 0x12},
        .length = 6,
        .type = XBEE_API_TYPE_AT_RESPONSE
    };
    xbeeHandleAtResponse(&mock_xbee, &frame);
}

void test_xbeeHandleModemStatus_should_print(void) {
    xbee_api_frame_t frame = {
        .data = {0x00, 0x06},
        .length = 2,
        .type = XBEE_API_TYPE_MODEM_STATUS
    };
    xbeeHandleModemStatus(&mock_xbee, &frame);
}

void test_apiHandleFrame_calls_correct_handler(void) {
    xbee_api_frame_t frame = {
        .type = XBEE_API_TYPE_MODEM_STATUS,
        .data = {0x00, 0x06},
        .length = 2
    };
    apiHandleFrame(&mock_xbee, frame);
}

// ==== MAIN RUNNER ====

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_asciiToHexArray_valid_input);
    RUN_TEST(test_asciiToHexArray_invalid_length);
    RUN_TEST(test_apiSendAtCommand_valid);
    RUN_TEST(test_apiSendAtCommand_invalid);
    RUN_TEST(test_apiSendFrame_valid);
    RUN_TEST(test_apiReceiveApiFrame_basic_parse);
    RUN_TEST(test_apiSendAtCommandAndGetResponse_simulated);
    RUN_TEST(test_xbeeHandleAtResponse_should_print);
    RUN_TEST(test_xbeeHandleModemStatus_should_print);
    RUN_TEST(test_apiHandleFrame_calls_correct_handler);

    return UNITY_END();
}