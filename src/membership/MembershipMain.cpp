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

#include "MembershipMain.h"

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

/*
 *******************************************************************************
 *  Functions                                                                  *
 *******************************************************************************
 */
int main(int argc, char* argv[])
{
    namespace po = boost::program_options;
    using namespace std;

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
            getlog()->sendlog(INFO, "%s", stream.str().c_str());
            return 0;
        }
        if (vm.count("ipckey")) {
            getlog()->sendlog(INFO, "%d", vm["ipckey"].as<int>());
        }
        else {
            getlog()->sendlog(ERROR, "Missing ipckey\nUsage: %s option\n%s\n", 
                                     argv[0], 
                                     stream.str().c_str());
            return 0;
        }
        if (vm.count("config")) {
            getlog()->sendlog(INFO, "%s", vm["config"].as<string>().c_str());
        }
        else {
            getlog()->sendlog(ERROR, "Missing config\nUsage: %s option\n%s\n", 
                                     argv[0], 
                                     stream.str().c_str());
            return 0;
        }

    }
    catch (exception & e) {
        cout << e.what() << endl;;
    }
    return 0;
}

