FROM ubuntu:18.04

# Install build deps
RUN apt-get update
RUN apt-get install -y gcc-arm-none-eabi git cmake

# Create work directory
RUN mkdir -p /opt/repo/build

# Clone the required repositories
WORKDIR /opt/repo
RUN git clone https://github.com/w5292c/mcode.git
RUN git clone https://github.com/w5292c/stm32-spd.git
RUN git clone https://github.com/w5292c/stm32-cmake.git

# Build/install the STM32 SPD/CMAKE projects
WORKDIR /opt/repo/build
ENV GITHUB_ROOT=/opt/repo
ENV STM32_CMAKE_PATH=/opt/repo/stm32-cmake
RUN /opt/repo/mcode/targets/stm32-common/build-stm32-cmake.sh

VOLUME ["/opt/repo/output"]
COPY build.sh /opt/repo/build

# Enter the interactive shell
ENTRYPOINT ["/bin/bash"]
CMD ["./build.sh"]
