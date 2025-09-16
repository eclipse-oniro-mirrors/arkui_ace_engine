/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "frameworks/base/geometry/matrix4.h"

#include <algorithm>
#include <ani.h>
#include <array>
#include <chrono>
#include <future>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "base/geometry/matrix4.h"
#include "core/components_ng/base/view_abstract_model_ng.h"
#include "core/components_ng/render/render_context.h"
#include "frameworks/base/geometry/ng/point_t.h"
#include "frameworks/bridge/js_frontend/engine/common/js_constants.h"
#include "frameworks/core/components_ng/render/adapter/matrix2d.h"
#include "frameworks/core/components_ng/render/adapter/matrix_util.h"
#include "interop_js/arkts_esvalue.h"
namespace OHOS::Ace::MatrixAni {
constexpr int32_t MATRIX_LENGTH = 16;
constexpr int32_t MATRIX_LENGTH_TRANSFORM_POINT = 2;

static Matrix4_Obj* GetMatrixObj(ani_env* env, ani_object obj)
{
    ani_long matrix4x4;
    if (ANI_OK != env->Object_GetFieldByName_Long(obj, "matrix4Object", &matrix4x4)) {
        return nullptr;
    }
    return reinterpret_cast<Matrix4_Obj*>(matrix4x4);
}

static ani_object Matrix4_Identity([[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object object)
{
    static const char* className = "L@ohos/matrix4/matrix4/Matrix4TransitInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        ani_object nullobj = nullptr;
        return nullobj;
    }

    auto matrix4Object = new Matrix4_Obj(OHOS::Ace::Matrix4::CreateIdentity());
    ani_object matrix4ObjectCur = {};
    if (ANI_OK != env->Object_New(cls, ctor, &matrix4ObjectCur, reinterpret_cast<ani_long>(matrix4Object))) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    return matrix4ObjectCur;
}

Matrix4 ConvertToMatrix([[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object option)
{
    Matrix4 result = Matrix4::CreateIdentity();
    ani_tuple_value inputArray = static_cast<ani_tuple_value>(option);
    ani_size length = 0;
    if (ANI_OK != env->TupleValue_GetNumberOfItems(inputArray, &length)) {
        return result;
    }
    int32_t inputSize = static_cast<int32_t>(length);
    if (inputSize != MATRIX_LENGTH) {
        return result;
    }
    for (int32_t i = 0; i < Matrix4::DIMENSION; i++) {
        for (int32_t j = 0; j < Matrix4::DIMENSION; j++) {
            ani_double value;
            auto index = static_cast<ani_size>(i * Matrix4::DIMENSION + j);
            if (ANI_OK != env->TupleValue_GetItem_Double(inputArray, index, &value)) {
                return result;
            }
            auto ret = static_cast<double>(value);
            result.Set(j, i, ret);
        }
    }
    return result;
}

Matrix4 ConvertToMatrixArray([[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object option)
{
    Matrix4 result = Matrix4::CreateIdentity();
    ani_array_double inputArray = static_cast<ani_array_double>(option);
    ani_size length = 0;
    if (ANI_OK != env->Array_GetLength(inputArray, &length)) {
        return result;
    }
    int32_t inputSize = static_cast<int32_t>(length);
    if (inputSize != MATRIX_LENGTH) {
        return result;
    }
    for (int32_t i = 0; i < Matrix4::DIMENSION; i++) {
        for (int32_t j = 0; j < Matrix4::DIMENSION; j++) {
            ani_double value;
            auto index = static_cast<ani_size>(i * Matrix4::DIMENSION + j);
            if (ANI_OK != env->Array_GetRegion_Double(inputArray, index, static_cast<ani_size>(1), &value)) {
                return result;
            }
            auto ret = static_cast<double>(value);
            result.Set(j, i, ret);
        }
    }
    return result;
}

static ani_object Matrix4_Init([[maybe_unused]] ani_env* env,
    [[maybe_unused]] ani_object option)
{
    static const char* className = "L@ohos/matrix4/matrix4/Matrix4TransitInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        ani_object nullobj = nullptr;
        return nullobj;
    }

    auto matrix = ConvertToMatrix(env, option);
    auto matrix4Object = new Matrix4_Obj(matrix);

    ani_object matrix4ObjectCur;
    if (ANI_OK != env->Object_New(cls, ctor, &matrix4ObjectCur, reinterpret_cast<ani_long>(matrix4Object))) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    return matrix4ObjectCur;
}

static ani_object Matrix4_Copy([[maybe_unused]] ani_env* env, [[maybe_unused]] ani_object object)
{
    static const char* className = "L@ohos/matrix4/matrix4/Matrix4TransitInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    auto matrix4Object = new Matrix4_Obj(matrixObj->matrix4x4);
    ani_object matrix4ObjectCur;
    if (ANI_OK != env->Object_New(cls, ctor, &matrix4ObjectCur, reinterpret_cast<ani_long>(matrix4Object))) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    return matrix4ObjectCur;
}

bool ParseOption(ani_env* env, ani_object options, double& input, const char* property, const char* className)
{
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        return false;
    }
    ani_ref property_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(options, property, &property_ref)) {
        return false;
    }
    ani_boolean isUndefined;
    if (ANI_OK != env->Reference_IsUndefined(property_ref, &isUndefined)) {
        return false;
    }
    if (isUndefined) {
        return false;
    }
    ani_double propertyValue;
    if (ANI_OK != env->Object_CallMethodByName_Double(static_cast<ani_object>(property_ref),
        "unboxed", nullptr, &propertyValue)) {
        return false;
    }
    input = static_cast<double>(propertyValue);
    return true;
}

static ani_object Matrix4_Scale([[maybe_unused]] ani_env* env, ani_object object, ani_object options)
{
    double xValue = 1.0;
    ParseOption(env, options, xValue, "x", "L@ohos/matrix4/matrix4/ScaleOption;");
    double yValue = 1.0;
    ParseOption(env, options, yValue, "y", "L@ohos/matrix4/matrix4/ScaleOption;");
    double zValue = 1.0;
    ParseOption(env, options, zValue, "z", "L@ohos/matrix4/matrix4/ScaleOption;");
    double centerXValue = 0.0;
    ParseOption(env, options, centerXValue, "centerX", "L@ohos/matrix4/matrix4/ScaleOption;");
    double centerYValue = 0.0;
    ParseOption(env, options, centerYValue, "centerY", "L@ohos/matrix4/matrix4/ScaleOption;");

    auto scaleMatrix = Matrix4::CreateScale(xValue, yValue, zValue);
    if (!NearZero(centerXValue) || !NearZero(centerYValue)) {
        auto translate1 = Matrix4::CreateTranslate(centerXValue, centerYValue, 0.0);
        auto translate2 = Matrix4::CreateTranslate(-centerXValue, -centerYValue, 0.0);
        scaleMatrix = scaleMatrix * translate2;
        scaleMatrix = translate1 * scaleMatrix;
    }

    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    matrixObj->matrix4x4 = scaleMatrix * matrixObj->matrix4x4;
    return object;
}

static ani_object Matrix4_Rotate([[maybe_unused]] ani_env* env, ani_object object, ani_object options)
{
    double dx = 0.0;
    ParseOption(env, options, dx, "x", "L@ohos/matrix4/matrix4/RotateOption;");
    double dy = 0.0;
    ParseOption(env, options, dy, "y", "L@ohos/matrix4/matrix4/RotateOption;");
    double dz = 0.0;
    ParseOption(env, options, dz, "z", "L@ohos/matrix4/matrix4/RotateOption;");
    double angle = 0.0;
    ParseOption(env, options, angle, "angle", "L@ohos/matrix4/matrix4/RotateOption;");
    double centerX = 0.0;
    ParseOption(env, options, centerX, "centerX", "L@ohos/matrix4/matrix4/RotateOption;");
    double centerY = 0.0;
    ParseOption(env, options, centerY, "centerY", "L@ohos/matrix4/matrix4/RotateOption;");

    auto rotateMatrix = Matrix4::CreateRotate(angle, dx, dy, dz);
    if (!NearZero(centerX) || !NearZero(centerY)) {
        auto translate1 = Matrix4::CreateTranslate(centerX, centerY, 0.0);
        auto translate2 = Matrix4::CreateTranslate(-centerX, -centerY, 0.0);
        rotateMatrix = rotateMatrix * translate2;
        rotateMatrix = translate1 * rotateMatrix;
    }

    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    matrixObj->matrix4x4 = rotateMatrix * matrixObj->matrix4x4;
    return object;
}

static ani_object Matrix4_Skew([[maybe_unused]] ani_env* env, ani_object object, ani_double inputX, ani_double inputY)
{
    double sxInput = 0.0f;
    double syInput = 0.0f;
    if (!std::isnan(inputX)) {
        sxInput = static_cast<double>(inputX);
    }
    if (!std::isnan(inputX)) {
        syInput = static_cast<double>(inputX);
    }

    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    matrixObj->matrix4x4 =
        Matrix4::CreateFactorSkew(static_cast<float>(sxInput), static_cast<float>(syInput)) * matrixObj->matrix4x4;
    return object;
}

void ParseArray([[maybe_unused]] ani_env* env, const char* property, ani_object options,
    std::vector<OHOS::Ace::NG::PointT<int32_t>>& points)
{
    ani_ref params_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(options, property, &params_ref)) {
        return;
    }
    ani_class arrayClass;
    if (ANI_OK != env->FindClass("Lescompat/Array;", &arrayClass)) {
        return;
    }
    ani_boolean isArray;
    if (ANI_OK != env->Object_InstanceOf(static_cast<ani_object>(params_ref), arrayClass, &isArray)) {
        return;
    }
    ani_double length;
    if (ANI_OK != env->Object_GetPropertyByName_Double(static_cast<ani_object>(params_ref), "length", &length)) {
        return;
    }
    for (int i = 0; i < static_cast<int32_t>(length); i++) {
        ani_ref pointRef;
        if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(params_ref),
            "$_get", "I:Lstd/core/Object;", &pointRef, (ani_int)i)) {
            break;
        }
        ani_object pointObject = static_cast<ani_object>(pointRef);
        // parse x
        ani_double aniX;
        if (ANI_OK != env->Object_GetPropertyByName_Double(pointObject, "x", &aniX)) {
            aniX = 0.0f;
        }
        // parse y
        ani_double aniY;
        if (ANI_OK != env->Object_GetPropertyByName_Double(pointObject, "y", &aniY)) {
            aniY = 0.0f;
        }
        points.emplace_back(OHOS::Ace::NG::PointT(static_cast<int32_t>(aniX), static_cast<int32_t>(aniY)));
    }
}

