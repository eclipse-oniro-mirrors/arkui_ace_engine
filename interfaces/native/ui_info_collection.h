/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/**
 * @addtogroup ArkUI_NativeModule
 * @{
 *
 * @brief Provides page text query capabilities for ArkUI on the native side.
 *
 * @since 26.2.0
 */

/**
 * @file ui_info_collection.h
 * @brief Provides synchronous access to ArkUI page information.
 * @include <arkui/ui_info_collection.h>
 * @library libace_ndk.z.so
 * @syscap SystemCapability.ArkUI.ArkUI.Full
 * @since 26.2.0
 */
#ifndef ARKUI_NATIVE_UI_INFO_COLLECTION_H
#define ARKUI_NATIVE_UI_INFO_COLLECTION_H

#include "native_type.h"
#include "ui_json_wrapper.h"
#include <stddef.h>

typedef struct OH_PixelmapNative OH_PixelmapNative;

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque configuration; the caller owns and destroys the request. */
typedef struct OH_ArkUI_NativeModule_UIAgentTreeRequest OH_ArkUI_NativeModule_UIAgentTreeRequest;

typedef enum {
    OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_FULL = 0,
    OH_ARKUI_NATIVE_MODULE_UI_AGENT_TREE_VISIBLE = 1,
} OH_ArkUI_NativeModule_UIAgentTreeType;

typedef enum {
    OH_ARKUI_NATIVE_MODULE_UI_JSON_COMPACT = 0,
    OH_ARKUI_NATIVE_MODULE_UI_JSON_PRETTY = 1,
} OH_ArkUI_NativeModule_UIJsonFormat;

/** Accepted requests complete exactly once on UI. A successful json is owned by the caller. */
typedef void (*OH_ArkUI_NativeModule_UIAgentJsonCallback)(ArkUI_ContextHandle context, uint64_t requestId,
    ArkUI_ErrorCode errorCode, OH_ArkUI_NativeModule_UIJsonWrapper* json, void* userData);

ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestCreate(
    OH_ArkUI_NativeModule_UIAgentTreeType treeType, OH_ArkUI_NativeModule_UIAgentTreeRequest** request);
