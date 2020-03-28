#
# Copyright 2020, all rights reserved
#

FROM dev-centos-8 
MAINTAINER junchen

ENV KVDEV__USR=dev \
    KVDEV__GROUP=dev \
    KVDEV__PROJECT_HOME=/u01/project/kv \
    KVDEV__DATA_HOME=/u01/share/kv \
    KVDEV__BUILD_HOME=/u01/share/build/ \
    KVDEV__BOOST_HOME=/opt/boost_1_72_0 \
    KVDEV__BOOST_BUILD_HOME=/opt/boost_build/share/boost-build

RUN set -x \
    && groupadd "${KVDEV__GROUP}" \
    && useradd "${KVDEV__USR}" -g "${KVDEV__GROUP}" \
    && mkdir -p "${KVDEV__PROJECT_HOME}" "${KVDEV__DATA_HOME}" "${KVDEV__BUILD_HOME}" \
    && chown -R "${KVDEV__USR}"."${KVDEV__GROUP}" "${KVDEV__PROJECT_HOME}" \
    && chown -R "${KVDEV__USR}"."${KVDEV__GROUP}" "${KVDEV__DATA_HOME}" \
    && chown -R "${KVDEV__USR}"."${KVDEV__GROUP}" "${KVDEV__BUILD_HOME}" 

COPY entrypoint.sh /
COPY build/* ${KVDEV__BUILD_HOME}
#COPY bin/* /usr/local/bin

RUN chmod +x /entrypoint.sh /usr/local/bin/* \
    && chown -R "${KVDEV__USR}"."${KVDEV__GROUP}" "/home/dev/" \
    && chown -R "${KVDEV__USR}"."${KVDEV__GROUP}" "${KVDEV__BUILD_HOME}" 

WORKDIR "${KVDEV__PROJECT_HOME}"

#USER ${KVDEV__USR}

CMD [ "/u01/share/build/build.sh" ]
ENTRYPOINT [ "/entrypoint.sh" ]

