/**
 *******************************************************************************
 * NodeManager.h                                                               *
 *                                                                             *
 * Node Manager, performs following task(s):                                   *
 *   - Initializes runtime environment                                         *
 *   - Create & run kv store, membership process                               *
 *   - Clean up created processes                                              *
 *******************************************************************************
 */

#ifndef _NODE_MANAGER_H_
#define _NODE_MANAGER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "memberlist.h"
#include <string>

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

class NodeManager {
public:
    NodeManager();
    ~NodeManager();

    /**
     * Initialize 
     */
    int initialize();

    int startup();
    void terminate();

    static void signal_handler(int signo);
private:
    int start_membership();
    int start_store();
    pid_t start_process(const char* cmd, char *const argv[]);

    void cleanup();

    int init_lock();

private:
    pid_t  m_pid_membershipproc;
    pid_t  m_pid_storeproc;

    volatile bool   m_b_running;

    void * m_addr;
    int    m_shmid;

    MemberList m_shm;
};

void register_signal(int signo, 
                     void (*handler)(int) = NodeManager::signal_handler);
/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _NODE_MANAGER_H_

