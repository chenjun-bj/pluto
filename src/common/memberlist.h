/**
 *******************************************************************************
 * memberlist.h                                                                *
 *                                                                             *
 * Member list                                                                 *
 *   -- Shared memory layout                                                   *
 *******************************************************************************
 */

#ifndef _MEMBER_LIST_H_
#define _MEMBER_LIST_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <unistd.h>
#include <sys/ipc.h>
#include <pthread.h>

#include <ctime>

#include <vector>
#include <tuple>

#include "pltypes.h"

#include "entrytable.h"

/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */

#define PL_SHM_MAGIC    0x504C55544F303031l

#define PL_RUNNING_ST_RUN    1
#define PL_RUNNING_ST_STOP   0

#define PL_CHILD_ST_NONE     0X00
#define PL_CHILD_ST_INIT     0X01
#define PL_CHILD_ST_RUN      0X02
#define PL_CHILD_ST_STOP     0X04 
#define PL_CHILD_ST_FAIL     0X08
 
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

struct Membership {
    uint64  magic_number;
    key_t   ipckey;
    uint16  running_status;
    uint8   st_member;
    uint8   st_store;
    pid_t   pid_member;
    pid_t   pid_store;
    pthread_mutex_t mutex;
    uint64  ring_size;
    uint32  join_add1_af;   // AF_INET or AF_INET6
    uint32  join_add1_type; // SOCK_STREAM or SOCK_DGRAM
    uint8   join_addr1[16];
    uint16  join_addr1_port;
    uint16  join_addr2_exist;
    uint32  reserved1;
    uint32  join_add2_af;   
    uint32  join_add2_type; 
    uint8   join_addr2[16];
    uint16  join_addr2_port;
    uint16  reserved2;
    uint32  reserved3;
    struct st_entry ent_tab;
};

class MemberList {
public:
    //typedef entry_iterator iterator;
public:
    MemberList();
    ~MemberList();

    static size_t get_required_size(size_t ring_size);

    // Initialize the memory, the data in memory will be lost
    bool create(void* addr, key_t ipckey, size_t ring_size);
    // Attach to the memory
    bool attach(void* addr);
    void detach();
    // clear node table
    void clear();

    entry_iterator begin();
    entry_iterator end();

    void add_node(int af, const uint8 * addr, unsigned short port,
                  int64 hb, time_t now = std::time(NULL));
    void del_node(int af, const uint8 * addr, unsigned short port);
    void update_node_heartbeat(int af, const uint8 * addr, 
                               unsigned short port,
                               int64 hb, 
                               time_t now = std::time(NULL));

    int get_node_heartbeat(int af, const uint8 * addr, unsigned short port,
                           int64* hb);

    void bulk_add(const std::vector< struct MemberEntry > &);
    // Only updates heartbeat message, 
    void bulk_update(const std::vector< struct MemberEntry > &, time_t now = time(NULL));

    std::vector<bool> bulk_get(std::vector< struct MemberEntry > &);

    size_t size() const {
        if (m_ptab != nullptr) {
            return m_ptab->size();
        }
        return 0;
    }

    const struct MemberEntry& operator[](int i) const {
        if (m_ptab == nullptr) {
            throw std::runtime_error("no table");
        }
        return m_ptab->operator[](i);
    }
 
    uint64 get_magic_number() const;

    bool set_ipckey(key_t);
    key_t  get_ipckey() const;

    bool set_running_status(uint16 status);
    uint16 get_running_status() const;

    bool set_membership_status(uint8 status);
    uint8 get_membership_status() const;

    bool set_store_status(uint8 status);
    uint8 get_store_status() const;

    bool set_pid_member(pid_t pid);
    pid_t  get_pid_member() const;

    bool set_pid_store(pid_t pid);
    pid_t  get_pid_store()  const;

    bool set_ring_size(uint64 size);
    uint64  get_ring_size()  const;

    pthread_mutex_t* get_mutex() const;

protected:
    bool init_entry_table(bool create=false);
    bool valid_child_status(uint8 st) const;
    bool valid_node_addr(int af, int type, const uint8 * addr, 
                         unsigned short port);
private:
    struct Membership * m_paddr;

    // entry table stores node
    entry_table       * m_ptab;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

inline bool  MemberList::valid_child_status(uint8 status) const
{
    if ((status == PL_CHILD_ST_NONE) || 
        (status == PL_CHILD_ST_INIT) ||
        (status == PL_CHILD_ST_RUN)  ||
        (status == PL_CHILD_ST_STOP) ||
        (status == PL_CHILD_ST_FAIL)) {
        return true;
    }
    return false;
}

inline bool MemberList::valid_node_addr(int af, int type, 
                                        const uint8 * addr, unsigned short port)
{
    if ((af != AF_INET) && (af != AF_INET6)) {
        return false;
    }
    return true;
}

#endif // _MEMBER_LIST_H

