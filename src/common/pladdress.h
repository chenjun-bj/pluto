/**
 *******************************************************************************
 * pladdress.h                                                                 *
 *                                                                             *
 * IP address declaraction:                                                    *
 *******************************************************************************
 */

#ifndef _PL_ADDRESS_H_
#define _PL_ADDRESS_H_

/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <string>
#include <stdexcept>

#include <arpa/inet.h>

#include <cstdio>

#include "plexcept.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */
#define PL_IPv4_ADDR_LEN 4
#define PL_IPv6_ADDR_LEN 16

/**
 *******************************************************************************
 * Class declaraction                                                          *
 *******************************************************************************
 */

class Address {
public:
    Address(const unsigned char* buf, const size_t size) {
        parse(buf, size);
    };
  
    Address(int af, int type, 
            const unsigned char* addr, unsigned short port,
            size_t addr_len = PL_IPv4_ADDR_LEN) {
        set_ip_addr(af, addr, addr_len);
        set_port(port);
        set_type(type);
    }

    Address(const Address& other):
       m_af(other.m_af),
       m_type(other.m_type),
       m_portnum(other.m_portnum) {
        memcpy(m_addr, other.m_addr, other.get_addr_len());
    }

    Address& operator=(const Address& other) {
        if (this != &other) {
            memcpy(m_addr, other.m_addr, other.get_addr_len());
            m_af      = other.m_af;
            m_type    = other.m_type;
            m_portnum = other.m_portnum;
        }
        return *this;
    }

    Address(Address&& other):
       m_af(other.m_af),
       m_type(other.m_type),
       m_portnum(other.m_portnum) {
        memcpy(m_addr, other.m_addr, other.get_addr_len());
    }

    Address& operator=(Address&& other) {
        if (this != &other) {
            memcpy(m_addr, other.m_addr, other.get_addr_len());
            m_af      = other.m_af;
            m_type    = other.m_type;
            m_portnum = other.m_portnum;
        }
        return *this;
    }

    void parse(const unsigned char*buf, const size_t size) throw (parse_error) 
    {
        /* Format 
         * int - Address Family: AF_INET, AF_INET6 
         * int - Type: SOCK_STREAM, SOCK_DGRAM
         * unsigned char[] - Self address, 4/16 bytes
         * unsigned short - Self portnumber
         * unsigned short - Reserved 
         * int - Reserved2 for AF_INET6
         */
        if (buf==nullptr) {
            throw parse_error("null buffer");
        }

        size_t minlen = 2*sizeof(int32) + 2*sizeof(int16) + PL_IPv4_ADDR_LEN;
        if (size < minlen) {
            throw parse_error("size error");
        }

        int ival;
        // Address family
        memcpy(&ival, buf, sizeof(int32));
        buf += sizeof(int32);

        ival = ntohl(ival);
        if ((ival != AF_INET) && (ival != AF_INET6)) {
            throw parse_error("invalid address family: " +
                              std::to_string(ival));
        }
        m_af = ival;

        // Type
        memcpy(&ival, buf, sizeof(int32));
        buf += sizeof(int32);

        ival = ntohl(ival);
        if ((ival != SOCK_STREAM) && (ival != SOCK_DGRAM)) {
            throw parse_error("invalid type: " +
                              std::to_string(ival));
        }
        m_type = ival;

        // IP address
        if (m_af == AF_INET) {
            // IPv4
            memcpy(m_addr, buf, PL_IPv4_ADDR_LEN);
            buf += PL_IPv4_ADDR_LEN;
        } else {
            // IPv6
            if (size < minlen + 16) {
                throw parse_error("IPv6 size error");
            }
            memcpy(m_addr, buf, PL_IPv6_ADDR_LEN);
            buf += PL_IPv6_ADDR_LEN;
        }

        uint16 sval;
        // Port number
        memcpy(&sval, buf, sizeof(uint16));
        buf += sizeof(uint16);
        m_portnum = ntohs(sval);

        // Reserved
        memcpy(&sval, buf, sizeof(uint16));
        buf += sizeof(uint16);

        if (m_af == AF_INET6) {
            buf += sizeof(int32); // Reserved2
        }
    }

