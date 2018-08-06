#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#endif

#include <functional>
#include <thread>
#include <vector>
#include "peer.h"
#include "server_exception.h"

namespace tcp {
  class peer;

  class server {
   private:
    const char *m_host;
    unsigned short m_port;
#ifdef _WIN32
    SOCKET m_socket;
#else
    int m_socket;
#endif
    addrinfo *m_address;
    std::thread m_accept_thread;
    std::vector<peer *> m_peers;
    std::function<void(peer *, const char *)> m_data_callback;

   public:
    server(const char *host, unsigned short port);
    void start();
    void stop();
    std::vector<peer *> get_peers() { return m_peers; }
    void set_data_callback(const std::function<void(peer *, const char *)> &callback);
  };
}
