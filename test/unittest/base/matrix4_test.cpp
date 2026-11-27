/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cmath>

#include "gtest/gtest.h"

#define private public
#define protected public
#include "base/geometry/matrix4.h"
#include "base/geometry/point.h"
#undef private
#undef protected


using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace {
namespace {
const double DEFAULT_DOUBLE0 = 0.0;
const double DEFAULT_DOUBLE1 = 1.0;
const double DEFAULT_DOUBLE2 = 2.0;
const double DEFAULT_DOUBLE6 = 6.0;
const double DEFAULT_DOUBLE7 = 7.0;

constexpr double ANGLE_UNIT = 0.017453f;
const int32_t MATRIXS_LENGTH = 16;

const int32_t VALID_ROW0 = 0;
const int32_t VALID_COL0 = 0;
const int32_t VALID_ROW1 = 1;
const int32_t VALID_COL1 = 1;
const int32_t VALID_ROW2 = 2;
const int32_t VALID_COL2 = 2;
const int32_t VALID_ROW3 = 3;
const int32_t VALID_COL3 = 3;
const int32_t VALID_ROW4 = 4;
const int32_t VALID_DIMENSION = 4;

const int32_t INVALID_ROW_NEG = -1;
const int32_t INVALID_COL_NEG = -1;
const int32_t INVALID_ROW_POS = 6;
const int32_t INVALID_COL_POS = 6;

const uint32_t ROW_NUM = 5;
const uint32_t COLUMN_NUM = 5;
} // namespace

class Matrix4Test : public testing::Test {};

/**
 * @tc.name: Matrix4Test001
 * @tc.desc: Test functions beginning with Create of the class Matrix4.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test001, TestSize.Level1)
{
    /**
     * @tc.steps: Test the function CreateIdentity of the class Matrix4.
     */
    Matrix4 matrix4Obj1 = Matrix4::CreateIdentity();
    EXPECT_TRUE(matrix4Obj1.IsIdentityMatrix());
    EXPECT_EQ(matrix4Obj1.Count(), MATRIXS_LENGTH);
    EXPECT_DOUBLE_EQ(matrix4Obj1.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj1.Get(VALID_ROW1, VALID_COL1), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj1.Get(VALID_ROW2, VALID_COL2), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj1.Get(VALID_ROW3, VALID_COL3), DEFAULT_DOUBLE1);

    /**
     * @tc.steps: Test the function CreateTranslate of the class Matrix4.
     */
    Matrix4 matrix4Obj2 = Matrix4::CreateTranslate(DEFAULT_DOUBLE0, DEFAULT_DOUBLE1, DEFAULT_DOUBLE2);
    EXPECT_FALSE(matrix4Obj2.IsIdentityMatrix());
    EXPECT_DOUBLE_EQ(matrix4Obj2.Get(VALID_ROW0, VALID_COL3), DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj2.Get(VALID_ROW1, VALID_COL3), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj2.Get(VALID_ROW2, VALID_COL3), DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test the function CreateScale of the class Matrix4.
     */
    Matrix4 matrix4Obj3 = Matrix4::CreateScale(DEFAULT_DOUBLE0, DEFAULT_DOUBLE1, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW1, VALID_COL1), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW2, VALID_COL2), DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW3, VALID_COL3), DEFAULT_DOUBLE1);

    /**
     * @tc.steps: Test functions CreateRotate and Rotate of the class Matrix4.
     */
    Matrix4 matrix4Obj4 = Matrix4::CreateRotate(DEFAULT_DOUBLE1, DEFAULT_DOUBLE0, DEFAULT_DOUBLE0, DEFAULT_DOUBLE0);
    EXPECT_EQ(matrix4Obj1, matrix4Obj4);
    Matrix4 matrix4Obj5 = Matrix4::CreateRotate(DEFAULT_DOUBLE0, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1);
    Matrix4 matrix4Obj6 = Matrix4::CreateIdentity();
    matrix4Obj6.Rotate(DEFAULT_DOUBLE0, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1);
    EXPECT_EQ(matrix4Obj6, matrix4Obj5);

    /**
     * @tc.steps: Test the function CreateMatrix2D of the class Matrix4.
     */
    Matrix4 matrix4Obj7 = Matrix4::CreateMatrix2D(
        DEFAULT_DOUBLE0, DEFAULT_DOUBLE1, DEFAULT_DOUBLE2, DEFAULT_DOUBLE0, DEFAULT_DOUBLE1, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj7.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj7.Get(VALID_ROW1, VALID_COL0), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj7.Get(VALID_ROW0, VALID_COL1), DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj7.Get(VALID_ROW1, VALID_COL1), DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj7.Get(VALID_ROW0, VALID_COL3), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj7.Get(VALID_ROW1, VALID_COL3), DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test the function CreateSkew of the class Matrix4.
     */
    Matrix4 matrix4Obj8 = Matrix4::CreateSkew(DEFAULT_DOUBLE1, DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj8.Get(VALID_ROW1, VALID_COL0), std::tan(DEFAULT_DOUBLE1 * ANGLE_UNIT));
    EXPECT_DOUBLE_EQ(matrix4Obj8.Get(VALID_ROW0, VALID_COL1), std::tan(DEFAULT_DOUBLE1 * ANGLE_UNIT));

    /**
     * @tc.steps: Test the function CreatePerspective of the class Matrix4.
     */
    Matrix4 matrix4Obj9 = Matrix4::CreatePerspective(DEFAULT_DOUBLE0);
    EXPECT_EQ(matrix4Obj1, matrix4Obj9);
    Matrix4 matrix4Obj10 = Matrix4::CreatePerspective(DEFAULT_DOUBLE2);
    EXPECT_EQ(matrix4Obj10.Get(VALID_ROW3, VALID_COL2), -DEFAULT_DOUBLE1 / DEFAULT_DOUBLE2);
}