static ani_object Matrix4_SetPolyToPoly([[maybe_unused]] ani_env* env, ani_object object, ani_object options)
{
    double srcIndexInput = 0.0;
    ParseOption(env, options, srcIndexInput, "srcIndex", "L@ohos/matrix4/matrix4/PolyToPolyOptions;");
    double dstIndexInput = 0.0;
    ParseOption(env, options, dstIndexInput, "dstIndex", "L@ohos/matrix4/matrix4/PolyToPolyOptions;");
    double pointCountInput = 0.0;
    ParseOption(env, options, pointCountInput, "pointCount", "L@ohos/matrix4/matrix4/PolyToPolyOptions;");
    int srcIndex = static_cast<int>(srcIndexInput);
    int dstIndex = static_cast<int>(dstIndexInput);
    int pointCount = static_cast<int>(pointCountInput);
    std::vector<OHOS::Ace::NG::PointT<int32_t>> srcPoint;
    ParseArray(env, "src", options, srcPoint);
    std::vector<OHOS::Ace::NG::PointT<int32_t>> dstPoint;
    ParseArray(env, "dst", options, dstPoint);
    if (pointCount <= 0 || pointCount > static_cast<int32_t>(srcPoint.size()) ||
        pointCount > static_cast<int32_t>(dstPoint.size())) {
        LOGE("setpPolyToPoly pointCount out of range pointCount:%{public}d, src size:%{public}d, dst size:%{public}d",
            pointCount, static_cast<int>(srcPoint.size()), static_cast<int>(dstPoint.size()));
        return object;
    }
    if (srcIndex < 0 || (pointCount + srcIndex) > static_cast<int32_t>(srcPoint.size())) {
        LOGE("setpPolyToPoly srcIndex out of range srcIndex:%{public}d, pointCount:%{public}d, src size%{public}d",
            srcIndex, pointCount, static_cast<int>(srcPoint.size()));
        return object;
    }
    if (dstIndex < 0 || (pointCount + dstIndex) > static_cast<int32_t>(dstPoint.size())) {
        LOGE("setpPolyToPoly dstIndex out of range dstIndex:%{public}d, pointCount:%{public}d, dst size%{public}d",
            dstIndex, pointCount, static_cast<int>(dstPoint.size()));
        return object;
    }
    std::vector<OHOS::Ace::NG::PointT<int32_t>> totalPoint;
    int srcLastIndex = pointCount + srcIndex;
    for (int i = srcIndex; i < srcLastIndex; i++) {
        totalPoint.push_back(srcPoint[i]);
    }
    int dstLastIndex = pointCount + dstIndex;
    for (int i = dstIndex; i < dstLastIndex; i++) {
        totalPoint.push_back(dstPoint[i]);
    }
    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    Matrix4 ret = OHOS::Ace::NG::MatrixUtil::SetMatrixPolyToPoly(matrixObj->matrix4x4, totalPoint);
    matrixObj->matrix4x4 = ret;
    return object;
}

