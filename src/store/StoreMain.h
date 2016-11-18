/**
 *******************************************************************************
 * StoreMain.h                                                                 *
 *                                                                             *
 * Store process :                                                             *
 *   - Run store to serve CURD request                                         *
 *******************************************************************************
 */

#ifndef _STORE_MAIN_H_
#define _STORE_MAIN_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "memberlist.h"

#include <string>
#include <sys/ipc.h>
/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

class StoreServer;

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

class StoreProcess{
public:
    StoreProcess(key_t ipckey, int thr_size = 256 );
    ~StoreProcess();

    void run();
private:
    MemberList m_member;
    int        m_shmid;
    void *     m_shmaddr;

    int        m_thr_size; // threadpool size
    StoreServer * m_psvr;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_MAIN_H_