/**
 * @tc.name: Matrix4Test002
 * @tc.desc: Test the function SetEntry of the classes Matrix4, Matrix4N and MatrixN4.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test002, TestSize.Level1)
{
    /**
     * @tc.steps: Test the function SetEntry of the class Matrix4.
     */
    Matrix4 matrix4Obj1 = Matrix4::CreateIdentity();
    std::string initStr4Obj1 = matrix4Obj1.ToString();
    matrix4Obj1.SetEntry(INVALID_ROW_NEG, INVALID_COL_NEG, DEFAULT_DOUBLE2);
    EXPECT_EQ(matrix4Obj1.ToString(), initStr4Obj1);
    matrix4Obj1.SetEntry(INVALID_ROW_POS, INVALID_COL_POS, DEFAULT_DOUBLE2);
    EXPECT_EQ(matrix4Obj1.ToString(), initStr4Obj1);
    matrix4Obj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj1.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test the function SetEntry of the class Matrix4N.
     */
    Matrix4N matrix4NObj1(COLUMN_NUM);
    EXPECT_FALSE(matrix4NObj1.SetEntry(INVALID_ROW_NEG, INVALID_COL_NEG, DEFAULT_DOUBLE2));
    EXPECT_FALSE(matrix4NObj1.SetEntry(INVALID_ROW_POS, INVALID_COL_POS, DEFAULT_DOUBLE2));
    matrix4NObj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4NObj1[VALID_ROW0][VALID_COL0], DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test the function SetEntry of the class MatrixN4.
     */
    MatrixN4 matrixN4Obj1(ROW_NUM);
    EXPECT_FALSE(matrixN4Obj1.SetEntry(INVALID_ROW_NEG, INVALID_COL_NEG, DEFAULT_DOUBLE2));
    EXPECT_FALSE(matrixN4Obj1.SetEntry(INVALID_ROW_POS, INVALID_COL_POS, DEFAULT_DOUBLE2));
    matrixN4Obj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrixN4Obj1[VALID_ROW0][VALID_COL0], DEFAULT_DOUBLE2);
}