static ani_tuple_value Matrix4_TransformPoint([[maybe_unused]] ani_env* env, ani_object object, ani_object options)
{
    ani_tuple_value inputArray = static_cast<ani_tuple_value>(options);
    ani_tuple_value arrayRes = static_cast<ani_tuple_value>(options);
    ani_size length = 0;
    ani_status tmp = env->TupleValue_GetNumberOfItems(inputArray, &length);
    if (ANI_OK != tmp) {
        return arrayRes;
    }
    int32_t inputSize = static_cast<int32_t>(length);
    if (inputSize != MATRIX_LENGTH_TRANSFORM_POINT) {
        return arrayRes;
    }
    ani_double x;
    if (ANI_OK != env->TupleValue_GetItem_Double(inputArray, static_cast<ani_size>(0), &x)) {
        return arrayRes;
    }
    ani_double y;
    if (ANI_OK != env->TupleValue_GetItem_Double(inputArray, static_cast<ani_size>(1), &y)) {
        return arrayRes;
    }
    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return arrayRes;
    }
    Point point { static_cast<double>(x), static_cast<double>(y) };
    Point target = matrixObj->matrix4x4 * point;

    // // in column order
    env->TupleValue_SetItem_Double(arrayRes, static_cast<ani_size>(0), target.GetX());
    env->TupleValue_SetItem_Double(arrayRes, static_cast<ani_size>(1), target.GetY());
    return arrayRes;
}

