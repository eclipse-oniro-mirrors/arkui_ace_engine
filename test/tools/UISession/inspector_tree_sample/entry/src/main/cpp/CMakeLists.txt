# Copyright (c) 2026 Huawei Device Co., Ltd.
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

cmake_minimum_required(VERSION 3.5.0)
project(inspector_tree_provider)
add_library(inspector_tree_provider SHARED inspector_tree_provider.cpp)
target_compile_features(inspector_tree_provider PRIVATE cxx_std_17)
target_link_libraries(inspector_tree_provider PUBLIC libace_ndk.z.so libace_napi.z.so)
