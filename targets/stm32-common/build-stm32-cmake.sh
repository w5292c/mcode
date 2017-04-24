#!/bin/bash

# Check the project directory first
if [ -d "${GITHUB_ROOT}" ] ; then
  echo "Here is your project path: \"${GITHUB_ROOT}\""
else
  echo "Error: directory \"${GITHUB_ROOT}\" does not exist"
  exit 1
fi

# Now, check that the 'stm32-spd' exists in the project directory
STM32_SPD_PATH="${GITHUB_ROOT}/stm32-spd"
if [ -d "${STM32_SPD_PATH}" ] ; then
  echo "Found 'stm32-spd' at \"${STM32_SPD_PATH}\""
else
  echo "Error: 'stm32-spd' does not exist in your project directory \"${GITHUB_ROOT}\""
  echo "       Your can clone 'stm32-spd' like this:"
  echo "       $ cd ${GITHUB_ROOT}"
  echo "       $ git clone https://github.com/w5292c/stm32-spd.git"
  echo "       After this is done, you can try again."
  exit 1
fi

# Check 'stm32-cmake' now
STM32_CMAKE_PATH="${GITHUB_ROOT}/stm32-cmake"
if [ -d "${STM32_CMAKE_PATH}" ] ; then
  echo "Found 'stm32-cmake' at \"${STM32_CMAKE_PATH}\""
else
  echo "Error: 'stm32-cmake' does not exist in your project directory \"${GITHUB_ROOT}\""
  echo "       Your can clone 'stm32-cmake' like this:"
  echo "       $ cd ${GITHUB_ROOT}"
  echo "       $ git clone https://github.com/w5292c/stm32-cmake.git"
  echo "       After this is done, you can try again."
fi
CMSIS_PATH="${STM32_CMAKE_PATH}/cmsis"
if [ -d "${CMSIS_PATH}" ] ; then
  echo "Found 'CMSIS' at \"${CMSIS_PATH}\""
else
  echo "Error: 'CMSIS' does not exist, check 'stm32-cmake' at \"${STM32_CMAKE_PATH}\""
fi
STD_PERIPH_PATH="${STM32_CMAKE_PATH}/stdperiph"
if [ -d "${STD_PERIPH_PATH}" ] ; then
  echo "Found 'stdperiph' at \"${STD_PERIPH_PATH}\""
else
  echo "Error: 'stdperiph' does not exist, check 'stm32-cmake' at \"${STM32_CMAKE_PATH}\""
fi

# Store the user directory
USER_PATH=`pwd`

# Build 'stm32-cmake/CMSIS'
cd "${CMSIS_PATH}"
git clean -dfx
if ! cmake -DSTM32F1_StdPeriphLib_DIR=${STM32_SPD_PATH} -DCMAKE_TOOLCHAIN_FILE=../gcc_stm32.cmake -DSTM32_FAMILY=F1 -DCMAKE_INSTALL_PREFIX=/usr/arm-none-eabi/ -DCMAKE_BUILD_TYPE=Release ; then
  echo "Error: failed configuring 'stm32-cmake'"
  cd "${USER_PATH}"
  exit 1
fi

if ! make ; then
  echo "'CMSIS' compilation failed"
  cd "${USER_PATH}"
  exit 1
fi
if ! sudo make install ; then
  echo "'CMSIS' installation failed"
  cd "${USER_PATH}"
  exit 1
fi

# Build 'stm32-cmake/stdperiph'
cd "${STD_PERIPH_PATH}"
git clean -dfx
if ! cmake -DSTM32F1_StdPeriphLib_DIR=${STM32_SPD_PATH} -DCMAKE_TOOLCHAIN_FILE=../gcc_stm32.cmake -DCMAKE_MODULE_PATH=${STM32_CMAKE_PATH}/cmake/Modules -DSTM32_FAMILY=F1 -DCMAKE_INSTALL_PREFIX=/usr/arm-none-eabi/ -DCMAKE_BUILD_TYPE=Release ; then
  echo "Error: 'stm32-cmake/stdperiph' configuration failed"
  cd "${USER_PATH}"
  exit 1
fi
if ! make ; then
  echo "'stm32-cmake/stdperiph' compilation failed"
  cd "${USER_PATH}"
  exit 1
fi
if ! sudo make install ; then
  echo "'stm32-cmake/stdperiph' installation failed"
  cd "${USER_PATH}"
  exit 1
fi
echo "Success: 'stm32-cmake' has been built and installed at \"/usr/arm-none-eabi/\", enjoy"