static ani_object Matrix4_Translate([[maybe_unused]] ani_env* env, ani_object object, ani_object options)
{
    double dx = 0.0;
    ParseOption(env, options, dx, "x", "L@ohos/matrix4/matrix4/TranslateOption;");
    double dy = 0.0;
    ParseOption(env, options, dy, "y", "L@ohos/matrix4/matrix4/TranslateOption;");
    double dz = 0.0;
    ParseOption(env, options, dz, "z", "L@ohos/matrix4/matrix4/TranslateOption;");

    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    matrixObj->matrix4x4 =
        Matrix4::CreateTranslate(static_cast<float>(dx), static_cast<float>(dy), static_cast<float>(dz)) *
        matrixObj->matrix4x4;
    return object;
}

static ani_object Matrix4_Invert([[maybe_unused]] ani_env* env, ani_object object)
{
    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    matrixObj->matrix4x4 = Matrix4::Invert(matrixObj->matrix4x4);
    return object;
}

static ani_object Matrix4_Combine([[maybe_unused]] ani_env* env, ani_object object, ani_object option)
{
    Matrix4_Obj* matrixOption = GetMatrixObj(env, option);
    if (matrixOption == nullptr) {
        return nullptr;
    }

    Matrix4_Obj* matrixObj = GetMatrixObj(env, object);
    if (matrixObj == nullptr) {
        return nullptr;
    }
    matrixObj->matrix4x4 = matrixOption->matrix4x4 * matrixObj->matrix4x4;
    return object;
}

