#!/bin/bash

DOCKERFILE_PATH="./Dockerfile"
TEST_STAGE="lhwsutil-s3-test-env"
TEST_TAG="lhwsutil:test-env"
MAIN_STAGE="lhwsutil-s4-main-env"
MAIN_TAG="lhwsutil:main"
TARGET_STAGE=${MAIN_STAGE}
TARGET_TAG=${MAIN_TAG}
BASE_STAGE="lhwsutil-stage-base-env"

# Build the main image
docker build --target ${TARGET_STAGE} -t ${TARGET_TAG} -f ${DOCKERFILE_PATH} . || exit 1
