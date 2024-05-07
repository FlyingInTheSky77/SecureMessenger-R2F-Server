#!/bin/bash

# Hint: Run 'chmod +x build.sh' to make this script executable.
# Then you can run './build.sh' to execute it
# and you'll avoid error like build.sh: 26: [[: not found

BASEDIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="../$(basename ${BASEDIR})_build"

CLEAN_BUILD=false
THREADS_COUNT=4

usage() {
    echo "Usage: ./build.sh [options]"
    echo "Options:"
    echo "  -c, --clean     Clean the build directory"
    echo "  -t <n>, --threads <n>    Set the number of threads for building (default: 4)"
    exit 1
}

unknown_option() {
    echo "Error: Unknown option $1"
    usage
    exit 1  # Add exit 1 to terminate the script
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--clean) CLEAN_BUILD=true;;
        -t|--threads) THREADS_COUNT="$2"; shift;;
        -h|--help) usage;;
        *) echo "Error: Unknown option $1"; usage;;
    esac
    shift
done

echo "build.sh script location: ${BASEDIR}"
cd ${BASEDIR}

if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "${BUILD_DIR:?}"
fi

mkdir -p "${BUILD_DIR:?}"
echo "Build directory: ${BUILD_DIR}"

cd ${BUILD_DIR}

cmake ${BASEDIR}
make -j${THREADS_COUNT}

# in case You want to run app immediately after compiling uncomment line below
#./R2F-MessengerServer
