# Docker Setup for Radar Tracking System

This document explains how to use Docker to build, develop, and deploy the Modular C++ Radar Tracking System.

## 🐳 Quick Start

### Prerequisites

- Docker (version 20.10+)
- Docker Compose (version 2.0+)
- At least 4GB RAM available for Docker
- At least 10GB disk space

### Build and Run

```bash
# Quick development setup
./docker-build.sh --dev
docker-compose up radar-dev

# Quick production setup
./docker-build.sh --prod
docker-compose up -d radar-prod
```

## 📁 Docker Files Overview

- **`Dockerfile`** - Multi-stage Dockerfile with development and production targets
- **`docker-compose.yml`** - Complete orchestration setup
- **`.dockerignore`** - Optimizes build context
- **`docker-build.sh`** - Helper script for building images
- **`README-Docker.md`** - This documentation

## 🏗️ Multi-Stage Dockerfile

The Dockerfile uses a multi-stage build approach:

### Stage 1: Base (`base`)
- Ubuntu 22.04 LTS
- Essential build tools (cmake, gcc, clang)
- Basic utilities

### Stage 2: Dependencies (`dependencies`)
- All C++ libraries required by the project:
  - yaml-cpp (configuration)
  - spdlog (logging)
  - Eigen3 (mathematics)
  - Boost (utilities)
  - GTest (testing)
  - Google Benchmark (performance testing)
  - OpenMP (parallel processing)

### Stage 3: Development (`development`)
- Based on dependencies stage
- Additional development tools (gdb, valgrind, vim, etc.)
- Source code mounted as volume
- Interactive environment for development

### Stage 4: Production (`production`)
- Based on dependencies stage
- Builds the complete application
- Optimized for runtime
- Non-root user for security
- Health checks included

## 🔧 Build Options

### Using the Build Script

```bash
# Development build (default)
./docker-build.sh --dev

# Production build
./docker-build.sh --prod

# Build without cache
./docker-build.sh --prod --no-cache

# Build with custom arguments
./docker-build.sh --build-arg CMAKE_BUILD_TYPE=Debug

# Get help
./docker-build.sh --help
```

### Direct Docker Commands

```bash
# Build development image
docker build --target development -t radar-tracking-system:dev .

# Build production image
docker build --target production -t radar-tracking-system:latest .

# Build only dependencies (for faster subsequent builds)
docker build --target dependencies -t radar-tracking-system:deps .
```

## 🚀 Running Containers

### Development Environment

```bash
# Interactive development container
docker run -it --rm \
  -v $(pwd):/workspace \
  -v radar-build:/workspace/build \
  -p 8080:8080/udp \
  -p 8081:8081/tcp \
  radar-tracking-system:dev

# Using docker-compose (recommended)
docker-compose up radar-dev
```

Inside the container:
```bash
# Build the project
cd /workspace
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
make -j$(nproc)

# Run tests
ctest --output-on-failure

# Run the system
./radar_tracking_system --config ../config/system_config.yaml
```

### Production Environment

```bash
# Run production container
docker run -d \
  --name radar-system \
  -p 8080:8080/udp \
  -p 8081:8081/tcp \
  -v $(pwd)/config:/app/config:ro \
  -v $(pwd)/logs:/app/logs \
  radar-tracking-system:latest

# Using docker-compose (recommended)
docker-compose up -d radar-prod
```

### Full System with Simulator

```bash
# Run complete system with simulator
docker-compose up -d radar-prod radar-simulator

# View logs
docker-compose logs -f radar-prod
docker-compose logs -f radar-simulator
```

## 🔬 Development Workflow

### 1. Start Development Environment

```bash
docker-compose up -d radar-dev
docker-compose exec radar-dev /bin/bash
```

### 2. Develop and Test

```bash
# Inside container
cd /workspace
mkdir -p build && cd build

# Configure build
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DBUILD_SIMULATION=ON \
  -DBUILD_BENCHMARKS=ON

# Build
make -j$(nproc)

# Run tests
ctest --output-on-failure

# Run specific tests
./core_tests
./algorithm_tests
```

### 3. Debug

```bash
# Debug with GDB
gdb ./radar_tracking_system
(gdb) run --config ../config/system_config.yaml

# Memory check with Valgrind
valgrind --tool=memcheck --leak-check=full \
  ./radar_tracking_system --config ../config/system_config.yaml
```

### 4. Performance Testing

```bash
# Run benchmarks
./tracking_benchmark
./clustering_benchmark

# Profile with perf (if available)
perf record ./radar_tracking_system --config ../config/system_config.yaml
perf report
```

## 📊 Monitoring and Logging

### Basic Monitoring

```bash
# View container stats
docker stats radar-tracking-prod

# View logs
docker-compose logs -f radar-prod

# View system logs inside container
docker-compose exec radar-prod tail -f /app/logs/system.log
```

### Advanced Monitoring (Optional)

```bash
# Start monitoring stack
docker-compose --profile monitoring up -d

# Access Prometheus
open http://localhost:9090
```

