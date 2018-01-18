#!/bin/sh

PROJECT_DIR=`pwd`
SRC_DIR=${PROJECT_DIR}/src
RIOT_DIR=${SRC_DIR}/RIOT
BIN_DIR=${SRC_DIR}/bin
TAPSETUP_CMD=${RIOT_DIR}/dist/tools/tapsetup/tapsetup
TAP0_CLASS_FOLDER=/sys/class/net/tap0/

prepare () {
    echo ">>> Preparing environment..."

    USER=`id -u`

    if [ "${USER}" -ne 0 ]; then
        echo "! This preparation needs to run as root"
        SUDO=sudo
    fi

    echo "This preparation will install the following packages and its dependencies:"
    echo "- git"
    echo "- gcc-multilib"
    echo -n "Do you agree? (Y/n) "

    read opt
    echo

    if [ "${opt}" != "n" ]; then
        ${SUDO} apt-get -y install git \
            gcc-multilib
    fi
}

gitclone () {
    echo ">>> Cloning environment..."

    if [ ! -d ${RIOT_DIR} ]; then
        git clone https://github.com/RIOT-OS/RIOT ${RIOT_DIR}
    fi
}

interface () {
    echo -n">>> Creating interface tap0 and tap1..."

    ${TAPSETUP_CMD} -c
    if [ $? ]; then
        echo "OK"
    else
        echo "Error"
    fi
}

build () {
    echo ">>> Building..."

    if [ ! -d ${RIOT_DIR} ]; then
        echo "! RIOT dir does not exist."
        echo "Run: $0 -c"
        return
    fi

    make RIOT_DIR=${RIOT_DIR}
}

run () {
    echo ">>> Running sample..."

    if [ ! -d "${TAP0_CLASS_FOLDER}" ]; then
        echo "Tap0 interface not exist. Run: $0 -i"
    fi

    ${SRC_DIR}/bin/native/echo_server.elf tap0
}

clean () {
    echo ">>> Cleaning..."
    make clean RIOT_DIR=${RIOT_DIR}
    rm -rf ${BIN_DIR}
}

deepclean () {
    echo ">>> Erasing..."

    clean

    if [ ! -d ${RIOT_DIR} ]; then
        echo "! RIOT dir does not exist."
        return
    fi

    rm -rf ${RIOT_DIR} 2> /dev/null

    ${TAPSETUP_CMD} -d
}

invalid() {
    echo "Invalid option: -$1"
}

param() {
    echo "Option -$1 requires an argument"
}

check() {
    if [ $1 != 0 ]; then
        echo "! Error running last command =("
        exit 1
    fi
}

usage() {
    echo "Usage: $1 [OPTIONS]"
    echo ""
    echo "  -p \t\tPrepare environment (requires sudo)"
    echo "  -g \t\tClone RIOT repository"
    echo "  -i \t\tCreate ethernet interface (requires sudo)"
    echo "  -b \t\tBuild"
    echo "  -r <port> \tRun sample test on specific <port>"
    echo "  -c \t\tClean"
    echo "  -d \t\tErase build dir"
    echo ""
}

RUN=0
while getopts ":p :g :i :b :r :c :d" opt; do
    RUN=1
    case $opt in
        p)
            prepare
            ;;
        g)
            gitclone
            ;;
        i)
            interface
            ;;
        b)
            build
            ;;
        c)
            clean
            ;;
        r)
            run
            ;;
        d)
            deepclean
            ;;
        :)
            param $OPTARG
            RUN=0
            ;;
        \?)
            invalid $OPTARG
            RUN=0
            ;;
    esac
    check $?
done

if [ "$RUN" != "1" ]; then
    usage $0
    exit 1
fi
