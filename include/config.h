/**
 * @file config.h
 * @brief Define library configurations
 *
 *
 * @version 1.0
 * @date 2025-08-08
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


 #ifndef CONFIG_H
 #define CONFIG_H
 
 #if defined(__cplusplus)
 extern "C"
 {
 #endif
 
 #include "port.h"
 
 // Constants
 #define UART_READ_TIMEOUT_MS 2000
 #define UART_WRITE_TIMEOUT_MS 10
 
 #define API_FRAME_DEBUG_PRINT_ENABLED 0
 #if API_FRAME_DEBUG_PRINT_ENABLED
 #define APIFrameDebugPrint(...) portDebugPrintf(__VA_ARGS__)
 #else
 #define APIFrameDebugPrint(...)
 #endif
 
 #define XBEE_DEBUG_PRINT_ENABLED 0
 #if XBEE_DEBUG_PRINT_ENABLED
 #define XBEEDebugPrint(...) portDebugPrintf(__VA_ARGS__)
 #else
 #define XBEEDebugPrint(...)
 #endif
 
 #if defined(__cplusplus)
 }
 #endif
 
 #endif // CONFIG