/**
 * @tc.name: Matrix4Test003
 * @tc.desc: Test the function Transpose of the classes Matrix4, Matrix4N and MatrixN4.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test003, TestSize.Level1)
{
    /**
     * @tc.steps: Test the function Transpose of the class Matrix4.
     */
    Matrix4 matrix4Obj1 = Matrix4::CreateIdentity();
    matrix4Obj1.SetEntry(VALID_ROW3, VALID_COL0, DEFAULT_DOUBLE2);
    Matrix4 matrix4Obj2 = Matrix4::CreateIdentity();
    matrix4Obj2.SetEntry(VALID_ROW0, VALID_COL3, DEFAULT_DOUBLE2);
    matrix4Obj1.Transpose();
    EXPECT_EQ(matrix4Obj1, matrix4Obj2);

    /**
     * @tc.steps: Test the function Transpose of the class Matrix4N.
     */
    Matrix4N matrix4NObj1(COLUMN_NUM);
    matrix4NObj1.SetEntry(VALID_ROW2, VALID_COL0, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4NObj1.Transpose()[VALID_ROW0][VALID_COL2], DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test the function Transpose of the class MatrixN4.
     */
    MatrixN4 matrixN4Obj1(ROW_NUM);
    matrixN4Obj1.SetEntry(VALID_ROW0, VALID_COL2, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrixN4Obj1.Transpose()[VALID_ROW2][VALID_COL0], DEFAULT_DOUBLE2);
}

/**
 * @tc.name: Matrix4Test004
 * @tc.desc: Test the function MapScalars of the classes Matrix4.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test004, TestSize.Level1)
{
    /**
     * @tc.steps: initialize the input parameters of the function MapScalars.
     */
    double srcVec[VALID_DIMENSION] = { DEFAULT_DOUBLE1, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1 };
    double dstVec[VALID_DIMENSION] = { DEFAULT_DOUBLE0 };
    Matrix4 matrix4Obj1 = Matrix4::CreateIdentity();
    matrix4Obj1.SetEntry(VALID_ROW1, VALID_COL1, DEFAULT_DOUBLE2);
    matrix4Obj1.SetEntry(VALID_ROW2, VALID_COL2, DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test with given vectors srcVec and dstVec as the input and output parameters.
     */
    matrix4Obj1.MapScalars(srcVec, dstVec);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW0], DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW1], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW2], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW3], DEFAULT_DOUBLE1);

    /**
     * @tc.steps: Test with the single vector srcVec as the input and output parameters.
     */
    matrix4Obj1.MapScalars(srcVec, srcVec);
    EXPECT_DOUBLE_EQ(srcVec[VALID_ROW0], DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(srcVec[VALID_ROW1], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(srcVec[VALID_ROW2], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(srcVec[VALID_ROW3], DEFAULT_DOUBLE1);
}

/**
 * @tc.name: Matrix4Test005
 * @tc.desc: Test the function MapScalars of the classes Matrix4N.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test005, TestSize.Level1)
{
    /**
     * @tc.steps: initialize the input parameters of the function MapScalars.
     */
    Matrix4N matrix4NObj1(COLUMN_NUM);
    matrix4NObj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE0);
    matrix4NObj1.SetEntry(VALID_ROW1, VALID_COL1, DEFAULT_DOUBLE1);
    matrix4NObj1.SetEntry(VALID_ROW2, VALID_COL2, DEFAULT_DOUBLE2);
    matrix4NObj1.SetEntry(VALID_ROW3, VALID_COL3, DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Given the vector srcVec whose size is invalid, test the
     *            function MapScalars with single parameter.
     */
    std::vector<double> srcVec = { DEFAULT_DOUBLE1, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1 };
    std::vector<double> dstVec = matrix4NObj1.MapScalars(srcVec);
    EXPECT_EQ(dstVec.size(), VALID_DIMENSION);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW0], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW1], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW2], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW3], DEFAULT_DOUBLE0);

    /**
     * @tc.steps: Given the vector srcVec whose size is valid, test the
     *            function MapScalars with single parameter.
     */
    srcVec.push_back(DEFAULT_DOUBLE1);
    dstVec = matrix4NObj1.MapScalars(srcVec);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW0], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW1], DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW2], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW3], DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Given the vector srcVec whose size is invalid, test the
     *            function MapScalars with two parameters.
     */
    srcVec.pop_back();
    dstVec.clear();
    EXPECT_FALSE(matrix4NObj1.MapScalars(srcVec, dstVec));
    EXPECT_TRUE(dstVec.empty());

    /**
     * @tc.steps: Given the vector srcVec whose size is valid, test the
     *            function MapScalars with two parameters.
     */
    srcVec.push_back(DEFAULT_DOUBLE1);
    EXPECT_TRUE(matrix4NObj1.MapScalars(srcVec, dstVec));
    EXPECT_EQ(dstVec.size(), VALID_DIMENSION);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW0], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW1], DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW2], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW3], DEFAULT_DOUBLE2);
}

