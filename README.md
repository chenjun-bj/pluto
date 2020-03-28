PLUTO

1. Build

1.1 Requirement

    a/ Make
    
    b/ curl -- to download required packages, such as gosu, boost.

1.2 Build Docker Images

    a/ Build centos docker image
        cd build/images/dev-centos; make
        
    b/ Build kv docker image
        cd build/images/kv-dev; make
    
1.3 Run kv docker to build

    Mount project home to '/u01/project/kv' in container to run kv-dev, like below:
    docker run --rm -it -v /Users/chenjun/golang/src/user/pluto:/u01/project/kv kv-dev

2. TODO

2.1 Store implementation;

2.2 Add admin console;

2.3 Error handling: some code throws exception for error case, which is not handled properly;

2.4 Test;

2.5 using namespace to isolate pluto from others;

2.6 Enrich log, both the log service and logging point;

