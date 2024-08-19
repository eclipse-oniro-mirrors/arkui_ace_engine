#!/bin/bash -x
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DTS_DIR_ARG=$1
DEST_DIR_ARG=$2
TMP_DIR_ARG=$3
GENERATOR_ARG=$4
OHOS_DIR_ARG=$5

OHOS_DIR=${OHOS_DIR_ARG:=../../../../../../..}

if [[ ! -d $IDLIZE_PATH ]]; then
   echo "Please define IDLIZE_PATH environment that points to idlize source directory."
   exit 1
fi

DEST_DIR=${DEST_DIR_ARG:=.}
DTS_DIR=${DTS_DIR_ARG:=${OHOS_DIR}/interface/sdk-js/api/@internal/component/ets}
TMP_DIR=${TMP_DIR_ARG:=${OHOS_DIR}/out/libace_c_api_generated}
# GENERATOR=${GENERATOR_ARG:=npx --yes @azanat/idlize@1.3.5 --dts2peer}
# Use the below to run generator from your idlize workspace
GENERATOR=${GENERATOR_ARG:=node $IDLIZE_PATH/lib/index.js --dts2peer}

echo "Generating C API from ${DTS_DIR} to ${DEST_DIR} with ${GENERATOR}"

${GENERATOR} \
    --output-dir ${TMP_DIR} \
    --input-dir ${DTS_DIR} \
    --generator-target libace \
    --libace-destination ${DEST_DIR} \
    --api-version 99

