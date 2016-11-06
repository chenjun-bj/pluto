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
#include <vector>

#include "stdinclude.h"
#include "memberlist.h"
#include "messages.h"

#include "MembershipProtocol.h"

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
    void send_joinresponse();
    void send_heartbeat();
    void send_peerleave();

    std::vector<boost::asio::ip::udp::endpoint> get_B_neibors();
    boost::asio::ip::udp::endpoint get_node_endpoint(
                                const struct MemberEntry&) const;

    Message * construct_heartbeat_msg();
private:
    MemberList   * m_pmember;
    ConfigPortal * m_pconfig;
    MembershipServer * m_pnet;

    bool           m_ingroup;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _GOSSIP_PROTOCOL_H_

