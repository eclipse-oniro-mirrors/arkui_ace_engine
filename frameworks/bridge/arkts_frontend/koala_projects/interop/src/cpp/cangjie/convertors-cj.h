/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cmath>
#include <assert.h>
#include <memory>
#include <vector>
#include <string>

#include "koala-types.h"

#define KOALA_INTEROP_EXPORT extern "C"

#define  MAKE_CJ_EXPORT(name, type, flag) \
    __attribute__((constructor)) \
    static void __init_ets_##name() { \
        CJExports::getInstance()->addImpl("_"#name, type, reinterpret_cast<void *>(Ark_##name), flag); \
    }

class CJExports {
    std::vector<std::tuple<std::string, std::string, void*>> implementations;

public:
    static CJExports* getInstance();

    void addImpl(const char* name, const char* type, void* impl);
    const std::vector<std::tuple<std::string, std::string, void*>>& getImpls() {
        return implementations;
    }
};

template<class T>
struct InteropTypeConverter {
    using InteropType = T;
    static inline T convertFrom(InteropType value) { return value; }
    static inline InteropType convertTo(T value) { return value; }
};

template <typename T>
inline T getArgument(typename InteropTypeConverter<T>::InteropType arg) {
    return InteropTypeConverter<T>::convertFrom(arg);
}

template <typename T>
inline typename InteropTypeConverter<T>::InteropType makeResult(T value) {
    return InteropTypeConverter<T>::convertTo(value);
}

template<>
struct InteropTypeConverter<KInteropNumber> {
    using InteropType = KDouble;
    static inline KInteropNumber convertFrom(InteropType value) {
        return KInteropNumber::fromDouble(value);
    }
    static inline InteropType convertTo(KInteropNumber value) {
        return value.asDouble();
    }
};

template<>
struct InteropTypeConverter<KStringPtr> {
    using InteropType = char*;
    static KStringPtr convertFrom(InteropType value) {
        return KStringPtr(value);
    }
    static InteropType convertTo(const KStringPtr& value) {
        return value.data();
    }
};

// TODO: Rewrite all others to typed convertors.

#define KOALA_INTEROP_0(name, Ret)                                         \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name() {                                          \
    KOALA_MAYBE_LOG(name)                                                  \
    return makeResult<Ret>(impl_##name());                                 \
}
// MAKE_CJ_EXPORT(name, #Ret, 0)

#define KOALA_INTEROP_1(name, Ret, P0)                                     \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(InteropTypeConverter<P0>::InteropType _p0) { \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    return makeResult<Ret>(impl_##name(p0));                               \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0, 0)

#define KOALA_INTEROP_2(name, Ret, P0, P1)                                 \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    return makeResult<Ret>(impl_##name(p0, p1));                           \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1, 0)

#define KOALA_INTEROP_3(name, Ret, P0, P1, P2)                             \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2));                       \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2, 0)

#define KOALA_INTEROP_4(name, Ret, P0, P1, P2, P3)                         \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3));                   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3, 0)

#define KOALA_INTEROP_5(name, Ret, P0, P1, P2, P3, P4)                     \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4));               \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4, 0)

#define KOALA_INTEROP_6(name, Ret, P0, P1, P2, P3, P4, P5)                 \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5));           \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5, 0)

#define KOALA_INTEROP_7(name, Ret, P0, P1, P2, P3, P4, P5, P6)             \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6));       \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6, 0)

#define KOALA_INTEROP_8(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7)         \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6,                             \
    InteropTypeConverter<P7>::InteropType _p7                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6, p7));   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6 "|" #P7, 0)

#define KOALA_INTEROP_9(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8)     \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6,                             \
    InteropTypeConverter<P7>::InteropType _p7,                             \
    InteropTypeConverter<P8>::InteropType _p8                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8));   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6 "|" #P7 "|" #P8, 0)

#define KOALA_INTEROP_10(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6,                             \
    InteropTypeConverter<P7>::InteropType _p7,                             \
    InteropTypeConverter<P8>::InteropType _p8,                             \
    InteropTypeConverter<P9>::InteropType _p9                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9));   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6 "|" #P7 "|" #P8 "|" #P9, 0)

