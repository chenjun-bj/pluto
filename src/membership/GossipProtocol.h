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
#include "stdinclude.h"
#include "memberlist.h"
#include "messages.h"

#include "MembershipProtocol.h"

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

class GossipProtocol: public MembershipProtocol {
public:
    GossipProtocol(MemberList * mlst, ConfigPortal * cfg);
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
private:
    MemberList   * m_pmember;
    ConfigPortal * m_pconfig;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _GOSSIP_PROTOCOL_H_

