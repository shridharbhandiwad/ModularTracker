# Multi-stage Dockerfile for Radar Tracking System
# =================================================

# Stage 1: Base system with dependencies
FROM ubuntu:22.04 AS base

# Prevent interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Update package lists and install basic dependencies
RUN apt-get update && apt-get install -y \
    # Build essentials
    build-essential \
    cmake \
    git \
    pkg-config \
    # Compilers and tools
    gcc-11 \
    g++-11 \
    clang-14 \
    # Python for some build scripts
    python3 \
    python3-pip \
    # Networking tools
    curl \
    wget \
    # Utilities
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set default compiler
ENV CC=gcc-11
ENV CXX=g++-11

# Stage 2: Install C++ libraries and dependencies
FROM base AS dependencies

# Install C++ libraries required by the project
RUN apt-get update && apt-get install -y \
    # Core dependencies from CMakeLists.txt
    libyaml-cpp-dev \
    libspdlog-dev \
    libeigen3-dev \
    libboost-all-dev \
    # Threading and OpenMP
    libomp-dev \
    # Testing frameworks
    libgtest-dev \
    googletest \
    # Benchmarking
    libbenchmark-dev \
    # Profiling tools (optional)
    libgoogle-perftools-dev \
    # Additional utilities
    libssl-dev \
    # Documentation tools
    doxygen \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

# Build and install GTest (sometimes needed)
WORKDIR /tmp
RUN cd /usr/src/gtest && \
    cmake CMakeLists.txt && \
    make && \
    cp lib/*.a /usr/lib/ && \
    mkdir -p /usr/local/lib/gtest && \
    ln -s /usr/lib/libgtest.a /usr/local/lib/gtest/libgtest.a && \
    ln -s /usr/lib/libgtest_main.a /usr/local/lib/gtest/libgtest_main.a

# Stage 3: Optional ROS2 support (uncomment if needed)
# FROM dependencies AS ros2
# 
# # Install ROS2 Humble (optional)
# RUN apt-get update && apt-get install -y \
#     curl \
#     gnupg2 \
#     lsb-release \
#     && curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg \
#     && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(source /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null \
#     && apt-get update \
#     && apt-get install -y \
#     ros-humble-desktop \
#     python3-rosdep \
#     python3-colcon-common-extensions \
#     && rm -rf /var/lib/apt/lists/*

# Stage 4: Development environment
FROM dependencies AS development

# Create workspace directory
WORKDIR /workspace

# Set up environment variables
ENV CMAKE_BUILD_TYPE=Release
ENV BUILD_TESTING=ON
ENV BUILD_SIMULATION=ON
ENV BUILD_BENCHMARKS=ON
ENV BUILD_PLUGINS=ON

# Copy any cmake modules or find scripts if they exist
# COPY cmake/ /workspace/cmake/

# Create necessary directories
RUN mkdir -p /workspace/build \
             /workspace/logs \
             /workspace/logs/data \
             /workspace/scenarios

# Install additional development tools
RUN apt-get update && apt-get install -y \
    # Debugging tools
    gdb \
    valgrind \
    # Text editors
    vim \
    nano \
    # Version control
    git \
    # Documentation
    pandoc \
    # Network tools for testing
    netcat-openbsd \
    telnet \
    # Performance monitoring
    htop \
    iotop \
    && rm -rf /var/lib/apt/lists/*

# Configure Git (for development)
RUN git config --global user.name "Docker User" && \
    git config --global user.email "docker@example.com" && \
    git config --global init.defaultBranch main

# Set working directory
WORKDIR /workspace

# Stage 5: Production environment
FROM dependencies AS production

# Create application directory
WORKDIR /app

# Set production environment variables
ENV CMAKE_BUILD_TYPE=Release
ENV BUILD_TESTING=OFF
ENV BUILD_SIMULATION=ON
ENV BUILD_BENCHMARKS=OFF
ENV BUILD_PLUGINS=ON

# Copy the project source code
COPY . /app/

# Build the project
RUN mkdir -p build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DBUILD_TESTING=${BUILD_TESTING} \
        -DBUILD_SIMULATION=${BUILD_SIMULATION} \
        -DBUILD_BENCHMARKS=${BUILD_BENCHMARKS} \
        -DBUILD_PLUGINS=${BUILD_PLUGINS} && \
    make -j$(nproc) && \
    # Optional: run tests if enabled
    # ctest --output-on-failure && \
    # Install the system
    make install

# Create runtime user (security best practice)
RUN groupadd -r radar && useradd -r -g radar radar && \
    chown -R radar:radar /app

# Create directories for logs and data
RUN mkdir -p /app/logs /app/data /app/scenarios && \
    chown -R radar:radar /app/logs /app/data /app/scenarios

USER radar

# Expose default ports (adjust as needed)
EXPOSE 8080/udp 8081/tcp

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD pgrep -f radar_tracking_system || exit 1

# Default command
CMD ["./build/radar_tracking_system", "--config", "config/system_config.yaml"]

# Labels for metadata
LABEL maintainer="Radar Tracking System Team" \
      version="1.0.0" \
      description="Modular C++ Radar Tracking System" \
      org.opencontainers.image.title="Radar Tracking System" \
      org.opencontainers.image.description="High-performance modular radar tracking system for defense applications" \
      org.opencontainers.image.version="1.0.0" \
      org.opencontainers.image.vendor="Defense Systems Inc."