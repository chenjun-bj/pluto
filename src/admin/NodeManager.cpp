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
#include <pthread.h>
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
    m_shm()
{
}

NodeManager::~NodeManager()
{
}

int NodeManager::initialize()
{
    syslog(LOG_INFO, "initialize \n");
    key_t ipckey = GetConfigPortal()->get_ipckey();
    int ring_size = GetConfigPortal()->get_ringsize();
    if (ring_size < KV_RING_MIN_SIZE || ring_size > KV_RING_MAX_SIZE) {
        syslog(LOG_ERR, "Invalid ring size: %d,  valid range [%d, %d]\n", 
                        ring_size, KV_RING_MIN_SIZE, KV_RING_MAX_SIZE);
        return -1;
    }

    size_t size = MemberList::get_required_size(ring_size);

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

    if (!m_shm.create(m_addr, ipckey, ring_size)) {
        syslog(LOG_ERR, "Member list attach failed\n");
        cleanup();
        return -1;
    }

    /* Initialize lock */
    if (init_lock()!=0) {
        cleanup();
        return -1;
    }

    m_shm.set_running_status(PL_RUNNING_ST_RUN);

    return 0;
}

int NodeManager::init_lock()
{
    pthread_mutex_t * plock = m_shm.get_mutex();
    if (plock == nullptr) {
        syslog(LOG_ERR, "Mutex address null\n");
        return -1;
    }

    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
        syslog(LOG_ERR, "Mutex attribute init failed, errno=%d:%s\n",
                        errno,
                        strerror(errno));
        return -1;
    }
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
        syslog(LOG_ERR, "Mutex attribute set PTHREAD_MUTEX_RECURSIVE failed, errno=%d:%s\n",
                        errno,
                        strerror(errno));
        return -1;
    }
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
        syslog(LOG_ERR, "Mutex attribute set PTHREAD_MUTEX_ROBUST failed, errno=%d:%s\n",
                        errno,
                        strerror(errno));
        return -1;
    }
#ifdef __linux__
    if (pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) != 0) {
        syslog(LOG_ERR, "Mutex attribute set PTHREAD_MUTEX_ROBUST failed, errno=%d:%s\n",
                        errno,
                        strerror(errno));
        return -1;
    }
#endif
    if (pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT) != 0) {
        syslog(LOG_ERR, "Mutex attribute set PTHREAD_PRIO_INHERIT failed, errno=%d:%s\n",
                        errno,
                        strerror(errno));
        return -1;
    }

    return 0;
}

int NodeManager::startup()
{
    syslog(LOG_INFO, "startup\n");
    if ((start_membership() != 0) || 
        (start_store() != 0)) {
        syslog(LOG_ERR, "Failed to startup process\n");
        terminate();
        return -1;
    }

    m_b_running = true;
    pid_t pid = 0;
    int status;
    while ((pid=wait(&status)) > 0) {
        if (m_b_running) {
            if (!WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    syslog(LOG_ERR, "Child (%d) exit abnormally\n", pid);
                    continue;
                }
                int rc = 0;
                if (pid == m_pid_membershipproc) {
                    rc = start_membership();
                }
                else if (pid == m_pid_storeproc) {
                    rc = start_store();
                }
                if (rc != 0) {
                    syslog(LOG_ERR, "Restart process failed, terminated pid=%d\n", pid);
                    terminate();
                }
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
        syslog(LOG_INFO, "Send terminate to membership process %d\n", m_pid_membershipproc);
        kill(m_pid_membershipproc, SIGTERM);
    }

    if (m_pid_storeproc > 0) {
        syslog(LOG_INFO, "Send terminate to store process %d\n", m_pid_membershipproc);
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
    syslog(LOG_INFO, "clean\n");
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
    char * args[6] = { nullptr };

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

    static char cmd[PATH_MAX] = { 0 };
    static char aucpid[10] = { 0 };
    static char config[PATH_MAX] = { 0 };
    static char ploptpid[3] = { 0 };
    static char ploptfile[3] = { 0 };
    
    strncpy(ploptpid, "-k", sizeof(ploptpid));
    strncpy(ploptfile, "-f", sizeof(ploptfile));
    strncpy(cmd, "plmembership", sizeof(cmd));
    snprintf(aucpid, sizeof(aucpid), "%d", GetConfigPortal()->get_ipckey());
    strncpy(config, GetConfigPortal()->get_config_filename().c_str(),
            sizeof(config));

    args[0] = cmd;
    args[1] = ploptpid;
    args[2] = aucpid;
    args[3] = ploptfile;
    args[4] = config;
    args[5] = nullptr; 

    m_pid_membershipproc = start_process(cmd, args);
    if (m_pid_membershipproc<0) {
        return -1;
    }
    syslog(LOG_INFO, "Membership process %d start\n", m_pid_membershipproc);
    
    return 0;
}

int NodeManager::start_store()
{
    /*
    m_pid_storeproc = start_process("plstore");
    if (m_pid_storeproc<0) {
        return -1;
    }
    */
    syslog(LOG_INFO, "Membership process %d start\n", m_pid_storeproc);
    return 0;
}

pid_t NodeManager::start_process(const char* cmd, char *const argv[])
{
    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "fork failed for '%s', errno=%d:%s\n",
                        cmd,
                        errno,
                        strerror(errno));
    }
    else if (pid == 0) {
        if (execvp(cmd, argv) < 0) {
            syslog(LOG_ERR, "exec failed for '%s', errno=%d:%s\n",
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
    int err;
    if (::register_signal(signo, handler, &err)!= 0) {
        getlog()->sendlog(LogLevel::FATAL, "Register signal failed, err=%d:%s\n",
                                 err,
                                 strerror(err));
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

