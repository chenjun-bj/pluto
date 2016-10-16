/**
 *******************************************************************************
 * NodeAdmin.cpp                                                               *
 *                                                                             *
 * Node administrator, provides control console to user                        *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "memberlist.h"

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/*
 *******************************************************************************
 *  Declaractions                                                              *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Type definitions                                                           *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Functions                                                                  *
 *******************************************************************************
 */
int main(int argc, char* argv[])
{
    key_t  ipckey;
    int    shmid = -1;
    void * addr = nullptr;

    if (argc>1) {
        GetConfigPortal()->load(argv[1]);
        ipckey =GetConfigPortal()->get_ipckey();
    }
    else {
        char * env = getenv(ENV_NM_BASE_IPCKEY);
        if (env != nullptr) {
            ipckey = atoi(env);
        }
    }

#define SHM_MODE    0400    /*User read*/
    shmid = shmget(ipckey, 0, SHM_MODE);
    if (shmid == -1) {
        getlog()->sendlog(FATAL, "%s: failed to get memory, errno=%d:%s\n",
                                 argv[0], errno, strerror(errno));
        return 0;
    }

    addr = shmat(shmid, nullptr, SHM_RDONLY);
    if (addr == (void*)-1) {
        getlog()->sendlog(FATAL, "%s: failed to attach memory, errno=%d:%s\n",
                                 argv[0], errno, strerror(errno));
        return 0;
    }

    MemberList mgrinfo(nullptr);
    if (!mgrinfo.attach(addr)) {
        getlog()->sendlog(FATAL, "Load info failed\n");
        return 0;
    }
    getlog()->sendlog(FATAL, "Magic number=0X%lX\n", mgrinfo.get_magic_number());
    getlog()->sendlog(FATAL, "Running status=%s\n", 
                             mgrinfo.get_running_status() == PL_RUNNING_ST_RUN ? "running" : "not running");

}

