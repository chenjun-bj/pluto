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

#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>
#include <stdexcept>

#include "MembershipMain.h"
#include "MembershipServer.h"

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
MembershipProcess::MembershipProcess(key_t ipckey) :
    m_member(),
    m_shmid(-1),
    m_shmaddr(nullptr),
    m_psvr(nullptr)
{
    getlog()->sendlog(LogLevel::INFO, "Membership start\n");
#define SHM_MODE    0400    /*User read*/
    m_shmid = shmget(ipckey, 0, SHM_MODE);
    if (m_shmid == -1) {
        getlog()->sendlog(LogLevel::FATAL, "Membership: failed to get memory, errno=%d:%s\n",
                                            errno, strerror(errno));
        throw std::runtime_error("Fail to get shared memory, errno=" + std::to_string(errno));
    }

    m_shmaddr = shmat(m_shmid, nullptr, SHM_RDONLY);
    if (m_shmaddr == (void*)-1) {
        getlog()->sendlog(LogLevel::FATAL, "Membership: failed to attach memory, errno=%d:%s\n",
                                           errno, strerror(errno));
        throw std::runtime_error("Fail to attach memory, errno=" + std::to_string(errno));
    }

    if (!m_member.attach(m_shmaddr)) {
        getlog()->sendlog(LogLevel::FATAL, "Membership load info failed\n");
        throw std::runtime_error("Fail to attach member list");
    }

    m_psvr = new MembershipServer(GetConfigPortal(), &m_member, 0);
}

MembershipProcess::~MembershipProcess()
{
    getlog()->sendlog(LogLevel::INFO, "Membership exit\n");
    if (m_psvr) {
        delete m_psvr;
    }

    if (m_shmaddr) {
        shmdt(m_shmaddr);
    }
}

void MembershipProcess::run()
{
    m_psvr->run();
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

    try {
        po::options_description desc("allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("ipckey,k", po::value<int>(), "specify the IPCKEY")
            ("config,f", po::value<string>(), "configuration file")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        std::ostringstream stream;
        stream << desc << "\n";

        if (vm.count("help")) {
            getlog()->sendlog(LogLevel::INFO, "%s", stream.str().c_str());
            return 0;
        }
        if (vm.count("ipckey")) {
            getlog()->sendlog(LogLevel::INFO, "%d", vm["ipckey"].as<int>());
            ipckey = vm["ipckey"].as<int>();
        }
        else {
            getlog()->sendlog(LogLevel::ERROR, "Missing ipckey\nUsage: %s option\n%s\n", 
                                     argv[0], 
                                     stream.str().c_str());
            return 0;
        }
        if (vm.count("config")) {
            getlog()->sendlog(LogLevel::INFO, "%s", vm["config"].as<string>().c_str());
            GetConfigPortal()->load(vm["config"].as<string>().c_str());
            if (ipckey == -1) {
                ipckey = GetConfigPortal()->get_ipckey();
            }
        }
        else {
            getlog()->sendlog(LogLevel::ERROR, "Missing config\nUsage: %s option\n%s\n", 
                                     argv[0], 
                                     stream.str().c_str());
            return 0;
        }

    }
    catch (exception & e) {
        cout << e.what() << endl;;
    }

    if (ipckey == -1) {
        getlog()->sendlog(LogLevel::ERROR, "Unable to get ipckey\n");
        return 0;
    }

    try {
        MembershipProcess memship(ipckey);

        memship.run();
    }
    catch (std::exception & e) {
        getlog()->sendlog(LogLevel::ERROR, "Membership exception: %s\n", e.what());
        return 0;
    }

    return 0;
}

