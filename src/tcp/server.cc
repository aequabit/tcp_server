#include "server.h"

#if defined(_DEBUG) || !defined(NDEBUG)
#define DBG_PRINT(format, ...) printf(format, ##__VA_ARGS__);
#else
#define DBG_PRINT(format, ...)
#endif

namespace tcp {
  server::server(const char *host, const unsigned short port) : m_host(host), m_port(port) {
#ifdef _WIN32
    WSADATA wsa_data;
#endif

#ifdef _WIN32
    DBG_PRINT("initializing winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
      DBG_PRINT("err\n");
      throw server_exception::wsa_startup_failed("WSAStartup failed");
    }
    DBG_PRINT("ok\n");
#endif

    struct addrinfo hints{};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char port_str[8];
#ifdef _WIN32
    sprintf_s(port_str, sizeof(port_str), "%d", m_port);
#else
    sprintf(port_str, "%d", m_port);
#endif

    DBG_PRINT("resolving host...");
    if (getaddrinfo(nullptr, port_str, &hints, &m_address) != 0) {
      DBG_PRINT("err\n");
#ifdef _WIN32
      WSACleanup();
#endif
      throw server_exception::hostname_resolve_failed("Failed to resolve hostname");
    }
    DBG_PRINT("ok\n");

    DBG_PRINT("creating socket...");
    m_socket = socket(m_address->ai_family, m_address->ai_socktype, m_address->ai_protocol);
#ifdef _WIN32
    if (m_socket == INVALID_SOCKET) {
#else
    if (m_socket == -1) {
#endif
      DBG_PRINT("err\n");
      freeaddrinfo(m_address);
#ifdef _WIN32
      WSACleanup();
#endif
      throw server_exception::socket_creation_failed("Failed to create socket");
    }
    DBG_PRINT("ok\n");
  }

  void server::start() {
    DBG_PRINT("binding server socket...");
#ifdef _WIN32
    if (bind(m_socket, m_address->ai_addr, (int) m_address->ai_addrlen) == SOCKET_ERROR) {
#else
    if (bind(m_socket, m_address->ai_addr, (int) m_address->ai_addrlen) == -1) {
#endif
      DBG_PRINT("err\n");
      freeaddrinfo(m_address);
#ifdef _WIN32
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      WSACleanup();
#else
      close(m_socket);
      m_socket = -1;
#endif

      throw server_exception::bind_failed("Failed to bind socket");
    }
    DBG_PRINT("ok\n");

    freeaddrinfo(m_address);

    DBG_PRINT("listening...");

#ifdef _WIN32
    if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
#else
    if (listen(m_socket, SOMAXCONN) == -1) {
#endif

      DBG_PRINT("err\n");

#ifdef _WIN32
      closesocket(m_socket);
      WSACleanup();
#else
      close(m_socket);
#endif

      throw server_exception::listen_failed("Failed to listen on socket");
    }
    DBG_PRINT("ok\n");

    m_accept_thread = std::thread([this]() {
      while (true) {
        struct sockaddr_in address{};
        socklen_t address_length = sizeof(address);

#ifdef _WIN32
        SOCKET socket = accept(m_socket, (struct sockaddr *) &address, &address_length);
        if (socket == INVALID_SOCKET) {

#else
        int socket = accept(m_socket, (struct sockaddr *) &address, &address_length);
        if (socket == -1) {
#endif

          continue;
        }

        peer *client = new peer(m_data_callback, socket, address);
        m_peers.push_back(client);

        auto client_thread = std::thread([&client]() {
          client->begin_receive();
        });

        client_thread.detach();

      }
    });
  }

  void server::stop() {
    DBG_PRINT("shutting down socket...");
#ifdef _WIN32
    if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR) {
#else
    if (shutdown(m_socket, SHUT_WR) == -1) {
#endif
      DBG_PRINT("err\n");
#ifdef _WIN32
      closesocket(m_socket);
      WSACleanup();
#else
      close(m_socket);
#endif

      //throw exception::disconnect_failed("Failed to close the connection");
    }
    DBG_PRINT("ok\n");

#ifdef _WIN32
    closesocket(m_socket);
     WSACleanup();
#else
    close(m_socket);
#endif
  }

  void server::set_data_callback(const std::function<void(peer *, const char *)> &callback) {
    m_data_callback = callback;
  }
}
