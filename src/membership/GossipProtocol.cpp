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
#include <algorithm>
#include <stdexcept>
#include <random>

#include <ctime>

#include <arpa/inet.h>

using namespace std;
using namespace boost::asio;

#define HEARTBEAT_INITIAL_VALUE  -1

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
   m_ingroup(false),
   m_self_hb(HEARTBEAT_INITIAL_VALUE),
   m_failed()
{
    if ((mlist==nullptr) || (cfg==nullptr) || (psvr==nullptr)) {
        throw std::invalid_argument("nullptr");
    }
    m_pmember = mlist;
    m_pconfig = cfg;
    m_pnet    = psvr;

    string self_ip(m_pconfig->get_bindip());

    m_self_port = m_pconfig->get_bindport();
    m_self_addr = ip::address::from_string(self_ip);

    memset(m_self_rawip, '\0', PL_IPv6_ADDR_LEN);
    if (m_self_addr.is_v6()) {
        ip::address_v6::bytes_type rawaddr = m_self_addr.to_v6().to_bytes();
        m_self_af = AF_INET6;
        memcpy(m_self_rawip, rawaddr.data(), PL_IPv6_ADDR_LEN);
    } 
    else {
        ip::address_v4::bytes_type rawaddr = m_self_addr.to_v4().to_bytes();
        m_self_af = AF_INET;
        memcpy(m_self_rawip, rawaddr.data(), PL_IPv6_ADDR_LEN);
    }
}

GossipProtocol::~GossipProtocol()
{
}

int GossipProtocol::handle_messages(Message* msg)
{
    getlog()->sendlog(LogLevel::DEBUG, "Gossip protocol process message: 0x%x\n", msg);
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
        if (++m_self_hb == LLONG_MAX) {
            m_self_hb = HEARTBEAT_INITIAL_VALUE;
        }

        m_pmember->update_node_heartbeat(m_self_af, m_self_rawip, m_self_port,
                                         m_self_hb);
        detect_node_error();
        send_heartbeat();
    }
    return 0;
}

int GossipProtocol::node_up()
{
    getlog()->sendlog(LogLevel::DEBUG, "Gossip protocol I'm up\n");

    vector<ConfigPortal::IPAddr > joinaddr = m_pconfig->get_joinaddress();
    if (joinaddr.size() == 0) {
        throw config_error("No join address");
    }

    string self_ip(m_self_addr.to_string());
    unsigned short self_port = m_self_port;

    for (auto&& dest : joinaddr) {
        if ((dest.first == self_ip) && 
            (dest.second == self_port)) {
           m_ingroup = true;
        }
    }

    m_pmember->clear();

    if (!m_ingroup) {
        getlog()->sendlog(LogLevel::DEBUG, "Send join request\n");
        send_joinrequest();
    }
    else {
        getlog()->sendlog(LogLevel::DEBUG, "I'm joiner\n");
        m_pmember->add_node(m_self_af, m_self_rawip, m_self_port,
                            m_self_hb);
        send_heartbeat();
    }

    return 0;
}

int GossipProtocol::node_down()
{
    getlog()->sendlog(LogLevel::DEBUG, "Gossip protocol bye\n");
    m_ingroup = false;
    send_peerleave();
    return 0;
}

int GossipProtocol::detect_node_error()
{
    unsigned long now = (unsigned long)time(NULL);
    long  TFAIL   = m_pconfig->get_failtime();
    long  TREMOVE = m_pconfig->get_rmtime();

    std::vector< struct MemberEntry > detected;
    for (auto&& node : *m_pmember) {
        if (is_self_node(node.af, node.address, node.portnumber)) {
            // Should not delete ourselves
            continue;
        }
        if (node.tm_lasthb + TFAIL < now) {
            // detected failed node
            detected.push_back(node);
        }
    }

    std::sort(detected.begin(), detected.end(), entry_less);

    std::vector<struct MemberEntry > newfailed;

    unsigned int i,j;
    for (i=j=0; i<m_failed.size() && j<detected.size(); ) {
        if (entry_equal(m_failed[i], detected[j])) {
            if (m_failed[i].tm_lasthb + TREMOVE < now) {
                // TODO: optimize, the deleted node is in order, so there's 
                //       chance for underlying implementation to optimize
                getlog()->sendlog(LogLevel::INFO, "Node failed, remove it, address :"); 
                if (getlog()->is_level_allowed(LogLevel::INFO)) {
                    print_node_address(m_failed[i]);
                }
                m_pmember->del_node(m_failed[i].af, m_failed[i].address, 
                                    m_failed[i].portnumber);
            }
            else {
                newfailed.push_back( m_failed[i]);
            }
            i++;
            j++;
        }
        else if (entry_less(detected[j], m_failed[i])) {
            newfailed.push_back( detected[j]);
            j++;
        }
        else {
            // the previous failed node m_failed[i] is back
            i++;
        }
    }

    for (;j<detected.size(); j++) {
        newfailed.push_back( detected[j]);
    }

    m_failed = std::move(newfailed);

    return 0;
}

