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

#include <string>
#include <iostream>
#include <boost/program_options.hpp>

#include "AdminCmd.h"

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
    namespace po = boost::program_options;
    using namespace std;

    key_t  ipckey = -1;
    int    shmid = -1;
    void * addr = nullptr;

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

        if (vm.count("help")) {
            cout << desc << "\n";
            return 0;
        }
        if (vm.count("ipckey")) {
            ipckey = vm["ipckey"].as<int>();
        }
        if (vm.count("config")) {
            GetConfigPortal()->load(vm["config"].as<string>().c_str());
            ipckey = GetConfigPortal()->get_ipckey();
        }
    }
    catch (exception & e) {
        cout << e.what() << endl;;
    }

    if (ipckey == -1) {
        char * env = getenv(ENV_NM_BASE_IPCKEY);
        if (env != nullptr) {
            ipckey = atoi(env);
        }
    }

    if (ipckey == -1) {
        cout << "Missing option\n" ;
    }

#define SHM_MODE    0400    /*User read*/
    shmid = shmget(ipckey, 0, SHM_MODE);
    if (shmid == -1) {
        getlog()->sendlog(LogLevel::FATAL, "%s: failed to get memory, errno=%d:%s\n",
                                 argv[0], errno, strerror(errno));
        return 0;
    }

    addr = shmat(shmid, nullptr, SHM_RDONLY);
    if (addr == (void*)-1) {
        getlog()->sendlog(LogLevel::FATAL, "%s: failed to attach memory, errno=%d:%s\n",
                                 argv[0], errno, strerror(errno));
        return 0;
    }

    MemberList mgrinfo;
    if (!mgrinfo.attach(addr)) {
        getlog()->sendlog(LogLevel::FATAL, "Load info failed\n");
        return 0;
    }
    getlog()->sendlog(LogLevel::FATAL, "Magic number=0X%lX\n", mgrinfo.get_magic_number());
    getlog()->sendlog(LogLevel::FATAL, "Running status=%s\n", 
                             mgrinfo.get_running_status() == PL_RUNNING_ST_RUN ? "running" : "not running");

}

