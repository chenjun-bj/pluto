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

#include "JoinReqMsg.h"
#include "JoinRespMsg.h"
#include "HeartbeatMsg.h"
#include "PeerLeaveMsg.h"

#include "GossipProtocol.h"
#include "MembershipServer.h"

#include <boost/asio.hpp>

#include <array>
#include <stdexcept>
#include <random>

#include <arpa/inet.h>

using namespace std;
using namespace boost::asio;

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

GossipProtocol::GossipProtocol(MemberList *mlist, ConfigPortal * cfg,
                               MembershipServer* psvr) :
   m_ingroup(false)
{
    if ((mlist==nullptr) || (cfg==nullptr) || (psvr==nullptr)) {
        throw std::invalid_argument("nullptr");
    }
    m_pmember = mlist;
    m_pconfig = cfg;
    m_pnet    = psvr;
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
        getlog()->sendlog(LogLevel::ERROR, "'%s': unsupported message, type=%s\n", 
                          __func__, get_desc_msgtype(msg->get_msgtype()).c_str());
        return -1;
    }
    return 0;
}

int GossipProtocol::handle_timer(int id)
{
    if (m_ingroup) {
        detect_node_error();
        send_heartbeat();
    }
    return 0;
}

int GossipProtocol::node_up()
{
    vector<ConfigPortal::IPAddr > joinaddr = m_pconfig->get_joinaddress();
    if (joinaddr.size() == 0) {
        throw config_error("No join address");
    }

    string self_ip(m_pconfig->get_bindip());
    unsigned short self_port = m_pconfig->get_bindport();

    for (auto&& dest : joinaddr) {
        if ((dest.first == self_ip) && 
            (dest.second == self_port)) {
           m_ingroup = true;
        }
    }

    if (!m_ingroup) {
        send_joinrequest();
    }
    else {
        send_heartbeat();
    }

    // TODO: initilize membership list
    return 0;
}

int GossipProtocol::node_down()
{
    m_ingroup = false;
    send_peerleave();
    return 0;
}

int GossipProtocol::detect_node_error()
{
    return 0;
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
    vector<ConfigPortal::IPAddr > joinaddr = m_pconfig->get_joinaddress();
    if (joinaddr.size() == 0) {
        throw config_error("No join address");
    }

    static unsigned int joinid = 0;
    if (joinid >= joinaddr.size()) {
        joinid = 0;
    }

    string self_ip(m_pconfig->get_bindip());
    unsigned short self_port = m_pconfig->get_bindport();

    ConfigPortal::IPAddr dest = joinaddr[joinid++];
    if ((dest.first == self_ip) && 
        (dest.second == self_port)) {
        if (joinaddr.size()>1) {
            send_joinrequest();
        }
        else {
            throw runtime_error("Illegal: send join request");
        }
    }

    Message * pmsg = nullptr;
    int af = AF_INET;

    ip::udp::endpoint self_ep = ip2udpend(self_ip, self_port);
    if (self_ep.address().is_v6()) {
        ip::address_v6::bytes_type rawaddr = self_ep.address().to_v6().to_bytes();
        af = AF_INET6;
        pmsg = new JoinRequestMessage(af, self_port, rawaddr.data());
    } 
    else {
        ip::address_v4::bytes_type rawaddr = self_ep.address().to_v4().to_bytes();
        af = AF_INET;
        pmsg = new JoinRequestMessage(af, self_port, rawaddr.data());
    }

    pmsg->build_msg();

    ip::udp::endpoint dest_ep = ip2udpend(dest.first, dest.second);
    pmsg->set_destination(dest_ep.address(), dest_ep.port());

    m_pnet->do_send(pmsg);
    // network should delete pmsg
}

void GossipProtocol::send_joinresponse()
{
}

void GossipProtocol::send_heartbeat()
{
    Message * pmsg = construct_heartbeat_msg();
    
}

void GossipProtocol::send_peerleave()
{
}

Message * GossipProtocol::construct_heartbeat_msg()
{
    HeartbeatMessage * pmsg = new HeartbeatMessage();

    for (auto&& node : *m_pmember) {
        Address addr(node.af, node.type, node.address, node.portnumber);
        HeartMsgStruct hmsg(node.heartbeat, addr);

        pmsg->add_member(hmsg);
    }

    pmsg->build_msg();

    return pmsg;
}

std::vector<ip::udp::endpoint> GossipProtocol::get_B_neibors()
{
    std::vector<ip::udp::endpoint> neibors;

    int peer_cnt = m_pmember->size();
    if (peer_cnt > 0) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, m_pmember->size());

        int b = peer_cnt < m_pconfig->get_gossipb() ? 
                      peer_cnt : m_pconfig->get_gossipb();

        for (int i=0; i<b; i++) {
            const struct MemberEntry& node = m_pmember->operator[](i);
            neibors.push_back(get_node_endpoint(node));
        }
    }
    else {
        // find join
    }
    return neibors;
}

ip::udp::endpoint GossipProtocol::get_node_endpoint(
                                const struct MemberEntry& node) const
{
    ip::udp::endpoint end;
    if (node.af == AF_INET) {
        ip::address_v4::bytes_type rawip;

        for (int i=0; i<rawip.max_size(); i++) {
            rawip[i] = node.address[i];
        }
        
        ip::address_v4 addr(rawip);
        end.address(addr);
        end.port(node.portnumber);
    }
    else {
        ip::address_v6::bytes_type rawip;
        
        for (int i=0; i<rawip.max_size(); i++) {
            rawip[i] = node.address[i];
        }

        ip::address_v6 addr(rawip);
        end.address(addr);
        end.port(node.portnumber);
    }
    return end;
}

