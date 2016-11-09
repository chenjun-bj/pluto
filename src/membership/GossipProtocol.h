/**
 *******************************************************************************
 * GossipProtocol.h                                                            *
 *                                                                             *
 * Gossip protocol:                                                            *
 *   - Gossip protocol that implements membership protocol                     *
 *******************************************************************************
 */

#ifndef _GOSSIP_PROTOCOL_H_
#define _GOSSIP_PROTOCOL_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <tuple>
#include <vector>

#include "stdinclude.h"
#include "memberlist.h"
#include "messages.h"

#include "MembershipProtocol.h"
#include "pladdress.h"
#include "PeerLeaveMsg.h"

#include <boost/asio.hpp>

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

class MembershipServer;

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

class GossipProtocol: public MembershipProtocol {
public:
    GossipProtocol(MemberList * mlst, ConfigPortal * cfg,
                   MembershipServer * psvr);
    virtual ~GossipProtocol();

    int handle_messages(Message* msg);
    // Protocol sends heartbeat message periodically and 
    // check member health 
    int handle_timer(int id);

    // self up, should introduce self to group
    virtual int node_up();
    // self down, should notify group that I'm leaving
    virtual int node_down();
    // detect (other) node errors
    virtual int detect_node_error();
    // disseminate error info
    virtual int disseminate_error();
protected:
    void handle_joinrequest(Message * msg);
    void handle_joinresponse(Message * msg);
    void handle_heartbeat(Message * msg);
    void handle_peerleave(Message * msg);

    void send_joinrequest();
    void send_joinresponse(const boost::asio::ip::address& ip, 
                           unsigned short port,
                           const MsgStatus& status=MsgStatus::OK);
    void send_heartbeat();
    void send_peerleave(const PeerLeaveMessage::LeaveReason& reason =
                        PeerLeaveMessage::LeaveReason::NORMAL_SHUTDOWN);

    std::vector<boost::asio::ip::udp::endpoint> get_B_neibors();
    boost::asio::ip::udp::endpoint get_node_endpoint(
                                const struct MemberEntry&) const;

    Message * construct_heartbeat_msg();

    int construct_member_from_htmsg(struct MemberEntry&, const HeartMsgStruct&);

    bool is_self_node(const boost::asio::ip::address &addr, unsigned short) const;
    bool is_self_node(int af, unsigned char* addr, unsigned short) const;

private:
    MemberList   * m_pmember;
    ConfigPortal * m_pconfig;
    MembershipServer * m_pnet;

    bool           m_ingroup;

    // self address
    boost::asio::ip::address m_self_addr;
    int            m_self_af;
    unsigned char  m_self_rawip[PL_IPv6_ADDR_LEN];
    unsigned short m_self_port;

    int64          m_self_hb;

    std::vector< struct MemberEntry > m_failed;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

inline bool GossipProtocol::is_self_node(const boost::asio::ip::address & addr, 
                                         unsigned short port) const
{
    if (port != m_self_port) return false;
    if (addr != m_self_addr) return false;
    return true;
}

inline bool GossipProtocol:: is_self_node(int af, unsigned char* addr, 
                                          unsigned short port) const
{
    if (af != m_self_af) return false;
    if (port != m_self_port) return false;
    int cmplen = PL_IPv6_ADDR_LEN;
    if (af == AF_INET) {
        cmplen = PL_IPv4_ADDR_LEN;
    }
    if (memcmp(m_self_rawip, addr, cmplen) != 0) {
        return false;
    }

    return true; 
}

#endif // _GOSSIP_PROTOCOL_H_

