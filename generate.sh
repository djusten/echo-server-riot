#!/bin/sh

PROJECT_DIR=`pwd`
SRC_DIR=${PROJECT_DIR}/src
RIOT_DIR=${SRC_DIR}/RIOT
BIN_DIR=${SRC_DIR}/bin

prepare () {
    echo ">>> Preparing environment..."
}

clone () {
    echo ">>> Cloning environment..."

    if [ ! -d ${RIOT_DIR} ]; then
        git clone https://github.com/RIOT-OS/RIOT ${RIOT_DIR}
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
    echo "  -c \t\tClone RIOT repository"
    echo "  -b \t\tBuild"
    echo "  -r \t\tRun sample test"
    echo "  -c \t\tClean"
    echo "  -d \t\tErase build dir"
    echo ""
}

RUN=0
while getopts ":p :g :b :r :c :d" opt; do
    RUN=1
    case $opt in
        p)
            prepare
            ;;
        g)
            clone
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
