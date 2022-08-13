#!/bin/bash

DOCKERFILE_PATH="./Dockerfile"
MAIN_STAGE="lhwsutil-s4-main-env"
MAIN_TAG="lhwsutil:main"
BUILD_STAGE="lhwsutil-s1-build-env"
BUILD_TAG="lhwsutil:build-env"
TARGET_STAGE=${MAIN_STAGE}
TARGET_TAG=${MAIN_TAG}
BASE_STAGE="lhwsutil-stage-base-env"

if [[ $1 = "-build" ]];
then
    TARGET_STAGE=${BUILD_STAGE}
    TARGET_TAG=${BUILD_TAG}
fi

# Build the main image
docker build --target ${TARGET_STAGE} -t ${TARGET_TAG} -f ${DOCKERFILE_PATH} . || exit 1