    // serialize 
    void build(unsigned char* buf, size_t size)
    {
        size_t req_len = get_required_buf_len();
        
        if (req_len == 0) {
            throw std::length_error("internal size error");
        }

        if (size < req_len) {
            throw std::length_error("no enough size");
        }

        if (buf == nullptr) {
            throw std::invalid_argument("nullptr");
        }

        int32 ival;
        uint16 sval;

        ival = htonl(m_af);
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);

        ival = htonl(m_type);
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);

        if (m_af == AF_INET) {
            memcpy(buf, m_addr, PL_IPv4_ADDR_LEN);
            buf += PL_IPv4_ADDR_LEN;
        }
        else if (m_af == AF_INET6) {
            memcpy(buf, m_addr, PL_IPv6_ADDR_LEN);
            buf += PL_IPv6_ADDR_LEN;
        }

        sval = htons(m_portnum);
        memcpy(buf, &sval, sizeof(uint16));
        buf += sizeof(uint16);

        sval = 0;
        memcpy(buf, &sval, sizeof(uint16));
        if (m_af == AF_INET6) {
            ival = 0;
            memcpy(buf, &ival, sizeof(int32));
        }
    }

    size_t get_addr_len() const  {
        if (m_af==AF_INET) return PL_IPv4_ADDR_LEN;
        if (m_af==AF_INET6) return PL_IPv6_ADDR_LEN;
        return 0;
    }

    size_t get_required_buf_len() const  {
        unsigned len = 2 * sizeof(int32) + 2 * sizeof(int16);
        if (m_af==AF_INET) return PL_IPv4_ADDR_LEN + len;
        // Additional int for reserved in AF_INET6
        if (m_af==AF_INET6) return PL_IPv6_ADDR_LEN + len + sizeof(int32);
        return 0;
    }

    void set_ip_addr(int af, const unsigned char* addr, 
                     unsigned addr_len = PL_IPv4_ADDR_LEN) {
        if ((af != AF_INET) && (af != AF_INET6)) {
            throw std::invalid_argument("invalid parameter 'af'"+
                                         std::to_string(af));
        }
        if ((af == AF_INET) && (addr_len < PL_IPv4_ADDR_LEN)) {
            throw std::invalid_argument("invalid parameter 'addr_len'="+
                                         std::to_string(addr_len));
        }
        if ((af == AF_INET6) && (addr_len < PL_IPv6_ADDR_LEN)) {
            throw std::invalid_argument("invalid parameter 'addr_len'="+
                                         std::to_string(addr_len));
        }
        if (addr == nullptr) {
            throw std::invalid_argument("nullptr 'addr'");
        }

        m_af = af;
        unsigned sz = af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN;
        memcpy(m_addr, addr, sz);
    }

    void set_type(int type) {
        if ((type != SOCK_STREAM) && (type != SOCK_DGRAM)) {
            throw std::invalid_argument("invalid parameter 'type'="+
                                         std::to_string(type));
        }
        m_type = type;
    }

    void set_port(unsigned short port) {
        m_portnum = port;
    }

    const unsigned char* get_ip_addr(int *af) const {
        *af = m_af;
        return m_addr;
    }

    unsigned short get_port() const {
        return m_portnum;
    }

    int get_type() const {
        return m_type;
    }

    void dump(int (*output)(const char*,...)=std::printf) const {
        if (m_af == AF_INET) {
            output("IPv4: ");
            output("%0d.%0d.%0d.%0d\n", m_addr[0],
                                        m_addr[1],
                                        m_addr[2],
                                        m_addr[3]);
        }
        else if (m_af == AF_INET6) {
            output("IPv6: ");
            output("%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X\n", 
                                        m_addr[0],  m_addr[1],  m_addr[2],  m_addr[3],
                                        m_addr[4],  m_addr[5],  m_addr[6],  m_addr[7],
                                        m_addr[8],  m_addr[9],  m_addr[10], m_addr[11],
                                        m_addr[12], m_addr[13], m_addr[14], m_addr[15]);
        }

        if (m_type == SOCK_STREAM) {
            output("TCP\n");
        } else if (m_type == SOCK_DGRAM) {
            output("UDP\n");
        }

        output("Port: %d\n", m_portnum);
    }
private:
    unsigned char  m_addr[PL_IPv6_ADDR_LEN];
    int            m_af;
    int            m_type;
    unsigned short m_portnum;
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _PL_ADDRESS_H_