/**
 * @tc.name: Matrix4Test006
 * @tc.desc: Test the function MapScalars of the classes MatrixN4.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test006, TestSize.Level1)
{
    /**
     * @tc.steps: initialize the input parameters of the function MapScalars.
     */
    MatrixN4 matrixN4Obj1(ROW_NUM);
    matrixN4Obj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE0);
    matrixN4Obj1.SetEntry(VALID_ROW1, VALID_COL1, DEFAULT_DOUBLE1);
    matrixN4Obj1.SetEntry(VALID_ROW2, VALID_COL2, DEFAULT_DOUBLE2);
    matrixN4Obj1.SetEntry(VALID_ROW3, VALID_COL3, DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test the function MapScalars with given the vector srcVec whose size is invalid.
     */
    std::vector<double> srcVec = { DEFAULT_DOUBLE1, DEFAULT_DOUBLE1, DEFAULT_DOUBLE1 };
    std::vector<double> dstVec = matrixN4Obj1.MapScalars(srcVec);
    EXPECT_EQ(dstVec.size(), ROW_NUM);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW0], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW1], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW2], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW3], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW4], DEFAULT_DOUBLE0);

    /**
     * @tc.steps: Test the function MapScalars with given the vector srcVec whose size is valid.
     */
    srcVec.push_back(DEFAULT_DOUBLE1);
    dstVec = matrixN4Obj1.MapScalars(srcVec);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW0], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW1], DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW2], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW3], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(dstVec[VALID_ROW4], DEFAULT_DOUBLE0);
}