ani_status BindMatrix(ani_env* env)
{
    static const char* className = "L@ohos/matrix4/matrix4;";
    ani_namespace cls;
    if (ANI_OK != env->FindNamespace(className, &cls)) {
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function { "identity", nullptr, reinterpret_cast<void*>(Matrix4_Identity) },
        ani_native_function { "init", nullptr, reinterpret_cast<void*>(Matrix4_Init) },
    };
    if (ANI_OK != env->Namespace_BindNativeFunctions(cls, methods.data(), methods.size())) {
        return ANI_ERROR;
    };
    return ANI_OK;
}

static ani_object MatrixTransferStatic(ani_env *env, ani_object obj, ani_object input)
{
    if (env == nullptr) {
        LOGE("[ANI] aniEnv is null");
        return nullptr;
    }

    if (input == nullptr) {
        LOGE("[ANI] MatrixTransferStatic is null");
        return nullptr;
    }

    static const char* className = "L@ohos/matrix4/matrix4/Matrix4TransitInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        return nullptr;
    }
    auto matrix = ConvertToMatrixArray(env, static_cast<ani_object>(input));
    auto matrix4Object = new Matrix4_Obj(matrix);
    ani_object matrix4ObjectCur;
    if (ANI_OK != env->Object_New(cls, ctor, &matrix4ObjectCur, reinterpret_cast<ani_long>(matrix4Object))) {
        ani_object nullobj = nullptr;
        return nullobj;
    }
    return matrix4ObjectCur;
}

ani_status BindMatrixTransit(ani_env* env)
{
    static const char* className = "L@ohos/matrix4/matrix4/Matrix4TransitInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function { "copy", nullptr, reinterpret_cast<void*>(Matrix4_Copy) },
        ani_native_function { "scale", nullptr, reinterpret_cast<void*>(Matrix4_Scale) },
        ani_native_function { "rotate", nullptr, reinterpret_cast<void*>(Matrix4_Rotate) },
        ani_native_function { "translate", nullptr, reinterpret_cast<void*>(Matrix4_Translate) },
        ani_native_function { "invert", nullptr, reinterpret_cast<void*>(Matrix4_Invert) },
        ani_native_function { "combine", nullptr, reinterpret_cast<void*>(Matrix4_Combine) },
        ani_native_function { "skew", nullptr, reinterpret_cast<void*>(Matrix4_Skew) },
        ani_native_function { "setPolyToPoly", nullptr, reinterpret_cast<void*>(Matrix4_SetPolyToPoly) },
        ani_native_function { "transformPoint", nullptr, reinterpret_cast<void*>(Matrix4_TransformPoint) },
        ani_native_function { "nativeTransferStatic", nullptr, reinterpret_cast<void*>(MatrixTransferStatic) },
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        return ANI_ERROR;
    };
    return ANI_OK;
}
} // namespace OHOS::Ace::MatrixAni

ANI_EXPORT ani_status ANI_Constructor(ani_vm* vm, uint32_t* result)
{
    ani_env* env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }

    ani_status retBindMatrix = OHOS::Ace::MatrixAni::BindMatrix(env);
    if (retBindMatrix != ANI_OK) {
        return retBindMatrix;
    }

    ani_status retBindMatrixTransit = OHOS::Ace::MatrixAni::BindMatrixTransit(env);
    if (retBindMatrixTransit != ANI_OK) {
        return retBindMatrixTransit;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