int GossipProtocol::disseminate_error()
{
    // TODO: adds error nodes in heartbeat message
    return 0;
}

void GossipProtocol::handle_joinrequest(Message * msg)
{
    getlog()->sendlog(LogLevel::DEBUG, "handle join request, msg=0x%x\n", msg);

    if (msg == nullptr) return;
    pair<ip::address, unsigned short> source = msg->get_source();
    if (!is_self_node(source.first, source.second)) {
        getlog()->sendlog(LogLevel::INFO, "Node joined, address=%s:%d\n", 
                                          source.first.to_string().c_str(),
                                          source.second);
        int af;
        unsigned char rawip[PL_IPv6_ADDR_LEN] = { '\0' };
        if (source.first.is_v4()) {
            af = AF_INET;
            memcpy(rawip, source.first.to_v4().to_bytes().data(), PL_IPv6_ADDR_LEN);
        }
        else {
            af = AF_INET6;
            memcpy(rawip, source.first.to_v4().to_bytes().data(), PL_IPv4_ADDR_LEN);
        }
        m_pmember->add_node(af, rawip, source.second, HEARTBEAT_INITIAL_VALUE);
        send_joinresponse(source.first, source.second);
    }
}

void GossipProtocol::handle_joinresponse(Message * msg)
{
    getlog()->sendlog(LogLevel::DEBUG, "handle join response, msg=0x%x\n", msg);
    if (msg == nullptr) return;

    JoinResponseMessage* pmsg = dynamic_cast<JoinResponseMessage*>(msg);
    if (pmsg->get_status() != MsgStatus::OK) {
        // Error
        getlog()->sendlog(LogLevel::ERROR, "Join response return failed\n");
        return;
    }

    m_pmember->clear();

    bool am_in_list = false;
    vector< struct MemberEntry > nodes;
    for (auto&& node : pmsg->get_members()) {
        struct MemberEntry m = { 0 };
        construct_member_from_htmsg(m, node);
        
        if (is_self_node(m.af, m.address, m.portnumber)) {
            am_in_list = true;
            m.heartbeat = HEARTBEAT_INITIAL_VALUE;
        }

        nodes.emplace_back( m );

        getlog()->sendlog(LogLevel::INFO, "Response node add:");
        if (getlog()->is_level_allowed(LogLevel::INFO)) {
            print_node_address(m);
        }
    }

    if (!am_in_list) {
        getlog()->sendlog(LogLevel::ERROR, "I'm not in response message!\n");
        struct MemberEntry m;

        m.af        = m_self_af;
        m.heartbeat = HEARTBEAT_INITIAL_VALUE;
        m.tm_lasthb = time(NULL);
        m.portnumber= m_self_port;
        m.type      = SOCK_STREAM;
        memcpy(m.address, m_self_rawip, PL_IPv6_ADDR_LEN);

        // Debug
        //print_node_address(m);

        nodes.emplace_back(m);
    }

    m_pmember->bulk_add(nodes);

    m_ingroup = true;
}

