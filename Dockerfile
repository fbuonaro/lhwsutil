##################################################################################
# STAGE 0 - base environment with first set of runtime dependencies
##################################################################################
FROM centos:centos7 as lhwsutil-s0-base-env
LABEL lhwsutil-stage-base-env="yes"
LABEL lhwsutil-stage-build-env="no"
LABEL lhwsutil-stage-build="no"
LABEL lhwsutil-stage-test-env="no"
LABEL lhwsutil-stage-main="no"

# okay repo is for gtest/gmock 1.8, 1.6 is permanently broken
RUN yum -y --enablerepo=extras install epel-release && \
    yum -y install https://repo.ius.io/ius-release-el7.rpm && \
    yum -y install http://repo.okay.com.mx/centos/7/x86_64/release/okay-release-1-5.el7.noarch.rpm && \
    yum clean all

##################################################################################
# STAGE 1 - build tools and libraries needed to build lhwsutil
##################################################################################
FROM lhwsutil-s0-base-env as lhwsutil-s1-build-env
LABEL lhwsutil-stage-base-env="no"
LABEL lhwsutil-stage-build-env="yes"
LABEL lhwsutil-stage-build="no"
LABEL lhwsutil-stage-test-env="no"
LABEL lhwsutil-stage-main="no"

# for compiling and unit testing
# x86_64 versions from okay repo for gmock/gtest 1.8
# zlib/pcre devel for cppcms 1.2.1
RUN yum -y install \
        cmake3 \
        gcc \
        gcc-c++ \
        gtest-devel.x86_64 \
        gmock-devel.x86_64 \
        wget \
        bzip2 \
        python36u \
        zlib-devel \ 
        pcre-devel \
        git \
        openssl-devel \
        curl-devel \
        rapidjson-devel \
        boost169-devel \
        make && \
    yum clean all

RUN git clone https://github.com/fbuonaro/lhmiscutil.git && \
    cd lhmiscutil && \
    mkdir build && \
    cd build && \
    cmake3 \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        ../ && \
    make && \
    make test && \
    make install-lhmiscutil

# it uses a different ssl1.1.1 by default, build manually to use 1.0.2
RUN yum install -y jansson-devel check-devel libtool automake autoconf perl-Thread-Queue.noarch && \
    git clone https://github.com/benmcollins/libjwt.git && \
    cd libjwt && \
    autoreconf -i && \
    ./configure --prefix=/usr && \
    make && \
    make check && \
    make install

RUN ldconfig

ENTRYPOINT [ "bash" ]

##################################################################################
# STAGE 2 - the lhwsutil source and compiled binaries
##################################################################################
FROM lhwsutil-s1-build-env as lhwsutil-s2-build
LABEL lhwsutil-stage-base-env="no"
LABEL lhwsutil-stage-build-env="no"
LABEL lhwsutil-stage-build="yes"
LABEL lhwsutil-stage-test-env="no"
LABEL lhwsutil-stage-main="no"

ADD . /lhwsutil
RUN cd /lhwsutil && \
    mkdir ./build && \
    cd ./build && \
    cmake3 \
        -DBOOST_INCLUDEDIR=/usr/include/boost169 \
        -DBOOST_LIBRARYDIR=/usr/lib64/boost169 \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_BUILD_TYPE=Release \
        ../ && \
    make && \
    make test

##################################################################################
# STAGE 3 - the base image with additional built runtime dependencies, lhwsutil 
#           binaries and test binaries needed for running integration tests
#           includes everything from build-env
##################################################################################
FROM lhwsutil-s2-build as lhwsutil-s3-test-env
LABEL lhwsutil-stage-base-env="no"
LABEL lhwsutil-stage-build-env="no"
LABEL lhwsutil-stage-build="no"
LABEL lhwsutil-stage-test-env="yes"
LABEL lhwsutil-stage-main="no"

RUN cd /lhwsutil/build && \
    make install
RUN ldconfig

##################################################################################
# STAGE 4 - the base image with additional built runtime dependencies and 
#           lhwsutil binaries includes nothing from build-env
##################################################################################
FROM lhwsutil-s0-base-env as lhwsutil-s4-main-env
LABEL lhwsutil-stage-base-env="no"
LABEL lhwsutil-stage-build-env="no"
LABEL lhwsutil-stage-build="no"
LABEL lhwsutil-stage-test-env="no"
LABEL lhwsutil-stage-main="yes"

COPY --from=lhwsutil-s2-build /usr/ /usr/
COPY --from=lhwsutil-s2-build /lhwsutil/ /lhwsutil/
RUN cd /lhwsutil/build && \
    make install && \
    cd / && \
    rm -rf /lhwsutil && \
    rm -rf /lhmiscutil && \
    rm -rf /libjwt