#define KOALA_INTEROP_11(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6,                             \
    InteropTypeConverter<P7>::InteropType _p7,                             \
    InteropTypeConverter<P8>::InteropType _p8,                             \
    InteropTypeConverter<P9>::InteropType _p9,                             \
    InteropTypeConverter<P10>::InteropType _p10                           \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6 "|" #P7 "|" #P8 "|" #P9 "|" #P10, 0)

#define KOALA_INTEROP_12(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6,                             \
    InteropTypeConverter<P7>::InteropType _p7,                             \
    InteropTypeConverter<P8>::InteropType _p8,                             \
    InteropTypeConverter<P9>::InteropType _p9,                             \
    InteropTypeConverter<P10>::InteropType _p10,                           \
    InteropTypeConverter<P11>::InteropType _p11                            \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    P11 p11 = getArgument<P11>(_p11);                                      \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11));   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6 "|" #P7 "|" #P8 "|" #P9 "|" #P10 "|" #P11, 0)

#define KOALA_INTEROP_13(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6,                             \
    InteropTypeConverter<P7>::InteropType _p7,                             \
    InteropTypeConverter<P8>::InteropType _p8,                             \
    InteropTypeConverter<P9>::InteropType _p9,                             \
    InteropTypeConverter<P10>::InteropType _p10,                           \
    InteropTypeConverter<P11>::InteropType _p11,                           \
    InteropTypeConverter<P12>::InteropType _p12                          \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    P11 p11 = getArgument<P11>(_p11);                                      \
    P12 p12 = getArgument<P12>(_p12);                                      \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12));   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6 "|" #P7 "|" #P8 "|" #P9 "|" #P10 "|" #P11 "|" #P12, 0)

#define KOALA_INTEROP_14(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(                                             \
    InteropTypeConverter<P0>::InteropType _p0,                             \
    InteropTypeConverter<P1>::InteropType _p1,                             \
    InteropTypeConverter<P2>::InteropType _p2,                             \
    InteropTypeConverter<P3>::InteropType _p3,                             \
    InteropTypeConverter<P4>::InteropType _p4,                             \
    InteropTypeConverter<P5>::InteropType _p5,                             \
    InteropTypeConverter<P6>::InteropType _p6,                             \
    InteropTypeConverter<P7>::InteropType _p7,                             \
    InteropTypeConverter<P8>::InteropType _p8,                             \
    InteropTypeConverter<P9>::InteropType _p9,                             \
    InteropTypeConverter<P10>::InteropType _p10,                           \
    InteropTypeConverter<P11>::InteropType _p11,                           \
    InteropTypeConverter<P12>::InteropType _p12,                           \
    InteropTypeConverter<P13>::InteropType _p13                           \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    P11 p11 = getArgument<P11>(_p11);                                      \
    P12 p12 = getArgument<P12>(_p12);                                      \
    P13 p13 = getArgument<P13>(_p13);                                      \
    return makeResult<Ret>(impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13));   \
}
// MAKE_CJ_EXPORT(name, #Ret "|" #P0 "|" #P1 "|" #P2 "|" #P3 "|" #P4 "|" #P5 "|" #P6 "|" #P7 "|" #P8 "|" #P9 "|" #P10 "|" #P11 "|" #P12 "|" #P13, 0)

#define KOALA_INTEROP_V0(name)                                             \
KOALA_INTEROP_EXPORT void name() {                                         \
    KOALA_MAYBE_LOG(name)                                                  \
    impl_##name();                                                                \
    return;                                                                \
}

#define KOALA_INTEROP_V1(name, P0)                                         \
KOALA_INTEROP_EXPORT void name(typename InteropTypeConverter<P0>::InteropType _p0) { \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    impl_##name(p0);                                                              \
    return;                                                                \
}

#define KOALA_INTEROP_V2(name, P0, P1)                                     \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    impl_##name(p0, p1);                                                          \
    return;                                                                \
}

#define KOALA_INTEROP_V3(name, P0, P1, P2)                                 \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    impl_##name(p0, p1, p2);                                                      \
    return;                                                                \
}

#define KOALA_INTEROP_V4(name, P0, P1, P2, P3)                             \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    impl_##name(p0, p1, p2, p3);                                                  \
    return;                                                                \
}

#define KOALA_INTEROP_V5(name, P0, P1, P2, P3, P4)                         \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3,                             \
    typename InteropTypeConverter<P4>::InteropType _p4                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    impl_##name(p0, p1, p2, p3, p4);                                              \
    return;                                                                \
}

