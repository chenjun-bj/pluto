#
#

BOOST_TARGET=boost_1_72_0.tar.gz
BOOST_DOWNLOAD_URL=https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz

GOSU_DOWNLOAD_URL=https://github.com/tianon/gosu/releases/download/1.11/gosu-amd64

bin: bin/gosu

bin/gosu:
	mkdir bin; \
	curl -L ${GOSU_DOWNLOAD_URL} --output $@; \
	chmod +x $@ 

BOOST_TARGET:
	curl -L ${BOOST_DOWNLOAD_URL} --output $@

.PHONY: all
all: bin BOOST_TARGET
	docker rmi -f dev-centos-8
	docker build -t dev-centos-8 .