/**
 * @tc.name: Matrix4Test007
 * @tc.desc: Test the function operator* of classes Matrix4 and Matrix4N.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test007, TestSize.Level1)
{
    /**
     * @tc.steps: initialize parameters.
     */
    Matrix4 matrix4Obj1 = Matrix4::CreateIdentity();
    matrix4Obj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE1);
    matrix4Obj1.SetEntry(VALID_ROW1, VALID_COL0, DEFAULT_DOUBLE2);
    matrix4Obj1.SetEntry(VALID_ROW3, VALID_COL0, DEFAULT_DOUBLE2);
    matrix4Obj1.SetEntry(VALID_ROW0, VALID_COL1, DEFAULT_DOUBLE0);
    matrix4Obj1.SetEntry(VALID_ROW1, VALID_COL1, DEFAULT_DOUBLE2);
    matrix4Obj1.SetEntry(VALID_ROW3, VALID_COL1, DEFAULT_DOUBLE2);

    /**
     * @tc.steps: Test the function operator* of the class Matrix4 whose parameter is Point.
     */
    Point point(DEFAULT_DOUBLE1, DEFAULT_DOUBLE2);
    Point result = matrix4Obj1 * point;
    EXPECT_EQ(result.GetX(), DEFAULT_DOUBLE7);
    EXPECT_EQ(result.GetY(), DEFAULT_DOUBLE6);

    /**
     * @tc.steps: Test the function operator* of the class Matrix4 whose parameter is Matrix4N.
     */
    Matrix4 matrix4Obj2 = Matrix4::CreateIdentity() * DEFAULT_DOUBLE2;
    Matrix4N matrix4NObj1(COLUMN_NUM);
    matrix4NObj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE1);
    matrix4NObj1.SetEntry(VALID_ROW1, VALID_COL1, DEFAULT_DOUBLE2);
    matrix4NObj1.SetEntry(VALID_ROW2, VALID_COL2, DEFAULT_DOUBLE2);
    matrix4NObj1.SetEntry(VALID_ROW3, VALID_COL3, DEFAULT_DOUBLE0);
    Matrix4N matrix4NObj2 = matrix4Obj2 * matrix4NObj1;
    EXPECT_DOUBLE_EQ(matrix4NObj2[VALID_ROW0][VALID_COL0], DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4NObj2[VALID_ROW1][VALID_COL1], DEFAULT_DOUBLE2 * DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4NObj2[VALID_ROW2][VALID_COL2], DEFAULT_DOUBLE2 * DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4NObj2[VALID_ROW3][VALID_COL3], DEFAULT_DOUBLE0);

    /**
     * @tc.steps: Test the function operator* of the class Matrix4N whose parameter is MatrixN4.
     */
    MatrixN4 matrixN4Obj1(ROW_NUM);
    matrixN4Obj1.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE1);
    matrixN4Obj1.SetEntry(VALID_ROW1, VALID_COL1, DEFAULT_DOUBLE2);
    matrixN4Obj1.SetEntry(VALID_ROW2, VALID_COL2, DEFAULT_DOUBLE2);
    matrixN4Obj1.SetEntry(VALID_ROW3, VALID_COL3, DEFAULT_DOUBLE0);
    Matrix4 matrix4Obj3 = matrix4NObj1 * matrixN4Obj1;
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE1);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW1, VALID_COL1), DEFAULT_DOUBLE2 * DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW2, VALID_COL2), DEFAULT_DOUBLE2 * DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Get(VALID_ROW3, VALID_COL3), DEFAULT_DOUBLE0);

    /**
     * @tc.steps: Test the function operator* of the class Matrix4N with invalid input.
     */
    MatrixN4 matrixN4Obj2(ROW_NUM - 1);
    Matrix4 matrix4Obj4 = matrix4NObj1 * matrixN4Obj2;
    EXPECT_TRUE(matrix4Obj4.IsIdentityMatrix());
}

