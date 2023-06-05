#if defined(ARDUINO_ARCH_SAMD)

#ifndef WiFiNINA_TCP_Client_CPP
#define WiFiNINA_TCP_Client_CPP

#include "WiFiNINA_TCP_Client.h"

WiFiNINA_TCP_Client::WiFiNINA_TCP_Client()
{
}

WiFiNINA_TCP_Client::~WiFiNINA_TCP_Client()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
  if (_wcs)
  {
    _wcs->stop();
    delete _wcs;
  }

  if (_wc)
  {
    _wc->stop();
    delete _wc;
  }
#pragma GCC diagnostic pop
  std::string().swap(_host);
}

int WiFiNINA_TCP_Client::firmwareBuildNumber()
{
  if (_fwBuild < 0)
  {
    std::string build = WiFi.getBuild();
    _fwBuild = atoi(build.c_str());
    if (_fwBuild < 21060)
      _fwBuild = 0;
  }
  return _fwBuild;
}

bool WiFiNINA_TCP_Client::begin(const char *host, uint16_t port)
{
  _host = host;
  _port = port;
  return true;
}

bool WiFiNINA_TCP_Client::connected()
{
  if (_fwBuild > 0)
  {
    if (_wcs)
      return _wcs->connected();
  }
  else
  {
    if (_secured)
    {
      if (_wcs)
        return _wcs->connected();
    }
    else
    {
      if (_wc)
        return _wc->connected();
    }
  }

  return false;
}

int WiFiNINA_TCP_Client::send(const char *data)
{
  if (!connect(_secured, _verifyRootCA))
    return TCP_CLIENT_ERROR_CONNECTION_REFUSED;

  if (_fwBuild > 0)
  {
    if (_wcs->write((const uint8_t *)data, strlen(data)) != strlen(data))
      return TCP_CLIENT_ERROR_SEND_DATA_FAILED;
  }
  else
  {
    if (_secured)
    {
      if (_wcs->write((const uint8_t *)data, strlen(data)) != strlen(data))
        return TCP_CLIENT_ERROR_SEND_DATA_FAILED;
    }
    else
    {
      if (_wc->write((const uint8_t *)data, strlen(data)) != strlen(data))
        return TCP_CLIENT_ERROR_SEND_DATA_FAILED;
    }
  }

  return 0;
}

WiFiSSLClient *WiFiNINA_TCP_Client::stream(void)
{
  if (connected())
    return _wcs;
  return nullptr;
}

bool WiFiNINA_TCP_Client::connect(bool secured, bool verify)
{

  _secured = secured;
  _verifyRootCA = verify;

  firmwareBuildNumber();

  if (connected())
  {
    if (_fwBuild > 0)
    {
      while (_wcs->available() > 0)
        _wcs->read();
    }
    else
    {
      while (_wc->available() > 0)
        _wc->read();
    }
    return true;
  }

  if (_fwBuild <= 0)
  {
    if (secured)
    {
      if (!_wcs)
        _wcs = new WiFiSSLClient();

      //use the default SSL connection
      if (_wcs->connect(_host.c_str(), _port) == 0)
        return false;
    }
    else
    {
      if (!_wc)
        _wc = new WiFiClient();

      //use the default TCP connection
      if (_wc->connect(_host.c_str(), _port) == 0)
        return false;
    }
  }
  else
  {

    if (!_wcs)
      _wcs = new WiFiSSLClient();

    //use upgradable connection
    if (_wcs->ns_connect(_host.c_str(), _port) == 0)
      return false;

    if (secured)
    {
      if (_wcs->ns_connectSSL(_host.c_str(), _port, verify) == 0)
        return false;
    }
  }

  return connected();
}

bool WiFiNINA_TCP_Client::connectSSL(bool verify)
{

  if (_fwBuild <= 0)
    return false;

  _verifyRootCA = verify;

  //upgrade connection
  if (!_wcs->ns_connectSSL(_host.c_str(), _port, verify))
    return false;

  _secured = connected();
  return _secured;
}

#endif

#endif