void GossipProtocol::handle_heartbeat(Message * msg)
{
    getlog()->sendlog(LogLevel::DEBUG, "handle heartbeat, msg=0x%x\n", msg);
    if (msg == nullptr) return;
    HeartbeatMessage * pmsg = dynamic_cast<HeartbeatMessage*>(msg);

    time_t now = time(NULL);
    vector< struct MemberEntry > ups;
    vector< struct MemberEntry > adds;

    vector< struct MemberEntry > nodes;
    for (auto &&n : pmsg->get_members()) {
        struct MemberEntry m = { 0 };
        construct_member_from_htmsg(m, n);
        if (is_self_node(m.af, m.address, m.portnumber)) {
            // Do not update ourselves!!!
            continue;
        }
        nodes.emplace_back(m);
    }
    vector<std::pair<bool, int64> > rc = move(m_pmember->bulk_get(nodes));
    for (unsigned int i=0; i<rc.size(); i++) {
        if (rc[i].first) {
            #if 0
            // TODO: remove debug print
            getlog()->sendlog(LogLevel::DEBUG, "Heartbeat node update:");
            if (getlog()->is_level_allowed(LogLevel::DEBUG)) {
                print_node_address(nodes[i]);
            }
            #endif
            ups.emplace_back(nodes[i]);
        }
        else {
            adds.emplace_back(nodes[i]);
            getlog()->sendlog(LogLevel::INFO, "Heartbeat node add:");
            if (getlog()->is_level_allowed(LogLevel::INFO)) {
                print_node_address(nodes[i]);
            }
        }
    }

    /*
    for (auto&& node : pmsg->get_members()) {
        struct MemberEntry m;
        construct_member_from_htmsg(m, node);
        if (is_self_node(m.af, m.address, m.portnumber)) {
            // Do not update ourselves!!!
            continue;
        }
        // TODO: optimize: underlying may provide a bulk get operation which
        //       can leaverge sorted acceleration
        int rc = m_pmember->get_node_heartbeat(m.af, m.address, m.portnumber, &(m.heartbeat));
        if (rc != 0) {
            adds.emplace_back(m);
            continue;
        } 
        if ((m.heartbeat > node.get_heartbeat()) ||
            (node.get_heartbeat() == HEARTBEAT_INITIAL_VALUE)) { 
            ups.emplace_back(m);
        }
    }*/
    m_pmember->bulk_update(ups, now);
    m_pmember->bulk_add(adds);
}

void GossipProtocol::handle_peerleave(Message * msg)
{
    getlog()->sendlog(LogLevel::DEBUG, "handle peer leave, msg=0x%x\n", msg);
    if (msg == nullptr) return;
    PeerLeaveMessage * pmsg = dynamic_cast<PeerLeaveMessage*>(msg);

    int af = 0;
    const unsigned char* paddr = pmsg->get_ip_addr(&af);

    getlog()->sendlog(LogLevel::INFO, "node left:");
    if (getlog()->is_level_allowed(LogLevel::INFO)) {
        print_node_address(af, paddr, pmsg->get_port());
    } 
    m_pmember->del_node(af, paddr, pmsg->get_port());
}

void GossipProtocol::send_joinrequest()
{
    getlog()->sendlog(LogLevel::DEBUG, "send join request\n");

    vector<ConfigPortal::IPAddr > joinaddr = m_pconfig->get_joinaddress();
    if (joinaddr.size() == 0) {
        throw config_error("No join address");
    }

    static unsigned int joinid = 0;
    if (joinid >= joinaddr.size()) {
        joinid = 0;
    }

    ConfigPortal::IPAddr dest = joinaddr[joinid++];
    if ((dest.first == m_self_addr.to_string()) && 
        (dest.second == m_self_port)) {
        if (joinaddr.size()>1) {
            send_joinrequest();
        }
        else {
            throw runtime_error("Illegal: send join request");
        }
    }

    getlog()->sendlog(LogLevel::DEBUG, "Joiner address: %s:%d\n",
                                       dest.first.c_str(),
                                       dest.second);
    Message * pmsg = nullptr;

    pmsg = new JoinRequestMessage(m_self_af, m_self_port, m_self_rawip);

    pmsg->build_msg();

    ip::udp::endpoint dest_ep = ip2udpend(dest.first, dest.second);
    pmsg->set_destination(dest_ep.address(), dest_ep.port());

    m_pnet->do_send(pmsg);
    // network should delete pmsg
}

void GossipProtocol::send_joinresponse(const ip::address& ip, 
                                       unsigned short port, 
                                       const MsgStatus& status)
{
    getlog()->sendlog(LogLevel::DEBUG, "send join response\n");

    JoinResponseMessage * pmsg = new JoinResponseMessage(status);

    for (auto&& node : *m_pmember) {
        Address addr(node.af, node.type, node.address, node.portnumber);
        HeartMsgStruct hmsg(node.heartbeat, addr);

        pmsg->add_member(hmsg);
    }

    pmsg->build_msg();
    pmsg->set_destination(ip, port);
    m_pnet->do_send(pmsg);
}