#define KOALA_INTEROP_V6(name, P0, P1, P2, P3, P4, P5)                     \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                           \
    typename InteropTypeConverter<P1>::InteropType _p1,                            \
    typename InteropTypeConverter<P2>::InteropType _p2,                          \
    typename InteropTypeConverter<P3>::InteropType _p3,                           \
    typename InteropTypeConverter<P4>::InteropType _p4,                         \
    typename InteropTypeConverter<P5>::InteropType _p5                            \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    impl_##name(p0, p1, p2, p3, p4, p5);                                          \
    return;                                                                \
}

#define KOALA_INTEROP_V7(name, P0, P1, P2, P3, P4, P5, P6)                 \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                          \
    typename InteropTypeConverter<P1>::InteropType _p1,                            \
    typename InteropTypeConverter<P2>::InteropType _p2,                             \
    typename InteropTypeConverter<P3>::InteropType _p3,                           \
    typename InteropTypeConverter<P4>::InteropType _p4,                        \
    typename InteropTypeConverter<P5>::InteropType _p5,                              \
    typename InteropTypeConverter<P6>::InteropType _p6                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    impl_##name(p0, p1, p2, p3, p4, p5, p6);                                      \
    return;                                                                \
}

#define KOALA_INTEROP_V8(name, P0, P1, P2, P3, P4, P5, P6, P7)             \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                             \
    typename InteropTypeConverter<P2>::InteropType _p2,                            \
    typename InteropTypeConverter<P3>::InteropType _p3,                             \
    typename InteropTypeConverter<P4>::InteropType _p4,                            \
    typename InteropTypeConverter<P5>::InteropType _p5,                           \
    typename InteropTypeConverter<P6>::InteropType _p6,                            \
    typename InteropTypeConverter<P7>::InteropType _p7                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    impl_##name(p0, p1, p2, p3, p4, p5, p6, p7);                                  \
    return;                                                                \
}

#define KOALA_INTEROP_V9(name, P0, P1, P2, P3, P4, P5, P6, P7, P8)         \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                             \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3,                              \
    typename InteropTypeConverter<P4>::InteropType _p4,                              \
    typename InteropTypeConverter<P5>::InteropType _p5,                              \
    typename InteropTypeConverter<P6>::InteropType _p6,                              \
    typename InteropTypeConverter<P7>::InteropType _p7,                              \
    typename InteropTypeConverter<P8>::InteropType _p8                              \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8);                              \
    return;                                                                \
}

#define KOALA_INTEROP_V10(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)    \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3,                              \
    typename InteropTypeConverter<P4>::InteropType _p4,                              \
    typename InteropTypeConverter<P5>::InteropType _p5,                              \
    typename InteropTypeConverter<P6>::InteropType _p6,                              \
    typename InteropTypeConverter<P7>::InteropType _p7,                              \
    typename InteropTypeConverter<P8>::InteropType _p8,                              \
    typename InteropTypeConverter<P9>::InteropType _p9                             \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);                          \
    return;                                                                \
}

#define KOALA_INTEROP_V11(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3,                              \
    typename InteropTypeConverter<P4>::InteropType _p4,                              \
    typename InteropTypeConverter<P5>::InteropType _p5,                              \
    typename InteropTypeConverter<P6>::InteropType _p6,                              \
    typename InteropTypeConverter<P7>::InteropType _p7,                              \
    typename InteropTypeConverter<P8>::InteropType _p8,                              \
    typename InteropTypeConverter<P9>::InteropType _p9,                              \
    typename InteropTypeConverter<P10>::InteropType _p10                            \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);                     \
    return;                                                                \
}

#define KOALA_INTEROP_V12(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3,                              \
    typename InteropTypeConverter<P4>::InteropType _p4,                              \
    typename InteropTypeConverter<P5>::InteropType _p5,                              \
    typename InteropTypeConverter<P6>::InteropType _p6,                              \
    typename InteropTypeConverter<P7>::InteropType _p7,                              \
    typename InteropTypeConverter<P8>::InteropType _p8,                              \
    typename InteropTypeConverter<P9>::InteropType _p9,                              \
    typename InteropTypeConverter<P10>::InteropType _p10,                            \
    typename InteropTypeConverter<P11>::InteropType _p11                            \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    P11 p11 = getArgument<P11>(_p11);                                      \
    impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);                \
    return;                                                                \
}

