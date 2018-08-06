#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <functional>
#include <thread>
#include "peer_exception.h"
#include "server.h"

namespace tcp {
class peer {
 private:
#ifdef _WIN32
    SOCKET m_socket;
#else
    int m_socket;
#endif
  sockaddr_in m_address;
  char m_host[INET_ADDRSTRLEN];
  unsigned short m_port;
  std::thread m_thread;
  std::function<void(peer*, const char*)> m_data_callback;

 public:
#ifdef _WIN32
    peer(std::function<void(peer*, const char*)> data_callback, SOCKET socket, sockaddr_in address);
#else
    peer(std::function<void(peer*, const char*)> data_callback, int socket, sockaddr_in address);
#endif
  void begin_receive();
  void disconnect();
  void send(const char* payload);
};
}
 