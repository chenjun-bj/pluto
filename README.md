PLUTO

1. Build

1.1 Requirement

    a/ C/C++ compilier that supports c++11 standard
    
    b/ Boost 1.62 and above
    
    c/ Boost build system

1.2 Configure

    a/ Copy build/boost.jam into $BOOST_BUILD/src/tools directory if not exist;
    
       The boost.jam is the toolset used to build programs that using pre-built boost libray, the file is distributed with BOOST source code under tools/build/src/contrib.
       
    b/ Edit/copy build/user-config.jam to your home directory, the file defines toolset (compiler and boost) for user, for Linux, you could use tool set 'gcc', for Mac, you could use 'darwin';
    
    c/ Edit boost-build.jam, to specify $BOOST_BUILD directory;

1.3 Build

    Goto src directory, issue 'b2' to build

1.4 Build pass: Linux, Mac


2. TODO

2.1 Store implementation;

2.2 Add admin console;

2.3 Error handling: some code throws exception for error case, which is not handled properly;

2.4 Test;

2.5 using namespace to isolate pluto from others;

2.6 Enrich log, both the log service and logging point;

