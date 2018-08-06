#include "peer.h"
#include <utility>

#if defined(_DEBUG) || !defined(NDEBUG)
#define DBG_PRINT(format, ...) printf(format, ##__VA_ARGS__);
#else
#define DBG_PRINT(format, ...)
#endif

namespace tcp {
#ifdef _WIN32
    peer::peer(std::function<void(peer*, const char*)> data_callback, const SOCKET socket, const sockaddr_in address)
        : m_socket(socket), m_address(address), m_data_callback(std::move(data_callback)) {
#else
    peer::peer(std::function<void(peer *, const char *)> data_callback, const int socket, const sockaddr_in address)
        : m_data_callback(data_callback), m_socket(socket), m_address(address) {
#endif
        inet_ntop(AF_INET, &m_address.sin_addr, m_host, INET_ADDRSTRLEN);
        m_port = ntohs(m_address.sin_port);
        DBG_PRINT("new peer from %s:%d\n", m_host, m_port);
    }

    void peer::begin_receive() {
        char buffer[4096];
        while (true) {
            memset(buffer, 0, sizeof(buffer));

            const int result = recv(m_socket, buffer, sizeof(buffer), 0);

            if (result > 0) {
                DBG_PRINT("received %d bytes from %s:%d: %s\n", result, m_host, m_port, buffer);

                if (m_data_callback) {
                    m_data_callback(this, buffer);
                }
            } else if (result == 0) {
                disconnect();
                break;
            } else {
                DBG_PRINT("receive failed\n");

#ifdef _WIN32
                closesocket(m_socket);
                WSACleanup();
#else
                close(m_socket);
#endif
                throw peer_exception::receive_failed("Receiving data failed");
            }
        }
    }

    void peer::disconnect() {
        DBG_PRINT("shutting down peer at %s:%d...", m_host, m_port);
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
            throw peer_exception::disconnect_failed("Failed to close the connection");
        }
        DBG_PRINT("ok\n");

#ifdef _WIN32
        closesocket(m_socket);
        WSACleanup();
#else
        close(m_socket);
#endif
    }

    void peer::send(const char *payload) {
        DBG_PRINT("sending %d bytes...", sizeof(payload));
#ifdef _WIN32
        if (::send(m_socket, payload, sizeof(payload), 0) == SOCKET_ERROR) {
#else
        if (::send(m_socket, payload, sizeof(payload), 0) == -1) {
#endif
            DBG_PRINT("err\n");
#ifdef _WIN32
            closesocket(m_socket);
            WSACleanup();
#else
            close(m_socket);
#endif
            throw peer_exception::send_failed("Failed to send payload");
        }
        DBG_PRINT("ok\n");
    }
}