#define KOALA_INTEROP_V13(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3,                              \
    typename InteropTypeConverter<P4>::InteropType _p4,                              \
    typename InteropTypeConverter<P5>::InteropType _p5,                              \
    typename InteropTypeConverter<P6>::InteropType _p6,                              \
    typename InteropTypeConverter<P7>::InteropType _p7,                              \
    typename InteropTypeConverter<P8>::InteropType _p8,                              \
    typename InteropTypeConverter<P9>::InteropType _p9,                              \
    typename InteropTypeConverter<P10>::InteropType _p10,                            \
    typename InteropTypeConverter<P11>::InteropType _p11,                            \
    typename InteropTypeConverter<P12>::InteropType _p12                            \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    P11 p11 = getArgument<P11>(_p11);                                      \
    P12 p12 = getArgument<P12>(_p12);                                      \
    impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);           \
    return;                                                                \
}

#define KOALA_INTEROP_V14(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) \
KOALA_INTEROP_EXPORT void name(                                            \
    typename InteropTypeConverter<P0>::InteropType _p0,                              \
    typename InteropTypeConverter<P1>::InteropType _p1,                              \
    typename InteropTypeConverter<P2>::InteropType _p2,                              \
    typename InteropTypeConverter<P3>::InteropType _p3,                              \
    typename InteropTypeConverter<P4>::InteropType _p4,                              \
    typename InteropTypeConverter<P5>::InteropType _p5,                              \
    typename InteropTypeConverter<P6>::InteropType _p6,                              \
    typename InteropTypeConverter<P7>::InteropType _p7,                              \
    typename InteropTypeConverter<P8>::InteropType _p8,                              \
    typename InteropTypeConverter<P9>::InteropType _p9,                              \
    typename InteropTypeConverter<P10>::InteropType _p10,                            \
    typename InteropTypeConverter<P11>::InteropType _p11,                            \
    typename InteropTypeConverter<P12>::InteropType _p12,                            \
    typename InteropTypeConverter<P13>::InteropType _p13                            \
) {                                                                        \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    P5 p5 = getArgument<P5>(_p5);                                          \
    P6 p6 = getArgument<P6>(_p6);                                          \
    P7 p7 = getArgument<P7>(_p7);                                          \
    P8 p8 = getArgument<P8>(_p8);                                          \
    P9 p9 = getArgument<P9>(_p9);                                          \
    P10 p10 = getArgument<P10>(_p10);                                      \
    P11 p11 = getArgument<P11>(_p11);                                      \
    P12 p12 = getArgument<P12>(_p12);                                      \
    P13 p13 = getArgument<P13>(_p13);                                      \
    impl_##name(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);      \
    return;                                                                \
}

#define KOALA_INTEROP_CTX_0(name, Ret)                                     \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name() {                                          \
    KOALA_MAYBE_LOG(name)                                                  \
    KVMContext ctx = (KVMContext)0;                                  \
    return makeResult<Ret>(impl_##name(ctx));                        \
}

#define KOALA_INTEROP_CTX_1(name, Ret, P0)                                 \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(InteropTypeConverter<P0>::InteropType _p0) { \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    KVMContext ctx = (KVMContext)0;                                        \
    return makeResult<Ret>(impl_##name(ctx, p0));                          \
}