/**
 * @tc.name: Matrix4Test008
 * @tc.desc: Test other functions of classes Matrix4 and Matrix4N.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test008, TestSize.Level1)
{
    /**
     * @tc.steps: Test the function about scale of the class Matrix4.
     */
    Matrix4 matrix4Obj1 = Matrix4::CreateIdentity();
    matrix4Obj1.SetScale(DEFAULT_DOUBLE0, DEFAULT_DOUBLE1, DEFAULT_DOUBLE2);
    EXPECT_DOUBLE_EQ(matrix4Obj1.GetScaleX(), DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj1.GetScaleY(), DEFAULT_DOUBLE1);

    /**
     * @tc.steps: Test the function operator[].
     */
    EXPECT_DOUBLE_EQ(matrix4Obj1[INVALID_ROW_NEG], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj1[MATRIXS_LENGTH], DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj1[MATRIXS_LENGTH - 1], DEFAULT_DOUBLE1);

    /**
     * @tc.steps: The determinant of identity matrix is 1.
     */
    Matrix4 matrix4Obj2 = Matrix4::CreateIdentity();
    EXPECT_DOUBLE_EQ(matrix4Obj2.Determinant(), DEFAULT_DOUBLE1);

    /**
     * @tc.steps: The inverse matrix of the matrix with determinant 0 returns the identity matrix by default.
     */
    Matrix4 matrix4Obj3 = matrix4Obj2;
    matrix4Obj3.SetEntry(VALID_ROW0, VALID_COL0, DEFAULT_DOUBLE0);
    EXPECT_DOUBLE_EQ(matrix4Obj3.Determinant(), DEFAULT_DOUBLE0);
    EXPECT_EQ(Matrix4::Invert(matrix4Obj3), matrix4Obj2);

    /**
     * @tc.steps: Test the inverse operator of matrix with non-zero determinant.
     */
    Matrix4 matrix4Obj4 = matrix4Obj2 * DEFAULT_DOUBLE2;
    EXPECT_EQ(matrix4Obj4.Get(VALID_ROW0, VALID_COL0), DEFAULT_DOUBLE2);
    Matrix4 matrix4Obj5 = Matrix4::Invert(matrix4Obj4);
    EXPECT_DOUBLE_EQ(matrix4Obj5.Get(VALID_ROW0, VALID_COL0), 1 / DEFAULT_DOUBLE2);

    /**
     * @tc.steps: The product of a given matrix and its inverse matrix is the identity matrix.
     */
    Matrix4 matrix4Obj6 = matrix4Obj4 * matrix4Obj5;
    EXPECT_EQ(matrix4Obj6, matrix4Obj2);
}

/**
 * @tc.name: Matrix4Test009
 * @tc.desc: Test  function QuaternionToMatrix/SetEntry/operator=
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4Test, Matrix4Test009, TestSize.Level1)
{
    /**
     * @tc.steps: LessOrEqual 0.0f or not
     */
    Matrix4 ret = Matrix4::QuaternionToMatrix(0.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_EQ(ret.Get(0, 0) == 1.0f, true);
    Matrix4 ret2 = Matrix4::QuaternionToMatrix(1.0f, 1.0f, 1.0f, 1.0f);
    EXPECT_EQ(ret2.Get(0, 1) == 0.0f, true);

    /**
     * @tc.steps: Matrix4 SetEntry row or col is out of range
     */
    // true
    Matrix4 matrix4Obj = Matrix4::CreateIdentity();
    matrix4Obj.SetEntry(-1, 0, 1.0f);
    // false true
    matrix4Obj.SetEntry(10.0, 0, 1.0f);
    // false false true
    matrix4Obj.SetEntry(1.0, -1, 1.0f);
    // false false false true
    matrix4Obj.SetEntry(1.0, 10.0, 1.0f);

    /**
     * @tc.steps: Matrix4N SetEntry row or col is out of range
     */
    Matrix4N mtrix4NObj = Matrix4N(COLUMN_NUM);
    bool ret3 = mtrix4NObj.SetEntry(-1, 0, 1.0f);
    EXPECT_EQ(ret3, false);
    bool ret4 = mtrix4NObj.SetEntry(10.0, 0, 1.0f);
    EXPECT_EQ(ret4, false);
    bool ret5 = mtrix4NObj.SetEntry(1.0, -1, 1.0f);
    EXPECT_EQ(ret5, false);
    bool ret6 = mtrix4NObj.SetEntry(1.0, COLUMN_NUM, 1.0f);
    EXPECT_EQ(ret6, false);

    /**
     * @tc.steps: MatrixN4 SetEntry row or col is out of range
     */
    MatrixN4 mtrixN4Obj = MatrixN4(ROW_NUM);
    bool ret7 = mtrixN4Obj.SetEntry(-1, 0, 1.0f);
    EXPECT_EQ(ret7, false);
    bool ret8 = mtrixN4Obj.SetEntry(ROW_NUM, 0, 1.0f);
    EXPECT_EQ(ret8, false);
    bool ret9 = mtrixN4Obj.SetEntry(1.0, -1, 1.0f);
    EXPECT_EQ(ret9, false);
    bool ret10 = mtrixN4Obj.SetEntry(1.0, 10.0, 1.0f);
    EXPECT_EQ(ret10, false);
}


