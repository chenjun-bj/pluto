#
# Copyright 2020, all rights reserved
#

FROM centos
MAINTAINER junchen

ENV DEVOS__PROJECT_HOME=/u01/project \
    DEVOS__DATA_HOME=/u01/share \
    DEVOS__BOOST_PACKAGE=boost_1_72_0.tar.gz 
    #DEVOS__BOOST_VERSION=${DEVOS__BOOST_PACKAGE%%.*}

RUN set -x \
    && mkdir -p "${DEVOS__PROJECT_HOME}" "${DEVOS__DATA_HOME}" \
    && yum install -y which-2.21-10.el8.x86_64 make-1:4.2.1-9.el8.x86_64 diffutils-3.6-5.el8.x86_64 \
    && yum install -y gcc gcc-c++ \
    && yum install -y python3 platform-python-devel-3.6.8

COPY ${DEVOS__BOST_PACKAGE} /tmp

RUN set -x \
    && cd /tmp && tar zxf "${DEVOS__BOOST_PACKAGE}" \
    && DEVOS__BOOST_VERSION="${DEVOS__BOOST_PACKAGE%%.*}" \ 
    && cd "${DEVOS__BOOST_VERSION}" && ls -l \
    && ./bootstrap.sh --with-python=/usr/bin/python3 --prefix="/opt/${DEVOS__BOOST_VERSION}" && ./b2 install include=/usr/include/python3.6m \
    && cd "tools/build" && ./bootstrap.sh && ./b2 install --prefix="/opt/boost_build" \
    && cd / && rm -rf "/tmp/${DEVOS__BOOST_VERSION}*"

RUN chmod +x /usr/local/bin/*

WORKDIR "${DEVOS__PROJECT_HOME}"

CMD [ "/bin/bash" ]

