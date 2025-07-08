#!/bin/bash

# Docker Build Script for Radar Tracking System
# ============================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

function print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

function print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

function print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

function print_header() {
    echo -e "${BLUE}[DOCKER]${NC} $1"
}

# Default values
TARGET="development"
BUILD_ARGS=""
NO_CACHE=""
PUSH=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--target)
            TARGET="$2"
            shift 2
            ;;
        --prod|--production)
            TARGET="production"
            shift
            ;;
        --dev|--development)
            TARGET="development"
            shift
            ;;
        --no-cache)
            NO_CACHE="--no-cache"
            shift
            ;;
        --push)
            PUSH=true
            shift
            ;;
        --build-arg)
            BUILD_ARGS="$BUILD_ARGS --build-arg $2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -t, --target TARGET    Build target (development, production, dependencies)"
            echo "  --dev, --development   Build development target (default)"
            echo "  --prod, --production   Build production target"
            echo "  --no-cache            Don't use cache when building"
            echo "  --push                Push to registry after building"
            echo "  --build-arg ARG=VAL   Pass build argument"
            echo "  -h, --help            Show this help"
            echo ""
            echo "Examples:"
            echo "  $0 --dev              # Build development image"
            echo "  $0 --prod --no-cache  # Build production image without cache"
            echo "  $0 --target dependencies  # Build only dependencies layer"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_header "Building Radar Tracking System Docker Image"
print_status "Target: $TARGET"

# Check if Docker is available
if ! command -v docker &> /dev/null; then
    print_error "Docker is not installed or not in PATH"
    exit 1
fi

# Check if Dockerfile exists
if [[ ! -f "Dockerfile" ]]; then
    print_error "Dockerfile not found in current directory"
    exit 1
fi

# Set image name based on target
IMAGE_NAME="radar-tracking-system"
case $TARGET in
    "development")
        IMAGE_TAG="${IMAGE_NAME}:dev"
        ;;
    "production")
        IMAGE_TAG="${IMAGE_NAME}:latest"
        ;;
    "dependencies")
        IMAGE_TAG="${IMAGE_NAME}:deps"
        ;;
    *)
        IMAGE_TAG="${IMAGE_NAME}:${TARGET}"
        ;;
esac

print_status "Building image: $IMAGE_TAG"

# Build the Docker image
print_header "Starting Docker build..."
docker build \
    $NO_CACHE \
    --target $TARGET \
    --tag $IMAGE_TAG \
    $BUILD_ARGS \
    .

if [[ $? -eq 0 ]]; then
    print_status "Build successful!"
    
    # Show image size
    IMAGE_SIZE=$(docker images $IMAGE_TAG --format "table {{.Size}}" | tail -n 1)
    print_status "Image size: $IMAGE_SIZE"
    
    # Push if requested
    if [[ $PUSH == true ]]; then
        print_header "Pushing image to registry..."
        docker push $IMAGE_TAG
        if [[ $? -eq 0 ]]; then
            print_status "Push successful!"
        else
            print_error "Push failed!"
            exit 1
        fi
    fi
    
    print_header "Build Summary"
    echo "  Image: $IMAGE_TAG"
    echo "  Target: $TARGET"
    echo "  Size: $IMAGE_SIZE"
    echo ""
    print_status "Quick start commands:"
    echo ""
    
    case $TARGET in
        "development")
            echo "  # Run development container:"
            echo "  docker run -it --rm -v \$(pwd):/workspace $IMAGE_TAG"
            echo ""
            echo "  # Or use docker-compose:"
            echo "  docker-compose up radar-dev"
            ;;
        "production")
            echo "  # Run production container:"
            echo "  docker run -d --name radar-system -p 8080:8080/udp $IMAGE_TAG"
            echo ""
            echo "  # Or use docker-compose:"
            echo "  docker-compose up -d radar-prod"
            ;;
    esac
    
else
    print_error "Build failed!"
    exit 1
fi