void OH_ArkUI_NativeModule_UIAgentTreeRequestDestroy(OH_ArkUI_NativeModule_UIAgentTreeRequest* request);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetFilterPureLayoutNodes(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetInteractionInfo(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetAccessibilityInfo(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetCollectVisualProperties(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentTreeRequestSetCollectFunctionalProperties(
    OH_ArkUI_NativeModule_UIAgentTreeRequest* request, bool enabled);

/** UI thread only. Failure clears a valid output slot; no partial tree is returned. */
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentGetTreeJson(ArkUI_ContextHandle context,
    const OH_ArkUI_NativeModule_UIAgentTreeRequest* request, OH_ArkUI_NativeModule_UIJsonFormat format,
    OH_ArkUI_NativeModule_UIJsonWrapper** json);
/** Copies configuration before returning. userData is borrowed through callback completion. */
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentGetTreeJsonAsync(ArkUI_ContextHandle context,
    const OH_ArkUI_NativeModule_UIAgentTreeRequest* request, OH_ArkUI_NativeModule_UIJsonFormat format,
    OH_ArkUI_NativeModule_UIAgentJsonCallback callback, void* userData, uint64_t* requestId);

/**
 * @brief Collects texts from the current ArkUI page synchronously as JSON.
 * The returned JSON contains a "schemaVersion" field fixed at 1 and a "texts" array, for example:
 * @code{.json}
 * {"schemaVersion":1,"texts":[
 *     {"id":101,"rect":[0,0,120,32],"content":"hello"},
 *     {"id":102,"rect":[0,40,160,32],"content":"world"}
 * ]}
 * @endcode
 * Each item contains an integer "id" identifying the text carrier in the
 * snapshot, a numeric "rect" array [x, y, w, h], and a string "content".
 * All four rectangle values are in px. x and y use the same window
 * coordinates as the text carrier's "windowOffset" in hidumper -default,
 * relative to the top-left of the target window, with x increasing rightward
 * and y increasing downward. w and h are the carrier's layout width and
 * height, matching "FrameRect" in the same dump. Positioning, scaling and
 * rotation follow the existing windowOffset behavior; the rectangle is not
 * recomputed as a transformed bounding box or clipped to visible regions.
 * Values may be fractional; x and y may be negative. All values are finite,
 * and w and h are non-negative.
 * @details Calling outside the UI thread of uiContext terminates the process.
 * For a nonzero return, error information is recorded through the existing
 * error-message facility. When that facility is available and recording and
 * formatting succeed, immediately read OH_ArkUI_NativeModule_GetErrorMessage
 * on the same thread for the code, query name, and fixed failure reason.
 * Its existing empty result when unavailable and allocation behavior apply;
 * this query adds no low-memory diagnostic guarantee or fallback.
 * Text is collected from eligible ArkUI text components in the UI instance
 * specified by uiContext. Web content is excluded. Eligible content during
 * page transitions and in overlays may be included. Results are not clipped
 * to the viewport or filtered by pixel occlusion.
 * Collection depends on the component's available text, active state,
 * active and visible ancestors, and positive transformed width and height.
 * This function does not flush layout, wait for drawing, instantiate lazy
 * content, or change the page state. It requires no prior collection call.
 * On success, the immutable wrapper uses schema version 1 and contains a
 * JSON object with a required "texts" array. Each eligible text component
 * produces one item in ascending node ID order. Text uses the component's
 * available source content and text representation, including Span/Symbol
 * content. Input fields and rich editors provide eligible placeholder text;
 * their entered body text is not collected.
 * Independently eligible child text components are evaluated separately.
 * Different components have distinct IDs within a snapshot, even if their
 * text is identical. With no eligible text, the result is {"schemaVersion":1,"texts":[]}.
 * A missing page alone is not an error.
 * Use OH_ArkUI_NativeModule_UIJsonWrapperGetData and
 * OH_ArkUI_NativeModule_UIJsonWrapperGetSize to read the JSON. The size is the
 * UTF-8 byte length of the entire JSON payload,
 * excluding its terminating null character. Release the result with
 * OH_ArkUI_NativeModule_UIJsonWrapperDestroy.
 * @param uiContext A valid framework-provided context of the calling app.
 * @param pageText Writable output slot, initialized to NULL by the caller.
 * A valid slot is set to NULL on failure; an empty page still returns a
 * wrapper.
 * @return ARKUI_ERROR_CODE_NO_ERROR on success.
 * ARKUI_ERROR_CODE_PARAM_INVALID if pageText is NULL.
 * ARKUI_ERROR_CODE_UI_CONTEXT_INVALID if uiContext is NULL or no longer
 * valid.
 * ARKUI_ERROR_CODE_CAPI_INIT_ERROR if the native implementation is
 * unavailable.
 * ARKUI_ERROR_CODE_INTERNAL_ERROR if the JSON payload
 * cannot be represented by uint32_t size, or collection/serialization fails.
 * @release ui_json_wrapper/OH_ArkUI_NativeModule_UIJsonWrapperDestroy {pageText}
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIAgentGetPageText(
    ArkUI_ContextHandle uiContext, OH_ArkUI_NativeModule_UIJsonWrapper** pageText);

/**
 * @brief Defines an opaque collection of image results for ArkUI nodes.
 *
 * The caller takes ownership of each non-<b>null</b> collection received by an
 * <b>OH_ArkUI_NativeModule_ImageCollectionCallback</b> and must release the collection by calling
 * <b>OH_ArkUI_NativeModule_ImageCollectionDestroy</b>.
 *
 * The caller must serialize calls to the collection APIs for the same
 * collection.
 *
 * @since 26.2.0
 */
typedef struct OH_ArkUI_NativeModule_ImageCollection OH_ArkUI_NativeModule_ImageCollection;

/**
 * @brief Defines the callback used to return the result of collecting images of ArkUI nodes.
 *
 * The framework invokes this callback exactly once on the UI thread for each accepted request. Every non-<b>null</b>
 * <b>collection</b> is transferred to the caller. A collection is also returned for partial item failure.
 * The caller must release each non-<b>null</b> collection by calling
 * <b>OH_ArkUI_NativeModule_ImageCollectionDestroy</b>.
 *
 * @param errorCode [in] Indicates the overall request result.
 * @param collection [in] Indicates the result collection.
 *     The value is non-<b>null</b> when item results are available, including partial failure, and <b>null</b> when the
 *     request cannot produce a collection.
 * @param userData [in] Indicates the caller-defined context pointer. The value can be <b>null</b>.
 * @release OH_ArkUI_NativeModule_ImageCollectionDestroy {collection}
 * @since 26.2.0
 */
typedef void (*OH_ArkUI_NativeModule_ImageCollectionCallback)(ArkUI_ErrorCode errorCode,
    OH_ArkUI_NativeModule_ImageCollection* collection, void* userData);

/**
 * @brief Asynchronously collects images for specified ArkUI node IDs.
 *
 * The framework accepts from <b>1</b> to <b>20</b> unique node IDs in one request and copies the <b>nodeIds</b> array
 * before this function returns.
 * Image collection runs asynchronously after the request is accepted.
 * Node content changes made after the call can be reflected in the result.
 *
 * The caller can call this function from any thread. For each accepted request, the framework invokes the
 * <b>callback</b> exactly once on the UI thread associated with <b>context</b>.
 * Only one request can be active for a given context. A second request for the same context is rejected synchronously
 * with {@link ARKUI_ERROR_CODE_COMMAND_UNFINISHED} until the first callback returns.
 *
 * The caller must release each non-<b>null</b> collection received by the callback by calling
 * <b>OH_ArkUI_NativeModule_ImageCollectionDestroy</b>.
 *
 * @param context [in] Indicates the ArkUI context used to process the request.
 *     The parameter must not be <b>null</b>.
 * @param nodeIds [in] Indicates the first element of an array containing <b>count</b> unique ArkUI node IDs.
 *     The parameter must not be <b>null</b>.
 * @param count [in] Indicates the number of node IDs. The valid range is <b>1</b> to <b>20</b>.
 * @param callback [in] Indicates the callback used to receive the result. The parameter must not be <b>null</b>.
 * @param userData [in] Indicates the caller-defined context pointer passed unchanged to the callback.
 *     The value can be <b>null</b>.
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_NativeModule_GetImagesByNodeIdAsync(ArkUI_ContextHandle context, const int32_t* nodeIds,
    size_t count, OH_ArkUI_NativeModule_ImageCollectionCallback callback, void* userData);

/**
 * @brief Consumes the result for a specified node ID.
 *
 * For a successful image result, this function transfers ownership of the <b>PixelMap</b> to the caller. The caller
 * must release it by calling <b>OH_PixelmapNative_Destroy</b>. This function also consumes a failed image result. In
 * that case, it sets <b>*outPixelmap</b> to <b>null</b> and writes the item error to <b>*outItemError</b>. Each node ID
 * in a collection can be consumed only once.
 *
 * @param collection [in] Indicates the image collection whose item is consumed. The parameter must not be <b>null</b>.
 * @param nodeId [in] Indicates the ArkUI node ID of the item to consume.
 * @param outPixelmap [out] Indicates the output pointer that receives the transferred <b>PixelMap</b>.
 *     The parameter must not be <b>null</b>. For a failed item, the function sets <b>*outPixelmap</b> to <b>null</b>.
 * @param outItemError [out] Indicates the output pointer to the result code for the item.
 *     The parameter must not be <b>null</b>.
 * @release multimedia/image_framework/image/OH_PixelmapNative_Destroy {outPixelmap}
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId(
    OH_ArkUI_NativeModule_ImageCollection* collection, int32_t nodeId,
    OH_PixelmapNative** outPixelmap, ArkUI_ErrorCode* outItemError);

/**
 * @brief Destroys an image collection and releases <b>PixelMap</b> instances that have not been consumed.
 *
 * The caller must destroy every non-<b>null</b> collection that a callback returns, including a collection after the
 * caller has consumed all its items.
 * This function does not release or invalidate <b>PixelMap</b> instances that the caller obtained through
 * <b>OH_ArkUI_NativeModule_ImageCollectionTakeItemByNodeId</b>.
 *
 * @param collection [in] Indicates the image collection to destroy.
 *     The parameter can be <b>null</b>. If <b>collection</b> is <b>null</b>, this function does nothing.
 * @since 26.2.0
 */
void OH_ArkUI_NativeModule_ImageCollectionDestroy(OH_ArkUI_NativeModule_ImageCollection* collection);

/**
 * @brief Enumerates the observable UI interaction event types.
 *
 * The values are bit flags. They are used to build the eventMask passed to
 * {@link OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver}.
 *
 * @since 26.2.0
 */
typedef enum {
    /**
     * @brief No event.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_NONE = 0,
    /**
     * @brief Tap gesture.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP = 1 << 0,
    /**
     * @brief Click.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_CLICK = 1 << 1,
    /**
     * @brief Long press gesture.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_LONG_PRESS = 1 << 2,
    /**
     * @brief Pan gesture.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_PAN = 1 << 3,
    /**
     * @brief Pinch gesture.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_PINCH = 1 << 4,
    /**
     * @brief Rotation gesture.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_ROTATION = 1 << 5,
    /**
     * @brief Swipe gesture.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_SWIPE = 1 << 6,
    /**
     * @brief Unified drag and drop.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_DRAG = 1 << 7,
    /**
     * @brief Raw touch down or up.
     *
     * @since 26.2.0
     */
    OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TOUCH = 1 << 8
} OH_ArkUI_NativeModule_UIInfoCollection_InteractionEventType;

/**
 * @brief Callback type for receiving a sensed interaction event.
 *
 * The callback carries a borrowed {@link OH_ArkUI_NativeModule_UIJsonWrapper} object that holds the
 * event payload. The object is valid only within the callback invocation; the callback must not
 * retain or destroy it. All positional coordinates in the payload are relative to the screen
 * (display), in physical pixels (px). The payload's top-level structure is:
 *
 * { "schemaVersion": 1, "type": "<event type>", ... }
 *
 * The event-specific fields are as follows:
 * - Tap or click: id, point, count, fingers.
 * - Long press: id, point, actualDuration (milliseconds), action ("end").
 * - Pan: id, point, direction, action ("start" | "end" | "cancel").
 * - Pinch: id, point (array of [x, y]), fingers, action ("start" | "end" | "cancel"); scale is
 * present only on "end".
 * - Rotation: id, point (array of [x, y]), fingers, action ("start" | "end" | "cancel"); angle
 * (degrees) is present only on "end".
 * - Swipe: id, downPoint (array of [x, y]), upPoint (array of [x, y]), direction, speed,
 * actualSpeed.
 * - Drag: action ("start" | "end"); "start" carries id, point, hostName, actualDuration; "end"
 * carries point, dropResult ("success" | "fail"), id (target, present only on success), hostName.
 * - Touch: action ("down" | "up"), fingerId, point; "down" also carries id (hit node ID).
 *
 * @param json [in] Borrowed JSON object. Valid only during the callback invocation.
 * @param userData [in] Custom user data passed during registration. It can be NULL. The framework
 *     does not own, dereference, or free it.
 * @since 26.2.0
 */
typedef void (*OH_ArkUI_NativeModule_UIInfoCollectionInteractionJsonCallback)(
    const OH_ArkUI_NativeModule_UIJsonWrapper *json, void *userData);

/**
 * @brief Registers an observer for UI interaction events.
 *
 * The observer receives callbacks only for the event types included in eventMask. Multiple
 * observers can be registered for the same UI instance. Registering the same callback and userData
 * pair again creates an additional independent observer with a new registration ID.
 *
 * Remember to unregister the callback by {@link OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver}
 * when it's not used anymore.
 *
 * This function must be called on the UI main thread of the UI instance. Calling it from any other
 * thread is not supported.
 *
 * @param uiContext [in] Pointer to a UI instance. It must not be NULL.
 * @param eventMask [in] Bitmask of {@link OH_ArkUI_NativeModule_UIInfoCollection_InteractionEventType}
 *     values to observe, combined with the bitwise OR operator. The supported bits are
 *     OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TAP through
 *     OH_ARKUI_NATIVEMODULE_UIINFOCOLLECTION_INTERACTION_EVENT_TOUCH. Bits outside this range are
 *     ignored. If no supported bit is set, {@link ARKUI_ERROR_CODE_PARAM_INVALID} is returned.
 * @param registerID [out] Receives the observer registration ID on success. It must not be NULL,
 *     requires no initialization, and on failure is set to 0.
 * @param callback [in] Callback invoked when a matching event occurs. It must not be NULL.
 * @param userData [in] Custom user data passed to the callback. It can be NULL. The framework does
 *     not take ownership of it; the caller must keep it valid until the observer is unregistered.
 * @return <ul>
 *         <li>{@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *         <li>{@link ARKUI_ERROR_CODE_PARAM_INVALID} if a required pointer is NULL or eventMask
 *             contains no supported bit.</li>
 *         <li>{@link ARKUI_ERROR_CODE_UI_CONTEXT_INVALID} if the UI context is invalid.</li>
 *         </ul>
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIInfoCollectionRegisterInteractionObserver(
    ArkUI_ContextHandle uiContext,
    uint32_t eventMask,
    uint32_t *registerID,
    OH_ArkUI_NativeModule_UIInfoCollectionInteractionJsonCallback callback,
    void *userData);

/**
 * @brief Unregisters an interaction observer.
 *
 * After this call returns, no new callback is started for the registration: callbacks whose
 * dispatch had not yet begun are skipped, including observers unregistered from inside another
 * callback of the same dispatch batch. When this function is called from inside the observer's
 * own callback, the running invocation completes normally afterwards; in that case keep userData
 * valid until the callback returns.
 *
 * This function must be called on the UI main thread of the UI instance. Calling it from any
 * other thread is not supported. This function does not block, and is not async-signal-safe.
 *
 * @param registerID [in] Registration ID returned by a successful registration.
 * @return <ul>
 *         <li>{@link ARKUI_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *         <li>{@link ARKUI_ERROR_CODE_PARAM_INVALID} if the registration ID is invalid.</li>
 *         </ul>
 * @since 26.2.0
 */
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIInfoCollectionUnregisterInteractionObserver(uint32_t registerID);

typedef struct OH_ArkUI_NativeModule_UIContentChangeEvent OH_ArkUI_NativeModule_UIContentChangeEvent;

typedef enum OH_ArkUI_NativeModule_UIContentChangeEventCategory {
    OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_PAGE = 1U << 0,
    OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_SCROLL = 1U << 1,
    OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_OVERLAY = 1U << 2,
    OH_ARKUI_NATIVEMODULE_EVENT_CATEGORY_ALL = ~0U
} OH_ArkUI_NativeModule_UIContentChangeEventCategory;

typedef enum OH_ArkUI_NativeModule_UIContentChangeIgnoreType {
    OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLBY = 1U << 0,
    OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_SCROLLTO = 1U << 1,
    OH_ARKUI_NATIVEMODULE_CONTENTCHANGE_IGNORE_TOAST = 1U << 2
} OH_ArkUI_NativeModule_UIContentChangeIgnoreType;

typedef enum OH_ArkUI_NativeModule_UIContentChangeEventType {
    OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_START = 0,
    OH_ARKUI_NATIVEMODULE_EVENT_PAGE_CHANGE_END = 1,
    OH_ARKUI_NATIVEMODULE_EVENT_SCROLL_START = 2,
    OH_ARKUI_NATIVEMODULE_EVENT_SCROLL_END = 3,
    OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_SHOW = 4,
    OH_ARKUI_NATIVEMODULE_EVENT_OVERLAY_HIDE = 5,
    OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_START = 6,
    OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_END = 7,
    OH_ARKUI_NATIVEMODULE_EVENT_SWIPER_CANCEL = 8,
    OH_ARKUI_NATIVEMODULE_EVENT_TABS_START = 9,
    OH_ARKUI_NATIVEMODULE_EVENT_TABS_END = 10,
    OH_ARKUI_NATIVEMODULE_EVENT_TABS_CANCEL = 11
} OH_ArkUI_NativeModule_UIContentChangeEventType;

typedef void (*OH_ArkUI_NativeModule_UIContentChangeEventCallback)(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, void* userData);

ArkUI_ErrorCode OH_ArkUI_NativeModule_RegisterUIContentChangeEvent(
    ArkUI_ContextHandle uiContext, bool withStart, uint32_t eventMask, uint32_t ignoreMask, void* userData,
    OH_ArkUI_NativeModule_UIContentChangeEventCallback callback, uint64_t* subscriptionId);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UnRegisterUIContentChangeEvent(
    ArkUI_ContextHandle uiContext, uint64_t subscriptionId);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetType(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, OH_ArkUI_NativeModule_UIContentChangeEventType* type);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetTimestamp(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, uint64_t* nanoTimestamp);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetContext(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, ArkUI_ContextHandle* uiContext);
ArkUI_ErrorCode OH_ArkUI_NativeModule_UIContentChangeEventGetContentChangeEventJson(
    const OH_ArkUI_NativeModule_UIContentChangeEvent* event, const OH_ArkUI_NativeModule_UIJsonWrapper** json);

#ifdef __cplusplus
}
#endif

#endif /* ARKUI_NATIVE_UI_INFO_COLLECTION_H */
/** @} */
