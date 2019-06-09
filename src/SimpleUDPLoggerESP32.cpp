#ifdef ARDUINO_ARCH_ESP32
#include <IPAddress.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#include "SimpleUDPLogger.h"

static int _mode;
static int _fd = -1;
static struct sockaddr_in _sa;
static SemaphoreHandle_t _semaphore = 0;

void UdpLoggerPriBegin(const char* server, int mode) {
  u32_t srv;
  _mode = LOG_MODE_NONE;
  if (server != 0 && *server != 0 && (srv = ipaddr_addr(server)) != 0 && (mode == LOG_MODE_ERROR || mode == LOG_MODE_INFO || mode == LOG_MODE_DEBUG)) {
    // первый вызов UdpLoggerPriBegin
    if (_semaphore == 0) {
      if ((_semaphore = xSemaphoreCreateBinary()) == 0) {
        DBG_SERTAL_LOG("%s: xSemaphoreCreateBinary failed\r\n", __FUNCTION__);
      } else {
        if (xSemaphoreGive(_semaphore) != pdTRUE) {
          DBG_SERTAL_LOG("%s: xSemaphoreGive failed\r\n", __FUNCTION__);
          vSemaphoreDelete(_semaphore);
          _semaphore = 0;
        } else if ((_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
          DBG_SERTAL_LOG("%s: socket failed\r\n", __FUNCTION__);
          vSemaphoreDelete(_semaphore);
          _semaphore = 0;
        }
      }
    }
    if (_fd >= 0) {
      memset(&_sa, 0, sizeof(_sa));
      _sa.sin_family = AF_INET;
      _sa.sin_addr.s_addr = srv;
      _sa.sin_port = htons(LOG_REMOTE_UDP_PORT);
      DBG_SERTAL_LOG("%s: log started\r\n", __FUNCTION__);
      _mode = mode;
    }
  } else {
    DBG_SERTAL_LOG("%s: invalid arguments\r\n", __FUNCTION__);
  }
}

void UdpLoggerPriWrite(int mode, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  if (_mode >= LOG_MODE_ERROR && mode <= _mode && xSemaphoreTake(_semaphore, portMAX_DELAY) == pdTRUE) {
    static char buffer[LOG_BUFFER_SIZE] = LOG_HEADER;
    strcpy(buffer + sizeof(LOG_HEADER) - 1, mode == LOG_MODE_ERROR ? LOG_ERROR_ID : mode == LOG_MODE_INFO ? LOG_INFO_ID : LOG_DEBUG_ID);
    size_t len = strlen(buffer);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, fmt, ap);
    sendto(_fd, buffer, len, 0, (struct sockaddr*)&_sa, sizeof(_sa));
    xSemaphoreGive(_semaphore);
  }
  va_end(ap);
}

#endif