#define KOALA_INTEROP_CTX_2(name, Ret, P0, P1)                             \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(InteropTypeConverter<P0>::InteropType _p0,   \
                            InteropTypeConverter<P1>::InteropType _p1) {   \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    KVMContext ctx = (KVMContext)0;                                        \
    return makeResult<Ret>(impl_##name(ctx, p0, p1));                      \
}


#define KOALA_INTEROP_CTX_3(name, Ret, P0, P1, P2)                         \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(          \
                             InteropTypeConverter<P0>::InteropType _p0,    \
                            InteropTypeConverter<P1>::InteropType _p1,     \
                            InteropTypeConverter<P2>::InteropType _p2) {   \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    KVMContext ctx = (KVMContext)0;                                        \
    return makeResult<Ret>(impl_##name(ctx, p0, p1, p2));                  \
}

#define KOALA_INTEROP_CTX_4(name, Ret, P0, P1, P2, P3)                     \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(          \
                            InteropTypeConverter<P0>::InteropType _p0,     \
                            InteropTypeConverter<P1>::InteropType _p1,     \
                            InteropTypeConverter<P2>::InteropType _p2,     \
                            InteropTypeConverter<P3>::InteropType _p3) {   \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    KVMContext ctx = (KVMContext)0;                                        \
    return makeResult<Ret>(impl_##name(ctx, p0, p1, p2, p3));              \
}

#define KOALA_INTEROP_CTX_5(name, Ret, P0, P1, P2, P3, P4)                    \
KOALA_INTEROP_EXPORT InteropTypeConverter<Ret>::InteropType name(             \
                            InteropTypeConverter<P0>::InteropType _p0,        \
                            InteropTypeConverter<P1>::InteropType _p1,        \
                            InteropTypeConverter<P2>::InteropType _p2,        \
                            InteropTypeConverter<P3>::InteropType _p3,        \
                            InteropTypeConverter<P4>::InteropType _p4) {      \
    KOALA_MAYBE_LOG(name)                                                     \
    P0 p0 = getArgument<P0>(_p0);                                             \
    P1 p1 = getArgument<P1>(_p1);                                             \
    P2 p2 = getArgument<P2>(_p2);                                             \
    P3 p3 = getArgument<P3>(_p3);                                             \
    P4 p4 = getArgument<P4>(_p4);                                             \
    KVMContext ctx = (KVMContext)0;                                           \
    return makeResult<Ret>(impl_##name(ctx, p0, p1, p2, p3, p4));             \
}

#define KOALA_INTEROP_CALL_INT(venv, id, length, args)                           \
{                                                                                \
  int32_t rv = 0;                                                                \
  return rv;                                                                     \
}
#define KOALA_INTEROP_CALL_VOID(venv, id, length, args)                            \
{                                                                                  \
}


#define KOALA_INTEROP_CALL_VOID_INTS32(venv, id, argc, args) KOALA_INTEROP_CALL_VOID(venv, id, (argc) * sizeof(int32_t), args)
#define KOALA_INTEROP_CALL_INT_INTS32(venv, id, argc, args) KOALA_INTEROP_CALL_INT(venv, id, (argc) * sizeof(int32_t), args)


#define KOALA_INTEROP_CTX_V1(name, P0)                                     \
KOALA_INTEROP_EXPORT void name(InteropTypeConverter<P0>::InteropType _p0) {\
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    impl_##name(nullptr, p0);                                              \
}

#define KOALA_INTEROP_CTX_V2(name, P0, P1)                                 \
KOALA_INTEROP_EXPORT void name(InteropTypeConverter<P0>::InteropType _p0,  \
                            InteropTypeConverter<P1>::InteropType _p1) {   \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    impl_##name(nullptr, p0, p1);                                          \
}

#define KOALA_INTEROP_CTX_V3(name, P0, P1, P2)                             \
KOALA_INTEROP_EXPORT void name(InteropTypeConverter<P0>::InteropType _p0,  \
                            InteropTypeConverter<P1>::InteropType _p1,     \
                            InteropTypeConverter<P2>::InteropType _p2) {   \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    impl_##name(nullptr, p0, p1, p2);                                      \
}

#define KOALA_INTEROP_CTX_V4(name, P0, P1, P2, P3)                         \
KOALA_INTEROP_EXPORT void name(InteropTypeConverter<P0>::InteropType _p0,  \
                            InteropTypeConverter<P1>::InteropType _p1,     \
                            InteropTypeConverter<P2>::InteropType _p2,     \
                            InteropTypeConverter<P3>::InteropType _p3) {   \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    impl_##name(nullptr, p0, p1, p2, p3);                                  \
}

#define KOALA_INTEROP_CTX_V5(name, P0, P1, P2, P3, P4)                     \
KOALA_INTEROP_EXPORT void name(InteropTypeConverter<P0>::InteropType _p0,  \
                            InteropTypeConverter<P1>::InteropType _p1,     \
                            InteropTypeConverter<P2>::InteropType _p2,     \
                            InteropTypeConverter<P3>::InteropType _p3,     \
                            InteropTypeConverter<P4>::InteropType _p4) {   \
    KOALA_MAYBE_LOG(name)                                                  \
    P0 p0 = getArgument<P0>(_p0);                                          \
    P1 p1 = getArgument<P1>(_p1);                                          \
    P2 p2 = getArgument<P2>(_p2);                                          \
    P3 p3 = getArgument<P3>(_p3);                                          \
    P4 p4 = getArgument<P4>(_p4);                                          \
    impl_##name(nullptr, p0, p1, p2, p3, p4);                                  \
}

  #define KOALA_INTEROP_DIRECT_0(name, Ret) \
  KOALA_INTEROP_0(name, Ret)
#define KOALA_INTEROP_DIRECT_1(name, Ret, P0) \
  KOALA_INTEROP_1(name, Ret, P0)
#define KOALA_INTEROP_DIRECT_2(name, Ret, P0, P1) \
  KOALA_INTEROP_2(name, Ret, P0, P1)
#define KOALA_INTEROP_DIRECT_3(name, Ret, P0, P1, P2) \
  KOALA_INTEROP_3(name, Ret, P0, P1, P2)
#define KOALA_INTEROP_DIRECT_4(name, Ret, P0, P1, P2, P3) \
  KOALA_INTEROP_4(name, Ret, P0, P1, P2, P3)
#define KOALA_INTEROP_DIRECT_5(name, Ret, P0, P1, P2, P3, P4) \
  KOALA_INTEROP_5(name, Ret, P0, P1, P2, P3, P4)
#define KOALA_INTEROP_DIRECT_6(name, Ret, P0, P1, P2, P3, P4, P5) \
  KOALA_INTEROP_6(name, Ret, P0, P1, P2, P3, P4, P5)
#define KOALA_INTEROP_DIRECT_7(name, Ret, P0, P1, P2, P3, P4, P5, P6) \
  KOALA_INTEROP_7(name, Ret, P0, P1, P2, P3, P4, P5, P6)
#define KOALA_INTEROP_DIRECT_8(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7) \
  KOALA_INTEROP_8(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7)
#define KOALA_INTEROP_DIRECT_9(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8) \
  KOALA_INTEROP_9(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8)
#define KOALA_INTEROP_DIRECT_10(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) \
  KOALA_INTEROP_10(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)
#define KOALA_INTEROP_DIRECT_11(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) \
  KOALA_INTEROP_11(name, Ret, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)
#define KOALA_INTEROP_DIRECT_V0(name) \
  KOALA_INTEROP_V0(name)
#define KOALA_INTEROP_DIRECT_V1(name, P0) \
  KOALA_INTEROP_V1(name, P0)
#define KOALA_INTEROP_DIRECT_V2(name, P0, P1) \
  KOALA_INTEROP_V2(name, P0, P1)
#define KOALA_INTEROP_DIRECT_V3(name, P0, P1, P2) \
  KOALA_INTEROP_V3(name, P0, P1, P2)
#define KOALA_INTEROP_DIRECT_V4(name, P0, P1, P2, P3) \
  KOALA_INTEROP_V4(name, P0, P1, P2, P3)
#define KOALA_INTEROP_DIRECT_V5(name, P0, P1, P2, P3, P4) \
  KOALA_INTEROP_V5(name, P0, P1, P2, P3, P4)
#define KOALA_INTEROP_DIRECT_V6(name, P0, P1, P2, P3, P4, P5) \
  KOALA_INTEROP_V6(name, P0, P1, P2, P3, P4, P5)
#define KOALA_INTEROP_DIRECT_V7(name, P0, P1, P2, P3, P4, P5, P6) \
  KOALA_INTEROP_V7(name, P0, P1, P2, P3, P4, P5, P6)
#define KOALA_INTEROP_DIRECT_V8(name, P0, P1, P2, P3, P4, P5, P6, P7) \
  KOALA_INTEROP_V8(name, P0, P1, P2, P3, P4, P5, P6, P7)
#define KOALA_INTEROP_DIRECT_V9(name, P0, P1, P2, P3, P4, P5, P6, P7, P8) \
  KOALA_INTEROP_V9(name, P0, P1, P2, P3, P4, P5, P6, P7, P8)
#define KOALA_INTEROP_DIRECT_V10(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9) \
  KOALA_INTEROP_V10(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9)
#define KOALA_INTEROP_DIRECT_V11(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) \
  KOALA_INTEROP_V11(name, P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)

#define KOALA_INTEROP_THROW(vmContext, object, ...) \
   do { \
     /* TODO: implement*/  assert(false); \
     return __VA_ARGS__; \
   } while (0)

#define KOALA_INTEROP_THROW_STRING(vmContext, message, ...) \
   do { \
     /* TODO: implement*/  assert(false); \
     return __VA_ARGS__; \
   } while (0)
