#!/bin/bash

TARGET_STAGE="$1"
if [[ "x${TARGET_STAGE}" == "x" ]];
then
    TARGET_STAGE="dist"
fi

# lhmiscutil
pushd ./modules/lhmiscutil
./scripts/buildLHMiscUtilImage.sh
popd

# lhsslutil
pushd ./modules/lhsslutil
./scripts/buildLHSSLUtilImage.sh
popd

# libjwt
./scripts/buildLibJwtLHDistImage.sh

./modules/lhscriptutil/scripts/buildImage.sh ./Dockerfiles/Dockerfile.lhwsutil lhwsutil "${TARGET_STAGE}"