#!/bin/bash
# Run this from within a bash shell
# x86_64 is for simulation do not enable RK platform
export AIQ_BUILD_HOST_DIR=/home/camera/camera/rv1109_sdk/buildroot/output/rockchip_rv1126_rv1109/host
#export AIQ_BUILD_HOST_DIR=/work/linux/rv1109/src/buildroot/output/rockchip_rv1126_rv1109/host
TOOLCHAIN_FILE=$(pwd)/arm_linux_buildroot.cmake
#TOOLCHAIN_FILE=$AIQ_BUILD_HOST_DIR/share/buildroot/toolchainfile.cmake
OUTPUT=$(pwd)/output/arm
SOURCE_PATH=$OUTPUT/../../

mkdir -p $OUTPUT
pushd $OUTPUT

cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=debug \
    -DARCH="arm" \
    -DRKPLATFORM=OFF \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    $SOURCE_PATH \
&& make -j$(nproc)

popd