void GossipProtocol::send_heartbeat()
{
    getlog()->sendlog(LogLevel::DEBUG, "send heartbeat\n");
    Message * pmsg = construct_heartbeat_msg();
    std::vector<ip::udp::endpoint> peers = get_B_neibors();
    m_pnet->do_multicast(peers, pmsg);
}

void GossipProtocol::send_peerleave(const PeerLeaveMessage::LeaveReason& reason)
{
    getlog()->sendlog(LogLevel::DEBUG, "send peer leave\n");

    Message * pmsg = nullptr;

    pmsg = new PeerLeaveMessage(reason, m_self_af, m_self_port, m_self_rawip);

    pmsg->build_msg();

    std::vector<ip::udp::endpoint> peers = get_B_neibors();

    m_pnet->do_multicast(peers, pmsg);
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
    if (peer_cnt > 1) {
        // ourselves always exist
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, peer_cnt);

        unsigned int b = peer_cnt < m_pconfig->get_gossipb() ? 
                              peer_cnt : m_pconfig->get_gossipb();

        for (int i=0; i<peer_cnt; i++) {
            const struct MemberEntry& node = m_pmember->operator[](i);
            if (is_self_node(node.af, node.address, node.portnumber)) {
                continue;
            }
            getlog()->sendlog(LogLevel::DEBUG, "Neibor: ");
            if (getlog()->is_level_allowed(LogLevel::DEBUG)) {
                print_node_address(node);
            }
            neibors.emplace_back(get_node_endpoint(node));
            if (neibors.size()>=b) {
                break;
            }
        }
    }
    else {
        // If no neibors, send to join
        vector<ConfigPortal::IPAddr > joinaddr = m_pconfig->get_joinaddress();
        string self_ip(m_pconfig->get_bindip());
        unsigned short self_port = m_pconfig->get_bindport();

        getlog()->sendlog(LogLevel::DEBUG, "No neibors, return join address\n");

        for (auto&& dest : joinaddr) {
            if ((dest.first != self_ip) || 
                (dest.second != self_port)) {
                getlog()->sendlog(LogLevel::DEBUG, "Neibor address: %s:%d\n", 
                                                   dest.first.c_str(), dest.second);
                neibors.emplace_back(ip2udpend(dest.first, dest.second));
            }
        }
    }

    return neibors;
}

ip::udp::endpoint GossipProtocol::get_node_endpoint(
                                const struct MemberEntry& node) const
{
    ip::udp::endpoint end;
    if (node.af == AF_INET) {
        ip::address_v4::bytes_type rawip;

        for (unsigned int i=0; i<rawip.max_size(); i++) {
            rawip[i] = node.address[i];
        }
        
        ip::address_v4 addr(rawip);
        end.address(addr);
        end.port(node.portnumber);
    }
    else {
        ip::address_v6::bytes_type rawip;
        
        for (unsigned int i=0; i<rawip.max_size(); i++) {
            rawip[i] = node.address[i];
        }

        ip::address_v6 addr(rawip);
        end.address(addr);
        end.port(node.portnumber);
    }
    return end;
}

int GossipProtocol::construct_member_from_htmsg(struct MemberEntry& m, const HeartMsgStruct & hm)
{

    Address addr= hm.get_address();

    memset(&m, 0, sizeof(m));
    m.heartbeat = hm.get_heartbeat();
    m.tm_lasthb = time(NULL);
    m.portnumber= addr.get_port();
    m.type      = SOCK_STREAM;

    int af;
    const unsigned char* paddr = addr.get_ip_addr(&af);
    m.af = af;
    memcpy(m.address, paddr, addr.get_addr_len());

    return 0;
}

void GossipProtocol::print_node_address(const struct MemberEntry& node) const
{
    print_node_address(node.af, node.address, node.portnumber);
}

void GossipProtocol::print_node_address(int af, const unsigned char* addr, 
                                        unsigned short port) const
{
    if (addr==nullptr) return;
    if (af == AF_INET) {
        getlog()->get_print_handle()("%0d.%0d.%0d.%0d:%d\n", addr[0],
                                                             addr[1],
                                                             addr[2],
                                                             addr[3],
                                                             port);
    }
    else if (af == AF_INET6) {
        getlog()->get_print_handle()("%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X:%d\n",
                       addr[0],  addr[1],  addr[2],  addr[3],
                       addr[4],  addr[5],  addr[6],  addr[7],
                       addr[8],  addr[9],  addr[10], addr[11],
                       addr[12], addr[13], addr[14], addr[15],
                       port);
    }
}

