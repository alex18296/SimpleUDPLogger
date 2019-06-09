#ifndef _SIMPLE_UDP_LOGGER_H_
#define _SIMPLE_UDP_LOGGER_H_
#include <Arduino.h>

// Library can be used on ESP8266 or ESP32 architecture.
// Библиотека может быть использована на архитектуре ESP8266 или ESP32.
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

#define LOG_BUFFER_SIZE     1400
#define LOG_REMOTE_UDP_PORT 514
#define LOG_HEADER "<14>ESP: "		// define facility for syslog as "log alert"
#define LOG_ERROR_ID "ERROR: "
#define LOG_INFO_ID "INFO: "
#define LOG_DEBUG_ID "DEBUG: "

// Logging mode enumerator.
// Перечислитель режимов логирования.
enum UdpLoggerMode {

	// Logging disabled
	// Логирование отключено
	LOG_MODE_NONE = -1,

	// Only errors
	// Только ошибки
	LOG_MODE_ERROR,

	// Errors and informational messages
	// Ошибки и информационные сообщения
	LOG_MODE_INFO,

	// Errors, informational and debugging messages
	// Ошибки, информационные и отладочные сообщения
	LOG_MODE_DEBUG
};

// If the use of logging is not defined, define empty macros.
// Если не определено использование логирования, определяем пустые макросы.
#ifndef UDP_LOGGER
#define UDP_LOG_BEGIN(server, mode)
#define UDP_LOG_ERROR(...)
#define UDP_LOG_INFO(...)
#define UDP_LOG_DEBUG(...)
#else
void UdpLoggerPriBegin(const char* server, int mode);
void UdpLoggerPriWrite(int mode, const char* fmt, ...);
#define UDP_LOG_BEGIN(server, mode) UdpLoggerPriBegin(server, mode)
#define UDP_LOG_ERROR(...) UdpLoggerPriWrite(LOG_MODE_ERROR, __VA_ARGS__)
#define UDP_LOG_INFO(...) UdpLoggerPriWrite(LOG_MODE_INFO, __VA_ARGS__)
#define UDP_LOG_DEBUG(...) UdpLoggerPriWrite(LOG_MODE_DEBUG, __VA_ARGS__)

#endif

#define DBG_SERTAL_LOG(...)
//#define DBG_SERTAL_LOG(...) Serial.printf(__VA_ARGS__)

#else
#error Unsupported architecture, use ARDUINO_ARCH_ESP8266 or ARDUINO_ARCH_ESP32
#endif

#endif //_SIMPLE_UDP_LOGGER_H_
