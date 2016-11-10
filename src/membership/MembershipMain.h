/**
 *******************************************************************************
 * MembershipMain.h                                                            *
 *                                                                             *
 * Membership process :                                                        *
 *   - Run membership protocol                                                 *
 *******************************************************************************
 */

#ifndef _MEMBER_SHIP_MAIN_H_
#define _MEMBER_SHIP_MAIN_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "memberlist.h"
#include "MembershipServer.h"

#include <string>

#include <sys/ipc.h>
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

class MembershipProcess{
public:
    MembershipProcess(key_t ipckey);
    ~MembershipProcess();

    void run();
private:
    MemberList m_member;
    int        m_shmid;
    void *     m_shmaddr;

    MembershipServer * m_psvr;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _MEMBER_SHIP_MAIN_H_

