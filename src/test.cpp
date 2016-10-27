#include "stdinclude.h"
#include <stdio.h>
#include <vector>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>

#include "JoinReqMsg.h"
#include "JoinRespMsg.h"
#include "HeartbeatMsg.h"
#include "PeerLeaveMsg.h"

using namespace std;

void test_config(char* path) 
{
    if (!GetConfigPortal()->load(path)) {
        printf("Load config file failed\n");
    }

    printf("IPCKEY: %d\n", GetConfigPortal()->get_ipckey());
    printf("LOGPATH: %s\n", GetConfigPortal()->get_logpath().c_str());
    printf("TFAIL: %ld\n", GetConfigPortal()->get_failtime());
    printf("BIND IP: %s, PORT: %d\n", GetConfigPortal()->get_bindip().c_str(),
                                      GetConfigPortal()->get_bindport());

    vector<ConfigPortal::IPAddr > joins = GetConfigPortal()->get_joinaddress();
    for (auto it : joins) {
        printf("Join IP: %s, PORT: %d\n", it.first.c_str(), it.second);
    }
}

void test_joinreq_msg()
{
    unsigned char addr[4];

    addr[0] = 0x7F;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 1;
   
    JoinRequestMessage req(AF_INET, 1234, addr);
    req.build_msg();
    puts("Join request message...");
    req.dump();

    JoinRequestMessage other(std::move(req));
    puts("Copy message...");
    other.dump();
    puts("After copy, orignal message...");
    req.dump();

    puts("Construct from raw buffer...");
    JoinRequestMessage *rcv =new JoinRequestMessage(other.get_raw(), other.get_bodysize()+16);
    rcv->parse_msg();
    rcv->dump();
}

void test_msg_common()
{
    unsigned char addr[4];

    addr[0] = 0x7F;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 1;
 
    long long hb  = 0x0102030405060708l; 
    Address self(AF_INET, SOCK_STREAM, addr, 1234);
    HeartMsgStruct hm(hb, self);
    hm.dump();

    unsigned char buf[1024] = { 0 };
    hm.build(buf, sizeof(buf)); 

    dump_memory("HEARTBEAT MSG", (const char*)buf, 128);

    HeartMsgStruct peer(buf, sizeof(buf));
    peer.dump();
}

void test_joinresp_msg()
{
    unsigned char addr[4];

    addr[0] = 0x7F;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 1;
 
    long long hb  = 0x0102030405060708l; 
    Address self(AF_INET, SOCK_STREAM, addr, 1234);
    HeartMsgStruct hm(hb, self);

    JoinResponseMessage resp(MsgStatus::OK);
    //resp.build_msg();
    resp.add_member(hm);
    resp.build_msg();
    resp.dump();

    unsigned char buf[1024] = { '\0' };

    size_t len = resp.get_bodysize() + sizeof(MsgCommonHdr);
    memcpy(buf, resp.get_raw(), len);

    dump_memory("RESPONSE MSG", (const char*)buf, len);

    JoinResponseMessage peer(buf, len, false);
    peer.parse_msg();
    peer.dump();
}

void test_heartbeat_msg()
{
    unsigned char addr[4];

    addr[0] = 0x7F;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 1;
 
    long long hb  = 0x1l; 
    Address node1(AF_INET, SOCK_STREAM, addr, 1234);
    HeartMsgStruct hm1(hb, node1);

    addr[0] = 0x7F;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 2;
 
    hb  = 0x2l; 
    Address node2(AF_INET, SOCK_STREAM, addr, 1234);
    HeartMsgStruct hm2(hb, node2);

    HeartbeatMessage msg;
    //msg.build_msg();
    msg.add_member(hm1);
    msg.add_member(hm2);
    msg.build_msg();
    msg.dump();

    unsigned char buf[1024] = { '\0' };

    size_t len = msg.get_bodysize() + sizeof(MsgCommonHdr);
    memcpy(buf, msg.get_raw(), len);

    dump_memory("RESPONSE MSG", (const char*)buf, len);

    HeartbeatMessage peer(buf, len, false);
    peer.parse_msg();
    peer.dump();
}

void test_peerleave_msg()
{
    unsigned char addr[4];

    addr[0] = 0x7F;
    addr[1] = 0;
    addr[2] = 0;
    addr[3] = 1;
  
    PeerLeaveMessage::LeaveReason reason = 
                  PeerLeaveMessage::LeaveReason::ERROR_DOWN; 
    PeerLeaveMessage leave(reason, AF_INET, 1234, addr);
    leave.build_msg();
    leave.dump();

    unsigned char* buf = leave.get_raw();
    size_t sz = leave.get_bodysize() + 16;
    dump_memory("PEERLEAVE MSG", (const char*)buf, sz);
    PeerLeaveMessage *rcv =new PeerLeaveMessage(buf, sz);
    rcv->parse_msg();
    rcv->dump();
}

int main(int argc, char* argv[])
{
    test_joinreq_msg();
    //test_joinresp_msg();
    //test_heartbeat_msg();
    //test_peerleave_msg();
}