## 🔧 Configuration

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `CMAKE_BUILD_TYPE` | Build type (Debug/Release) | Release |
| `BUILD_TESTING` | Enable testing | ON (dev), OFF (prod) |
| `BUILD_SIMULATION` | Enable simulation tools | ON |
| `BUILD_BENCHMARKS` | Enable benchmarks | ON (dev), OFF (prod) |
| `BUILD_PLUGINS` | Enable plugin system | ON |

### Volume Mounts

| Host Path | Container Path | Purpose |
|-----------|----------------|---------|
| `./` | `/workspace` | Source code (dev only) |
| `./config` | `/app/config` | Configuration files |
| `./logs` | `/app/logs` | Log files |
| `./scenarios` | `/app/scenarios` | Test scenarios |

### Port Mappings

| Host Port | Container Port | Protocol | Purpose |
|-----------|----------------|----------|---------|
| 8080 | 8080 | UDP | Primary radar data |
| 8081 | 8081 | TCP | Control interface |
| 8082 | 8080 | UDP | Simulator (when used) |
| 9090 | 9090 | TCP | Monitoring (optional) |

## 🚨 Troubleshooting

### Common Issues

#### Build Failures
```bash
# Clean build cache
docker-compose down -v
docker system prune -f
./docker-build.sh --prod --no-cache
```

#### Permission Issues
```bash
# Fix file permissions
sudo chown -R $USER:$USER logs/
sudo chown -R $USER:$USER build/
```

#### Container Won't Start
```bash
# Check logs
docker-compose logs radar-prod

# Check configuration
docker-compose exec radar-prod \
  ./build/radar_tracking_system --validate --config config/system_config.yaml
```

#### Network Issues
```bash
# Check port conflicts
netstat -tulpn | grep :8080

# Test connectivity
docker-compose exec radar-prod nc -l 8080 &
echo "test" | nc localhost 8080
```

### Performance Issues

#### High Memory Usage
```bash
# Monitor memory
docker stats --no-stream radar-tracking-prod

# Check for memory leaks
docker-compose exec radar-dev valgrind --tool=memcheck ./build/radar_tracking_system
```

#### High CPU Usage
```bash
# Profile CPU usage
docker-compose exec radar-dev perf top
```

## 🔐 Security Considerations

### Production Security

1. **Non-root User**: Production containers run as non-root user `radar`
2. **Read-only Mounts**: Configuration and scenarios mounted read-only
3. **Limited Capabilities**: No unnecessary Linux capabilities
4. **Network Isolation**: Custom Docker network with restricted access

### Best Practices

```bash
# Use specific image tags in production
docker pull radar-tracking-system:v1.0.0

# Scan for vulnerabilities
docker scan radar-tracking-system:latest

# Update base images regularly
./docker-build.sh --prod --no-cache
```

## 📈 Performance Optimization

### Build Optimization

```bash
# Use build cache between builds
./docker-build.sh --dev  # Reuses cached layers

# Multi-core compilation
docker build --build-arg MAKEFLAGS=-j$(nproc) .
```

### Runtime Optimization

```bash
# Allocate more memory
docker run --memory=8g radar-tracking-system:latest

# Use specific CPU cores
docker run --cpuset-cpus="0-3" radar-tracking-system:latest

# Optimize for real-time performance
docker run --ulimit rtprio=99 radar-tracking-system:latest
```

## 🔄 CI/CD Integration

### GitLab CI Example

```yaml
# .gitlab-ci.yml
stages:
  - build
  - test
  - deploy

build:
  stage: build
  script:
    - ./docker-build.sh --prod
    - docker tag radar-tracking-system:latest $CI_REGISTRY_IMAGE:$CI_COMMIT_SHA

test:
  stage: test
  script:
    - docker-compose up -d radar-dev
    - docker-compose exec -T radar-dev bash -c "cd /workspace && ./build_and_test.sh"

deploy:
  stage: deploy
  script:
    - docker push $CI_REGISTRY_IMAGE:$CI_COMMIT_SHA
  only:
    - main
```

### GitHub Actions Example

```yaml
# .github/workflows/docker.yml
name: Docker Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build Docker image
        run: ./docker-build.sh --prod
      - name: Run tests
        run: |
          docker-compose up -d radar-dev
          docker-compose exec -T radar-dev bash -c "cd /workspace && ./build_and_test.sh"
```

## 📚 Additional Resources

- [Docker Best Practices](https://docs.docker.com/develop/dev-best-practices/)
- [Docker Compose Reference](https://docs.docker.com/compose/compose-file/)
- [C++ Docker Guidelines](https://github.com/cpp-best-practices/cpp-docker)

## 🆘 Support

If you encounter issues with the Docker setup:

1. Check the troubleshooting section above
2. Review container logs: `docker-compose logs radar-prod`
3. Validate your environment: `docker version && docker-compose version`
4. Open an issue with detailed error messages and system information

---

**Happy containerized development!** 🐳✨