class Matrix4GapTest : public testing::Test {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

/**
 * @tc.name: Matrix4GapCreateInvert001
 * @tc.desc: Test CreateInvert returns the adjugate matrix.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4GapTest, Matrix4GapCreateInvert001, TestSize.Level1)
{
    /**
     * @tc.steps: compute adjugate of identity and a diagonal scale matrix.
     * @tc.expected: adj(I) == I; adj(diag(2,3,4,1)) == diag(12,8,6,24).
     */
    auto adjIdentity = Matrix4::CreateInvert(Matrix4::CreateIdentity());
    for (int32_t i = 0; i < 4; ++i) {
        for (int32_t j = 0; j < 4; ++j) {
            EXPECT_DOUBLE_EQ(adjIdentity(i, j), i == j ? 1.0 : 0.0) << "i=" << i << " j=" << j;
        }
    }

    // adj(diag(a,b,c,d)) = diag(bcd, acd, abd, abc)
    Matrix4 scale(2.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    auto adjScale = Matrix4::CreateInvert(scale);
    EXPECT_DOUBLE_EQ(adjScale(0, 0), 12.0);
    EXPECT_DOUBLE_EQ(adjScale(1, 1), 8.0);
    EXPECT_DOUBLE_EQ(adjScale(2, 2), 6.0);
    EXPECT_DOUBLE_EQ(adjScale(3, 3), 24.0);
    EXPECT_DOUBLE_EQ(adjScale(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(adjScale(2, 3), 0.0);
}

/**
 * @tc.name: Matrix4GapInvert001
 * @tc.desc: Test Invert for invertible and singular matrices.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4GapTest, Matrix4GapInvert001, TestSize.Level1)
{
    /**
     * @tc.steps: invert a diagonal scale matrix and a singular matrix.
     * @tc.expected: scale * inverse == identity; singular returns identity.
     */
    Matrix4 scale(2.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    auto inverse = Matrix4::Invert(scale);
    auto product = scale * inverse;
    for (int32_t i = 0; i < 4; ++i) {
        for (int32_t j = 0; j < 4; ++j) {
            EXPECT_NEAR(product(i, j), i == j ? 1.0 : 0.0, 1e-9) << "i=" << i << " j=" << j;
        }
    }

    // singular matrix (zero row) -> identity fallback
    Matrix4 singular(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    auto singularInverse = Matrix4::Invert(singular);
    EXPECT_TRUE(singularInverse == Matrix4::CreateIdentity());
}

/**
 * @tc.name: Matrix4GapTransformPoint001
 * @tc.desc: Test TransformPoint perspective divide branches.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4GapTest, Matrix4GapTransformPoint001, TestSize.Level1)
{
    /**
     * @tc.steps: transform a point with identity, translate, perspective and
     *            degenerate w=0 perspective matrices.
     * @tc.expected: w==1 skips division; w!=1 divides; w==0 skips division.
     */
    // identity: w == 1, no divide
    EXPECT_EQ(Matrix4::CreateIdentity().TransformPoint(Point(3.0, 4.0)), Point(3.0, 4.0));

    // translate: w == 1, no divide
    auto translate = Matrix4::CreateTranslate(1.0, 2.0, 3.0);
    EXPECT_EQ(translate.TransformPoint(Point(1.0, 1.0)), Point(2.0, 3.0));

    // perspective factor: w = 0.5*x + 1 -> divide by w
    auto perspective = Matrix4::CreateFactorPerspective(0.5, 0.0);
    auto transformed = perspective.TransformPoint(Point(2.0, 0.0));
    EXPECT_DOUBLE_EQ(transformed.GetX(), 1.0); // (2 + 0) / (0.5*2 + 1)
    EXPECT_DOUBLE_EQ(transformed.GetY(), 0.0);

    // perspective factor leading to w == 0: skip division
    auto degenerate = Matrix4::CreateFactorPerspective(-0.5, 0.0);
    auto rawPoint = degenerate.TransformPoint(Point(2.0, 0.0));
    EXPECT_DOUBLE_EQ(rawPoint.GetX(), 2.0);
    EXPECT_DOUBLE_EQ(rawPoint.GetY(), 0.0);
}

/**
 * @tc.name: Matrix4GapCreateFactor001
 * @tc.desc: Test CreateFactorSkew and CreateFactorPerspective entries.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4GapTest, Matrix4GapCreateFactor001, TestSize.Level1)
{
    /**
     * @tc.steps: build skew and perspective factor matrices and inspect entries.
     * @tc.expected: skew stores x/y in transposed off-diagonal slots;
     *               perspective stores x/y in the w row slots.
     */
    auto skew = Matrix4::CreateFactorSkew(0.5, 0.25);
    EXPECT_DOUBLE_EQ(skew(1, 0), 0.5);
    EXPECT_DOUBLE_EQ(skew(0, 1), 0.25);
    EXPECT_DOUBLE_EQ(skew(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(skew(3, 3), 1.0);
    EXPECT_DOUBLE_EQ(skew(2, 3), 0.0);

    auto perspective = Matrix4::CreateFactorPerspective(0.5, 0.25);
    EXPECT_DOUBLE_EQ(perspective(0, 3), 0.5);
    EXPECT_DOUBLE_EQ(perspective(1, 3), 0.25);
    EXPECT_DOUBLE_EQ(perspective(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(perspective(3, 3), 1.0);
}

/**
 * @tc.name: Matrix4GapOperatorIndex001
 * @tc.desc: Test operator[] bounds checking.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4GapTest, Matrix4GapOperatorIndex001, TestSize.Level1)
{
    /**
     * @tc.steps: index valid and out-of-range positions on identity.
     * @tc.expected: valid indices return stored values; negative and >= 16
     *               return 0.0.
     */
    auto identity = Matrix4::CreateIdentity();
    EXPECT_DOUBLE_EQ(identity[0], 1.0);
    EXPECT_DOUBLE_EQ(identity[5], 1.0);   // [1][1]
    EXPECT_DOUBLE_EQ(identity[15], 1.0);  // [3][3]
    EXPECT_DOUBLE_EQ(identity[1], 0.0);
    EXPECT_DOUBLE_EQ(identity[-1], 0.0);
    EXPECT_DOUBLE_EQ(identity[16], 0.0);
    EXPECT_DOUBLE_EQ(identity[100], 0.0);
}

/**
 * @tc.name: Matrix4GapSelfAssign001
 * @tc.desc: Test operator= self assignment guard.
 * @tc.type: FUNC
 */
HWTEST_F(Matrix4GapTest, Matrix4GapSelfAssign001, TestSize.Level1)
{
    /**
     * @tc.steps: assign a matrix to itself through a reference.
     * @tc.expected: content unchanged, no crash.
     */
    auto translate = Matrix4::CreateTranslate(1.0, 2.0, 3.0);
    Matrix4& self = translate;
    translate = self;
    EXPECT_DOUBLE_EQ(translate(3, 0), 1.0);
    EXPECT_DOUBLE_EQ(translate(3, 1), 2.0);
    EXPECT_DOUBLE_EQ(translate(3, 2), 3.0);
    EXPECT_TRUE(translate == Matrix4::CreateTranslate(1.0, 2.0, 3.0));
}
} // namespace OHOS::Ace
