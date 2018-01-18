#!/bin/sh

PROJECT_DIR=`pwd`
SRC_DIR=${PROJECT_DIR}/src
RIOT_DIR=${SRC_DIR}/RIOT
BIN_DIR=${SRC_DIR}/bin
TAPSETUP_CMD=${RIOT_DIR}/dist/tools/tapsetup/tapsetup
TAP0_CLASS_FOLDER=/sys/class/net/tap0/
BINARY_FILE=${SRC_DIR}/bin/native/echo_server.elf

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

    if [ -d ${RIOT_DIR} ]; then
        echo "RIOT directory already exist"
        return
    fi

    git clone https://github.com/RIOT-OS/RIOT ${RIOT_DIR}
}

interface () {
    echo ">>> Creating interface tap0 and tap1..."

    if [ -d "${TAP0_CLASS_FOLDER}" ]; then
        echo "tap0 interface already exist"
        return
    fi

    if [ ! -f "${TAPSETUP_CMD}" ]; then
        echo "RIOT not cloned. Run: $0 -g"
        return
    fi

    ${TAPSETUP_CMD} -c
}

build () {
    echo ">>> Building..."

    if [ ! -d ${RIOT_DIR} ]; then
        echo "! RIOT dir does not exist."
        echo "Run: $0 -g"
        return
    fi

    make RIOT_DIR=${RIOT_DIR}
}

run () {
    echo ">>> Running sample..."

    if [ ! -d "${TAP0_CLASS_FOLDER}" ]; then
        echo "Tap0 interface not exist. Run: $0 -i"
        return
    fi

    if [ ! -f "${BINARY_FILE}" ]; then
        echo "Project not compiled. Run: $0 -b"
        return
    fi

    ${BINARY_FILE} tap0
}

clean () {
    echo ">>> Cleaning..."

    make clean RIOT_DIR=${RIOT_DIR}

    if [ -d ${BIN_DIR} ]; then
        rm -rf ${BIN_DIR}
    fi
}

deepclean () {
    echo ">>> Erasing..."

    clean

    if [ ! -d ${RIOT_DIR} ]; then
        echo "! RIOT dir does not exist."
        return
    fi

    ${TAPSETUP_CMD} -d

    rm -rf ${RIOT_DIR} 2> /dev/null
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
    echo "  -r \t\tRun application"
    echo "  -c \t\tClean"
    echo "  -d \t\tClean all temporary files and remove ethernet interface (requires sudo)"
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
