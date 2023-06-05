

#if defined(ARDUINO_ARCH_SAMD)

#include <string.h>
#include "utility/server_drv.h"

extern "C" {
  #include "utility/debug.h"
}

#include "WiFi.h"
#include "WiFiClient.h"
#include "WiFiServer.h"

WiFiServer::WiFiServer(uint16_t port) :
  _sock(NO_SOCKET_AVAIL),
  _lastSock(NO_SOCKET_AVAIL)
{
    _port = port;
}

void WiFiServer::begin()
{
    _sock = ServerDrv::getSocket();
    if (_sock != NO_SOCKET_AVAIL)
    {
        ServerDrv::startServer(_port, _sock);
    }
}

WiFiClient WiFiServer::available(byte* status)
{
    int sock = NO_SOCKET_AVAIL;

    if (_sock != NO_SOCKET_AVAIL) {
      // check previous received client socket
      if (_lastSock != NO_SOCKET_AVAIL) {
          WiFiClient client(_lastSock);

          if (client.connected() && client.available()) {
              sock = _lastSock;
          }
      }

      if (sock == NO_SOCKET_AVAIL) {
          // check for new client socket
          sock = ServerDrv::availServer(_sock);
      }
    }

    if (sock != NO_SOCKET_AVAIL) {
        WiFiClient client(sock);

        if (status != NULL) {
            *status = client.status();
        }

        _lastSock = sock;

        return client;
    }

    return WiFiClient(255);
}

uint8_t WiFiServer::status() {
    if (_sock == NO_SOCKET_AVAIL) {
        return CLOSED;
    } else {
        return ServerDrv::getServerState(_sock);
    }
}


size_t WiFiServer::write(uint8_t b)
{
    return write(&b, 1);
}

size_t WiFiServer::write(const uint8_t *buffer, size_t size)
{
    if (size==0)
    {
        setWriteError();
        return 0;
    }

    size_t written = ServerDrv::sendData(_sock, buffer, size);
    if (!written)
    {
        setWriteError();
        return 0;
    }

    if (!ServerDrv::checkDataSent(_sock))
    {
        setWriteError();
        return 0;
    }

    return written;
}

#endif