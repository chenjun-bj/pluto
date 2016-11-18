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

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>

#include <libgen.h>

#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>
#include <stdexcept>

#include "StoreMain.h"

#include <boost/program_options.hpp>

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
StoreProcess::StoreProcess(key_t ipckey, int thr_size) :
    m_member(),
    m_shmid(-1),
    m_shmaddr(nullptr),
    m_thr_size(thr_size), 
    m_psvr(nullptr)
{
    getlog()->sendlog(LogLevel::INFO, "Store start\n");
#define SHM_MODE    0600    /*User read/wirte */
    m_shmid = shmget(ipckey, 0, SHM_MODE);
    if (m_shmid == -1) {
        getlog()->sendlog(LogLevel::FATAL, "Store: failed to get memory, errno=%d:%s\n",
                                            errno, strerror(errno));
        throw std::runtime_error("Fail to get shared memory, errno=" + std::to_string(errno));
    }

    m_shmaddr = shmat(m_shmid, nullptr, 0);
    if (m_shmaddr == (void*)-1) {
        getlog()->sendlog(LogLevel::FATAL, "Store: failed to attach memory, errno=%d:%s\n",
                                           errno, strerror(errno));
        throw std::runtime_error("Fail to attach memory, errno=" + std::to_string(errno));
    }

    if (!m_member.attach(m_shmaddr)) {
        getlog()->sendlog(LogLevel::FATAL, "Store load info failed\n");
        throw std::runtime_error("Fail to attach member list");
    }

    //m_psvr = new MembershipServer(GetConfigPortal(), &m_member, 0);
}

StoreProcess::~StoreProcess()
{
    getlog()->sendlog(LogLevel::INFO, "Store exit\n");
    if (m_psvr) {
        //delete m_psvr;
    }

    if (m_shmaddr) {
        shmdt(m_shmaddr);
    }
}

void StoreProcess::run()
{
    //m_psvr->run();
}

/*
 *******************************************************************************
 *  Functions                                                                  *
 *******************************************************************************
 */
int main(int argc, char* argv[])
{
    namespace po = boost::program_options;
    using namespace std;

    key_t ipckey = -1;
    int   thr_size = 256;

    try {
        po::options_description desc("allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("ipckey,k", po::value<int>(), "specify the IPCKEY")
            ("size,n", po::value<int>(), "thread pool size")
            ("config,f", po::value<string>(), "configuration file")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        std::ostringstream stream;
        stream << desc << "\n";

        if (vm.count("help")) {
            getlog()->sendlog(LogLevel::INFO, "%s", stream.str().c_str());
            return PLERROR;
        }

        if (vm.count("ipckey")) {
            getlog()->sendlog(LogLevel::INFO, "ipckey: %d\n", vm["ipckey"].as<int>());
            ipckey = vm["ipckey"].as<int>();
        }

        if (vm.count("size")) {
            getlog()->sendlog(LogLevel::INFO, "ipckey: %d\n", vm["ipckey"].as<int>());
            thr_size = vm["size"].as<int>();
        }

        if (vm.count("config")) {
            getlog()->sendlog(LogLevel::INFO, "configuration file: %s\n", vm["config"].as<string>().c_str());
            GetConfigPortal()->load(vm["config"].as<string>().c_str());
            if (ipckey == -1) {
                ipckey = GetConfigPortal()->get_ipckey();
            }
            std::string logpath = std::move(GetConfigPortal()->get_logpath());
            if (logpath.length()>0) {
                logpath = logpath + "/" + basename(argv[0]) + "." + std::to_string(getpid());
                
                getlog()->set_log_file(logpath.c_str());
            }
        }
        else {
            getlog()->sendlog(LogLevel::ERROR, "Missing config\nUsage: %s option\n%s\n", 
                                     argv[0], 
                                     stream.str().c_str());
            return PLERROR;
        }

    }
    catch (exception & e) {
        getlog()->sendlog(LogLevel::ERROR, "Exception during parse option: %s\n",
                                           e.what());
        return PLERROR;
    }

    if (ipckey == -1) {
        getlog()->sendlog(LogLevel::ERROR, "Unable to get ipckey\n");
        return PLERROR;
    }

    try {
        StoreProcess store(ipckey, thr_size);

        store.run();
    }
    catch (std::exception & e) {
        getlog()->sendlog(LogLevel::ERROR, "Store exception: %s\n", e.what());
        return PLERROR;
    }

    return PLSUCCESS;
}

