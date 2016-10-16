/**
 *******************************************************************************
 * NodeManager.cpp                                                             *
 *                                                                             *
 * Node Manager, performs following task(s):                                   *
 *   - Initializes runtime environment                                         *
 *   - Create & run kv store, membership process                               *
 *   - Clean up created processes                                              *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "NodeManager.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <syslog.h>

#include <errno.h>
#include <string.h>

#include "memberlist.h"
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

NodeManager thenode;
/*
 *******************************************************************************
 *  Functions                                                                  *
 *******************************************************************************
 */
NodeManager::NodeManager() :
    m_pid_membershipproc(0),
    m_pid_storeproc(0),
    m_b_running(false),
    m_addr(nullptr),
    m_shmid(-1),
    m_shm(nullptr)
{
}

NodeManager::~NodeManager()
{
}

int NodeManager::initialize()
{
    key_t ipckey = GetConfigPortal()->get_ipckey();
    long ring_size = GetConfigPortal()->get_ringsize();
    // The struct is 4 byte alignment, there's one additional MemberEntry
    size_t size = sizeof(struct Membership) + 
                  sizeof(struct MemberEntry) * ring_size; 

#define SHM_MODE  0600  /* User read/write */
    m_shmid = shmget(ipckey, size, IPC_CREAT|IPC_EXCL|SHM_MODE);
    if (m_shmid == -1) {
        syslog(LOG_ERR, "Create shared memory failed, errno=%d:%s\n", 
               errno, 
               strerror(errno));
        return -1;
    }
   
    m_addr = shmat(m_shmid, nullptr, 0);
    if (m_addr == (void*)-1) {
        syslog(LOG_ERR, "Shared memory attach failed, errno=%d:%s\n", 
               errno, 
               strerror(errno));
        return -1;
    }

    long *magic = (long*)m_addr;
    *magic = PL_SHM_MAGIC;

    if (!m_shm.attach(m_addr)) {
        syslog(LOG_ERR, "Member list attach failed\n");
        return -1;
    }

    m_shm.set_running_status(PL_RUNNING_ST_RUN);

    return 0;
}

int NodeManager::startup()
{
    if ((start_membership() != 0) || 
        (start_store() != 0)) {
        getlog()->sendlog(FATAL, "Failed to startup process\n");
        terminate();
        return -1;
    }

    m_b_running = true;
    pid_t pid = 0;
    int status;
    while ((pid=wait(&status)) > 0) {
        if (m_b_running) {
            int rc = 0;
            if (pid == m_pid_membershipproc) {
                rc = start_membership();
            }
            else if (pid == m_pid_storeproc) {
                rc = start_store();
            }
            if (rc != 0) {
                getlog()->sendlog(FATAL, "Restart process failed");
                terminate();
            }
        }
    }

    // Wait supposes always success unless we don't have any child 
    cleanup();

    return 0;
}

void NodeManager::terminate()
{
    // TODO: set terminate flag 
    m_shm.set_running_status(PL_RUNNING_ST_STOP);
    m_b_running = false;
    if (m_pid_membershipproc > 0) {
        kill(m_pid_membershipproc, SIGTERM);
    }

    if (m_pid_storeproc > 0) {
        kill(m_pid_storeproc, SIGTERM);
    }
}

void NodeManager::signal_handler(int signo)
{
    if (signo==SIGHUP) {
    }
    else if (signo==SIGTERM) {
        thenode.terminate();
    }
}

void NodeManager::cleanup()
{
    m_shm.detach();

    if (m_addr) {
        shmdt(m_addr);
        m_addr = nullptr;
    }

    if (m_shmid != -1) {
        shmctl(m_shmid, IPC_RMID, NULL);
        m_shmid = -1;
    }
}

int NodeManager::start_membership()
{
    m_pid_membershipproc = start_process("plmembership");
    if (m_pid_membershipproc<0) {
        return -1;
    }
    
    return 0;
}

int NodeManager::start_store()
{
    m_pid_storeproc = start_process("plstore");
    if (m_pid_storeproc<0) {
        return -1;
    }
    return 0;
}

pid_t NodeManager::start_process(const char* cmd)
{
    pid_t pid = fork();
    if (pid < 0) {
        getlog()->sendlog(FATAL, "fork failed for '%s', errno=%d:%s\n",
                                 cmd,
                                 errno,
                                 strerror(errno));
    }
    else if (pid == 0) {
        if (execlp(cmd, cmd, NULL) < 0) {
            getlog()->sendlog(FATAL, "exec failed for '%s', errno=%d:%s\n",
                                     cmd,
                                     errno,
                                     strerror(errno));
            exit(0);
        }
        // We should never reach this point
    }

    return pid;
}

void register_signal(int signo, void (*handler)(int)) 
{
    struct sigaction sa = { 0 };
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(signo, &sa, NULL) < 0) {
        getlog()->sendlog(FATAL, "Register signal failed, errno=%d:%s\n",
                                 errno,
                                 strerror(errno));
    }
}

int main(int argc, char* argv[])
{
    char * cmd = NULL;
    int    rc = 0;

    if (argc < 2) {
        printf("%s: missing parameter\nusage: %s config_file\n", argv[0], argv[0]);
        return 0;
    }

    if (!GetConfigPortal()->load(argv[1])) {
        printf("Load config file failed\n");
        return 0;
    }

    if ((cmd = strrchr(argv[0], '/')) == NULL) {
        cmd = argv[0];
    }
    else {
        cmd ++;
    }

    // Become a daemon.
    daemonize(cmd);

    register_signal(SIGTERM);
    register_signal(SIGHUP);

    rc = thenode.initialize();
    if (rc != 0) {
        syslog(LOG_ERR, "%s initialize failed\n", argv[0]);
        return 0;
    }

    rc = thenode.startup();
    if (rc != 0) {
        syslog(LOG_ERR, "%s startup failed\n", argv[0]);
        return 0;
    }

    return 0; 
}

