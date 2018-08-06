#pragma once
#include <exception>
#include <string>

#define CUSTOM_EXCEPTION(NAME) \
    class NAME : public virtual std::exception \
    {\
    private:\
        std::string m_message;\
    public:\
        NAME(std::string const &message) : m_message(message) {}\
        const char *what() const throw() {\
            return m_message.c_str();\
        }\
    };

namespace tcp {
    namespace peer_exception {
        CUSTOM_EXCEPTION(send_failed);
        CUSTOM_EXCEPTION(receive_failed);
        CUSTOM_EXCEPTION(disconnect_failed);
    }
}