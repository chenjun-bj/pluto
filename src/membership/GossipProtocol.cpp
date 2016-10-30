/**
 *******************************************************************************
 * GossipProtocol.cpp                                                          *
 *                                                                             *
 * Gossip protocol:                                                            *
 *   - Gossip protocol that implements membership protocol                     *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "memberlist.h"
#include "messages.h"

#include "GossipProtocol.h"

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Class definition                                                           *
 *******************************************************************************
 */

GossipProtocol::GossipProtocol(MemberList *mlist, ConfigPortal * cfg)
{
    if ((mlist==nullptr) || (cfg==nullptr)) {
        throw std::invalid_argument("nullptr");
    }
    m_pmember = mlist;
    m_pconfig = cfg;
}

GossipProtocol::~GossipProtocol()
{
}

int GossipProtocol::handle_messages(Message* msg)
{
    if (msg == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "'%s': nullptr\n", __func__);
        return -1; 
    }

    switch(msg->get_msgtype()) {
    case MsgType::JOINREQ:
        handle_joinrequest(msg);
        break;
    case MsgType::JOINRESP:
        handle_joinresponse(msg);
        break;
    case MsgType::HEARTBEAT:
        handle_heartbeat(msg);
        break;
    case MsgType::PEERLEAVE:
        handle_peerleave(msg);
        break;
    default:
        getlog()->sendlog(LogLevel::ERROR, "'%s': unsupported message, type=%s\n", __func__, get_desc_msgtype(msg->get_msgtype()).c_str());
        return -1;
    }
    return 0;
}

int GossipProtocol::handle_timer(int id)
{
}

int GossipProtocol::node_up()
{
}

int GossipProtocol::node_down()
{
}

int GossipProtocol::detect_node_error()
{
}

int GossipProtocol::disseminate_error()
{
    // TODO: adds error nodes in heartbeat message
    return 0;
}

void GossipProtocol::handle_joinrequest(Message * msg)
{
}

void GossipProtocol::handle_joinresponse(Message * msg)
{
}

void GossipProtocol::handle_heartbeat(Message * msg)
{
}

void GossipProtocol::handle_peerleave(Message * msg)
{
}

void GossipProtocol::send_joinrequest()
{
}

void GossipProtocol::send_joinresponse()
{
}

void GossipProtocol::send_heartbeat()
{
}

void GossipProtocol::send_peerleave()
{
}

