#ifdef ARDUINO_ARCH_ESP8266
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <user_interface.h>
#include <espconn.h>

#include "SimpleUDPLogger.h"

static int _mode;
static struct espconn _conn;
static esp_udp _udp = {
  .remote_port = 0,
  .local_port = 0,
  .local_ip = {0, 0, 0, 0},
  .remote_ip = {0, 0, 0, 0}
};

static void UdpLoggerPriFin(void *arg) {
  if (arg) {
//    DBG_SERTAL_LOG("%s: state: %d\r\n", __FUNCTION__, ((struct espconn*)arg)->state);
  }
}

void UdpLoggerPriBegin(const char* server, int mode) {
  volatile int err = 0;
  struct ip_info cfg;
  uint32_t srv = ipaddr_addr(server);
  _mode = LOG_MODE_NONE;
  if (server != 0 && *server != 0 && (mode == LOG_MODE_ERROR || mode == LOG_MODE_INFO || mode == LOG_MODE_DEBUG)) {
    // первый вызов UdpLoggerPriBegin
    if (_udp.remote_port == 0) {
      // однажды инициализируем номера портов
      _udp.remote_port = LOG_REMOTE_UDP_PORT;
      _udp.local_port = espconn_port();
    }
    // заполняем в esp_udp адрес удаленного сервера
    _udp.remote_ip[0] = srv & 0xff;
    _udp.remote_ip[1] = (srv >> 8) & 0xff;
    _udp.remote_ip[2] = (srv >> 16) & 0xff;
    _udp.remote_ip[3] = (srv >> 24) & 0xff;
    DBG_SERTAL_LOG("%s: remote: %d.%d.%d.%d:%u\r\n", __FUNCTION__, _udp.remote_ip[0], _udp.remote_ip[1], _udp.remote_ip[2], _udp.remote_ip[3], _udp.remote_port);
    // заполняем в esp_udp локальный адрес
    if (wifi_get_opmode() == STATION_MODE || wifi_get_opmode() == SOFTAP_MODE) {
      wifi_get_ip_info(wifi_get_opmode() == STATION_MODE ? STATION_IF : SOFTAP_IF, &cfg); 
      _udp.local_ip[0] = cfg.ip.addr & 0xff;
      _udp.local_ip[1] = (cfg.ip.addr >> 8) & 0xff;
      _udp.local_ip[2] = (cfg.ip.addr >> 16) & 0xff;
      _udp.local_ip[3] = (cfg.ip.addr >> 24) & 0xff;
      DBG_SERTAL_LOG("%s: local:  %d.%d.%d.%d:%u\r\n", __FUNCTION__, _udp.local_ip[0], _udp.local_ip[1], _udp.local_ip[2], _udp.local_ip[3], _udp.local_port);
    } else if (wifi_get_opmode() == STATIONAP_MODE) {
      struct ip_info cfg2;
      wifi_get_ip_info(STATION_IF, &cfg);
      wifi_get_ip_info(SOFTAP_IF, &cfg2);
      // проверим по маске в какой подсети расположен сервер
      if ((srv & cfg.netmask.addr) == (cfg.ip.addr & cfg.netmask.addr)) {
        _udp.local_ip[0] = cfg.ip.addr & 0xff;
        _udp.local_ip[1] = (cfg.ip.addr >> 8) & 0xff;
        _udp.local_ip[2] = (cfg.ip.addr >> 16) & 0xff;
        _udp.local_ip[3] = (cfg.ip.addr >> 24) & 0xff;
      } else if ((srv & cfg2.netmask.addr) == (cfg2.ip.addr & cfg2.netmask.addr)) {
        _udp.local_ip[0] = cfg2.ip.addr & 0xff;
        _udp.local_ip[1] = (cfg2.ip.addr >> 8) & 0xff;
        _udp.local_ip[2] = (cfg2.ip.addr >> 16) & 0xff;
        _udp.local_ip[3] = (cfg2.ip.addr >> 24) & 0xff;
      }
      DBG_SERTAL_LOG("%s: local:  %d.%d.%d.%d:%u\r\n", __FUNCTION__, _udp.local_ip[0], _udp.local_ip[1], _udp.local_ip[2], _udp.local_ip[3], _udp.local_port);
    } else {
      DBG_SERTAL_LOG("%s: invalid wifi mode for udp logging\r\n", __FUNCTION__);
    }
    memset(&_conn, 0, sizeof(_conn));
    _conn.proto.udp = &_udp;
    _conn.type = ESPCONN_UDP;
    _conn.state = ESPCONN_NONE;
    if ((err = espconn_regist_sentcb(&_conn, UdpLoggerPriFin))) {
      DBG_SERTAL_LOG("%s: espconn_regist_sentcb error %d\r\n", __FUNCTION__, err);
    } else if ((err = espconn_create(&_conn)) && err != ESPCONN_ISCONN) {
      DBG_SERTAL_LOG("%s: espconn_create error %d\r\n", __FUNCTION__, err);
    } else {
      DBG_SERTAL_LOG("%s: log started\r\n", __FUNCTION__);
      _mode = mode;
    }
  } else {
    DBG_SERTAL_LOG("%s: invalid arguments\r\n", __FUNCTION__);
  }
}

void UdpLoggerPriWrite(int mode, const char* fmt, ...) {
  volatile int err;
  va_list ap;
  va_start(ap, fmt);
  if (_mode >= LOG_MODE_ERROR && mode <= _mode) {
    static char buffer[LOG_BUFFER_SIZE] = LOG_HEADER;
    strcpy(buffer + sizeof(LOG_HEADER) - 1, mode == LOG_MODE_ERROR ? LOG_ERROR_ID : mode == LOG_MODE_INFO ? LOG_INFO_ID : LOG_DEBUG_ID);
    size_t len = strlen(buffer);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, fmt, ap);
    if ((err = espconn_sendto(&_conn, (uint8_t*)buffer, len))) {
      DBG_SERTAL_LOG("%s: espconn_sendto error %d\r\n", __FUNCTION__, err);
    }
  }
  va_end(ap);
}

#endif
