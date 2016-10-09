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
#include <signal.h>

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
    m_b_running(false)
{
}

NodeManager::~NodeManager()
{
}

int NodeManager::initialize()
{
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

    return 0;
}

void NodeManager::terminate()
{
    // TODO: set terminate flag 
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
    struct sigaction sa = { 0 };

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
    rc = thenode.startup();
    return 0;
}

