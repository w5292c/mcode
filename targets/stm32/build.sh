#!/bin/bash

BUILD_DIR=${PWD}
REPO_DIR=$(dirname ${BUILD_DIR})
OUTPUT_DIR=${REPO_DIR}/output
MCODE_DIR=${REPO_DIR}/mcode
MCODE_REVISION="624d65c13b40c8e2ee32d48b8db47853ca1a31ff"

echo "Build directory: ${BUILD_DIR}, repo is in ${REPO_DIR}, output dir: ${OUTPUT_DIR}, mcode is in: ${MCODE_DIR}"

echo "Checking out the requested version for mcode"
cd ${MCODE_DIR}
git fetch origin
git checkout ${MCODE_REVISION}
cd ${BUILD_DIR}

echo "Configure mcode"
cmake ${MCODE_DIR}/targets/stm32
make console-test.hex

echo "Copy the results"
cp console-test.hex ${OUTPUT_DIR}

echo "Done."
