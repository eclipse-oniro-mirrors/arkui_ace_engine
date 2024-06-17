/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "core/components_ng/pattern/rich_editor/rich_editor_pattern.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/offset.h"
#include "base/log/dump_log.h"
#include "base/log/ace_trace.h"
#include "base/log/log_wrapper.h"
#include "base/memory/ace_type.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/text/text_emoji_processor.h"
#include "core/common/ai/data_detector_mgr.h"
#include "core/common/clipboard/paste_data.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/common/ime/text_input_client.h"
#include "core/components/common/layout/constants.h"
#include "core/components/common/properties/text_style_parser.h"
#include "core/components_ng/base/inspector_filter.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/event/event_hub.h"
#include "core/components_ng/event/gesture_event_hub.h"
#include "core/components_ng/event/long_press_event.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/pattern/overlay/keyboard_base_pattern.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_event_hub.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_model.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_theme.h"
#include "core/components_ng/pattern/rich_editor/selection_info.h"
#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_info.h"
#include "core/components_ng/pattern/rich_editor_drag/rich_editor_drag_pattern.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text/typed_text.h"
#include "core/components_ng/pattern/text_field/text_field_manager.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/pattern/text_field/text_input_ai_checker.h"
#include "core/components_ng/property/property.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/gestures/gesture_info.h"
#include "core/pipeline/base/element_register.h"

#if not defined(ACE_UNITTEST)
#if defined(ENABLE_STANDARD_INPUT)
#include "commonlibrary/c_utils/base/include/refbase.h"

#include "core/components_ng/pattern/text_field/on_text_changed_listener_impl.h"
#endif
#endif

#include "core/common/ace_engine_ext.h"
#include "core/common/udmf/udmf_client.h"

#ifdef WINDOW_SCENE_SUPPORTED
#include "core/components_ng/pattern/window_scene/helper/window_scene_helper.h"
#endif

namespace OHOS::Ace::NG {

namespace {
#if defined(ENABLE_STANDARD_INPUT)
// should be moved to theme
constexpr float CARET_WIDTH = 1.5f;
constexpr float DEFAULT_CARET_HEIGHT = 18.5f;
constexpr Dimension KEYBOARD_AVOID_OFFSET = 24.0_vp;
#endif
constexpr int32_t IMAGE_SPAN_LENGTH = 1;
constexpr int32_t SYMBOL_SPAN_LENGTH = 2;
constexpr int32_t RICH_EDITOR_TWINKLING_INTERVAL_MS = 500;
constexpr float DEFAULT_TEXT_SIZE = 16.0f;
constexpr int32_t AUTO_SCROLL_INTERVAL = 15;
constexpr Dimension CARET_BOTTOM_DISTANCE = 16.0_vp;
constexpr Dimension AUTO_SCROLL_EDGE_DISTANCE = 15.0_vp;
constexpr Dimension AUTO_SCROLL_DRAG_EDGE_DISTANCE = 58.0_vp;
constexpr float MAX_DRAG_SCROLL_SPEED = 2400.0f;
constexpr float TIME_UNIT = 1000.0f;
constexpr float DOUBLE_CLICK_INTERVAL_MS = 300.0f;
constexpr uint32_t RECORD_MAX_LENGTH = 20;
constexpr float DEFAILT_OPACITY = 0.2f;
constexpr int64_t COLOR_OPAQUE = 255;

constexpr Color SYSTEM_CARET_COLOR = Color(0xff007dff);
constexpr Color SYSTEM_SELECT_BACKGROUND_COLOR = Color(0x33007dff);

constexpr int32_t ERROR_BAD_PARAMETERS = -1;
constexpr char PREVIEW_STYLE_NORMAL[] = "normal";
constexpr char PREVIEW_STYLE_UNDERLINE[] = "underline";
const std::wstring lineSeparator = L"\n";
// hen do ai anaylsis, we should limit the left an right limit of the string
constexpr static int32_t AI_TEXT_RANGE_LEFT = 50;
constexpr static int32_t AI_TEXT_RANGE_RIGHT = 50;
constexpr static int32_t NONE_SELECT_TYPE = -1;
constexpr static int32_t FIRST_LINE = 1;
constexpr static int32_t SECOND_LINE = 2;

constexpr float RICH_DEFAULT_SHADOW_COLOR = 0x33000000;
constexpr float RICH_DEFAULT_ELEVATION = 120.0f;
} // namespace

RichEditorPattern::RichEditorPattern()
{
    selectOverlay_ = AceType::MakeRefPtr<RichEditorSelectOverlay>(WeakClaim(this));
    magnifierController_ = MakeRefPtr<MagnifierController>(WeakClaim(this));
    styledString_ = MakeRefPtr<MutableSpanString>("");
    styledString_->SetSpanWatcher(WeakClaim(this));
}

RichEditorPattern::~RichEditorPattern()
{
    if (isCustomKeyboardAttached_) {
        CloseCustomKeyboard();
    }
}

void RichEditorPattern::SetStyledString(const RefPtr<SpanString>& value)
{
    CHECK_NULL_VOID(value);
    CHECK_NULL_VOID(styledString_);
    auto length = styledString_->GetLength();
    bool isPreventChange = !BeforeStyledStringChange(0, length, value);
    CHECK_NULL_VOID(!isPreventChange);
    styledString_->ReplaceSpanString(0, length, value);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    AfterStyledStringChange(0, length, styledString_->GetString());
}

void RichEditorPattern::UpdateSpanItems(const std::list<RefPtr<NG::SpanItem>>& spanItems)
{
    SetSpanItemChildren(spanItems);
    ProcessStyledString();
}

void RichEditorPattern::ProcessStyledString()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::string textCache = textForDisplay_;
    textForDisplay_.clear();
    dataDetectorAdapter_->textForAI_.clear();
    host->Clean();
    for (const auto& span : spans_) {
        auto imageSpan = DynamicCast<ImageSpanItem>(span);
        if (imageSpan) {
            MountImageNode(imageSpan);
            dataDetectorAdapter_->textForAI_ += '\n';
        } else {
            dataDetectorAdapter_->textForAI_ += span->content;
        }
        textForDisplay_ += span->content;
    }
    if (textForDisplay_ != textCache) {
        dataDetectorAdapter_->aiDetectInitialized_ = false;
    }
    if (CanStartAITask() && !dataDetectorAdapter_->aiDetectInitialized_) {
        dataDetectorAdapter_->StartAITask();
    }
}

void RichEditorPattern::MountImageNode(const RefPtr<ImageSpanItem>& imageItem)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto imageNode = ImageSpanNode::GetOrCreateSpanNode(V2::IMAGE_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
    auto pattern = imageNode->GetPattern<ImagePattern>();
    CHECK_NULL_VOID(pattern);
    pattern->SetSyncLoad(true);
    auto index = host->GetChildren().size();
    imageNode->MountToParent(host, index);
    CHECK_NULL_VOID(imageItem);
    auto options = imageItem->options;
    SetImageLayoutProperty(imageNode, options);
    imageItem->imageNodeId = imageNode->GetId();
    imageNode->SetImageItem(imageItem);
}

void RichEditorPattern::SetImageLayoutProperty(RefPtr<ImageSpanNode> imageNode, const ImageSpanOptions& options)
{
    CHECK_NULL_VOID(imageNode);
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    std::function<ImageSourceInfo()> createSourceInfoFunc = CreateImageSourceInfo(options);
    imageLayoutProperty->UpdateImageSourceInfo(createSourceInfoFunc());
    CHECK_NULL_VOID(options.imageAttribute.has_value());
    auto imgAttr = options.imageAttribute.value();
    if (imgAttr.size.has_value()) {
        imageLayoutProperty->UpdateUserDefinedIdealSize(imgAttr.size->GetSize());
    }
    if (imgAttr.verticalAlign.has_value()) {
        imageLayoutProperty->UpdateVerticalAlign(imgAttr.verticalAlign.value());
    }
    if (imgAttr.objectFit.has_value()) {
        imageLayoutProperty->UpdateImageFit(imgAttr.objectFit.value());
    }
    if (imgAttr.marginProp.has_value()) {
        imageLayoutProperty->UpdateMargin(imgAttr.marginProp.value());
    }
    if (imgAttr.paddingProp.has_value()) {
        imageLayoutProperty->UpdatePadding(imgAttr.paddingProp.value());
    }
    if (imgAttr.borderRadius.has_value()) {
        auto imageRenderCtx = imageNode->GetRenderContext();
        imageRenderCtx->UpdateBorderRadius(imgAttr.borderRadius.value());
        imageRenderCtx->SetClipToBounds(true);
    }
    imageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
}

void RichEditorPattern::InsertValueInStyledString(const std::string& insertValue)
{
    CHECK_NULL_VOID(styledString_);
    int32_t changeStart = caretPosition_;
    int32_t changeLength = 0;
    if (textSelector_.IsValid()) {
        auto start = textSelector_.GetTextStart();
        auto end = textSelector_.GetTextEnd();
        SetCaretPosition(start);
        changeStart = caretPosition_;
        changeLength = end - start;
        DeleteForwardInStyledString(changeLength, false);
        CloseSelectOverlay();
        ResetSelection();
    }
    bool isPreventChange = false;
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        auto insertStyledString =
            CreateStyledStringByTextStyle(insertValue, typingStyle_.value(), typingTextStyle_.value());
        isPreventChange = !BeforeStyledStringChange(changeStart, changeLength, insertStyledString);
        CHECK_NULL_VOID(!isPreventChange);
        styledString_->InsertSpanString(changeStart, insertStyledString);
    } else {
        isPreventChange = !BeforeStyledStringChange(changeStart, changeLength, insertValue);
        CHECK_NULL_VOID(!isPreventChange);
        styledString_->InsertString(changeStart, insertValue);
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
    isTextChange_ = true;
    moveDirection_ = MoveDirection::FORWARD;
    moveLength_ = StringUtils::ToWstring(insertValue).length();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    AfterStyledStringChange(changeStart, changeLength, insertValue);
}

RefPtr<SpanString> RichEditorPattern::CreateStyledStringByTextStyle(
    const std::string& insertValue, const struct UpdateSpanStyle& updateSpanStyle, const TextStyle& textStyle)
{
    auto styledString = AceType::MakeRefPtr<SpanString>(insertValue);
    auto length = styledString->GetLength();
    auto fontSpan = CreateFontSpanByTextStyle(updateSpanStyle, textStyle, length);
    styledString->AddSpan(fontSpan);
    auto decorationSpan = CreateDecorationSpanByTextStyle(updateSpanStyle, textStyle, length);
    styledString->AddSpan(decorationSpan);
    if (updateSpanStyle.updateTextShadows.has_value()) {
        auto textShadowSpan = AceType::MakeRefPtr<TextShadowSpan>(textStyle.GetTextShadows(), 0, length);
        styledString->AddSpan(textShadowSpan);
    }
    if (updateSpanStyle.updateLineHeight.has_value()) {
        auto lineHeightSpan = AceType::MakeRefPtr<LineHeightSpan>(textStyle.GetLineHeight(), 0, length);
        styledString->AddSpan(lineHeightSpan);
    }
    if (updateSpanStyle.updateLetterSpacing.has_value()) {
        auto letterSpacingSpan = AceType::MakeRefPtr<LetterSpacingSpan>(textStyle.GetLetterSpacing(), 0, length);
        styledString->AddSpan(letterSpacingSpan);
    }
    return styledString;
}

RefPtr<FontSpan> RichEditorPattern::CreateFontSpanByTextStyle(
    const struct UpdateSpanStyle& updateSpanStyle, const TextStyle& textStyle, int32_t length)
{
    Font font;
    if (updateSpanStyle.updateFontWeight.has_value()) {
        font.fontWeight = textStyle.GetFontWeight();
    }
    if (updateSpanStyle.updateFontSize.has_value()) {
        font.fontSize = textStyle.GetFontSize();
    }
    if (updateSpanStyle.updateItalicFontStyle.has_value()) {
        font.fontStyle = textStyle.GetFontStyle();
    }
    if (updateSpanStyle.updateFontFamily.has_value()) {
        font.fontFamilies = textStyle.GetFontFamilies();
    }
    if (updateSpanStyle.updateTextColor.has_value()) {
        font.fontColor = textStyle.GetTextColor();
    }
    return AceType::MakeRefPtr<FontSpan>(font, 0, length);
}

RefPtr<DecorationSpan> RichEditorPattern::CreateDecorationSpanByTextStyle(
    const struct UpdateSpanStyle& updateSpanStyle, const TextStyle& textStyle, int32_t length)
{
    TextDecoration type = TextDecoration::NONE;
    std::optional<Color> colorOption;
    std::optional<TextDecorationStyle> styleOption;
    if (updateSpanStyle.updateTextDecoration.has_value()) {
        type = textStyle.GetTextDecoration();
    }
    if (updateSpanStyle.updateTextDecorationColor.has_value()) {
        colorOption = textStyle.GetTextDecorationColor();
    }
    return AceType::MakeRefPtr<DecorationSpan>(type, colorOption, styleOption, 0, length);
}

void RichEditorPattern::DeleteBackwardInStyledString(int32_t length)
{
    CHECK_NULL_VOID(styledString_);
    if (textSelector_.IsValid()) {
        if (!textSelector_.StartEqualToDest()) {
            length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        }
        SetCaretPosition(textSelector_.GetTextEnd());
        CloseSelectOverlay();
        ResetSelection();
    }
    CHECK_NULL_VOID(caretPosition_ != 0);
    int32_t changeStart = caretPosition_ - length;
    bool isPreventChange = !BeforeStyledStringChange(changeStart, length, "");
    CHECK_NULL_VOID(!isPreventChange);
    styledString_->RemoveString(changeStart, length);
    if (!caretVisible_) {
        StartTwinkling();
    }
    isTextChange_ = true;
    moveDirection_ = MoveDirection::BACKWARD;
    moveLength_ = length;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    AfterStyledStringChange(changeStart, length, "");
}

void RichEditorPattern::DeleteForwardInStyledString(int32_t length, bool isIME)
{
    CHECK_NULL_VOID(styledString_);
    if (textSelector_.IsValid()) {
        if (!textSelector_.StartEqualToDest()) {
            length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        }
        SetCaretPosition(textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    CHECK_NULL_VOID(GetTextContentLength() != 0);
    int32_t changeStart = caretPosition_;
    bool isPreventChange = isIME && !BeforeStyledStringChange(changeStart, length, "");
    CHECK_NULL_VOID(!isPreventChange);
    styledString_->RemoveString(changeStart, length);
    if (!caretVisible_) {
        StartTwinkling();
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    if (isIME) {
        AfterStyledStringChange(changeStart, length, "");
    }
}

bool RichEditorPattern::BeforeStyledStringChange(int32_t start, int32_t length, const std::string& string)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    CHECK_NULL_RETURN(eventHub->HasOnStyledStringWillChange(), true);
    auto styledString = AceType::MakeRefPtr<SpanString>(string);
    auto stringLength = styledString->GetLength();
    auto changeStart = std::clamp(start, 0, GetTextContentLength());
    if (stringLength != 0) {
        auto lastStyles = styledString_->GetSpans(changeStart - 1, 1);
        for (auto && style : lastStyles) {
            if (!style) {
                continue;
            }
            style->UpdateStartIndex(0);
            style->UpdateEndIndex(stringLength);
            styledString->AddSpan(style);
        }
    }
    return BeforeStyledStringChange(changeStart, length, styledString);
}

bool RichEditorPattern::BeforeStyledStringChange(int32_t start, int32_t length, const RefPtr<SpanString>& styledString)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    CHECK_NULL_RETURN(eventHub->HasOnStyledStringWillChange(), true);
    auto replaceMentString = AceType::MakeRefPtr<MutableSpanString>("");
    replaceMentString->AppendSpanString(styledString);
    StyledStringChangeValue changeValue;
    auto changeStart = std::clamp(start, 0, GetTextContentLength());
    auto changeEnd = std::clamp(changeStart + length, 0, GetTextContentLength());
    changeValue.SetRangeBefore({ changeStart, changeEnd });
    changeValue.SetReplacementString(replaceMentString);
    return eventHub->FireOnStyledStringWillChange(changeValue);
}

void RichEditorPattern::AfterStyledStringChange(int32_t start, int32_t length, const std::string& string)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    CHECK_NULL_VOID(eventHub->HasOnStyledStringDidChange());
    StyledStringChangeValue changeValue;
    auto changeStart = std::clamp(start, 0, GetTextContentLength());
    auto changeEnd = std::clamp(changeStart + length, 0, GetTextContentLength());
    auto stringLength = static_cast<int32_t>(StringUtils::ToWstring(string).length());
    auto stringEnd = changeStart + stringLength;
    changeValue.SetRangeBefore({ changeStart, changeEnd });
    changeValue.SetRangeAfter({ changeStart, stringEnd });
    eventHub->FireOnStyledStringDidChange(changeValue);
}

void RichEditorPattern::OnModifyDone()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    copyOption_ = layoutProperty->GetCopyOption().value_or(CopyOptions::Distributed);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    ResetKeyboardIfNeed();
    context->AddOnAreaChangeNode(host->GetId());
    if (!clipboard_ && context) {
        clipboard_ = ClipboardProxy::GetInstance()->GetClipboard(context->GetTaskExecutor());
    }
    instanceId_ = context->GetInstanceId();
    InitMouseEvent();
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitFocusEvent(focusHub);
    auto gestureEventHub = host->GetOrCreateGestureEventHub();
    InitClickEvent(gestureEventHub);
    InitLongPressEvent(gestureEventHub);
    InitTouchEvent();
    HandleEnabled();
    ProcessInnerPadding();
    InitScrollablePattern();
    SetAccessibilityAction();
    if (CanStartAITask() && !dataDetectorAdapter_->aiDetectInitialized_) {
        dataDetectorAdapter_->StartAITask();
    }
    if (host->IsDraggable() && copyOption_ != CopyOptions::None) {
        InitDragDropEvent();
        AddDragFrameNodeToManager(host);
    } else {
        ClearDragDropEvent();
        RemoveDragFrameNodeFromManager(host);
    }
    Register2DragDropManager();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);

    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    bool enabledCache = eventHub->IsEnabled();
    if (textDetectEnable_ && enabledCache != enabled_) {
        enabled_ = enabledCache;
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
}

void RichEditorPattern::HandleEnabled()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    if (IsDisabled()) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
        CHECK_NULL_VOID(richEditorTheme);
        auto disabledAlpha = richEditorTheme->GetDisabledAlpha();
        renderContext->OnOpacityUpdate(disabledAlpha);
    } else {
        auto opacity = renderContext->GetOpacity().value_or(1.0);
        renderContext->OnOpacityUpdate(opacity);
    }
}

void RichEditorPattern::BeforeCreateLayoutWrapper()
{
    ACE_SCOPED_TRACE("RichEditorBeforeCreateLayoutWrapper");
    if (!isSpanStringMode_) {
        TextPattern::PreCreateLayoutWrapper();
    } else if (contentMod_) {
        contentMod_->ContentChange();
    }
}

bool RichEditorPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (config.skipMeasure || dirty->SkipMeasureContent()) {
        return false;
    }
    auto originHeight = frameRect_.Height();
    frameRect_ = dirty->GetGeometryNode()->GetFrameRect();
    auto layoutAlgorithmWrapper = DynamicCast<LayoutAlgorithmWrapper>(dirty->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(layoutAlgorithmWrapper, false);
    auto richEditorLayoutAlgorithm =
        DynamicCast<RichEditorLayoutAlgorithm>(layoutAlgorithmWrapper->GetLayoutAlgorithm());
    CHECK_NULL_RETURN(richEditorLayoutAlgorithm, false);
    parentGlobalOffset_ = richEditorLayoutAlgorithm->GetParentGlobalOffset();
    richTextRect_ = richEditorLayoutAlgorithm->GetTextRect();
    UpdateTextFieldManager(Offset(parentGlobalOffset_.GetX(), parentGlobalOffset_.GetY()), frameRect_.Height());
    bool ret = TextPattern::OnDirtyLayoutWrapperSwap(dirty, config);
    UpdateScrollStateAfterLayout(config.frameSizeChange);
    if (!isRichEditorInit_) {
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_RETURN(eventHub, ret);
        eventHub->FireOnReady();
        ClearOperationRecords();
        isFirstCallOnReady_ = true;
        isRichEditorInit_ = true;
    }
    MoveCaretOnLayoutSwap(LessNotEqual(originHeight, frameRect_.Height()));
    if (textSelector_.IsValid() && SelectOverlayIsOn() && isShowMenu_) {
        CalculateHandleOffsetAndShowOverlay();
        ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
    }
    isShowMenu_ = true;
    UpdateCaretInfoToController();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, ret);
    auto context = host->GetRenderContext();
    CHECK_NULL_RETURN(context, ret);
    if (context->GetClipEdge().has_value()) {
        auto geometryNode = host->GetGeometryNode();
        auto frameOffset = geometryNode->GetFrameOffset();
        auto frameSize = geometryNode->GetFrameSize();
        auto height = static_cast<float>(paragraphs_.GetHeight() + std::fabs(baselineOffset_));
        if (!context->GetClipEdge().value() && LessNotEqual(frameSize.Height(), height)) {
            RectF boundsRect(frameOffset.GetX(), frameOffset.GetY(), frameSize.Width(), height);
            CHECK_NULL_RETURN(overlayMod_, ret);
            overlayMod_->SetBoundsRect(boundsRect);
        }
    }
    caretUpdateType_ = CaretUpdateType::NONE;
    return ret;
}

void RichEditorPattern::MoveCaretOnLayoutSwap(bool isReduceSize)
{
    MoveCaretAfterTextChange();
    if (HasFocus() && !isReduceSize) {
        MoveCaretToContentRect();
    }
}

std::function<ImageSourceInfo()> RichEditorPattern::CreateImageSourceInfo(const ImageSpanOptions& options)
{
    std::string src;
    RefPtr<PixelMap> pixMap = nullptr;
    std::string bundleName;
    std::string moduleName;
    if (options.image.has_value()) {
        src = options.image.value();
    }
    if (options.imagePixelMap.has_value()) {
        pixMap = options.imagePixelMap.value();
    }
    if (options.bundleName.has_value()) {
        bundleName = options.bundleName.value();
    }
    if (options.moduleName.has_value()) {
        moduleName = options.moduleName.value();
    }
    auto createSourceInfoFunc = [src, noPixMap = !options.imagePixelMap.has_value(), pixMap, bundleName,
                                    moduleName]() -> ImageSourceInfo {
#if defined(PIXEL_MAP_SUPPORTED)
        if (noPixMap) {
            return { src, bundleName, moduleName };
        }
        return ImageSourceInfo(pixMap);
#else
        return { src, bundleName, moduleName };
#endif
    };
    return std::move(createSourceInfoFunc);
}

int32_t RichEditorPattern::GetTextContentLength()
{
    if (isSpanStringMode_ && styledString_) {
        return styledString_->GetLength();
    }
    if (!spans_.empty()) {
        auto it = spans_.rbegin();
        return (*it)->position;
    }
    return 0;
}

void RichEditorPattern::DisableDrag(RefPtr<ImageSpanNode> imageNode)
{
    CHECK_NULL_VOID(imageNode);
    // Disable the image itself event
    imageNode->SetDraggable(false);
    auto gesture = imageNode->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gesture);
    gesture->SetDragEvent(nullptr, { PanDirection::DOWN }, 0, Dimension(0));
}

void RichEditorPattern::SetGestureOptions(UserGestureOptions userGestureOptions, RefPtr<SpanItem> spanItem)
{
    if (userGestureOptions.onClick) {
        spanItem->SetOnClickEvent(std::move(userGestureOptions.onClick));
    }

    if (userGestureOptions.onLongPress) {
        spanItem->SetLongPressEvent(std::move(userGestureOptions.onLongPress));
    }
}

int32_t RichEditorPattern::AddImageSpan(const ImageSpanOptions& options, bool isPaste, int32_t index,
    bool updateCaret)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "options=%{public}s", options.ToString().c_str());
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "isPaste=%{public}d, index=%{public}d", isPaste, index);
    auto imageNode = ImageSpanNode::GetOrCreateSpanNode(V2::IMAGE_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<ImagePattern>(); });
    auto pattern = imageNode->GetPattern<ImagePattern>();
    CHECK_NULL_RETURN(pattern, -1);
    pattern->SetSyncLoad(true);
    int32_t insertIndex = options.offset.value_or(GetTextContentLength());
    insertIndex = std::min(insertIndex, GetTextContentLength());
    RichEditorChangeValue changeValue;
    CHECK_NULL_RETURN(BeforeAddImage(changeValue, options, insertIndex), -1);

    DisableDrag(imageNode);

    OperationRecord record;
    record.beforeCaretPosition = options.offset.value_or(static_cast<int32_t>(GetTextContentLength()));
    record.addText = " ";
    ClearRedoOperationRecords();
    record.afterCaretPosition = record.beforeCaretPosition + 1;
    AddOperationRecord(record);

    int32_t spanIndex = TextSpanSplit(insertIndex);
    if (spanIndex == -1) {
        spanIndex = static_cast<int32_t>(host->GetChildren().size());
    }
    imageNode->MountToParent(host, spanIndex);
    SetImageLayoutProperty(imageNode, options);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    auto spanItem = imageNode->GetSpanItem();
    // The length of the imageSpan defaults to the length of a character to calculate the position
    spanItem->content = " ";
    AddSpanItem(spanItem, spanIndex);
    SetGestureOptions(options.userGestureOption, spanItem);
    placeholderCount_++;
    if (updateCaret) {
        SetCaretPosition(insertIndex + spanItem->content.length());
    }
    if (!isPaste && textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    AfterAddImage(changeValue);
    return spanIndex;
}

void RichEditorPattern::AddSpanItem(const RefPtr<SpanItem>& item, int32_t offset)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (offset == -1) {
        offset = host->GetChildren().size();
    }
    offset = std::clamp(offset, 0, static_cast<int32_t>(host->GetChildren().size()) - 1);
    auto it = spans_.begin();
    std::advance(it, offset);
    spans_.insert(it, item);
    UpdateSpanPosition();
}

int32_t RichEditorPattern::AddPlaceholderSpan(const RefPtr<UINode>& customNode, const SpanOptionBase& options)
{
    CHECK_NULL_RETURN(customNode, 0);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 0);
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "AddPlaceholderSpan");
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "options=%{public}s", options.ToString().c_str());
    auto placeholderSpanNode = PlaceholderSpanNode::GetOrCreateSpanNode(V2::PLACEHOLDER_SPAN_ETS_TAG,
        ElementRegister::GetInstance()->MakeUniqueId(), []() { return AceType::MakeRefPtr<PlaceholderSpanPattern>(); });
    CHECK_NULL_RETURN(placeholderSpanNode, 0);
    customNode->MountToParent(placeholderSpanNode);
    SetSelfAndChildDraggableFalse(customNode);
    auto focusHub = placeholderSpanNode->GetOrCreateFocusHub();
    focusHub->SetFocusable(false);
    int32_t insertIndex = options.offset.value_or(GetTextContentLength());
    int32_t spanIndex = TextSpanSplit(insertIndex);
    if (spanIndex == -1) {
        spanIndex = static_cast<int32_t>(host->GetChildren().size());
    }
    placeholderSpanNode->MountToParent(host, spanIndex);
    auto spanItem = placeholderSpanNode->GetSpanItem();
    spanItem->content = " ";
    AddSpanItem(spanItem, spanIndex);
    placeholderCount_++;
    SetCaretPosition(insertIndex + spanItem->content.length());
    if (textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    placeholderSpanNode->MarkModifyDone();
    placeholderSpanNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    return spanIndex;
}

void RichEditorPattern::SetSelfAndChildDraggableFalse(const RefPtr<UINode>& customNode)
{
    CHECK_NULL_VOID(customNode);
    auto frameNode = DynamicCast<FrameNode>(customNode);
    if (frameNode) {
        auto eventHub = frameNode->GetEventHub<EventHub>();
        CHECK_NULL_VOID(eventHub);
        auto gestureEventHub = eventHub->GetGestureEventHub();
        CHECK_NULL_VOID(gestureEventHub);
        gestureEventHub->SetDragForbiddenForcely(true);
    }
    for (const auto& child : customNode->GetChildren()) {
        SetSelfAndChildDraggableFalse(child);
    }
}

int32_t RichEditorPattern::AddTextSpan(const TextSpanOptions& options, bool isPaste, int32_t index)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "options=%{public}s", options.ToString().c_str());
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "isPaste=%{public}d, index=%{public}d", isPaste, index);
    OperationRecord record;
    record.beforeCaretPosition = options.offset.value_or(static_cast<int32_t>(GetTextContentLength()));
    record.addText = options.value;
    RichEditorChangeValue changeValue;
    CHECK_NULL_RETURN(BeforeChangeText(changeValue, options), -1);
    ClearRedoOperationRecords();
    record.afterCaretPosition =
        record.beforeCaretPosition + static_cast<int32_t>(StringUtils::ToWstring(options.value).length());
    AddOperationRecord(record);
    auto ret = AddTextSpanOperation(options, isPaste, index, false);
    if (!previewTextRecord_.isPreviewTextInputting) {
        AfterChangeText(changeValue);
    }
    return ret;
}

int32_t RichEditorPattern::AddTextSpanOperation(
    const TextSpanOptions& options, bool isPaste, int32_t index, bool needLeadingMargin, bool updateCaretPosition)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);

    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto spanNode = SpanNode::GetOrCreateSpanNode(nodeId);

    int32_t spanIndex = 0;
    int32_t offset = -1;
    if (options.offset.has_value()) {
        offset = TextSpanSplit(options.offset.value(), needLeadingMargin);
        if (offset == -1) {
            spanIndex = static_cast<int32_t>(host->GetChildren().size());
        } else {
            spanIndex = offset;
        }
        spanNode->MountToParent(host, offset);
    } else if (index != -1) {
        spanNode->MountToParent(host, index);
        spanIndex = index;
    } else {
        spanIndex = static_cast<int32_t>(host->GetChildren().size());
        spanNode->MountToParent(host);
    }
    spanNode->UpdateContent(options.value);
    spanNode->AddPropertyInfo(PropertyInfo::NONE);
    if (options.style.has_value()) {
        spanNode->UpdateTextColor(options.style.value().GetTextColor());
        spanNode->AddPropertyInfo(PropertyInfo::FONTCOLOR);
        spanNode->UpdateFontSize(options.style.value().GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
        spanNode->UpdateItalicFontStyle(options.style.value().GetFontStyle());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSTYLE);
        spanNode->UpdateFontWeight(options.style.value().GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
        spanNode->UpdateFontFamily(options.style.value().GetFontFamilies());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFAMILY);
        spanNode->UpdateTextDecoration(options.style.value().GetTextDecoration());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
        spanNode->UpdateTextDecorationColor(options.style.value().GetTextDecorationColor());
        spanNode->AddPropertyInfo(PropertyInfo::NONE);
        spanNode->UpdateTextShadow(options.style.value().GetTextShadows());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTSHADOW);
        spanNode->UpdateLineHeight(options.style.value().GetLineHeight());
        spanNode->AddPropertyInfo(PropertyInfo::LINEHEIGHT);
        spanNode->UpdateLetterSpacing(options.style.value().GetLetterSpacing());
        spanNode->AddPropertyInfo(PropertyInfo::LETTERSPACE);
        spanNode->UpdateFontFeature(options.style.value().GetFontFeatures());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFEATURE);
    }
    auto spanItem = spanNode->GetSpanItem();
    spanItem->content = options.value;
    spanItem->SetTextStyle(options.style);
    spanItem->hasResourceFontColor = options.hasResourceFontColor;
    spanItem->hasResourceDecorationColor = options.hasResourceDecorationColor;
    AddSpanItem(spanItem, offset);
    if (!options.style.has_value()) {
        SetDefaultColor(spanNode);
    }
    if (options.paraStyle) {
        UpdateParagraphStyle(spanNode, *options.paraStyle);
    }
    if (options.userGestureOption.onClick) {
        auto tmpClickFunc = options.userGestureOption.onClick;
        spanItem->SetOnClickEvent(std::move(tmpClickFunc));
    }
    if (options.userGestureOption.onLongPress) {
        auto tmpLongPressFunc = options.userGestureOption.onLongPress;
        spanItem->SetLongPressEvent(std::move(tmpLongPressFunc));
    }
    if (updateCaretPosition) {
        if (options.offset.has_value()) {
            SetCaretPosition(options.offset.value() + StringUtils::ToWstring(options.value).length());
        } else {
            SetCaretPosition(GetTextContentLength());
        }
    }
    if (!isPaste && textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    SpanNodeFission(spanNode);
    return spanIndex;
}

int32_t RichEditorPattern::AddSymbolSpan(const SymbolSpanOptions& options, bool isPaste, int32_t index)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "options=%{public}s", options.ToString().c_str());
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "isPaste=%{public}d, index=%{public}d", isPaste, index);
    OperationRecord record;
    record.beforeCaretPosition = options.offset.value_or(static_cast<int32_t>(GetTextContentLength()));
    record.addText = " ";
    ClearRedoOperationRecords();
    record.afterCaretPosition = record.beforeCaretPosition + 1;
    AddOperationRecord(record);
    return AddSymbolSpanOperation(options, isPaste, index);
}

int32_t RichEditorPattern::AddSymbolSpanOperation(const SymbolSpanOptions& options, bool isPaste, int32_t index)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);

    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    auto spanNode = SpanNode::GetOrCreateSpanNode(V2::SYMBOL_SPAN_ETS_TAG, nodeId);

    int32_t insertIndex = options.offset.value_or(GetTextContentLength());
    int32_t spanIndex = TextSpanSplit(insertIndex);
    if (spanIndex == -1) {
        spanIndex = static_cast<int32_t>(host->GetChildren().size());
    }
    spanNode->MountToParent(host, spanIndex);
    spanNode->UpdateContent(options.symbolId);
    spanNode->AddPropertyInfo(PropertyInfo::NONE);
    if (options.style.has_value()) {
        spanNode->UpdateFontSize(options.style.value().GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
        spanNode->UpdateFontWeight(options.style.value().GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
        spanNode->UpdateSymbolColorList(options.style.value().GetSymbolColorList());
        spanNode->AddPropertyInfo(PropertyInfo::SYMBOL_COLOR);
        spanNode->UpdateSymbolRenderingStrategy(options.style.value().GetRenderStrategy());
        spanNode->AddPropertyInfo(PropertyInfo::SYMBOL_RENDERING_STRATEGY);
        spanNode->UpdateSymbolEffectStrategy(options.style.value().GetEffectStrategy());
        spanNode->AddPropertyInfo(PropertyInfo::SYMBOL_EFFECT_STRATEGY);
    }
    auto spanItem = spanNode->GetSpanItem();
    spanItem->content = "  ";
    spanItem->SetTextStyle(options.style);
    spanItem->SetResourceObject(options.resourceObject);
    AddSpanItem(spanItem, spanIndex);
    SetCaretPosition(insertIndex + spanItem->content.length());
    SpanNodeFission(spanNode);
    return spanIndex;
}

void RichEditorPattern::SpanNodeFission(RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    auto content = StringUtils::ToWstring(spanItem->content);
    auto contentLen = content.length();
    auto spanStart = spanItem->position - contentLen;
    for (size_t i = 0; i < content.length(); i++) {
        auto character = content[i];
        if (character == '\n') {
            auto charPosition = spanStart + i;
            TextSpanSplit(static_cast<int32_t>(charPosition + 1));
        }
    }
}

void RichEditorPattern::DeleteSpans(const RangeOptions& options)
{
    auto length = GetTextContentLength();
    int32_t start = options.start.value_or(0);
    int32_t end = options.end.value_or(length);
    if (start > end) {
        std::swap(start, end);
    }
    start = std::max(0, start);
    end = std::min(length, end);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "index range=[%{public}d, %{public}d]", start, end);
    if (start > length || end < 0 || start == end) {
        return;
    }
    AdjustSelector(start, end);
    OperationRecord record;
    record.beforeCaretPosition = start;
    std::wstringstream wss;
    for (auto iter = spans_.cbegin(); iter != spans_.cend(); iter++) {
        wss << StringUtils::ToWstring((*iter)->content);
    }
    std::wstring deleteText = wss.str().substr(start, end - start);
    record.deleteText = StringUtils::ToString(deleteText);
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, record, RecordType::DEL_FORWARD, deleteText.length()));
    ClearRedoOperationRecords();
    record.afterCaretPosition = start;
    AddOperationRecord(record);
    auto startInfo = GetSpanPositionInfo(start);
    auto endInfo = GetSpanPositionInfo(end - 1);
    if (startInfo.spanIndex_ == endInfo.spanIndex_) {
        DeleteSpanByRange(start, end, startInfo);
    } else {
        DeleteSpansByRange(start, end, startInfo, endInfo);
    }
    RemoveEmptySpanItems();
    if (textSelector_.IsValid()) {
        SetCaretPosition(textSelector_.GetTextStart());
        CloseSelectOverlay();
        ResetSelection();
    }
    SetCaretOffset(start);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (host->GetChildren().empty() || GetTextContentLength() == 0) {
        SetCaretPosition(0);
        textForDisplay_.clear();
    }
    UpdateSpanPosition();
    AfterChangeText(changeValue);
}

void RichEditorPattern::RemoveEmptySpanItems()
{
    for (auto it = spans_.begin(); it != spans_.end();) {
        if ((*it)->content.empty()) {
            it = spans_.erase(it);
        } else {
            ++it;
        }
    }
}

void RichEditorPattern::DeleteSpanByRange(int32_t start, int32_t end, SpanPositionInfo info)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    auto it = childrens.begin();
    std::advance(it, info.spanIndex_);
    if (start == info.spanStart_ && end == info.spanEnd_) {
        ClearContent(*it);
        host->RemoveChild(*it);
    } else {
        auto spanNode = DynamicCast<SpanNode>(*it);
        CHECK_NULL_VOID(spanNode);
        auto spanItem = spanNode->GetSpanItem();
        auto beforStr = StringUtils::ToWstring(spanItem->content).substr(0, start - info.spanStart_);
        auto endStr = StringUtils::ToWstring(spanItem->content).substr(end - info.spanStart_);
        std::wstring result = beforStr + endStr;
        auto str = StringUtils::ToString(result);
        spanNode->UpdateContent(str);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

void RichEditorPattern::DeleteSpansByRange(
    int32_t start, int32_t end, SpanPositionInfo startInfo, SpanPositionInfo endInfo)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto childrens = host->GetChildren();
    if (childrens.empty()) {
        return;
    }

    auto itStart = childrens.begin();
    if (startInfo.spanIndex_ >= static_cast<int32_t>(childrens.size())) {
        std::advance(itStart, static_cast<int32_t>(childrens.size()) - 1);
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "startInfo.spanIndex_ is larger than childrens size");
    } else {
        std::advance(itStart, startInfo.spanIndex_);
    }
    auto saveStartSpan = (start == startInfo.spanStart_) ? 0 : 1;
    if (saveStartSpan) {
        auto spanNodeStart = DynamicCast<SpanNode>(*itStart);
        CHECK_NULL_VOID(spanNodeStart);
        auto spanItemStart = spanNodeStart->GetSpanItem();
        auto beforStr = StringUtils::ToWstring(spanItemStart->content).substr(0, start - startInfo.spanStart_);
        auto strStart = StringUtils::ToString(beforStr);
        spanNodeStart->UpdateContent(strStart);
    }
    auto itEnd = childrens.begin();
    std::advance(itEnd, endInfo.spanIndex_);
    auto delEndSpan = (end == endInfo.spanEnd_) ? 1 : 0;
    if (!delEndSpan) {
        auto spanNodeEnd = DynamicCast<SpanNode>(*itEnd);
        CHECK_NULL_VOID(spanNodeEnd);
        auto spanItemEnd = spanNodeEnd->GetSpanItem();
        auto endStr =
            StringUtils::ToWstring(spanItemEnd->content).substr(end - endInfo.spanStart_, endInfo.spanEnd_ - end);
        auto strEnd = StringUtils::ToString(endStr);
        spanNodeEnd->UpdateContent(strEnd);
    }
    auto startIter = childrens.begin();
    std::advance(startIter, startInfo.spanIndex_ + saveStartSpan);
    auto endIter = childrens.begin();
    std::advance(endIter, endInfo.spanIndex_);
    for (auto iter = startIter; iter != endIter; ++iter) {
        ClearContent(*iter);
        host->RemoveChild(*iter);
    }
    if (endIter != childrens.end() && delEndSpan) {
        ClearContent(*endIter);
        host->RemoveChild(*endIter);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

std::u16string RichEditorPattern::GetLeftTextOfCursor(int32_t number)
{
    if (number > caretPosition_) {
        number = caretPosition_;
    }
    auto start = caretPosition_;
    if (IsSelected()) {
        start = std::min(textSelector_.GetStart(), textSelector_.GetEnd());
    }
    auto stringText = GetSelectedText(start - number, start);
    return StringUtils::Str8ToStr16(stringText);
}

std::u16string RichEditorPattern::GetRightTextOfCursor(int32_t number)
{
    auto end = caretPosition_;
    if (IsSelected()) {
        end = std::max(textSelector_.GetStart(), textSelector_.GetEnd());
    }
    auto stringText = GetSelectedText(end, end + number);
    return StringUtils::Str8ToStr16(stringText);
}

int32_t RichEditorPattern::GetTextIndexAtCursor()
{
    return caretPosition_;
}

void RichEditorPattern::ClearContent(const RefPtr<UINode>& child)
{
    CHECK_NULL_VOID(child);
    if (child->GetTag() == V2::SPAN_ETS_TAG) {
        auto spanNode = DynamicCast<SpanNode>(child);
        if (spanNode) {
            spanNode->UpdateContent("");
            spanNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }
}

SpanPositionInfo RichEditorPattern::GetSpanPositionInfo(int32_t position)
{
    SpanPositionInfo spanPositionInfo(-1, -1, -1, -1);
    CHECK_NULL_RETURN(!spans_.empty(), spanPositionInfo);
    position = std::clamp(position, 0, GetTextContentLength());
    // find the spanItem where the position is
    auto it = std::find_if(spans_.begin(), spans_.end(), [position](const RefPtr<SpanItem>& spanItem) {
        return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                   position) &&
               (position < spanItem->position);
    });
    if (it != spans_.end() && (*it)->unicode != 0 && (*it)->position - caretPosition_ + moveLength_ == 1) {
        it++;
        moveLength_++;
        position++;
    }

    // the position is at the end
    if (it == spans_.end()) {
        return spanPositionInfo;
    }

    spanPositionInfo.spanIndex_ = std::distance(spans_.begin(), it);
    auto contentLen = StringUtils::ToWstring((*it)->content).length();
    spanPositionInfo.spanStart_ = (*it)->position - contentLen;
    spanPositionInfo.spanEnd_ = (*it)->position;
    spanPositionInfo.spanOffset_ = position - spanPositionInfo.spanStart_;
    return spanPositionInfo;
}

void RichEditorPattern::CopyTextSpanStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target, bool needLeadingMargin)
{
    CHECK_NULL_VOID(source);
    CHECK_NULL_VOID(target);

    CopyTextSpanFontStyle(source, target);
    CopyTextSpanLineStyle(source, target, needLeadingMargin);
}

void RichEditorPattern::CopyTextSpanFontStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target)
{
    if (source->HasFontSize()) {
        target->UpdateFontSize(source->GetFontSizeValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }

    if (source->HasTextColor()) {
        target->UpdateTextColor(source->GetTextColorValue(Color::BLACK));
        target->AddPropertyInfo(PropertyInfo::FONTCOLOR);
    }

    if (source->HasItalicFontStyle()) {
        target->UpdateItalicFontStyle(source->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
        target->AddPropertyInfo(PropertyInfo::FONTSTYLE);
    }

    if (source->HasFontWeight()) {
        target->UpdateFontWeight(source->GetFontWeightValue(FontWeight::NORMAL));
        target->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
    }

    if (source->HasFontFamily()) {
        target->UpdateFontFamily(source->GetFontFamilyValue({ "HarmonyOS Sans" }));
        target->AddPropertyInfo(PropertyInfo::FONTFAMILY);
    }

    if (source->HasTextDecoration()) {
        target->UpdateTextDecoration(source->GetTextDecorationValue(TextDecoration::NONE));
        target->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
    }

    if (source->HasTextDecorationColor()) {
        target->UpdateTextDecorationColor(source->GetTextDecorationColorValue(Color::BLACK));
        target->AddPropertyInfo(PropertyInfo::NONE);
    }

    if (source->HasTextCase()) {
        target->UpdateTextCase(source->GetTextCaseValue(TextCase::NORMAL));
        target->AddPropertyInfo(PropertyInfo::TEXTCASE);
    }

    if (source->HasLineHeight()) {
        target->UpdateLineHeight(source->GetLineHeightValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::LINEHEIGHT);
    }

    if (source->HasLetterSpacing()) {
        target->UpdateLetterSpacing(source->GetLetterSpacingValue(Dimension()));
        target->AddPropertyInfo(PropertyInfo::LETTERSPACE);
    }

    if (source->HasFontFeature()) {
        target->UpdateFontFeature(source->GetFontFeatureValue(ParseFontFeatureSettings("\"pnum\" 1")));
        target->AddPropertyInfo(PropertyInfo::FONTFEATURE);
    }
}

void RichEditorPattern::CopyTextSpanLineStyle(
    RefPtr<SpanNode>& source, RefPtr<SpanNode>& target, bool needLeadingMargin)
{
    if (source->HasTextShadow()) {
        target->UpdateTextShadow(source->GetTextShadowValue({Shadow()}));
        target->AddPropertyInfo(PropertyInfo::TEXTSHADOW);
    }

    if (source->HasLeadingMargin()) {
        auto leadingMargin = source->GetLeadingMarginValue({});
        if (!needLeadingMargin) {
            leadingMargin.pixmap.Reset();
        }
        target->UpdateLeadingMargin(leadingMargin);
        target->AddPropertyInfo(PropertyInfo::LEADING_MARGIN);
    }

    if (source->HasTextAlign()) {
        target->UpdateTextAlign(source->GetTextAlignValue(TextAlign::LEFT));
        target->AddPropertyInfo(PropertyInfo::TEXT_ALIGN);
    }

    if (source->HasWordBreak()) {
        target->UpdateWordBreak(source->GetWordBreakValue(WordBreak::BREAK_WORD));
        target->AddPropertyInfo(PropertyInfo::WORD_BREAK);
    }

    if (source->HasLineBreakStrategy()) {
        target->UpdateLineBreakStrategy(source->GetLineBreakStrategyValue(LineBreakStrategy::GREEDY));
        target->AddPropertyInfo(PropertyInfo::LINE_BREAK_STRATEGY);
    }
}

int32_t RichEditorPattern::TextSpanSplit(int32_t position, bool needLeadingMargin)
{
    if (spans_.empty()) {
        return -1;
    }

    auto positionInfo = GetSpanPositionInfo(position);
    int32_t spanIndex = positionInfo.spanIndex_;
    int32_t spanStart = positionInfo.spanStart_;
    int32_t offsetInSpan = positionInfo.spanOffset_;
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT,
        "position=%{public}d, spanIndex=%{public}d, spanStart=%{public}d, offsetInSpan=%{public}d",
        position, spanIndex, spanStart, offsetInSpan);

    if (offsetInSpan <= 0) {
        return spanIndex;
    }

    auto host = GetHost();
    CHECK_NULL_RETURN(host, -1);
    auto it = host->GetChildren().begin();
    std::advance(it, spanIndex);

    auto spanNode = DynamicCast<SpanNode>(*it);
    CHECK_NULL_RETURN(spanNode, -1);
    auto spanItem = spanNode->GetSpanItem();
    auto spanItemContent = StringUtils::ToWstring(spanItem->content);
    if (offsetInSpan > static_cast<int32_t>(spanItemContent.length())) {
        offsetInSpan = static_cast<int32_t>(spanItemContent.length());
    }
    auto newContent = spanItemContent.substr(offsetInSpan);
    auto deleteContent = spanItemContent.substr(0, offsetInSpan);

    auto* stack = ViewStackProcessor::GetInstance();
    CHECK_NULL_RETURN(stack, -1);
    auto nodeId = stack->ClaimNodeId();
    auto newSpanNode = SpanNode::GetOrCreateSpanNode(nodeId);
    CHECK_NULL_RETURN(newSpanNode, -1);

    auto newSpanItem = newSpanNode->GetSpanItem();
    newSpanItem->position = spanStart + offsetInSpan;
    auto spanIter = spans_.begin();
    std::advance(spanIter, spanIndex);
    spans_.insert(spanIter, newSpanItem);

    spanNode->UpdateContent(StringUtils::ToString(newContent));
    newSpanNode->UpdateContent(StringUtils::ToString(deleteContent));

    CopyTextSpanStyle(spanNode, newSpanNode, needLeadingMargin);
    newSpanNode->MountToParent(host, spanIndex);

    return spanIndex + 1;
}

int32_t RichEditorPattern::GetCaretPosition()
{
    return caretPosition_;
}

bool RichEditorPattern::SetCaretOffset(int32_t caretPosition)
{
    CHECK_NULL_RETURN(!IsPreviewTextInputting(), false);
    bool success = false;
    AdjustSelector(caretPosition, false);
    success = SetCaretPosition(caretPosition);
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "caretPosition=%{public}d, success=%{public}d", caretPosition, success);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    if (focusHub->IsCurrentFocus()) {
        StartTwinkling();
    }
    CloseSelectOverlay();
    ResetSelection();
    return success;
}

OffsetF RichEditorPattern::CalcCursorOffsetByPosition(
    int32_t position, float& selectLineHeight, bool downStreamFirst, bool needLineHighest)
{
    selectLineHeight = 0.0f;
    auto host = GetHost();
    CHECK_NULL_RETURN(host, OffsetF(0, 0));
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, OffsetF(0, 0));
    auto rootOffset = pipeline->GetRootRect().GetOffset();
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0f, std::min(baselineOffset_, 0.0f));
    auto startOffset = paragraphs_.ComputeCursorOffset(position, selectLineHeight, downStreamFirst, needLineHighest);
    auto children = host->GetChildren();
    if (NearZero(selectLineHeight)) {
        if (children.empty() || GetTextContentLength() == 0) {
            return textPaintOffset - rootOffset;
        }
        if (std::all_of(children.begin(), children.end(), [](RefPtr<UINode>& node) {
                CHECK_NULL_RETURN(node, false);
                return (node->GetTag() == V2::IMAGE_ETS_TAG || node->GetTag() == V2::PLACEHOLDER_SPAN_ETS_TAG);
            })) {
            bool isTail = false;
            auto it = children.begin();
            if (position >= static_cast<int32_t>(children.size())) {
                std::advance(it, (static_cast<int32_t>(children.size()) - 1));
                isTail = true;
            } else {
                std::advance(it, position);
            }
            if (it == children.end()) {
                return startOffset;
            }
            auto imageNode = DynamicCast<FrameNode>(*it);
            if (imageNode) {
                auto geometryNode = imageNode->GetGeometryNode();
                CHECK_NULL_RETURN(geometryNode, OffsetF(0.0f, 0.0f));
                startOffset = geometryNode->GetMarginFrameOffset();
                selectLineHeight = geometryNode->GetMarginFrameSize().Height();
                startOffset += isTail ? OffsetF(geometryNode->GetMarginFrameSize().Width(), 0.0f) : OffsetF(0.0f, 0.0f);
            }
            return startOffset;
        }
    }
    auto caretOffset = startOffset + textPaintOffset + rootOffset;
    auto geometryNode = host->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, caretOffset);
    auto frameSize = geometryNode->GetFrameRect().GetSize();
    CHECK_NULL_RETURN(overlayMod_, caretOffset);
    float caretWidth = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretWidth();
    caretOffset.SetX(std::clamp(caretOffset.GetX(), 0.0f, static_cast<float>(frameSize.Width()) - caretWidth));
    return caretOffset;
}

bool RichEditorPattern::SetCaretPosition(int32_t pos)
{
    auto correctPos = std::clamp(pos, 0, GetTextContentLength());
    ResetLastClickOffset();
    if (pos == correctPos) {
        FireOnSelectionChange(correctPos);
        if (caretPosition_ != correctPos) {
            TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "caret:%{public}d->%{public}d", caretPosition_, correctPos);
            if (caretChangeListener_) {
                caretChangeListener_(correctPos);
            }
        }
        caretPosition_ = correctPos;
        UpdateCaretInfoToController();
        return true;
    }
    return false;
}

void RichEditorPattern::FireOnSelectionChange(const int32_t caretPosition)
{
    if (!textSelector_.SelectNothing() || caretPosition == caretPosition_ || !caretTwinkling_) {
        return;
    }
    FireOnSelectionChange(caretPosition, caretPosition);
}

void RichEditorPattern::FireOnSelectionChange(const TextSelector& selector)
{
    if (selector.SelectNothing()) {
        return;
    }
    FireOnSelectionChange(selector.GetStart(), selector.GetEnd());
}

void RichEditorPattern::FireOnSelectionChange(int32_t start, int32_t end)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    CHECK_NULL_VOID(HasFocus());
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "range=[%{public}d,%{public}d], caretTwinkling_=%{public}d",
        start, end, caretTwinkling_);
    if (start < 0 || end < 0) {
        return;
    }
    if (start == end && !caretTwinkling_) {
        return;
    }
    if (start > end) {
        std::swap(start, end);
    }
    auto range = SelectionRangeInfo(start, end);
    if (range == lastSelectionRange_) {
        return;
    }
    eventHub->FireOnSelectionChange(&range);
    lastSelectionRange_ = std::move(range);
}

bool RichEditorPattern::GetCaretVisible() const
{
    return caretVisible_;
}

void RichEditorPattern::SetUpdateSpanStyle(struct UpdateSpanStyle updateSpanStyle)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "SetUpdateSpanStyle");
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "updateSpanStyle=%{public}s", updateSpanStyle.ToString().c_str());
    updateSpanStyle_ = updateSpanStyle;
}

void RichEditorPattern::SetTypingStyle(struct UpdateSpanStyle typingStyle, TextStyle textStyle)
{
    typingStyle_ = typingStyle;
    typingTextStyle_ = textStyle;
}

void RichEditorPattern::UpdateFontFeatureTextStyle(
    RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle& updateSpanStyle, TextStyle& textStyle)
{
    if (updateSpanStyle.updateFontFeature.has_value()) {
        spanNode->UpdateFontFeature(textStyle.GetFontFeatures());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFEATURE);
    }
}

void RichEditorPattern::UpdateTextStyle(
    RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle)
{
    if (spanNode->GetTag() != V2::SPAN_ETS_TAG || updateSpanStyle_.isSymbolStyle) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    UpdateFontFeatureTextStyle(spanNode, updateSpanStyle, textStyle);
    if (updateSpanStyle.updateTextColor.has_value()) {
        spanNode->UpdateTextColor(textStyle.GetTextColor());
        spanNode->AddPropertyInfo(PropertyInfo::FONTCOLOR);
    }
    if (updateSpanStyle.updateLineHeight.has_value()) {
        spanNode->UpdateLineHeight(textStyle.GetLineHeight());
        spanNode->AddPropertyInfo(PropertyInfo::LINEHEIGHT);
    }
    if (updateSpanStyle.updateLetterSpacing.has_value()) {
        spanNode->UpdateLetterSpacing(textStyle.GetLetterSpacing());
        spanNode->AddPropertyInfo(PropertyInfo::LETTERSPACE);
    }
    if (updateSpanStyle.updateFontSize.has_value()) {
        spanNode->UpdateFontSize(textStyle.GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }
    if (updateSpanStyle.updateItalicFontStyle.has_value()) {
        spanNode->UpdateItalicFontStyle(textStyle.GetFontStyle());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSTYLE);
    }
    if (updateSpanStyle.updateFontWeight.has_value()) {
        spanNode->UpdateFontWeight(textStyle.GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
    }
    if (updateSpanStyle.updateFontFamily.has_value()) {
        spanNode->UpdateFontFamily(textStyle.GetFontFamilies());
        spanNode->AddPropertyInfo(PropertyInfo::FONTFAMILY);
    }
    if (updateSpanStyle.updateTextDecoration.has_value()) {
        spanNode->UpdateTextDecoration(textStyle.GetTextDecoration());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTDECORATION);
    }
    if (updateSpanStyle.updateTextDecorationColor.has_value()) {
        spanNode->UpdateTextDecorationColor(textStyle.GetTextDecorationColor());
        spanNode->AddPropertyInfo(PropertyInfo::NONE);
    }
    if (updateSpanStyle.updateTextShadows.has_value()) {
        spanNode->UpdateTextShadow(textStyle.GetTextShadows());
        spanNode->AddPropertyInfo(PropertyInfo::TEXTSHADOW);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

void RichEditorPattern::UpdateSymbolStyle(
    RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle)
{
    if (spanNode->GetTag() != V2::SYMBOL_SPAN_ETS_TAG || !updateSpanStyle_.isSymbolStyle) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    UpdateFontFeatureTextStyle(spanNode, updateSpanStyle, textStyle);
    if (updateSpanStyle.updateFontSize.has_value()) {
        spanNode->UpdateFontSize(textStyle.GetFontSize());
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
    }
    if (updateSpanStyle.updateLineHeight.has_value()) {
        spanNode->UpdateLineHeight(textStyle.GetLineHeight());
        spanNode->AddPropertyInfo(PropertyInfo::LINEHEIGHT);
    }
    if (updateSpanStyle.updateLetterSpacing.has_value()) {
        spanNode->UpdateLetterSpacing(textStyle.GetLetterSpacing());
        spanNode->AddPropertyInfo(PropertyInfo::LETTERSPACE);
    }
    if (updateSpanStyle.updateFontWeight.has_value()) {
        spanNode->UpdateFontWeight(textStyle.GetFontWeight());
        spanNode->AddPropertyInfo(PropertyInfo::FONTWEIGHT);
    }
    if (updateSpanStyle.updateSymbolColor.has_value()) {
        spanNode->UpdateSymbolColorList(textStyle.GetSymbolColorList());
        spanNode->AddPropertyInfo(PropertyInfo::SYMBOL_COLOR);
    }
    if (updateSpanStyle.updateSymbolRenderingStrategy.has_value()) {
        spanNode->UpdateSymbolRenderingStrategy(textStyle.GetRenderStrategy());
        spanNode->AddPropertyInfo(PropertyInfo::SYMBOL_RENDERING_STRATEGY);
    }
    if (updateSpanStyle.updateSymbolEffectStrategy.has_value()) {
        spanNode->UpdateSymbolEffectStrategy(textStyle.GetEffectStrategy());
        spanNode->AddPropertyInfo(PropertyInfo::SYMBOL_EFFECT_STRATEGY);
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

bool RichEditorPattern::HasSameTypingStyle(const RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_RETURN(spanItem, false);
    auto spanTextStyle = spanItem->GetTextStyle();
    if (spanTextStyle.has_value() && typingTextStyle_.has_value()) {
        return spanTextStyle.value() == typingTextStyle_.value();
    } else {
        return !(spanTextStyle.has_value() || typingTextStyle_.has_value());
    }
}

void RichEditorPattern::UpdateImageStyle(RefPtr<FrameNode>& imageNode, const ImageSpanAttribute& imageStyle)
{
    CHECK_NULL_VOID(imageNode);
    CHECK_NULL_VOID(imageNode->GetTag() == V2::IMAGE_ETS_TAG);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto imageLayoutProperty = imageNode->GetLayoutProperty<ImageLayoutProperty>();
    CHECK_NULL_VOID(imageLayoutProperty);
    if (updateSpanStyle_.updateImageWidth.has_value() || updateSpanStyle_.updateImageHeight.has_value()) {
        imageLayoutProperty->UpdateUserDefinedIdealSize(imageStyle.size->GetSize());
    }
    if (updateSpanStyle_.updateImageFit.has_value()) {
        imageLayoutProperty->UpdateImageFit(imageStyle.objectFit.value());
    }
    if (updateSpanStyle_.updateImageVerticalAlign.has_value()) {
        imageLayoutProperty->UpdateVerticalAlign(imageStyle.verticalAlign.value());
    }
    if (updateSpanStyle_.borderRadius.has_value()) {
        auto imageRenderCtx = imageNode->GetRenderContext();
        imageRenderCtx->UpdateBorderRadius(imageStyle.borderRadius.value());
        imageRenderCtx->SetClipToBounds(true);
    }
    if (updateSpanStyle_.marginProp.has_value()) {
        imageLayoutProperty->UpdateMargin(imageStyle.marginProp.value());
    }

    imageNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    imageNode->MarkModifyDone();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    host->MarkModifyDone();
}

bool RichEditorPattern::SymbolSpanUpdateStyle(
    RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle)
{
    if (spanNode->GetTag() == V2::SYMBOL_SPAN_ETS_TAG) {
        UpdateSymbolStyle(spanNode, updateSpanStyle_, textStyle);
        return true;
    }
    return false;
}

void RichEditorPattern::UpdateSpanStyle(
    int32_t start, int32_t end, const TextStyle& textStyle, const ImageSpanAttribute& imageStyle)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "range=[%{public}d,%{public}d]", start, end);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "textStyle=%{public}s, imageStyle=%{public}s",
        textStyle.ToString().c_str(), imageStyle.ToString().c_str());
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t spanStart = 0;
    int32_t spanEnd = 0;
    for (auto it = host->GetChildren().begin(); it != host->GetChildren().end(); ++it) {
        auto spanNode = DynamicCast<SpanNode>(*it);
        auto imageNode = DynamicCast<FrameNode>(*it);
        if (!spanNode) {
            if (spanEnd != 0) {
                spanStart = spanEnd;
            }
            spanEnd = spanStart + 1;
        } else {
            spanNode->GetSpanItem()->GetIndex(spanStart, spanEnd);
        }
        if (spanEnd < start) {
            continue;
        }

        if (spanStart >= start && spanEnd <= end) {
            if (spanNode) {
                UpdateSymbolStyle(spanNode, updateSpanStyle_, textStyle);
                UpdateTextStyle(spanNode, updateSpanStyle_, textStyle);
            } else {
                UpdateImageStyle(imageNode, imageStyle);
            }
            if (spanEnd == end) {
                break;
            }
        } else if (spanStart < start && start < spanEnd) {
            if (SymbolSpanUpdateStyle(spanNode, updateSpanStyle_, textStyle)) {
                continue;
            }
            TextSpanSplit(start, true);
            --it;
        } else if (spanStart < end && end < spanEnd) {
            if (SymbolSpanUpdateStyle(spanNode, updateSpanStyle_, textStyle)) {
                continue;
            }
            TextSpanSplit(end, true);
            --(--it);
        } else if (spanStart >= end) {
            break;
        }
    }
    CloseSystemMenu();
}

void RichEditorPattern::CloseSystemMenu()
{
    if (!SelectOverlayIsOn()) {
        return;
    }
    auto selectOverlayInfo = selectOverlay_->GetSelectOverlayInfo();
    if (selectOverlayInfo && !selectOverlayInfo->menuInfo.menuBuilder) {
        CloseSelectOverlay();
    }
}

void RichEditorPattern::SetAccessibilityAction()
{
    auto host = GetHost();
    auto textAccessibilityProperty = host->GetAccessibilityProperty<AccessibilityProperty>();
    CHECK_NULL_VOID(textAccessibilityProperty);
    textAccessibilityProperty->SetActionSetSelection(
        [weakPtr = WeakClaim(this)](int32_t start, int32_t end, bool isForward) {
            const auto& pattern = weakPtr.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->SetSelection(start, end, std::nullopt, isForward);
        });

    textAccessibilityProperty->SetActionSetIndex([weakPtr = WeakClaim(this)](int32_t index) {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->SetCaretPosition(index);
    });

    textAccessibilityProperty->SetActionGetIndex([weakPtr = WeakClaim(this)]() -> int32_t {
        const auto& pattern = weakPtr.Upgrade();
        CHECK_NULL_RETURN(pattern, -1);
        auto index = pattern->GetCaretPosition();
        return index;
    });
}

std::vector<ParagraphInfo> RichEditorPattern::GetParagraphInfo(int32_t start, int32_t end)
{
    std::vector<ParagraphInfo> res;
    auto spanNodes = GetParagraphNodes(start, end);
    CHECK_NULL_RETURN(!spanNodes.empty(), {});

    auto&& firstSpan = spanNodes.front()->GetSpanItem();
    auto paraStart = firstSpan->position - StringUtils::ToWstring(firstSpan->content).length();

    for (auto it = spanNodes.begin(); it != spanNodes.end(); ++it) {
        if (it == std::prev(spanNodes.end()) || StringUtils::ToWstring((*it)->GetSpanItem()->content).back() == L'\n') {
            ParagraphInfo info;
            auto lm = (*it)->GetLeadingMarginValue({});

            res.emplace_back(ParagraphInfo {
                .leadingMarginPixmap = lm.pixmap,
                .leadingMarginSize = { lm.size.Width().ToString(),
                    lm.size.Height().ToString() },
                .textAlign = static_cast<int32_t>((*it)->GetTextAlignValue(TextAlign::START)),
                .wordBreak = static_cast<int32_t>((*it)->GetWordBreakValue(WordBreak::BREAK_WORD)),
                .lineBreakStrategy = static_cast<int32_t>((*it)->GetLineBreakStrategyValue(LineBreakStrategy::GREEDY)),
                .range = { paraStart, (*it)->GetSpanItem()->position },
            });
            paraStart = (*it)->GetSpanItem()->position;
        }
    }

    return res;
}

int32_t RichEditorPattern::GetParagraphLength(const std::list<RefPtr<UINode>>& spans) const
{
    if (spans.empty()) {
        return 0;
    }
    int32_t imageSpanCnt = 0;
    for (auto it = spans.rbegin(); it != spans.rend(); ++it) {
        auto spanNode = DynamicCast<SpanNode>(*it);
        if (spanNode) {
            return spanNode->GetSpanItem()->position + imageSpanCnt;
        }
        ++imageSpanCnt;
    }
    return imageSpanCnt;
}

std::vector<RefPtr<SpanNode>> RichEditorPattern::GetParagraphNodes(int32_t start, int32_t end) const
{
    CHECK_NULL_RETURN(start != end, {});
    auto host = GetHost();
    CHECK_NULL_RETURN(host, {});
    CHECK_NULL_RETURN(!host->GetChildren().empty(), {});

    const auto& spans = host->GetChildren();
    int32_t length = GetParagraphLength(spans);
    std::vector<RefPtr<SpanNode>> res;

    if (start >= length) {
        return res;
    }

    auto headIt = spans.begin();
    auto flagNode = headIt;
    bool isEnd = false;
    int32_t spanEnd = -1;
    while (flagNode != spans.end()) {
        auto spanNode = DynamicCast<SpanNode>(*flagNode);
        if (spanNode) {
            auto&& info = spanNode->GetSpanItem();
            spanEnd = info->position;
            isEnd = StringUtils::ToWstring(info->content).back() == '\n';
        } else {
            ++spanEnd;
            isEnd = false;
        }
        flagNode++;
        if (spanEnd > start) {
            break;
        }
        if (isEnd) {
            headIt = flagNode;
        }
    }
    while (headIt != flagNode) {
        auto spanNode = DynamicCast<SpanNode>(*headIt);
        if (spanNode) {
            res.emplace_back(spanNode);
        }
        headIt++;
    }
    while (flagNode != spans.end() && (spanEnd < end || !isEnd)) {
        auto spanNode = DynamicCast<SpanNode>(*flagNode);
        if (spanNode) {
            res.emplace_back(spanNode);
            auto&& info = spanNode->GetSpanItem();
            spanEnd = info->position;
            isEnd = StringUtils::ToWstring(info->content).back() == '\n';
        } else {
            ++spanEnd;
            isEnd = false;
        }
        flagNode++;
    }

    return res;
}

void RichEditorPattern::UpdateParagraphStyle(int32_t start, int32_t end, const struct UpdateParagraphStyle& style)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "range=[%{public}d,%{public}d]", start, end);
    auto spanNodes = GetParagraphNodes(start, end);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "spanNode cnt=%{public}d, style=%{public}s",
        static_cast<int32_t>(spanNodes.size()), style.ToString().c_str());
    for (const auto& spanNode : spanNodes) {
        UpdateParagraphStyle(spanNode, style);
    }
}

void RichEditorPattern::UpdateParagraphStyle(RefPtr<SpanNode> spanNode, const struct UpdateParagraphStyle& style)
{
    CHECK_NULL_VOID(spanNode);
    spanNode->UpdateTextAlign(style.textAlign.value_or(TextAlign::START));
    spanNode->UpdateWordBreak(style.wordBreak.value_or(WordBreak::BREAK_WORD));
    spanNode->UpdateLineBreakStrategy(style.lineBreakStrategy.value_or(LineBreakStrategy::GREEDY));
    if (style.leadingMargin.has_value()) {
        spanNode->GetSpanItem()->leadingMargin = *style.leadingMargin;
        spanNode->UpdateLeadingMargin(*style.leadingMargin);
    }
}

void RichEditorPattern::ScheduleCaretTwinkling()
{
    ContainerScope scope(richEditorInstanceId_);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);

    if (!context->GetTaskExecutor()) {
        return;
    }

    if (isCursorAlwaysDisplayed_) {
        return;
    }

    auto weak = WeakClaim(this);
    caretTwinklingTask_.Reset([weak, instanceId = richEditorInstanceId_] {
        ContainerScope scope(instanceId);
        auto client = weak.Upgrade();
        CHECK_NULL_VOID(client);
        client->OnCaretTwinkling();
    });
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostDelayedTask(caretTwinklingTask_, TaskExecutor::TaskType::UI, RICH_EDITOR_TWINKLING_INTERVAL_MS,
        "ArkUIRichEditorScheduleCaretTwinkling");
}

void RichEditorPattern::StartTwinkling()
{
    caretTwinklingTask_.Cancel();
    // Fire on selecion change when caret invisible -> visible
    if (!caretTwinkling_) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "StartTwinkling");
        caretTwinkling_ = true;
        FireOnSelectionChange(caretPosition_, caretPosition_);
    }
    caretVisible_ = true;
    auto tmpHost = GetHost();
    CHECK_NULL_VOID(tmpHost);
    tmpHost->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    ScheduleCaretTwinkling();
}

void RichEditorPattern::OnCaretTwinkling()
{
    caretTwinklingTask_.Cancel();
    caretVisible_ = !caretVisible_;
    GetHost()->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    ScheduleCaretTwinkling();
}

void RichEditorPattern::StopTwinkling()
{
    if (caretTwinkling_) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "StopTwinkling");
    }
    caretTwinkling_ = false;
    caretTwinklingTask_.Cancel();
    if (caretVisible_) {
        caretVisible_ = false;
        GetHost()->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

void RichEditorPattern::HandleClickEvent(GestureEvent& info)
{
    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    if (!focusHub->IsFocusable()) {
        return;
    }
    selectionMenuOffsetClick_ = OffsetF(
        static_cast<float>(info.GetGlobalLocation().GetX()), static_cast<float>(info.GetGlobalLocation().GetY()));
    if (dataDetectorAdapter_->hasClickedAISpan_) {
        dataDetectorAdapter_->hasClickedAISpan_ = false;
    } else if (hasClicked_) {
        hasClicked_ = false;
        TimeStamp clickTimeStamp = info.GetTimeStamp();
        std::chrono::duration<float, std::ratio<1, InputAIChecker::SECONDS_TO_MILLISECONDS>> timeout =
            clickTimeStamp - lastClickTimeStamp_;
        lastClickTimeStamp_ = info.GetTimeStamp();
        if (timeout.count() < DOUBLE_CLICK_INTERVAL_MS) {
            HandleDoubleClickEvent(info);
            return;
        }
    }
    HandleSingleClickEvent(info);
}

void RichEditorPattern::HandleSingleClickEvent(OHOS::Ace::GestureEvent& info)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "handleSingleClick");
    hasClicked_ = true;
    lastClickTimeStamp_ = info.GetTimeStamp();
    if (info.GetSourceDevice() != SourceType::MOUSE && SelectOverlayIsOn() &&
        BetweenSelectedPosition(info.GetGlobalLocation())) {
        selectOverlay_->ProcessOverlay({.animation = true, .requestCode = REQUEST_RECREATE});
        return;
    }

    Offset textOffset = ConvertTouchOffsetToTextOffset(info.GetLocalLocation());
    if (!isMousePressed_) {
        HandleClickAISpanEvent(PointF(textOffset.GetX(), textOffset.GetY()));
    }
    if (dataDetectorAdapter_->hasClickedAISpan_ || dataDetectorAdapter_->pressedByLeftMouse_) {
        if (selectOverlay_->SelectOverlayIsOn()) {
            selectOverlay_->DisableMenu();
        }
        return;
    }

    HandleUserClickEvent(info);
    CHECK_NULL_VOID(!info.IsPreventDefault());

    if (textSelector_.IsValid() && !isMouseSelect_) {
        CloseSelectOverlay();
        ResetSelection();
    }

    caretUpdateType_ = CaretUpdateType::PRESSED;
    CHECK_NULL_VOID(overlayMod_);
    auto lastCaretPosition = caretPosition_;
    float caretHeight = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretHeight();
    OffsetF caretOffset = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretOffset();
    RectF lastCaretRect = RectF{ caretOffset.GetX() - caretHeight / 2, caretOffset.GetY(), caretHeight, caretHeight };
    bool isCaretTwinkling = caretTwinkling_;
    auto position = paragraphs_.GetIndex(textOffset);
    AdjustCursorPosition(position);
    auto focusHub = GetFocusHub();
    if (focusHub) {
        SetCaretPosition(position);
        if (focusHub->RequestFocusImmediately()) {
            StartTwinkling();
            RequestKeyboard(false, true, true);
            HandleOnEditChanged(true);
        }
    }
    UseHostToUpdateTextFieldManager();
    CalcCaretInfoByClick(info.GetLocalLocation());
    if (RepeatClickCaret(info.GetLocalLocation(), lastCaretPosition, lastCaretRect) && isCaretTwinkling) {
        CreateAndShowSingleHandle(info);
    }
}

Offset RichEditorPattern::ConvertTouchOffsetToTextOffset(const Offset& touchOffset)
{
    richTextRect_.SetTop(richTextRect_.GetY() - std::min(baselineOffset_, 0.0f));
    richTextRect_.SetHeight(richTextRect_.Height() - std::max(baselineOffset_, 0.0f));
    return touchOffset - Offset(richTextRect_.GetX(), richTextRect_.GetY());
}

bool RichEditorPattern::RepeatClickCaret(const Offset& offset, int32_t lastCaretPosition, const RectF& lastCaretRect)
{
    CHECK_NULL_RETURN(caretTwinkling_, false);
    if (lastCaretPosition != GetTextContentLength() && lastCaretPosition != 0) {
        Offset textOffset = ConvertTouchOffsetToTextOffset(offset);
        auto position = paragraphs_.GetIndex(textOffset);
        return lastCaretPosition == position && HasFocus();
    }
    return lastCaretRect.IsInRegion(PointF(offset.GetX(), offset.GetY()));
}

void RichEditorPattern::CreateAndShowSingleHandle(GestureEvent& info)
{
    if (info.GetSourceDevice() == SourceType::MOUSE) {
        return;
    }
    selectOverlay_->SetIsSingleHandle(true);
    isSingleHandle_ = true;
    textSelector_.Update(caretPosition_);
    CalculateHandleOffsetAndShowOverlay();
    ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
}

void RichEditorPattern::MoveCaretAndStartFocus(const Offset& textOffset)
{
    auto position = paragraphs_.GetIndex(textOffset);
    AdjustCursorPosition(position);

    auto focusHub = GetFocusHub();
    if (focusHub) {
        SetCaretPosition(position);
        if (focusHub->RequestFocusImmediately()) {
            StartTwinkling();
            if (overlayMod_) {
                RequestKeyboard(false, true, true);
            }
            magnifierController_->UpdateShowMagnifier();
            HandleOnEditChanged(true);
        }
    }
    UseHostToUpdateTextFieldManager();
}

void RichEditorPattern::HandleDoubleClickEvent(OHOS::Ace::GestureEvent& info)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "HandleDoubleClickEvent");
    caretUpdateType_ = CaretUpdateType::DOUBLE_CLICK;
    HandleDoubleClickOrLongPress(info);
    caretUpdateType_ = CaretUpdateType::NONE;
}

bool RichEditorPattern::HandleUserGestureEvent(
    GestureEvent& info, std::function<bool(RefPtr<SpanItem> item, GestureEvent& info)>&& gestureFunc)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "HandleUserGestureEvent");
    RectF textContentRect = contentRect_;
    textContentRect.SetTop(contentRect_.GetY() - std::min(baselineOffset_, 0.0f));
    textContentRect.SetHeight(contentRect_.Height() - std::max(baselineOffset_, 0.0f));
    if (!textContentRect.IsInRegion(PointF(info.GetLocalLocation().GetX(), info.GetLocalLocation().GetY())) ||
        spans_.empty()) {
        return false;
    }
    PointF textOffset = { info.GetLocalLocation().GetX() - GetTextRect().GetX(),
        info.GetLocalLocation().GetY() - GetTextRect().GetY() };
    int32_t start = 0;
    bool isParagraphHead = true;
    Offset paragraphOffset(0, 0);
    for (const auto& item : spans_) {
        if (!item) {
            continue;
        }
        std::vector<RectF> selectedRects = paragraphs_.GetRects(start, item->position);
        start = item->position;
        if (isParagraphHead && !selectedRects.empty()) {
            if (item->leadingMargin.has_value()) {
                auto addWidth = item->leadingMargin.value().size.Width();
                selectedRects[0].SetLeft(selectedRects[0].GetX() - addWidth.ConvertToPx());
                selectedRects[0].SetWidth(selectedRects[0].GetSize().Width() + addWidth.ConvertToPx());
            }
            paragraphOffset.SetX(selectedRects[0].GetOffset().GetX());
            paragraphOffset.SetY(selectedRects[0].GetOffset().GetY());
            isParagraphHead = false;
        }
        if (!isParagraphHead && item->content.back() == '\n') {
            isParagraphHead = true;
        }
        for (auto&& rect : selectedRects) {
            if (!rect.IsInRegion(textOffset)) {
                continue;
            }
            info = info.SetScreenLocation(
                Offset(textOffset.GetX() - paragraphOffset.GetX(), textOffset.GetY() - paragraphOffset.GetY()));
            return gestureFunc(item, info);
        }
    }
    return false;
}

void RichEditorPattern::HandleOnlyImageSelected(const Offset& globalOffset, bool isFingerSelected)
{
    if (IsSelected() && (!isFingerSelected || IsHandlesShow())) {
        return;
    }
    auto textRect = GetTextRect();
    textRect.SetTop(textRect.GetY() - std::min(baselineOffset_, 0.0f));
    textRect.SetHeight(textRect.Height() - std::max(baselineOffset_, 0.0f));
    Offset offset = Offset(textRect.GetX(), textRect.GetY());
    auto textOffset = globalOffset - offset;
    int32_t currentPosition = paragraphs_.GetIndex(textOffset);
    currentPosition = std::min(currentPosition, GetTextContentLength());
    int32_t nextPosition = currentPosition + GetGraphemeClusterLength(GetWideText(), currentPosition);
    nextPosition = std::min(nextPosition, GetTextContentLength());
    AdjustPlaceholderSelection(currentPosition, nextPosition, textOffset);
    auto textSelectInfo = GetSpansInfo(currentPosition, nextPosition, GetSpansMethod::ONSELECT);
    auto results = textSelectInfo.GetSelection().resultObjects;
    if (results.size() == 1 && results.front().type == SelectSpanType::TYPEIMAGE && results.front().valueString != " "
        && !isOnlyImageDrag_) {
        textSelector_.Update(currentPosition, nextPosition);
        isOnlyImageDrag_ = true;
        showSelect_ = false;
        CalculateHandleOffsetAndShowOverlay();
        if (IsHandlesShow()) {
            TextPattern::CloseSelectOverlay(false);
        }
    }
}

bool RichEditorPattern::ClickAISpan(const PointF& textOffset, const AISpan& aiSpan)
{
    auto calculateHandleFunc = [weak = WeakClaim(this)]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->CalculateHandleOffsetAndShowOverlay();
    };
    auto showSelectOverlayFunc = [weak = WeakClaim(this)](const RectF& firstHandle, const RectF& secondHandle) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->SetCaretPosition(pattern->textSelector_.destinationOffset);
        auto focusHub = pattern->GetFocusHub();
        CHECK_NULL_VOID(focusHub);
        focusHub->RequestFocusImmediately();
        pattern->ShowSelectOverlay(firstHandle, secondHandle);
    };

    std::vector<RectF> aiRects = paragraphs_.GetRects(aiSpan.start, aiSpan.end);
    for (auto&& rect : aiRects) {
        if (rect.IsInRegion(textOffset)) {
            dataDetectorAdapter_->clickedAISpan_ = aiSpan;
            if (leftMousePress_) {
                dataDetectorAdapter_->pressedByLeftMouse_ = true;
                return true;
            }
            dataDetectorAdapter_->hasClickedAISpan_ = true;
            ShowUIExtensionMenu(aiSpan, calculateHandleFunc, showSelectOverlayFunc);
            return true;
        }
    }
    return false;
}

bool RichEditorPattern::HandleUserClickEvent(GestureEvent& info)
{
    auto clickFunc = [](RefPtr<SpanItem> item, GestureEvent& info) -> bool {
        if (item && item->onClick) {
            item->onClick(info);
            return true;
        }
        return false;
    };
    return HandleUserGestureEvent(info, std::move(clickFunc));
}

void RichEditorPattern::CalcCaretInfoByClick(const Offset& touchOffset)
{
    auto textRect = GetTextRect();
    textRect.SetTop(textRect.GetY() - std::min(baselineOffset_, 0.0f));
    textRect.SetHeight(textRect.Height() - std::max(baselineOffset_, 0.0f));
    Offset textOffset = { touchOffset.GetX() - textRect.GetX(), touchOffset.GetY() - textRect.GetY() };
    auto [lastClickOffset, caretHeight] = CalcAndRecordLastClickCaretInfo(textOffset);
    CHECK_NULL_VOID(overlayMod_);
    DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(lastClickOffset, caretHeight);

    MoveCaretToContentRect(lastClickOffset, caretHeight);
}

std::pair<OffsetF, float> RichEditorPattern::CalcAndRecordLastClickCaretInfo(const Offset& textOffset)
{
    // get the caret position
    auto position = paragraphs_.GetIndex(textOffset);
    // get the caret offset when click
    float caretHeight = 0.0f;
    auto lastClickOffset = paragraphs_.ComputeCursorInfoByClick(position, caretHeight,
        OffsetF(static_cast<float>(textOffset.GetX()), static_cast<float>(textOffset.GetY())));

    lastClickOffset += richTextRect_.GetOffset();
    if (isShowPlaceholder_) {
        lastClickOffset = CalculateEmptyValueCaretRect();
    }
    SetLastClickOffset(lastClickOffset);
    return std::make_pair(lastClickOffset, caretHeight);
}

void RichEditorPattern::InitClickEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!clickEventInitialized_);
    auto clickCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "click callback, sourceType=%{public}d", info.GetSourceDevice());
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->sourceType_ = info.GetSourceDevice();
        pattern->HandleClickEvent(info);
    };
    auto clickListener = MakeRefPtr<ClickEvent>(std::move(clickCallback));
    gestureHub->AddClickEvent(clickListener);
    clickEventInitialized_ = true;
}

void RichEditorPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    CHECK_NULL_VOID(!focusEventInitialized_);
    auto focusTask = [weak = WeakClaim(this)]() {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "rich editor in focus");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleFocusEvent();
    };
    focusHub->SetOnFocusInternal(focusTask);
    auto blurTask = [weak = WeakClaim(this)]() {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "rich editor in blur");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent();
    };
    focusHub->SetOnBlurInternal(blurTask);
    focusEventInitialized_ = true;
    auto keyTask = [weak = WeakClaim(this)](const KeyEvent& keyEvent) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->OnKeyEvent(keyEvent);
    };
    focusHub->SetOnKeyEventInternal(std::move(keyTask));
}

BlurReason RichEditorPattern::GetBlurReason()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, BlurReason::FOCUS_SWITCH);
    auto curFocusHub = host->GetFocusHub();
    CHECK_NULL_RETURN(curFocusHub, BlurReason::FOCUS_SWITCH);
    return curFocusHub->GetBlurReason();
}

void RichEditorPattern::HandleBlurEvent()
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "HandleBlurEvent");
    isLongPress_ = false;
    if (textDetectEnable_) {
        if (CanStartAITask()) {
            dataDetectorAdapter_->StartAITask();
        }
    }
    magnifierController_->UpdateShowMagnifier();
    StopTwinkling();
    auto reason = GetBlurReason();
    // The pattern handles blurevent, Need to close the softkeyboard first.
    if ((customKeyboardBuilder_ && isCustomKeyboardAttached_) || reason == BlurReason::FRAME_DESTROY) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "RichEditor Blur, Close Keyboard.");
        CloseKeyboard(true);
    }

    if (textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    if (reason != BlurReason::WINDOW_BLUR) {
        lastSelectionRange_.reset();
    }
    HandleOnEditChanged(false);
    FinishTextPreview();
}

void RichEditorPattern::HandleFocusEvent()
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "HandleFocusEvent");
    auto host = GetHost();
    if (host && textDetectEnable_ && !dataDetectorAdapter_->aiSpanMap_.empty()) {
        host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    }
    dataDetectorAdapter_->CancelAITask();
    UseHostToUpdateTextFieldManager();
    if (textSelector_.SelectNothing()) {
        StartTwinkling();
    }
    if (!usingMouseRightButton_ && !isLongPress_ && !isDragging_) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "Handle Focus Event, Request keyboard.");
        RequestKeyboard(false, true, true);
    }
    HandleOnEditChanged(true);
}

void RichEditorPattern::UseHostToUpdateTextFieldManager()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto globalOffset = host->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    UpdateTextFieldManager(Offset(globalOffset.GetX(), globalOffset.GetY()), frameRect_.Height());
}

void RichEditorPattern::OnVisibleChange(bool isVisible)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "isVisible=%{public}d", isVisible);
    TextPattern::OnVisibleChange(isVisible);
    StopTwinkling();
    CloseKeyboard(true);
}

bool RichEditorPattern::CloseKeyboard(bool forceClose)
{
    CloseSelectOverlay();
    ResetSelection();
    if (forceClose) {
        if (customKeyboardBuilder_ && isCustomKeyboardAttached_) {
            return CloseCustomKeyboard();
        }
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "Request close soft keyboard.");
#if defined(ENABLE_STANDARD_INPUT)
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        if (!imeAttached_) {
            return false;
        }
#endif
        auto inputMethod = MiscServices::InputMethodController::GetInstance();
        CHECK_NULL_RETURN(inputMethod, false);
        inputMethod->HideTextInput();
        inputMethod->Close();
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
        imeAttached_ = false;
#endif
#else
        if (HasConnection()) {
            connection_->Close(GetInstanceId());
            connection_ = nullptr;
        }
#endif
        return true;
    }
    return false;
}

void RichEditorPattern::HandleDraggableFlag(GestureEvent& info, bool& isInterceptEvent)
{
    auto gestureHub = GetGestureEventHub();
    if (isSpanStringMode_) {
        isInterceptEvent = false;
        if (gestureHub) {
            gestureHub->SetIsTextDraggable(false);
        }
        return;
    }
    if (gestureHub && copyOption_ != CopyOptions::None && BetweenSelectedPosition(info.GetGlobalLocation())) {
        dragBoxes_ = GetTextBoxes();
        // prevent long press event from being triggered when dragging
        bool isContentDraggalbe = JudgeContentDraggable();
        gestureHub->SetIsTextDraggable(isContentDraggalbe);
        isInterceptEvent = true;
    } else {
        isInterceptEvent = false;
        if (gestureHub) {
            gestureHub->SetIsTextDraggable(false);
        }
    }
}

bool RichEditorPattern::JudgeContentDraggable()
{
    if (!IsSelected() || copyOption_ == CopyOptions::None) {
        return false ;
    }
    auto selectInfo = GetSpansInfo(textSelector_.GetTextStart(), textSelector_.GetTextEnd(), GetSpansMethod::ONSELECT);
    auto selResult = selectInfo.GetSelection().resultObjects;
    auto iter = std::find_if(selResult.begin(), selResult.end(), [](ResultObject& obj) { return obj.isDraggable; });
    return iter != selResult.end();
}

std::pair<OffsetF, float> RichEditorPattern::CalculateCaretOffsetAndHeight()
{
    OffsetF caretOffset;
    float caretHeight = 0.0f;
    auto caretPosition = GetCaretPosition();
    float caretHeightUp = 0.0f;
    OffsetF caretOffsetUp = CalcCursorOffsetByPosition(caretPosition, caretHeightUp, false, false);
    if (GetTextContentLength() <= 0) {
        constexpr float DEFAULT_CARET_HEIGHT = 18.5f;
        caretOffset = CalculateEmptyValueCaretRect();
        caretHeight =
            isShowPlaceholder_ ? caretHeightUp : Dimension(DEFAULT_CARET_HEIGHT, DimensionUnit::VP).ConvertToPx();
        return std::make_pair(caretOffset, caretHeight);
    }
    float caretHeightDown = 0.0f;
    OffsetF caretOffsetDown = CalcCursorOffsetByPosition(caretPosition, caretHeightDown, true, false);
    bool isCaretPosInLineEnd = !NearEqual(caretOffsetDown.GetX(), caretOffsetUp.GetX(), 0.5f);
    bool isShowCaretDown = isCaretPosInLineEnd;
    if (!lastClickOffset_.IsNegative() && isCaretPosInLineEnd) {
        // show caret by click
        isShowCaretDown = NearEqual(lastClickOffset_.GetX(), caretOffsetDown.GetX());
    }
    caretOffset = isShowCaretDown ? caretOffsetDown : caretOffsetUp;
    caretHeight = isShowCaretDown ? caretHeightDown : caretHeightUp;
    CHECK_NULL_RETURN(overlayMod_, std::make_pair(caretOffset, caretHeight));
    auto overlayModifier = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    auto caretWidth = overlayModifier->GetCaretWidth();
    auto contentRect = GetTextContentRect();
    if (GreatOrEqual(caretOffset.GetX() + caretWidth, contentRect.Right())) {
        caretOffset.SetX(caretOffset.GetX() - caretWidth);
    }
    return std::make_pair(caretOffset, caretHeight);
}

OffsetF RichEditorPattern::CalculateEmptyValueCaretRect()
{
    OffsetF offset;
    auto layoutProperty = GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_RETURN(layoutProperty, offset);
    auto textAlign = layoutProperty->GetTextAlignValue(TextAlign::START);
    auto direction = layoutProperty->GetLayoutDirection();
    if (direction == TextDirection::RTL) {
        if (textAlign == TextAlign::START) {
            textAlign = TextAlign::END;
        } else {
            textAlign = TextAlign::START;
        }
    }
    switch (textAlign) {
        case TextAlign::START:
            offset.SetX(contentRect_.GetX());
            break;
        case TextAlign::CENTER:
            offset.SetX(contentRect_.GetX() + contentRect_.Width() / 2.0f);
            break;
        case TextAlign::END:
            offset.SetX(contentRect_.GetX() + contentRect_.Width());
            break;
        default:
            break;
    }
    offset.SetY(contentRect_.GetY());
    return offset;
}

void RichEditorPattern::HandleLongPress(GestureEvent& info)
{
    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    if (!focusHub->IsFocusable()) {
        return;
    }
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "HandleLongPress");
    caretUpdateType_ = CaretUpdateType::LONG_PRESSED;
    selectionMenuOffsetClick_ = OffsetF(
        static_cast<float>(info.GetGlobalLocation().GetX()), static_cast<float>(info.GetGlobalLocation().GetY()));
    HandleDoubleClickOrLongPress(info);
    caretUpdateType_ = CaretUpdateType::NONE;
}

void RichEditorPattern::HandleDoubleClickOrLongPress(GestureEvent& info)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "caretUpdateType=%{public}d", caretUpdateType_);
    if (IsPreviewTextInputting()) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "do not handle DoubleClickOrLongPress in previewTextInputting");
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    textResponseType_ = TextResponseType::LONG_PRESS;
    if (caretUpdateType_ == CaretUpdateType::LONG_PRESSED) {
        HandleUserLongPressEvent(info);
    }
    bool isDoubleClick = caretUpdateType_== CaretUpdateType::DOUBLE_CLICK;
    if (isDoubleClick && info.GetSourceTool() == SourceTool::FINGER && IsSelected()) {
        showSelect_ = true;
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle);
    }
    bool isInterceptEvent = false;
    HandleDraggableFlag(info, isInterceptEvent);
    // check current is mouse long press
    isInterceptEvent |= (isMousePressed_ && caretUpdateType_== CaretUpdateType::LONG_PRESSED);
    if (isInterceptEvent) {
        return;
    }
    HandleDoubleClickOrLongPress(info, host);
}

void RichEditorPattern::HandleDoubleClickOrLongPress(GestureEvent& info, RefPtr<FrameNode> host)
{
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    isLongPress_ = true;
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
        info.GetLocalLocation().GetY() - textPaintOffset.GetY() };
    InitSelection(textOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    if (IsSelected()) {
        showSelect_ = true;
    }
    FireOnSelect(selectStart, selectEnd);
    SetCaretPosition(std::min(selectEnd, GetTextContentLength()));
    MoveCaretToContentRect(false);
    CalculateHandleOffsetAndShowOverlay();
    if (IsShowSelectMenuUsingMouse()) {
        CloseSelectOverlay();
    }
    selectionMenuOffset_ = info.GetGlobalLocation();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    focusHub->RequestFocusImmediately();
    if (overlayMod_) {
        RequestKeyboard(false, true, true);
    }
    if (info.GetSourceDevice() != SourceType::MOUSE || caretUpdateType_ != CaretUpdateType::DOUBLE_CLICK) {
        int32_t requestCode = (selectOverlay_->SelectOverlayIsOn() && caretUpdateType_ == CaretUpdateType::LONG_PRESSED)
            ? REQUEST_RECREATE : 0;
        selectOverlay_->ProcessOverlay({.animation = true, .requestCode = requestCode});
        FireOnSelectionChange(selectStart, selectEnd);
        if (selectOverlay_->IsSingleHandle()) {
            StartTwinkling();
        }
    } else if (selectStart == selectEnd) {
        StartTwinkling();
    } else {
        StopTwinkling();
    }
    HandleOnEditChanged(true);
}

bool RichEditorPattern::HandleUserLongPressEvent(GestureEvent& info)
{
    auto longPressFunc = [](RefPtr<SpanItem> item, GestureEvent& info) -> bool {
        if (item && item->onLongPress) {
            item->onLongPress(info);
            return true;
        }
        return false;
    };
    return HandleUserGestureEvent(info, std::move(longPressFunc));
}

void RichEditorPattern::HandleMenuCallbackOnSelectAll()
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "HandleMenuCallbackOnSelectAll");
    auto textSize = static_cast<int32_t>(GetWideText().length()) + placeholderCount_;
    textSelector_.Update(0, textSize);
    CalculateHandleOffsetAndShowOverlay();
    if (selectOverlay_->IsUsingMouse()) {
        CloseSelectOverlay();
        StopTwinkling();
    }
    auto selectOverlayInfo = selectOverlay_->GetSelectOverlayInfo();
    textResponseType_ = selectOverlayInfo
                        ? static_cast<TextResponseType>(selectOverlayInfo->menuInfo.responseType.value_or(0))
                        : TextResponseType::LONG_PRESS;
    selectMenuInfo_.showCopyAll = false;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    FireOnSelect(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    showSelect_ = true;
    selectOverlay_->ProcessOverlay({.animation = true, .requestCode = REQUEST_RECREATE});
    SetCaretPosition(textSize);
    MoveCaretToContentRect();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    CHECK_NULL_VOID(!longPressEvent_);
    auto longPressCallback = [weak = WeakClaim(this)](GestureEvent& info) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "long press callback");
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->sourceType_ = info.GetSourceDevice();
        pattern->HandleLongPress(info);
    };
    longPressEvent_ = MakeRefPtr<LongPressEvent>(std::move(longPressCallback));
    gestureHub->SetLongPressEvent(longPressEvent_);

    auto onTextSelectorChange = [weak = WeakClaim(this), &selector = textSelector_]() {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!selector.SelectNothing()) {
            pattern->StopTwinkling();
        }
        pattern->FireOnSelectionChange(selector);
        auto frameNode = pattern->GetHost();
        CHECK_NULL_VOID(frameNode);
        frameNode->OnAccessibilityEvent(AccessibilityEventType::TEXT_SELECTION_UPDATE);
    };
    textSelector_.SetOnAccessibility(std::move(onTextSelectorChange));
}

void RichEditorPattern::UpdateSelector(int32_t start, int32_t end)
{
    AdjustSelector(start, end);
    textSelector_.Update(start, end);
}

void RichEditorPattern::AdjustSelector(int32_t& start, int32_t& end)
{
    AdjustSelector(start, true);
    AdjustSelector(end, false);
}

void RichEditorPattern::AdjustSelector(int32_t& index, bool isFirst)
{
    auto it = GetSpanIter(index);
    CHECK_NULL_VOID((it != spans_.end()));
    auto spanItem = *it;
    CHECK_NULL_VOID(spanItem);
    auto spanEnd = spanItem->position;
    auto spanStart = spanEnd - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length());
    if (spanItem->unicode != 0) {
        if (index > spanStart && index < spanEnd) {
            index = isFirst ? index - 1 : index + 1;
        }
        return;
    }
    int32_t emojiStartIndex;
    int32_t emojiEndIndex;
    bool isIndexInEmoji = TextEmojiProcessor::IsIndexInEmoji(index - spanStart, spanItem->content,
        emojiStartIndex, emojiEndIndex);
    if (isIndexInEmoji) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "AdjustSelector index=%{public}d isFirst=%{public}d"
            "startIndex=%{public}d endIndex=%{public}d",
            index, isFirst, emojiStartIndex, emojiEndIndex);
        index = isFirst ? emojiStartIndex : emojiEndIndex;
    }
}

std::list<RefPtr<SpanItem>>::iterator RichEditorPattern::GetSpanIter(int32_t index)
{
    return std::find_if(spans_.begin(), spans_.end(), [index](const RefPtr<SpanItem>& spanItem) {
        auto startIndex = spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length());
        auto endIndex = spanItem->position;
        return startIndex <= index && index < endIndex;
    });
}

void RichEditorPattern::InitDragDropEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->InitDragDropEvent();
    gestureHub->SetThumbnailCallback(GetThumbnailCallback());
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto onDragMove = [weakPtr = WeakClaim(this)](
                          const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& extraParams) {
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->showSelect_ = false;
        pattern->OnDragMove(event);
    };
    eventHub->SetOnDragMove(std::move(onDragMove));
    OnDragStartAndEnd();
    onDragDropAndLeave();
}

void RichEditorPattern::OnDragStartAndEnd()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto onDragStart = [weakPtr = WeakClaim(this)](const RefPtr<OHOS::Ace::DragEvent>& event,
                           const std::string& extraParams) -> NG::DragDropInfo {
        NG::DragDropInfo itemInfo;
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_RETURN(pattern, itemInfo);
        if (!pattern->isDragSponsor_) {
            pattern->isDragSponsor_ = true;
            pattern->dragRange_ = { pattern->textSelector_.GetTextStart(), pattern->textSelector_.GetTextEnd() };
        }
        pattern->timestamp_ = std::chrono::system_clock::now().time_since_epoch().count();
        auto eventHub = pattern->GetEventHub<RichEditorEventHub>();
        eventHub->SetTimestamp(pattern->GetTimestamp());
        CHECK_NULL_RETURN(eventHub, itemInfo);
        pattern->showSelect_ = false;
        return pattern->OnDragStart(event, extraParams);
    };
    eventHub->SetDefaultOnDragStart(std::move(onDragStart));
    auto onDragEnd = [weakPtr = WeakClaim(this), scopeId = Container::CurrentId()](
                         const RefPtr<OHOS::Ace::DragEvent>& event) {
        ContainerScope scope(scopeId);
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->isOnlyImageDrag_ = false;
        pattern->isDragSponsor_ = false;
        pattern->dragRange_ = { 0, 0 };
        pattern->showSelect_ = true;
        pattern->StopAutoScroll();
        pattern->ClearRedoOperationRecords();
        pattern->OnDragEnd(event);
    };
    eventHub->SetOnDragEnd(std::move(onDragEnd));
}

void RichEditorPattern::onDragDropAndLeave()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto onDragDrop = [weakPtr = WeakClaim(this), scopeId = Container::CurrentId()](
                          const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& value) {
        ContainerScope scope(scopeId);
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        auto host = pattern->GetHost();
        CHECK_NULL_VOID(host);
        auto eventHub = host->GetEventHub<EventHub>();
        CHECK_NULL_VOID(eventHub);
        if (!eventHub->IsEnabled()) {
            return;
        }
        pattern->status_ = Status::ON_DROP;
        pattern->HandleOnDragDrop(event);
        pattern->status_ = Status::NONE;
    };
    eventHub->SetOnDrop(std::move(onDragDrop));
    auto onDragDragLeave = [weakPtr = WeakClaim(this), scopeId = Container::CurrentId()](
                               const RefPtr<OHOS::Ace::DragEvent>& event, const std::string& value) {
        ContainerScope scope(scopeId);
        auto pattern = weakPtr.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->StopAutoScroll();
    };
    eventHub->SetOnDragLeave(onDragDragLeave);
}

void RichEditorPattern::ClearDragDropEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetIsTextDraggable(false);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetOnDragStart(nullptr);
    eventHub->SetOnDragEnter(nullptr);
    eventHub->SetOnDragMove(nullptr);
    eventHub->SetOnDragLeave(nullptr);
    eventHub->SetOnDragEnd(nullptr);
    eventHub->SetOnDrop(nullptr);
}

void RichEditorPattern::OnDragMove(const RefPtr<OHOS::Ace::DragEvent>& event)
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(theme);
    auto touchX = event->GetX();
    auto touchY = event->GetY();
    auto textRect = GetTextRect();
    textRect.SetTop(textRect.GetY() - std::min(baselineOffset_, 0.0f));
    Offset textOffset = { touchX - textRect.GetX() - GetParentGlobalOffset().GetX(),
        touchY - textRect.GetY() - GetParentGlobalOffset().GetY() - theme->GetInsertCursorOffset().ConvertToPx() };
    auto position = isShowPlaceholder_? 0 : paragraphs_.GetIndex(textOffset);
    SetCaretPosition(position);
    CalcAndRecordLastClickCaretInfo(textOffset);
    auto [caretOffset, caretHeight] = CalculateCaretOffsetAndHeight();
    CHECK_NULL_VOID(overlayMod_);
    auto overlayModifier = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    overlayModifier->SetCaretOffsetAndHeight(caretOffset, caretHeight);

    AutoScrollParam param = { .autoScrollEvent = AutoScrollEvent::DRAG, .showScrollbar = true };
    auto localOffset = OffsetF(touchX, touchY) - parentGlobalOffset_;
    AutoScrollByEdgeDetection(param, localOffset, EdgeDetectionStrategy::IN_BOUNDARY);
}

void RichEditorPattern::OnDragEnd(const RefPtr<Ace::DragEvent>& event)
{
    ResetDragRecordSize(-1);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (status_ == Status::DRAGGING) {
        status_ = Status::NONE;
    }
    std::unordered_set<int32_t> imgNodeIds;
    std::for_each(dragSpanItems_.begin(), dragSpanItems_.end(), [&imgNodeIds](RefPtr<SpanItem>& item) {
        CHECK_NULL_VOID(item);
        item->EndDrag();
        auto imageSpanItem = DynamicCast<ImageSpanItem>(item);
        CHECK_NULL_VOID(imageSpanItem);
        imgNodeIds.emplace(imageSpanItem->imageNodeId);
    });
    const auto& childrens = host->GetChildren();
    for (const auto& child : childrens) {
        auto findResult = imgNodeIds.find(child->GetId());
        if (findResult == imgNodeIds.end()) {
            continue;
        }
        auto imageNode = DynamicCast<ImageSpanNode>(child);
        if (!imageNode) {
            continue;
        }
        auto renderContext = imageNode->GetRenderContext();
        if (!renderContext) {
            continue;
        }
        renderContext->UpdateOpacity(1);
        imageNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
    dragSpanItems_.clear();
    if (recoverDragResultObjects_.empty()) {
        return;
    }
    UpdateSpanItemDragStatus(recoverDragResultObjects_, false);
    recoverDragResultObjects_.clear();
    if (event && event->GetResult() != DragRet::DRAG_SUCCESS) {
        HandleSelectionChange(recoverStart_, recoverEnd_);
        showSelect_ = true;
        isShowMenu_ = false;
        CalculateHandleOffsetAndShowOverlay();
        ResetSelection();
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

bool RichEditorPattern::SelectOverlayIsOn()
{
    return  selectOverlay_->SelectOverlayIsOn();
}

void RichEditorPattern::UpdateEditingValue(const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent)
{
    InsertValue(value->text);
}

void RichEditorPattern::InitMouseEvent()
{
    CHECK_NULL_VOID(!mouseEventInitialized_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(eventHub);
    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->sourceType_ = info.GetSourceDevice();
        pattern->HandleMouseEvent(info);
    };
    auto mouseEvent = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent);
    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "on hover event isHover=%{public}d", isHover);
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->OnHover(isHover);
        }
    };
    auto hoverEvent = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent);
    mouseEventInitialized_ = true;
}

void RichEditorPattern::OnHover(bool isHover)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "isHover=%{public}d", isHover);
    auto frame = GetHost();
    CHECK_NULL_VOID(frame);
    auto frameId = frame->GetId();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto scrollBar = GetScrollBar();
    if (isHover && !scrollBar->IsPressed()) {
        pipeline->SetMouseStyleHoldNode(frameId);
        pipeline->ChangeMouseStyle(frameId, MouseFormat::TEXT_CURSOR);
    } else {
        pipeline->ChangeMouseStyle(frameId, MouseFormat::DEFAULT);
        pipeline->FreeMouseStyleHoldNode(frameId);
    }
}

bool RichEditorPattern::RequestKeyboard(bool isFocusViewChanged, bool needStartTwinkling, bool needShowSoftKeyboard)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    CHECK_NULL_RETURN(needShowSoftKeyboard, false);
    if (needShowSoftKeyboard && customKeyboardBuilder_) {
        return RequestCustomKeyboard();
    }
#if defined(ENABLE_STANDARD_INPUT)
    if (!EnableStandardInput(needShowSoftKeyboard)) {
        return false;
    }
#else
    if (!UnableStandardInput(isFocusViewChanged)) {
        return false;
    }
#endif
    return true;
}

#if defined(ENABLE_STANDARD_INPUT)
#ifdef WINDOW_SCENE_SUPPORTED
uint32_t RichEditorPattern::GetSCBSystemWindowId()
{
    RefPtr<FrameNode> frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, {});
    auto focusSystemWindowId = WindowSceneHelper::GetFocusSystemWindowId(frameNode);
    return focusSystemWindowId;
}
#endif

bool RichEditorPattern::EnableStandardInput(bool needShowSoftKeyboard)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    MiscServices::Configuration configuration;
    configuration.SetEnterKeyType(static_cast<MiscServices::EnterKeyType>(
        static_cast<int32_t>(GetTextInputActionValue(GetDefaultTextInputAction()))));
    configuration.SetTextInputType(
        static_cast<MiscServices::TextInputType>(static_cast<int32_t>(TextInputType::UNSPECIFIED)));
    MiscServices::InputMethodController::GetInstance()->OnConfigurationChange(configuration);
    if (richEditTextChangeListener_ == nullptr) {
        richEditTextChangeListener_ = new OnTextChangedListenerImpl(WeakClaim(this));
    }
    auto inputMethod = MiscServices::InputMethodController::GetInstance();
    CHECK_NULL_RETURN(inputMethod, false);
    auto miscTextConfig = GetMiscTextConfig();
    CHECK_NULL_RETURN(miscTextConfig.has_value(), false);
    TAG_LOGD(
        AceLogTag::ACE_RICH_TEXT, "RequestKeyboard set calling window id is : %{public}u", miscTextConfig->windowId);
    MiscServices::TextConfig textconfig = miscTextConfig.value();
#ifdef WINDOW_SCENE_SUPPORTED
    auto systemWindowId = GetSCBSystemWindowId();
    if (systemWindowId) {
        miscTextConfig->windowId = systemWindowId;
    }
#endif
    inputMethod->Attach(richEditTextChangeListener_, needShowSoftKeyboard, textconfig);
    if (context) {
        inputMethod->SetCallingWindow(context->GetWindowId());
    }
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    imeAttached_ = true;
#endif
    return true;
}

std::optional<MiscServices::TextConfig> RichEditorPattern::GetMiscTextConfig()
{
    auto tmpHost = GetHost();
    CHECK_NULL_RETURN(tmpHost, {});
    auto pipeline = tmpHost->GetContextRefPtr();
    CHECK_NULL_RETURN(pipeline, {});
    auto windowRect = pipeline->GetCurrentWindowRect();
    double positionY = (tmpHost->GetPaintRectOffset() - pipeline->GetRootRect().GetOffset()).GetY() + windowRect.Top();
    double height = frameRect_.Height();

    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    if (NearZero(caretHeight)) {
        auto overlayModifier = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
        caretHeight = overlayModifier ? overlayModifier->GetCaretHeight() : DEFAULT_CARET_HEIGHT;
    }
    auto offset = KEYBOARD_AVOID_OFFSET.ConvertToPx();
    auto caretTop = caretOffset.GetY() + windowRect.Top() + parentGlobalOffset_.GetY();
    height = caretTop + caretHeight + offset - positionY;

    MiscServices::CursorInfo cursorInfo { .left = caretOffset.GetX() + windowRect.Left() + parentGlobalOffset_.GetX(),
        .top = caretTop,
        .width = CARET_WIDTH,
        .height = caretHeight };
    MiscServices::InputAttribute inputAttribute = { .inputPattern = (int32_t)TextInputType::UNSPECIFIED,
        .enterKeyType = (int32_t)GetTextInputActionValue(GetDefaultTextInputAction()),
        .isTextPreviewSupported = isTextPreviewSupported_ };
    auto start = textSelector_.IsValid() ? textSelector_.GetStart() : caretPosition_;
    auto end = textSelector_.IsValid() ? textSelector_.GetEnd() : caretPosition_;
    MiscServices::TextConfig textConfig = { .inputAttribute = inputAttribute,
        .cursorInfo = cursorInfo,
        .range = { .start = start, .end = end },
        .windowId = pipeline->GetFocusWindowId(),
        .positionY = positionY,
        .height = height };
    return textConfig;
}
#else
bool RichEditorPattern::UnableStandardInput(bool isFocusViewChanged)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    if (HasConnection()) {
        connection_->Show(isFocusViewChanged, GetInstanceId());
        return true;
    }
    TextInputConfiguration config;
    config.type = TextInputType::UNSPECIFIED;
    config.action = TextInputAction::DONE;
    config.obscureText = false;
    connection_ =
        TextInputProxy::GetInstance().Attach(WeakClaim(this), config, context->GetTaskExecutor(), GetInstanceId());
    if (!HasConnection()) {
        return false;
    }
    TextEditingValue value;
    if (spans_.empty()) {
        value.text = textForDisplay_;
    } else {
        for (auto it = spans_.begin(); it != spans_.end(); it++) {
            if ((*it)->placeholderIndex < 0) {
                value.text.append((*it)->content);
            } else {
                value.text.append(" ");
            }
        }
    }
    value.selection.Update(caretPosition_, caretPosition_);
    connection_->SetEditingState(value, GetInstanceId());
    connection_->Show(isFocusViewChanged, GetInstanceId());
    return true;
}
#endif

void RichEditorPattern::OnColorConfigurationUpdate()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    const auto& spans = host->GetChildren();
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto theme = context->GetTheme<TextTheme>();
    CHECK_NULL_VOID(theme);
    auto textLayoutProperty = GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(textLayoutProperty);
    const Color& themeTextColor = theme->GetTextStyle().GetTextColor();
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "themeTextColor=%{public}s", themeTextColor.ToString().c_str());
    textLayoutProperty->UpdateTextColor(themeTextColor);
    textLayoutProperty->UpdateTextDecorationColor(themeTextColor);
    for (auto span : spans) {
        auto spanNode = DynamicCast<SpanNode>(span);
        if (!spanNode) {
            continue;
        }
        auto spanItem = spanNode->GetSpanItem();
        if (!spanItem) {
            continue;
        }
        if (spanItem->hasResourceFontColor) {
            spanNode->UpdateTextColor(theme->GetTextStyle().GetTextColor());
        }
        if (spanItem->hasResourceDecorationColor) {
            spanNode->UpdateTextDecorationColor(theme->GetTextStyle().GetTextColor());
        }
    }
}

void RichEditorPattern::UpdateCaretInfoToController()
{
    CHECK_NULL_VOID(HasFocus());
    auto selectionResult = GetSpansInfo(0, GetTextContentLength(), GetSpansMethod::ONSELECT);
    auto resultObjects = selectionResult.GetSelection().resultObjects;
    std::string text = "";
    for (const auto& resultObj : resultObjects) {
        text += resultObj.type == SelectSpanType::TYPESPAN ? resultObj.valueString : " ";
    }
    auto start = textSelector_.IsValid() ? textSelector_.GetStart() : caretPosition_;
    auto end = textSelector_.IsValid() ? textSelector_.GetEnd() : caretPosition_;
#if defined(ENABLE_STANDARD_INPUT)
    auto miscTextConfig = GetMiscTextConfig();
    CHECK_NULL_VOID(miscTextConfig.has_value());
    MiscServices::CursorInfo cursorInfo = miscTextConfig.value().cursorInfo;
    MiscServices::InputMethodController::GetInstance()->OnCursorUpdate(cursorInfo);
    MiscServices::InputMethodController::GetInstance()->OnSelectionChange(
        StringUtils::Str8ToStr16(text), start, end);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT,
        "Caret update, pos=[%{public}.2f,%{public}.2f], size=[%{public}.2f,%{public}.2f], "
        "textSelector=[%{public}d,%{public}d]",
        cursorInfo.left, cursorInfo.top, cursorInfo.width, cursorInfo.height, textSelector_.GetStart(),
        textSelector_.GetEnd());
#else
    if (HasConnection()) {
        TextEditingValue editingValue;
        editingValue.text = text;
        editingValue.hint = "";
        editingValue.selection.Update(start, end);
        connection_->SetEditingState(editingValue, GetInstanceId());
    }
#endif
}

bool RichEditorPattern::HasConnection() const
{
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    return imeAttached_;
#else
    return connection_ != nullptr;
#endif
}

void RichEditorPattern::SetCustomKeyboardOption(bool supportAvoidance)
{
    keyboardAvoidance_ = supportAvoidance;
}

bool RichEditorPattern::RequestCustomKeyboard()
{
#if defined(ENABLE_STANDARD_INPUT)
    auto inputMethod = MiscServices::InputMethodController::GetInstance();
    if (inputMethod) {
        TAG_LOGI(AceLogTag::ACE_KEYBOARD, "RequestCKeyboard,close softkeyboard.");
        inputMethod->RequestHideInput();
        inputMethod->Close();
    }
#else
    if (HasConnection()) {
        connection_->Close(GetInstanceId());
        connection_ = nullptr;
    }
#endif

    if (isCustomKeyboardAttached_) {
        return true;
    }
    CHECK_NULL_RETURN(customKeyboardBuilder_, false);
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, false);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto overlayManager = pipeline->GetOverlayManager();
    CHECK_NULL_RETURN(overlayManager, false);
    overlayManager->SetCustomKeyboardOption(keyboardAvoidance_);
    overlayManager->BindKeyboard(customKeyboardBuilder_, frameNode->GetId());
    isCustomKeyboardAttached_ = true;
    contentChange_ = false;
    keyboardOverlay_ = overlayManager;
    auto [caretOffset, caretHeight] = CalculateCaretOffsetAndHeight();
    keyboardOverlay_->AvoidCustomKeyboard(frameNode->GetId(), caretHeight);
    return true;
}

bool RichEditorPattern::CloseCustomKeyboard()
{
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, false);
    CHECK_NULL_RETURN(keyboardOverlay_, false);
    keyboardOverlay_->CloseKeyboard(frameNode->GetId());
    isCustomKeyboardAttached_ = false;
    contentChange_ = false;
    return true;
}

int32_t RichEditorPattern::SetPreviewText(const std::string& previewTextValue, const PreviewRange range)
{
    CHECK_NULL_RETURN(!isSpanStringMode_, ERROR_BAD_PARAMETERS);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "previewTextValue = %{public}s, range = [%{public}d, %{public}d]",
        previewTextValue.c_str(), range.start, range.end);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, ERROR_BAD_PARAMETERS);

    if (!IsPreviewTextInputting()) {
        if (!InitPreviewText(previewTextValue, range)) {
            return ERROR_BAD_PARAMETERS;
        }
    } else {
        if (!UpdatePreviewText(previewTextValue, range)) {
            return ERROR_BAD_PARAMETERS;
        }
    }

    auto addLength = static_cast<int32_t>(StringUtils::ToWstring(previewTextValue).length());
    if (range.start == -1 && range.end == -1) {
        caretPosition_ = previewTextRecord_.endOffset;
    } else {
        caretPosition_ = previewTextRecord_.startOffset + addLength;
    }
    previewTextRecord_.currentClickedPosition = caretPosition_;
    UpdateCaretInfoToController();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    return NO_ERRORS;
}

bool RichEditorPattern::InitPreviewText(const std::string& previewTextValue, const PreviewRange range)
{
    if (range.start != -1 || range.end != -1) {
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "bad PreviewRange");
        return false;
    }
    if (textSelector_.IsValid()) {
        DeleteBackward(1);
    }
    previewTextRecord_.isPreviewTextInputting = true;
    TextSpanOptions options;
    options.value = previewTextValue;
    options.offset = caretPosition_;
    previewTextRecord_.startOffset = caretPosition_;
    previewTextRecord_.deltaStr = previewTextValue;
    BeforeIMEInsertValue("");
    auto index = AddTextSpan(options);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    RefPtr<SpanNode> spanNode = DynamicCast<SpanNode>(host->GetChildAtIndex(index));
    CHECK_NULL_RETURN(spanNode, false);
    AfterIMEInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(previewTextValue).length()), false);
    if (!spanNode || !spanNode->GetSpanItem()) {
        return false;
    }
    auto spanItem = spanNode->GetSpanItem();
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        UpdateTextStyle(spanNode, typingStyle_.value(), typingTextStyle_.value());
    }
    spanItem->SetTextStyle(typingTextStyle_);
    previewTextRecord_.previewTextSpan = spanItem;
    previewTextRecord_.spanIndex = index;
    auto length = static_cast<int32_t>(StringUtils::ToWstring(previewTextValue).length());
    previewTextRecord_.endOffset = previewTextRecord_.startOffset + length;
    return true;
}

bool RichEditorPattern::UpdatePreviewText(const std::string& previewTextValue, const PreviewRange range)
{
    auto previewTextSpan = previewTextRecord_.previewTextSpan;
    int32_t delta = 0;
    if (range.start == -1 && range.end == -1 && previewTextSpan) {
        auto beforeCallbackSucess = CallbackBeforeSetPreviewText(delta, previewTextValue, range, true);
        CHECK_NULL_RETURN(beforeCallbackSucess, false);
        previewTextSpan->content = previewTextValue;
        auto length = static_cast<int32_t>(StringUtils::ToWstring(previewTextValue).length());
        auto afterCallbackSucess = CallbackAfterSetPreviewText(delta);
        CHECK_NULL_RETURN(afterCallbackSucess, false);
        previewTextRecord_.endOffset = previewTextRecord_.startOffset + length;
    } else {
        if (range.start < previewTextRecord_.startOffset || range.end > previewTextRecord_.endOffset) {
            TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "bad PreviewRange");
            return false;
        }
        auto beforeCallbackSucess = CallbackBeforeSetPreviewText(delta, previewTextValue, range, false);
        CHECK_NULL_RETURN(beforeCallbackSucess, false);
        auto& content = previewTextSpan->content;
        auto replaceIndex = range.start - previewTextRecord_.startOffset;
        auto replaceLength = range.end - range.start;
        if (replaceIndex < 0 || replaceLength > static_cast<int32_t>(StringUtils::ToWstring(content).length())) {
            TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "replace range error : replaceIndex = %{public}d", replaceIndex);
            return false;
        }
        content.replace(replaceIndex, replaceLength, previewTextValue);
        auto afterCallbackSucess = CallbackAfterSetPreviewText(delta);
        CHECK_NULL_RETURN(afterCallbackSucess, false);
        auto length = static_cast<int32_t>(StringUtils::ToWstring(previewTextSpan->content).length());
        previewTextRecord_.endOffset = previewTextRecord_.startOffset + length;
    }
    UpdateSpanPosition();
    return true;
}

bool RichEditorPattern::CallbackBeforeSetPreviewText(
    int32_t& delta, const std::string& previewTextValue, const PreviewRange& range, bool isReplaceAll)
{
    auto previewTextLentgh = static_cast<int32_t>(StringUtils::ToWstring(previewTextValue).length());
    auto previewTextSpan = previewTextRecord_.previewTextSpan;
    auto spanContentLength = static_cast<int32_t>(StringUtils::ToWstring(previewTextSpan->content).length());
    delta = isReplaceAll ? previewTextLentgh - (previewTextRecord_.endOffset - previewTextRecord_.startOffset)
                         : previewTextLentgh - (range.end - range.start);
    if (delta > 0) {
        CHECK_NULL_RETURN(previewTextLentgh - delta >= 0, false);
        previewTextRecord_.deltaStr =
            StringUtils::ToString(StringUtils::ToWstring(previewTextValue).substr(previewTextLentgh - delta, delta));
        BeforeIMEInsertValue("");
    } else if (delta < 0) {
        RichEditorDeleteValue info;
        if (isReplaceAll) {
            CHECK_NULL_RETURN(spanContentLength + delta >= 0, false);
            previewTextRecord_.deltaStr = StringUtils::ToString(
                StringUtils::ToWstring(previewTextSpan->content).substr(spanContentLength + delta, -delta));
        } else {
            CHECK_NULL_RETURN(previewTextLentgh >= 0 && previewTextLentgh - delta <= spanContentLength, false);
            previewTextRecord_.deltaStr = StringUtils::ToString(
                StringUtils::ToWstring(previewTextSpan->content).substr(previewTextLentgh, -delta));
        }
        info.SetOffset(caretPosition_ + delta);
        info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::BACKWARD);
        info.SetLength(-delta);
        int32_t currenPositon_ = std::clamp((caretPosition_ + delta), 0, static_cast<int32_t>(GetTextContentLength()));
        if (spans_.empty()) {
            CalcDeleteValueObj(currenPositon_, -delta, info);
        }
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_RETURN(eventHub, false);
        eventHub->FireAboutToDelete(info);
    }
    return true;
}

bool RichEditorPattern::CallbackAfterSetPreviewText(int32_t& delta)
{
    auto previewTextSpan = previewTextRecord_.previewTextSpan;
    if (delta > 0) {
        auto spanNode = GetSpanNodeBySpanItem(previewTextSpan);
        CHECK_NULL_RETURN(spanNode, false);
        AfterIMEInsertValue(spanNode, delta, false);
    } else if (delta < 0) {
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_RETURN(eventHub, false);
        eventHub->FireOnDeleteComplete();
    }
    return true;
}

void RichEditorPattern::FinishTextPreview()
{
    CHECK_NULL_VOID(IsPreviewTextInputting());
    if (previewTextRecord_.previewTextSpan && previewTextRecord_.previewTextSpan->content.empty()) {
        auto iter = std::find(spans_.begin(), spans_.end(), previewTextRecord_.previewTextSpan);
        if (iter != spans_.end()) {
            auto spanIndex = std::distance(spans_.begin(), iter);
            spans_.erase(iter);
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            host->RemoveChildAtIndex(spanIndex);
        }
    }
    previewTextRecord_.Reset();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
}

std::vector<RectF> RichEditorPattern::GetPreviewTextRects()
{
    auto rects = paragraphs_.GetRects(previewTextRecord_.startOffset, previewTextRecord_.endOffset);
    auto offset = GetTextRect().GetOffset() - OffsetF(0.0f, std::min(baselineOffset_, 0.0f));
    for (RectF& rect : rects) {
        rect += offset;
    }
    return rects;
}

PreviewTextStyle RichEditorPattern::GetPreviewTextStyle() const
{
    auto previewTextStyle = PreviewTextStyle::UNDERLINE;
    auto property = GetLayoutProperty<RichEditorLayoutProperty>();
    if (property && property->HasPreviewTextStyle()) {
        auto style = property->GetPreviewTextStyle();
        if (style == PREVIEW_STYLE_NORMAL) {
            previewTextStyle = PreviewTextStyle::NORMAL;
        } else if (style == PREVIEW_STYLE_UNDERLINE) {
            previewTextStyle = PreviewTextStyle::UNDERLINE;
        } else {
            TAG_LOGW(
                AceLogTag::ACE_RICH_TEXT, "invalid previewTextStyle of RichEditorLayoutProperty");
        }
    }
    return previewTextStyle;
}

const Color& RichEditorPattern::GetPreviewTextDecorationColor() const
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, Color::TRANSPARENT);
    auto theme = pipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_RETURN(theme, Color::TRANSPARENT);
    if (GetPreviewTextStyle() == PreviewTextStyle::UNDERLINE) {
        return theme->GetPreviewUnderLineColor();
    }
    return Color::TRANSPARENT;
}

float RichEditorPattern::GetPreviewTextUnderlineWidth() const
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0.0f);
    auto theme = pipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_RETURN(theme, 0.0f);
    return theme->GetPreviewUnderlineWidth().ConvertToPx();
}

bool RichEditorPattern::BetweenPreviewTextPosition(const Offset& globalOffset)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    CHECK_NULL_RETURN(!IsPreviewTextInputting(), false);
    auto offset = host->GetPaintRectOffset();
    auto localOffset = globalOffset - Offset(offset.GetX(), offset.GetY());
    auto eventHub = host->GetEventHub<EventHub>();
    auto previewRects = paragraphs_.GetRects(previewTextRecord_.startOffset, previewTextRecord_.endOffset);
    auto panOffset = OffsetF(localOffset.GetX(), localOffset.GetY()) - GetTextRect().GetOffset() +
                     OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    for (const auto& previewRect : previewRects) {
        if (previewRect.IsInRegion(PointF(panOffset.GetX(), panOffset.GetY()))) {
            return true;
        }
    }
    return false;
}

void RichEditorPattern::InsertValueInPreview(const std::string& insertValue)
{
    if (previewTextRecord_.previewTextSpan) {
        previewTextRecord_.previewTextSpan->content.clear();
    }
    FinishTextPreview();
}

void RichEditorPattern::InsertValue(const std::string& insertValue)
{
    if (isSpanStringMode_) {
        InsertValueInStyledString(insertValue);
        return;
    }
    if (IsPreviewTextInputting()) {
        InsertValueInPreview(insertValue);
    }
    InsertValue(insertValue, true);
}

void RichEditorPattern::InsertValue(const std::string& insertValue, bool isIME)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "insertValue=[%{public}s]", StringUtils::RestoreEscape(insertValue).c_str());
    OperationRecord record;
    record.beforeCaretPosition = caretPosition_ + moveLength_;
    if (textSelector_.IsValid()) {
        record.beforeCaretPosition = textSelector_.GetTextStart();
    }
    record.addText = insertValue;
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, record, RecordType::INSERT));
    ClearRedoOperationRecords();
    InsertValueOperation(insertValue, &record, isIME);
    record.afterCaretPosition = caretPosition_;
    if (isDragSponsor_) {
        record.deleteCaretPostion = dragRange_.first;
    }
    AddOperationRecord(record);
    AfterChangeText(changeValue);
}

void RichEditorPattern::InsertValueOperation(const std::string& insertValue, OperationRecord* const record, bool isIME)
{
    bool isSelector = textSelector_.IsValid();
    if (isSelector) {
        SetCaretPosition(textSelector_.GetTextStart());
    }
    std::string insertValueTemp = insertValue;
    bool isLineSeparator = insertValueTemp == std::string("\n");

    if (isIME) {
        auto isInsert = BeforeIMEInsertValue(insertValueTemp);
        CHECK_NULL_VOID(isInsert);
    }
    TextInsertValueInfo info;
    CalcInsertValueObj(info);
    if (isSelector) {
        DeleteSelectOperation(record);
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    RefPtr<SpanNode> spanNode = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex()));
    if (spanNode == nullptr || spanNode->GetSpanItem()->unicode != 0) {
        InsertValueWithoutSpan(spanNode, info, insertValueTemp);
        return;
    }
    if (info.GetOffsetInSpan() == 0) {
        auto spanNodeBefore = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex() - 1));
        if (spanNodeBefore != nullptr && !IsLineSeparatorInLast(spanNodeBefore) &&
            spanNodeBefore->GetTag() == V2::SPAN_ETS_TAG) {
            InsertValueAfterBeforeSpan(spanNodeBefore, spanNode, info, insertValueTemp, isIME);
            return;
        }
    }
    if (typingStyle_.has_value() && !HasSameTypingStyle(spanNode)) {
        InsertDiffStyleValueInSpan(spanNode, info, insertValueTemp, isIME);
        return;
    }
    if (!isLineSeparator) {
        InsertValueToSpanNode(spanNode, insertValueTemp, info);
    } else {
        SpanNodeFission(spanNode, insertValueTemp, info);
    }
    magnifierController_->UpdateShowMagnifier();
    AfterInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValueTemp).length()), false, isIME);
}

void RichEditorPattern::DeleteSelectOperation(OperationRecord* const record)
{
    std::wstring deleteText = DeleteForwardOperation(textSelector_.GetTextEnd() - textSelector_.GetTextStart());
    if (record && deleteText.length() != 0) {
        record->deleteText = StringUtils::ToString(deleteText);
    }
    CloseSelectOverlay();
    ResetSelection();
}

void RichEditorPattern::InsertValueWithoutSpan(
    RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (info.GetSpanIndex() == 0) {
        CreateTextSpanNode(spanNode, info, insertValue, isIME);
        return;
    }
    auto spanNodeBefore = DynamicCast<SpanNode>(host->GetChildAtIndex(info.GetSpanIndex() - 1));
    if (spanNodeBefore == nullptr || spanNodeBefore->GetSpanItem()->unicode != 0) {
        CreateTextSpanNode(spanNode, info, insertValue, isIME);
        return;
    }
    InsertValueAfterBeforeSpan(spanNodeBefore, spanNode, info, insertValue, isIME);
}

void RichEditorPattern::InsertValueAfterBeforeSpan(RefPtr<SpanNode>& spanNodeBefore, RefPtr<SpanNode>& spanNode,
    const TextInsertValueInfo& info, const std::string& insertValue, bool isIME)
{
    if (typingStyle_.has_value() && !HasSameTypingStyle(spanNodeBefore)) {
        CreateTextSpanNode(spanNode, info, insertValue, isIME);
        CopyTextSpanLineStyle(spanNodeBefore, spanNode, true);
        return;
    }
    auto spanNodeGet = InsertValueToBeforeSpan(spanNodeBefore, insertValue);
    bool isCreate = spanNodeBefore->GetId() != spanNodeGet->GetId();
    AfterInsertValue(spanNodeGet, static_cast<int32_t>(StringUtils::ToWstring(insertValue).length()), isCreate, isIME);
}

void RichEditorPattern::InsertDiffStyleValueInSpan(
    RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    TextSpanOptions options;
    options.value = insertValue;
    options.offset = caretPosition_;
    options.style = typingTextStyle_;
    auto newSpanIndex = AddTextSpanOperation(options, false, -1,  true, false);
    auto newSpanNode = DynamicCast<SpanNode>(host->GetChildAtIndex(newSpanIndex));
    CopyTextSpanLineStyle(spanNode, newSpanNode, true);
    AfterInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValue).length()), true, isIME);
}

bool RichEditorPattern::IsLineSeparatorInLast(RefPtr<SpanNode>& spanNode)
{
    auto spanItem = spanNode->GetSpanItem();
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        if (textAfter.empty()) {
            return true;
        }
    }
    return false;
}

void RichEditorPattern::InsertValueToSpanNode(
    RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_VOID(spanItem);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    auto textTempSize = static_cast<int32_t>(textTemp.size());
    if (textTempSize < info.GetOffsetInSpan()) {
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "InsertValue error, offsetInSpan is greater than the size of spanItem, "
            "spanItemSize = %{public}d, offsetInSpan = %{public}d", textTempSize, info.GetOffsetInSpan());
        return;
    }
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.insert(info.GetOffsetInSpan(), insertValueTemp);
    text = StringUtils::ToString(textTemp);
    spanNode->UpdateContent(text);
    spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
}

void RichEditorPattern::SpanNodeFission(
    RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info)
{
    auto spanItem = spanNode->GetSpanItem();
    CHECK_NULL_VOID(spanItem);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    InsertValueInSpanOffset(info, textTemp, insertValueTemp);
    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        text = StringUtils::ToString(textBefore);
        spanNode->UpdateContent(text);
        spanItem->position += 1 - static_cast<int32_t>(textAfter.length());
        if (!textAfter.empty()) {
            TextInsertValueInfo infoAfter;
            infoAfter.SetSpanIndex(info.GetSpanIndex() + 1);
            infoAfter.SetOffsetInSpan(0);
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
            RefPtr<SpanNode> spanNodeAfter = SpanNode::GetOrCreateSpanNode(nodeId);
            spanNodeAfter->MountToParent(host, infoAfter.GetSpanIndex());
            spanNodeAfter->UpdateContent(StringUtils::ToString(textAfter));
            auto targetIt = spans_.begin();
            std::advance(targetIt, infoAfter.GetSpanIndex());
            spans_.insert(targetIt, spanNodeAfter->GetSpanItem());
            CopyTextSpanStyle(spanNode, spanNodeAfter);
            auto spanItemAfter = spanNodeAfter->GetSpanItem();
            AddSpanItem(spanItemAfter, infoAfter.GetSpanIndex());
        }
    } else {
        text = StringUtils::ToString(textTemp);
        spanNode->UpdateContent(text);
        spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    }
}

void RichEditorPattern::InsertValueInSpanOffset(
    const TextInsertValueInfo& info, std::wstring& text, const std::wstring& insertValue)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "insert value info: %{public}s", info.ToString().c_str());
    auto offsetInSpan = std::clamp(info.GetOffsetInSpan(), 0, static_cast<int32_t>(text.length()));
    text.insert(offsetInSpan, insertValue);
}

RefPtr<SpanNode> RichEditorPattern::InsertValueToBeforeSpan(
    RefPtr<SpanNode>& spanNodeBefore, const std::string& insertValue)
{
    auto spanItem = spanNodeBefore->GetSpanItem();
    CHECK_NULL_RETURN(spanItem, spanNodeBefore);
    auto text = spanItem->content;
    std::wstring textTemp = StringUtils::ToWstring(text);
    std::wstring insertValueTemp = StringUtils::ToWstring(insertValue);
    textTemp.append(insertValueTemp);

    auto index = textTemp.find(lineSeparator);
    if (index != std::wstring::npos) {
        auto textBefore = textTemp.substr(0, index + 1);
        auto textAfter = textTemp.substr(index + 1);
        text = StringUtils::ToString(textBefore);
        spanNodeBefore->UpdateContent(text);
        spanItem->position += static_cast<int32_t>(insertValueTemp.length()) - static_cast<int32_t>(textAfter.length());
        if (!textAfter.empty()) {
            auto host = GetHost();
            CHECK_NULL_RETURN(spanItem, spanNodeBefore);
            TextInsertValueInfo infoAfter;
            infoAfter.SetSpanIndex(host->GetChildIndex(spanNodeBefore) + 1);
            infoAfter.SetOffsetInSpan(0);
            auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
            RefPtr<SpanNode> spanNodeAfter = SpanNode::GetOrCreateSpanNode(nodeId);
            spanNodeAfter->MountToParent(host, infoAfter.GetSpanIndex());
            spanNodeAfter->UpdateContent(StringUtils::ToString(textAfter));
            CopyTextSpanStyle(spanNodeBefore, spanNodeAfter);
            auto spanItemAfter = spanNodeAfter->GetSpanItem();
            spanItemAfter->position = static_cast<int32_t>(textTemp.length());
            spanItemAfter->hasResourceFontColor = spanItem->hasResourceFontColor;
            spanItemAfter->hasResourceDecorationColor = spanItem->hasResourceDecorationColor;
            AddSpanItem(spanItemAfter, host->GetChildIndex(spanNodeBefore) + 1);
            SpanNodeFission(spanNodeAfter);
            return spanNodeAfter;
        }
    } else {
        text = StringUtils::ToString(textTemp);
        spanNodeBefore->UpdateContent(text);
        spanItem->position += static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    }
    return spanNodeBefore;
}

void RichEditorPattern::CreateTextSpanNode(
    RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto nodeId = ViewStackProcessor::GetInstance()->ClaimNodeId();
    spanNode = SpanNode::GetOrCreateSpanNode(nodeId);
    spanNode->MountToParent(host, info.GetSpanIndex());
    auto spanItem = spanNode->GetSpanItem();
    spanItem->hasResourceFontColor = true;
    spanItem->hasResourceDecorationColor = true;
    spanNode->UpdateContent(insertValue);
    AddSpanItem(spanItem, info.GetSpanIndex());
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        UpdateTextStyle(spanNode, typingStyle_.value(), typingTextStyle_.value());
        auto spanItem = spanNode->GetSpanItem();
        spanItem->SetTextStyle(typingTextStyle_);
    } else {
        spanNode->UpdateFontSize(Dimension(DEFAULT_TEXT_SIZE, DimensionUnit::FP));
        spanNode->AddPropertyInfo(PropertyInfo::FONTSIZE);
        SetDefaultColor(spanNode);
    }
    AfterInsertValue(spanNode, static_cast<int32_t>(StringUtils::ToWstring(insertValue).length()), true, isIME);
}

void RichEditorPattern::SetDefaultColor(RefPtr<SpanNode>& spanNode)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richEditorTheme);
    Color textColor = richEditorTheme->GetTextStyle().GetTextColor();
    spanNode->UpdateTextColor(textColor);
    spanNode->AddPropertyInfo(PropertyInfo::FONTCOLOR);
    spanNode->UpdateTextDecorationColor(textColor);
    spanNode->AddPropertyInfo(PropertyInfo::NONE);
}

bool RichEditorPattern::BeforeIMEInsertValue(const std::string& insertValue)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    RichEditorInsertValue insertValueInfo;
    insertValueInfo.SetInsertOffset(caretPosition_);
    if (previewTextRecord_.isPreviewTextInputting && !previewTextRecord_.deltaStr.empty()) {
        insertValueInfo.SetPreviewText(previewTextRecord_.deltaStr);
    } else {
        insertValueInfo.SetInsertValue(insertValue);
    }
    return eventHub->FireAboutToIMEInput(insertValueInfo);
}

void RichEditorPattern::AfterInsertValue(
    const RefPtr<SpanNode>& spanNode, int32_t insertValueLength, bool isCreate, bool isIME)
{
    isTextChange_ = true;
    moveDirection_ = MoveDirection::FORWARD;
    moveLength_ += insertValueLength;
    UpdateSpanPosition();
    if (isIME) {
        AfterIMEInsertValue(spanNode, insertValueLength, isCreate);
    }
    MoveCaretAfterTextChange();
}

bool RichEditorPattern::AfterIMEInsertValue(const RefPtr<SpanNode>& spanNode, int32_t insertValueLength, bool isCreate)
{
    ACE_SCOPED_TRACE("RichEditorAfterIMEInsertValue");
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, false);

    RichEditorAbstractSpanResult retInfo;
    retInfo.SetSpanIndex(host->GetChildIndex(spanNode));
    retInfo.SetEraseLength(insertValueLength);
    auto spanItem = spanNode->GetSpanItem();
    if (previewTextRecord_.isPreviewTextInputting && !previewTextRecord_.deltaStr.empty()) {
        retInfo.SetPreviewText(previewTextRecord_.deltaStr);
    } else {
        retInfo.SetValue(spanItem->content);
    }
    auto contentLength = static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length());
    retInfo.SetSpanRangeStart(spanItem->position - contentLength);
    retInfo.SetSpanRangeEnd(spanItem->position);
    retInfo.SetOffsetInSpan(caretPosition_ - retInfo.GetSpanRangeStart());
    retInfo.SetFontColor(spanNode->GetTextColorValue(Color::BLACK).ColorToString());
    retInfo.SetFontSize(spanNode->GetFontSizeValue(Dimension(16.0f, DimensionUnit::VP)).ConvertToVp());
    retInfo.SetFontStyle(spanNode->GetItalicFontStyleValue(OHOS::Ace::FontStyle::NORMAL));
    retInfo.SetFontWeight(static_cast<int32_t>(spanNode->GetFontWeightValue(FontWeight::NORMAL)));
    retInfo.SetTextStyle(GetTextStyleObject(spanNode));
    std::string fontFamilyValue;
    auto fontFamily = spanNode->GetFontFamilyValue({ "HarmonyOS Sans" });
    for (const auto& str : fontFamily) {
        fontFamilyValue += str;
    }
    retInfo.SetFontFamily(fontFamilyValue);
    retInfo.SetTextDecoration(spanNode->GetTextDecorationValue(TextDecoration::NONE));
    retInfo.SetFontFeature(spanNode->GetFontFeatureValue(ParseFontFeatureSettings("\"pnum\" 1")));
    retInfo.SetColor(spanNode->GetTextDecorationColorValue(Color::BLACK).ColorToString());
    eventHub->FireOnIMEInputComplete(retInfo);
    return true;
}

void RichEditorPattern::ResetFirstNodeStyle()
{
    auto tmpHost = GetHost();
    CHECK_NULL_VOID(tmpHost);
    auto spans = tmpHost->GetChildren();
    if (!spans.empty()) {
        auto&& firstNode = DynamicCast<SpanNode>(*(spans.begin()));
        if (firstNode) {
            firstNode->ResetTextAlign();
            firstNode->ResetLeadingMargin();
            tmpHost->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
        }
    }
}

void RichEditorPattern::FireOnDeleteComplete(const RichEditorDeleteValue& info)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto isDelete = eventHub->FireAboutToDelete(info);
    if (isDelete) {
        DeleteByDeleteValueInfo(info);
        eventHub->FireOnDeleteComplete();
    }
}

std::pair<bool, bool> RichEditorPattern::IsEmojiOnCaretPosition(int32_t& emojiLength, bool isBackward, int32_t length)
{
    bool isEmojiOnCaretBackward = false;
    bool isEmojiOnCaretForward = false;
    std::stringstream ss;
    for (auto iter = spans_.cbegin(); iter != spans_.cend(); iter++) {
        ss << (*iter)->content;
    }
    auto content = ss.str();
    emojiLength = TextEmojiProcessor::Delete(caretPosition_, length, content, isBackward);
    if (emojiLength > 0) {
        if (isBackward) {
            isEmojiOnCaretBackward = true;
        } else {
            isEmojiOnCaretForward = true;
        }
    }
    return std::make_pair(isEmojiOnCaretBackward, isEmojiOnCaretForward);
}

void RichEditorPattern::HandleOnDelete(bool backward)
{
    if (backward) {
#if defined(PREVIEW)
        DeleteForward(1);
#else
        DeleteBackward(1);
#endif
    } else {
#if defined(PREVIEW)
        DeleteBackward(1);
#else
        DeleteForward(1);
#endif
    }
}

int32_t RichEditorPattern::CalculateDeleteLength(int32_t length, bool isBackward)
{
    int32_t emojiLength = 0;
    auto [isEmojiOnCaretBackward, isEmojiOnCaretForward] = IsEmojiOnCaretPosition(emojiLength, isBackward, length);
    if (textSelector_.IsValid()) {
        if (!textSelector_.StartEqualToDest()) {
            length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
            if (isBackward) {
                caretPosition_ = textSelector_.GetTextEnd();
            } else {
                caretPosition_ = textSelector_.GetTextStart();
            }
        } else if ((isBackward && isEmojiOnCaretBackward) || (!isBackward && isEmojiOnCaretForward)) {
            length = emojiLength;
        }
        CloseSelectOverlay();
        ResetSelection();
    } else if ((isBackward && isEmojiOnCaretBackward) || (!isBackward && isEmojiOnCaretForward)) {
        length = emojiLength;
    }
    return length;
}

void RichEditorPattern::DeleteBackward(int32_t length)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "length=%{public}d", length);
    if (isSpanStringMode_) {
        DeleteBackwardInStyledString(length);
        return;
    }
    if (IsPreviewTextInputting()) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "do not handle DeleteBackward on previewTextInputting");
        return;
    }
    OperationRecord record;
    record.beforeCaretPosition = caretPosition_;
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, record, RecordType::DEL_BACKWARD, length));
    std::wstring deleteText = DeleteBackwardOperation(length);
    if (deleteText.length() != 0) {
        ClearRedoOperationRecords();
        record.deleteText = StringUtils::ToString(deleteText);
        record.afterCaretPosition = caretPosition_;
        AddOperationRecord(record);
    }
    AfterChangeText(changeValue);
}

std::wstring RichEditorPattern::DeleteBackwardOperation(int32_t length)
{
    length = CalculateDeleteLength(length, true);
    std::wstringstream wss;
    for (auto iter = spans_.cbegin(); iter != spans_.cend(); iter++) {
        wss << StringUtils::ToWstring((*iter)->content);
    }
    auto textContent = wss.str();
    if (static_cast<int32_t>(textContent.length()) != GetTextContentLength()) {
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "textContent length mismatch, %{public}d vs. %{public}d",
            static_cast<int32_t>(textContent.length()), GetTextContentLength());
    }
    auto start = std::clamp(caretPosition_ - length, 0, static_cast<int32_t>(textContent.length()));
    std::wstring deleteText =
        textContent.substr(static_cast<uint32_t>(start), static_cast<uint32_t>(caretPosition_ - start));
    RichEditorDeleteValue info;
    info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::BACKWARD);
    if (caretPosition_ == 0) {
        info.SetLength(0);
        ResetFirstNodeStyle();
        FireOnDeleteComplete(info);
        return deleteText;
    }
    info.SetOffset(caretPosition_ - 1);
    info.SetLength(length);
    int32_t currentPosition = std::clamp((caretPosition_ - length), 0, static_cast<int32_t>(GetTextContentLength()));
    if (!spans_.empty()) {
        CalcDeleteValueObj(currentPosition, length, info);
        FireOnDeleteComplete(info);
    }
    auto host = GetHost();
    if (host && host->GetChildren().empty()) {
        textForDisplay_.clear();
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
    return deleteText;
}

void RichEditorPattern::DeleteForward(int32_t length)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "length=%{public}d", length);
    if (isSpanStringMode_) {
        DeleteForwardInStyledString(length);
        return;
    }
    if (IsPreviewTextInputting()) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "do not handle DeleteForward in previewTextInputting");
        return;
    }
    OperationRecord record;
    record.beforeCaretPosition = caretPosition_;
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, record, RecordType::DEL_FORWARD, length));
    std::wstring deleteText = DeleteForwardOperation(length);
    if (deleteText.length() != 0) {
        ClearRedoOperationRecords();
        record.deleteText = StringUtils::ToString(deleteText);
        record.afterCaretPosition = caretPosition_;
        AddOperationRecord(record);
    }
    AfterChangeText(changeValue);
}

std::wstring RichEditorPattern::DeleteForwardOperation(int32_t length)
{
    length = CalculateDeleteLength(length, false);
    std::wstringstream wss;
    for (auto iter = spans_.cbegin(); iter != spans_.cend(); iter++) {
        wss << StringUtils::ToWstring((*iter)->content);
    }
    auto textContent = wss.str();
    if (static_cast<int32_t>(textContent.length()) != GetTextContentLength()) {
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "textContent length mismatch, %{public}d vs. %{public}d",
            static_cast<int32_t>(textContent.length()), GetTextContentLength());
    }
    auto end = std::clamp(caretPosition_ + length, 0, static_cast<int32_t>(textContent.length()));
    std::wstring deleteText = textContent.substr(
        static_cast<uint32_t>(std::clamp(caretPosition_, 0, static_cast<int32_t>(textContent.length()))),
        static_cast<uint32_t>(end - caretPosition_));
    if (caretPosition_ == GetTextContentLength()) {
        return deleteText;
    }
    RichEditorDeleteValue info;
    info.SetOffset(caretPosition_);
    info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::FORWARD);
    info.SetLength(length);
    int32_t currentPosition = caretPosition_;
    if (!spans_.empty()) {
        CalcDeleteValueObj(currentPosition, length, info);
        auto eventHub = GetEventHub<RichEditorEventHub>();
        CHECK_NULL_RETURN(eventHub, deleteText);
        auto isDelete = eventHub->FireAboutToDelete(info);
        if (isDelete) {
            DeleteByDeleteValueInfo(info);
            eventHub->FireOnDeleteComplete();
        }
    }
    if (!caretVisible_) {
        StartTwinkling();
    }
    return deleteText;
}

bool RichEditorPattern::OnBackPressed()
{
    auto tmpHost = GetHost();
    CHECK_NULL_RETURN(tmpHost, false);
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "RichEditor %{public}d receives back press event", tmpHost->GetId());
    if (SelectOverlayIsOn()) {
        CloseSelectOverlay();
        textSelector_.Update(textSelector_.destinationOffset);
        StartTwinkling();
        return true;
    }
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    if (!imeShown_ && !isCustomKeyboardAttached_) {
#else
    if (!isCustomKeyboardAttached_) {
#endif
        return false;
    }
    tmpHost->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    CloseKeyboard(true);
    FocusHub::LostFocusToViewRoot();
#if defined(ANDROID_PLATFORM)
    return false;
#else
    return true;
#endif
}

void RichEditorPattern::SetInputMethodStatus(bool keyboardShown)
{
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    imeShown_ = keyboardShown;
#endif
}

bool RichEditorPattern::CursorMoveLeft()
{
    CloseSelectOverlay();
    ResetSelection();
    int32_t emojiLength = 0;
    int32_t caretPosition = caretPosition_;
    constexpr int32_t DELETE_COUNT = 1;
    auto [isEmojiOnCaretBackward, isEmojiOnCaretForward] = IsEmojiOnCaretPosition(emojiLength, true, DELETE_COUNT);
    if (isEmojiOnCaretBackward) {
        caretPosition = std::clamp((caretPosition_ - emojiLength), 0, static_cast<int32_t>(GetTextContentLength()));
    } else {
        caretPosition = std::clamp((caretPosition_ - 1), 0, static_cast<int32_t>(GetTextContentLength()));
    }
    if (caretPosition_ == caretPosition) {
        return false;
    }
    SetCaretPosition(caretPosition);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveRight()
{
    CloseSelectOverlay();
    ResetSelection();
    int32_t emojiLength = 0;
    int32_t caretPosition = caretPosition_;
    constexpr int32_t DELETE_COUNT = 1;
    auto [isEmojiOnCaretBackward, isEmojiOnCaretForward] = IsEmojiOnCaretPosition(emojiLength, false, DELETE_COUNT);
    if (isEmojiOnCaretForward) {
        caretPosition = std::clamp((caretPosition_ + emojiLength), 0, static_cast<int32_t>(GetTextContentLength()));
    } else {
        caretPosition = std::clamp((caretPosition_ + 1), 0, static_cast<int32_t>(GetTextContentLength()));
    }
    if (caretPosition_ == caretPosition) {
        return false;
    }
    SetCaretPosition(caretPosition);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveUp()
{
    CloseSelectOverlay();
    ResetSelection();
    float nextCaretHeight = 0.0f;
    OffsetF caretOffsetUp;
    OffsetF caretOffsetDown;
    OffsetF nextCaretOffset;
    if (static_cast<int32_t>(GetTextContentLength()) > 1) {
        int32_t caretPosition = CalcMoveUpPos(caretOffsetUp, caretOffsetDown);
        nextCaretOffset = CalcCursorOffsetByPosition(GetCaretPosition() + 1, nextCaretHeight, false, false);
        caretPosition = std::clamp(caretPosition, 0, static_cast<int32_t>(GetTextContentLength()));
        if (caretPosition_ == caretPosition) {
            caretPosition = 0;
        }
        SetCaretPosition(caretPosition);
        MoveCaretToContentRect();
        CursorMoveLineEndPos(caretOffsetUp, caretOffsetDown, nextCaretOffset);
    }
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveDown()
{
    CloseSelectOverlay();
    ResetSelection();
    if (static_cast<int32_t>(GetTextContentLength()) > 1) {
        float caretHeightUp = 0.0f;
        float caretHeightDown = 0.0f;
        float nextCaretHeight = 0.0f;
        Offset textOffset;
        int32_t caretPositionStart;
        int32_t caretPositionEnd;
        caretPositionStart = GetCaretPosition();
        OffsetF caretOffsetUp = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeightUp);
        OffsetF caretOffsetDown = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeightDown, true, false);
        OffsetF nextCaretOffset = CalcCursorOffsetByPosition(GetCaretPosition() + 1, nextCaretHeight, false, false);
        auto overlayMod = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
        auto caretOffsetOverlay = overlayMod->GetCaretOffset();
        float textOffsetX = GetTextRect().GetX();
        float textOffsetY = GetTextRect().GetY() - caretHeightDown - caretHeightDown / 2.0;
        float textOffsetDownY = caretOffsetOverlay.GetY() - textOffsetY;
        textOffset = Offset(caretOffsetOverlay.GetX() - textOffsetX, textOffsetDownY);
        caretPositionEnd = paragraphs_.GetIndex(textOffset);
        caretPositionEnd = std::clamp(caretPositionEnd, 0, static_cast<int32_t>(GetTextContentLength()));
        if (caretPosition_ == caretPositionEnd) {
            caretPositionEnd = GetTextContentLength();
        }
        SetCaretPosition(caretPositionEnd);
        CursorMoveLineEndPos(caretOffsetUp, caretOffsetDown, nextCaretOffset);
        MoveCaretToContentRect();
    }
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveLeftWord()
{
    CloseSelectOverlay();
    ResetSelection();
    auto newPos = GetLeftWordPosition(caretPosition_);
    if (newPos == caretPosition_) {
        return false;
    }
    SetCaretPosition(newPos);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveRightWord()
{
    CloseSelectOverlay();
    ResetSelection();
    auto newPos = GetRightWordPosition(caretPosition_);
    if (newPos == caretPosition_) {
        return false;
    }
    SetCaretPosition(newPos);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveToParagraphBegin()
{
    CloseSelectOverlay();
    ResetSelection();
    auto newPos = GetParagraphBeginPosition(caretPosition_);
    if (newPos == caretPosition_ && caretPosition_ > 0) {
        newPos = GetParagraphBeginPosition(caretPosition_ - 1);
    }
    if (newPos == caretPosition_) {
        return false;
    }
    SetCaretPosition(newPos);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveToParagraphEnd()
{
    CloseSelectOverlay();
    ResetSelection();
    auto newPos = GetParagraphEndPosition(caretPosition_);
    if (newPos == caretPosition_ && caretPosition_ < static_cast<int32_t>(GetTextContentLength())) {
        newPos = GetParagraphEndPosition(caretPosition_ + 1);
    }
    if (newPos == caretPosition_) {
        return false;
    }
    SetCaretPosition(newPos);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveHome()
{
    CloseSelectOverlay();
    ResetSelection();
    if (0 == caretPosition_) {
        return false;
    }
    SetCaretPosition(0);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveEnd()
{
    CloseSelectOverlay();
    ResetSelection();
    auto newPos = GetTextContentLength();
    if (newPos == caretPosition_) {
        return false;
    }
    SetCaretPosition(newPos);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

int32_t RichEditorPattern::GetLeftWordPosition(int32_t caretPosition)
{
    int32_t offset = 0;
    bool jumpSpace = true;
    for (auto iter = spans_.rbegin(); iter != spans_.rend(); iter++) {
        auto span = *iter;
        auto content = StringUtils::ToWstring(span->content);
        if (caretPosition <= span->position - static_cast<int32_t>(content.length())) {
            continue;
        }
        int32_t position = span->position;
        for (auto iterContent = content.rbegin(); iterContent != content.rend(); iterContent++) {
            if (position-- > caretPosition) {
                continue;
            }
            if (*iterContent != L' ' || span->placeholderIndex >= 0) {
                jumpSpace = false;
            }
            if (position + 1 == caretPosition) {
                if (!(StringUtils::IsLetterOrNumberForWchar(*iterContent) ||
                        (*iterContent == L' ' && span->placeholderIndex < 0))) {
                    return std::clamp(caretPosition - 1, 0, static_cast<int32_t>(GetTextContentLength()));
                }
            }
            if (!jumpSpace) {
                if (!StringUtils::IsLetterOrNumberForWchar(*iterContent)) {
                    return std::clamp(caretPosition - offset, 0, static_cast<int32_t>(GetTextContentLength()));
                }
            } else {
                if (*iterContent == L' ' && span->placeholderIndex >= 0) {
                    return std::clamp(caretPosition - offset, 0, static_cast<int32_t>(GetTextContentLength()));
                }
            }
            offset++;
        }
    }
    return std::clamp(caretPosition - offset, 0, static_cast<int32_t>(GetTextContentLength()));
}

int32_t RichEditorPattern::GetRightWordPosition(int32_t caretPosition)
{
    int32_t offset = 0;
    bool jumpSpace = false;
    for (auto iter = spans_.cbegin(); iter != spans_.cend(); iter++) {
        auto span = *iter;
        auto content = StringUtils::ToWstring(span->content);
        if (caretPosition > span->position) {
            continue;
        }
        int32_t position = span->position - static_cast<int32_t>(content.length());
        for (auto iterContent = content.cbegin(); iterContent != content.cend(); iterContent++) {
            if (position++ < caretPosition) {
                continue;
            }
            if (*iterContent == L' ' && span->placeholderIndex < 0) {
                jumpSpace = true;
                offset++;
                continue;
            }
            if (position - 1 == caretPosition) {
                if (!StringUtils::IsLetterOrNumberForWchar(*iterContent)) {
                    return std::clamp(caretPosition + 1, 0, static_cast<int32_t>(GetTextContentLength()));
                }
            }
            if (jumpSpace) {
                if (*iterContent != L' ' || span->placeholderIndex >= 0) {
                    return std::clamp(caretPosition + offset, 0, static_cast<int32_t>(GetTextContentLength()));
                }
            } else {
                if (!(StringUtils::IsLetterOrNumberForWchar(*iterContent) ||
                        (*iterContent == L' ' && span->placeholderIndex < 0))) {
                    return std::clamp(caretPosition + offset, 0, static_cast<int32_t>(GetTextContentLength()));
                }
            }
            offset++;
        }
    }
    return std::clamp(caretPosition + offset, 0, static_cast<int32_t>(GetTextContentLength()));
}

int32_t RichEditorPattern::GetParagraphBeginPosition(int32_t caretPosition)
{
    int32_t offset = 0;
    for (auto iter = spans_.rbegin(); iter != spans_.rend(); iter++) {
        auto span = *iter;
        auto content = StringUtils::ToWstring(span->content);
        if (caretPosition <= span->position - static_cast<int32_t>(content.length())) {
            continue;
        }
        int32_t position = span->position;
        for (auto iterContent = content.rbegin(); iterContent != content.rend(); iterContent++) {
            if (position-- > caretPosition) {
                continue;
            }
            if (*iterContent == L'\n') {
                return std::clamp(caretPosition - offset, 0, static_cast<int32_t>(GetTextContentLength()));
            }
            offset++;
        }
    }
    return std::clamp(caretPosition - offset, 0, static_cast<int32_t>(GetTextContentLength()));
}

int32_t RichEditorPattern::GetParagraphEndPosition(int32_t caretPosition)
{
    int32_t offset = 0;
    for (auto iter = spans_.cbegin(); iter != spans_.cend(); iter++) {
        auto span = *iter;
        auto content = StringUtils::ToWstring(span->content);
        if (caretPosition > span->position) {
            continue;
        }
        int32_t position = span->position - static_cast<int32_t>(content.length());
        for (auto iterContent = content.cbegin(); iterContent != content.cend(); iterContent++) {
            if (position++ < caretPosition) {
                continue;
            }
            if (*iterContent == L'\n') {
                return std::clamp(caretPosition + offset, 0, static_cast<int32_t>(GetTextContentLength()));
            }
            offset++;
        }
    }
    return std::clamp(caretPosition + offset, 0, static_cast<int32_t>(GetTextContentLength()));
}

void RichEditorPattern::HandleOnSelectAll()
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "HandleOnSelectAll");
    CloseSelectOverlay();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t newPos = static_cast<int32_t>(GetTextContentLength());
    textSelector_.Update(0, newPos);
    FireOnSelect(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    SetCaretPosition(newPos);
    MoveCaretToContentRect();
    StopTwinkling();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

int32_t RichEditorPattern::CaretPositionSelectEmoji(CaretMoveIntent direction)
{
    int32_t newPos = caretPosition_;
    int32_t emojiLength = 0;
    constexpr int32_t DELETE_COUNT = 1;
    if (direction == CaretMoveIntent::Left) {
        auto [isEmojiOnCaretBackward, isEmojiOnCaretForward] = IsEmojiOnCaretPosition(emojiLength, true, DELETE_COUNT);
        if (isEmojiOnCaretBackward) {
            newPos = caretPosition_ - emojiLength;
        } else {
            newPos = caretPosition_ - 1;
        }
        return newPos;
    }
    auto [isEmojiOnCaretBackward, isEmojiOnCaretForward] = IsEmojiOnCaretPosition(emojiLength, false, DELETE_COUNT);
    if (direction == CaretMoveIntent::Right) {
        if (isEmojiOnCaretForward) {
            newPos = caretPosition_ + emojiLength;
        } else {
            newPos = caretPosition_ + 1;
        }
    }
    return newPos;
}

void RichEditorPattern::HandleSelect(CaretMoveIntent direction)
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "direction=%{public}d", direction);
    CloseSelectOverlay();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    int32_t newPos, fixedPos = caretPosition_;
    if (IsSelected()) {
        fixedPos = (caretPosition_ == textSelector_.GetTextStart() ? textSelector_.GetTextEnd()
                                                                   : textSelector_.GetTextStart());
    }
    newPos = HandleSelectWrapper(direction);
    if (newPos == -1) {
        return;
    }
    newPos = std::clamp(newPos, 0, static_cast<int32_t>(GetTextContentLength()));
    if (newPos == caretPosition_) {
        return;
    }
    UpdateSelector(newPos, fixedPos);
    FireOnSelect(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    SetCaretPosition(newPos);
    MoveCaretToContentRect();
    if (textSelector_.SelectNothing()) {
        StartTwinkling();
    } else {
        StopTwinkling();
    }
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::ClearOperationRecords()
{
    ClearRedoOperationRecords();
    if (operationRecords_.empty()) {
        return;
    }
    operationRecords_.clear();
}

void RichEditorPattern::ClearRedoOperationRecords()
{
    if (redoOperationRecords_.empty()) {
        return;
    }
    redoOperationRecords_.clear();
}

void RichEditorPattern::AddOperationRecord(const OperationRecord& record)
{
    if (operationRecords_.size() >= RECORD_MAX_LENGTH) {
        // case of max length is 0
        if (operationRecords_.empty()) {
            return;
        }
        operationRecords_.erase(operationRecords_.begin());
    }
    operationRecords_.emplace_back(record);
}

bool RichEditorPattern::HandleOnEscape()
{
    CloseSelectOverlay();
    return false;
}

void RichEditorPattern::HandleOnUndoAction()
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "HandleOnUndoAction");
    if (operationRecords_.empty()) {
        return;
    }
    auto value = operationRecords_.back();
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, value, RecordType::UNDO));
    operationRecords_.pop_back();
    if (redoOperationRecords_.size() >= RECORD_MAX_LENGTH && !(redoOperationRecords_.empty())) {
        redoOperationRecords_.erase(redoOperationRecords_.begin());
    }
    redoOperationRecords_.push_back(value);
    CloseSelectOverlay();
    ResetSelection();
    if (value.addText.has_value() && value.deleteCaretPostion != -1) {
        UndoDrag(value);
        AfterChangeText(changeValue);
        return;
    }
    if (value.addText.has_value() && value.deleteText.has_value()) {
        SetCaretPosition(value.afterCaretPosition);
        DeleteBackwardOperation(StringUtils::ToWstring(value.addText.value_or("")).length());
        InsertValueOperation(value.deleteText.value_or(""));
        AfterChangeText(changeValue);
        return;
    }
    if (value.addText.has_value()) {
        SetCaretPosition(value.afterCaretPosition);
        DeleteBackwardOperation(StringUtils::ToWstring(value.addText.value_or("")).length());
    }
    if (value.deleteText.has_value()) {
        SetCaretPosition(value.afterCaretPosition);
        InsertValueOperation(value.deleteText.value_or(""));
    }
    AfterChangeText(changeValue);
}

void RichEditorPattern::HandleOnRedoAction()
{
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "HandleOnRedoAction");
    if (redoOperationRecords_.empty()) {
        return;
    }
    auto value = redoOperationRecords_.back();
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, value, RecordType::REDO));
    redoOperationRecords_.pop_back();
    if (value.addText.has_value() && value.deleteCaretPostion != -1) {
        RedoDrag(value);
        AfterChangeText(changeValue);
        return;
    }
    if (value.addText.has_value() && value.deleteText.has_value()) {
        SetCaretPosition(value.beforeCaretPosition);
        DeleteForwardOperation(StringUtils::ToWstring(value.deleteText.value_or("")).length());
        InsertValueOperation(value.addText.value_or(""));
        operationRecords_.push_back(value);
        AfterChangeText(changeValue);
        return;
    }
    if (value.deleteText.has_value()) {
        SetCaretPosition(value.beforeCaretPosition);
        if (value.beforeCaretPosition != value.afterCaretPosition) {
            DeleteBackwardOperation(StringUtils::ToWstring(value.deleteText.value_or("")).length());
        } else {
            DeleteForwardOperation(StringUtils::ToWstring(value.deleteText.value_or("")).length());
        }
    }
    if (value.addText.has_value()) {
        SetCaretPosition(value.beforeCaretPosition);
        InsertValueOperation(value.addText.value_or(""));
    }
    operationRecords_.push_back(value);
    AfterChangeText(changeValue);
}

void RichEditorPattern::CalcInsertValueObj(TextInsertValueInfo& info)
{
    if (spans_.empty()) {
        info.SetSpanIndex(0);
        info.SetOffsetInSpan(0);
        return;
    }
    auto it = std::find_if(
        spans_.begin(), spans_.end(), [caretPosition = caretPosition_ + moveLength_](const RefPtr<SpanItem>& spanItem) {
            auto spanLength = static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length());
            if (spanLength == 0) {
                return spanItem->position == caretPosition;
            }
            return (spanItem->position - spanLength <= caretPosition) && (caretPosition < spanItem->position);
        });
    if (it != spans_.end() && (*it)->unicode != 0 && (*it)->position - caretPosition_ + moveLength_ == 1) {
        it++;
        moveLength_++;
    }
    info.SetSpanIndex(std::distance(spans_.begin(), it));
    if (it == spans_.end()) {
        info.SetOffsetInSpan(0);
        return;
    }
    info.SetOffsetInSpan(
        caretPosition_ + moveLength_ - ((*it)->position - StringUtils::ToWstring((*it)->content).length()));
}

void RichEditorPattern::CalcDeleteValueObj(int32_t currentPosition, int32_t length, RichEditorDeleteValue& info)
{
    auto it =
        std::find_if(spans_.begin(), spans_.end(), [caretPosition = currentPosition](const RefPtr<SpanItem>& spanItem) {
            return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                       caretPosition) &&
                   (caretPosition < spanItem->position);
        });
    while (it != spans_.end() && length > 0) {
        if ((*it)->placeholderIndex >= 0 || (*it)->unicode != 0) {
            RichEditorAbstractSpanResult spanResult;
            spanResult.SetSpanIndex(std::distance(spans_.begin(), it));
            int32_t eraseLength = 0;
            if ((*it)->unicode != 0) {
                eraseLength = DeleteValueSetSymbolSpan(*it, spanResult);
                info.SetLength(SYMBOL_SPAN_LENGTH);
            } else if (AceType::InstanceOf<ImageSpanItem>(*it)) {
                eraseLength = DeleteValueSetImageSpan(*it, spanResult);
            } else {
                eraseLength = DeleteValueSetBuilderSpan(*it, spanResult);
            }
            currentPosition += eraseLength;
            length -= eraseLength;
            info.SetRichEditorDeleteSpans(spanResult);
        } else {
            RichEditorAbstractSpanResult spanResult;
            spanResult.SetSpanIndex(std::distance(spans_.begin(), it));
            auto eraseLength = DeleteValueSetTextSpan(*it, currentPosition, length, spanResult);
            length -= eraseLength;
            currentPosition += eraseLength;
            info.SetRichEditorDeleteSpans(spanResult);
        }
        std::advance(it, 1);
    }
}

RefPtr<SpanNode> RichEditorPattern::GetSpanNodeBySpanItem(const RefPtr<SpanItem> spanItem)
{
    RefPtr<SpanNode> spanNode;
    auto iter = std::find(spans_.begin(), spans_.end(), spanItem);
    if (iter == spans_.end()) {
        return spanNode;
    }
    auto spanIndex = std::distance(spans_.begin(), iter);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, spanNode);
    auto it = host->GetChildren().begin();
    std::advance(it, spanIndex);
    spanNode = AceType::DynamicCast<SpanNode>(*it);
    return spanNode;
}

int32_t RichEditorPattern::DeleteValueSetSymbolSpan(
    const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::SYMBOL);
    spanResult.SetSpanRangeEnd(spanItem->position);
    spanResult.SetSpanRangeStart(spanItem->position - SYMBOL_SPAN_LENGTH);
    spanResult.SetEraseLength(SYMBOL_SPAN_LENGTH);
    return SYMBOL_SPAN_LENGTH;
}

int32_t RichEditorPattern::DeleteValueSetImageSpan(
    const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::IMAGE);
    spanResult.SetSpanRangeEnd(spanItem->position);
    spanResult.SetSpanRangeStart(spanItem->position - 1);
    spanResult.SetEraseLength(1);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, IMAGE_SPAN_LENGTH);
    auto uiNode = host->GetChildAtIndex(spanResult.GetSpanIndex());
    CHECK_NULL_RETURN(uiNode, IMAGE_SPAN_LENGTH);
    auto imageNode = AceType::DynamicCast<FrameNode>(uiNode);
    CHECK_NULL_RETURN(imageNode, IMAGE_SPAN_LENGTH);
    auto imageRenderCtx = imageNode->GetRenderContext();
    if (imageRenderCtx->GetBorderRadius()) {
        BorderRadiusProperty brp;
        auto jsonObject = JsonUtil::Create(true);
        auto jsonBorder = JsonUtil::Create(true);
        InspectorFilter filter;
        imageRenderCtx->GetBorderRadiusValue(brp).ToJsonValue(jsonObject, jsonBorder, filter);
        spanResult.SetBorderRadius(jsonObject->GetValue("borderRadius")->IsObject()
                                       ? jsonObject->GetValue("borderRadius")->ToString()
                                       : jsonObject->GetString("borderRadius"));
    }
    auto geometryNode = imageNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, IMAGE_SPAN_LENGTH);
    auto imageLayoutProperty = DynamicCast<ImageLayoutProperty>(imageNode->GetLayoutProperty());
    CHECK_NULL_RETURN(imageLayoutProperty, IMAGE_SPAN_LENGTH);
    spanResult.SetSizeWidth(geometryNode->GetMarginFrameSize().Width());
    spanResult.SetSizeHeight(geometryNode->GetMarginFrameSize().Height());
    if (imageLayoutProperty->GetMarginProperty()) {
        spanResult.SetMargin(imageLayoutProperty->GetMarginProperty()->ToString());
    }
    if (!imageLayoutProperty->GetImageSourceInfo()->GetPixmap()) {
        spanResult.SetValueResourceStr(imageLayoutProperty->GetImageSourceInfo()->GetSrc());
    } else {
        spanResult.SetValuePixelMap(imageLayoutProperty->GetImageSourceInfo()->GetPixmap());
    }
    if (imageLayoutProperty->HasImageFit()) {
        spanResult.SetImageFit(imageLayoutProperty->GetImageFitValue());
    }
    if (imageLayoutProperty->HasVerticalAlign()) {
        spanResult.SetVerticalAlign(imageLayoutProperty->GetVerticalAlignValue());
    }
    return IMAGE_SPAN_LENGTH;
}

int32_t RichEditorPattern::DeleteValueSetBuilderSpan(
    const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::IMAGE);
    spanResult.SetSpanRangeEnd(spanItem->position);
    spanResult.SetSpanRangeStart(spanItem->position - 1);
    spanResult.SetEraseLength(1);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, 1);
    auto uiNode = host->GetChildAtIndex(spanResult.GetSpanIndex());
    CHECK_NULL_RETURN(uiNode, 1);
    auto builderNode = AceType::DynamicCast<FrameNode>(uiNode);
    CHECK_NULL_RETURN(builderNode, 1);
    auto geometryNode = builderNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 1);
    spanResult.SetSizeWidth(geometryNode->GetMarginFrameSize().Width());
    spanResult.SetSizeHeight(geometryNode->GetMarginFrameSize().Height());
    return 1;
}

int32_t RichEditorPattern::DeleteValueSetTextSpan(
    const RefPtr<SpanItem>& spanItem, int32_t currentPosition, int32_t length, RichEditorAbstractSpanResult& spanResult)
{
    spanResult.SetSpanType(SpanResultType::TEXT);
    auto contentStartPosition = spanItem->position - StringUtils::ToWstring(spanItem->content).length();
    spanResult.SetSpanRangeStart(contentStartPosition);
    int32_t eraseLength = 0;
    if (spanItem->position - currentPosition >= length) {
        eraseLength = length;
    } else {
        eraseLength = spanItem->position - currentPosition;
    }
    spanResult.SetSpanRangeEnd(spanItem->position);
    if (previewTextRecord_.isPreviewTextInputting && !previewTextRecord_.deltaStr.empty()) {
        spanResult.SetPreviewText(previewTextRecord_.deltaStr);
    } else {
        spanResult.SetValue(spanItem->content);
    }
    spanResult.SetOffsetInSpan(currentPosition - contentStartPosition);
    spanResult.SetEraseLength(eraseLength);
    if (!spanItem->GetTextStyle().has_value()) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "SpanItem text style is empty.");
        return eraseLength;
    }
    spanResult.SetFontColor(spanItem->GetTextStyle()->GetTextColor().ColorToString());
    spanResult.SetFontSize(spanItem->GetTextStyle()->GetFontSize().Value());
    spanResult.SetFontStyle(spanItem->GetTextStyle()->GetFontStyle());
    spanResult.SetFontWeight((int32_t)(spanItem->GetTextStyle()->GetFontWeight()));
    if (!spanItem->GetTextStyle()->GetFontFamilies().empty()) {
        spanResult.SetFontFamily(spanItem->GetTextStyle()->GetFontFamilies().at(0));
    }
    spanResult.SetColor(spanItem->GetTextStyle()->GetTextDecorationColor().ColorToString());
    spanResult.SetTextDecoration(spanItem->GetTextStyle()->GetTextDecoration());
    spanResult.SetFontFeature(spanItem->GetTextStyle()->GetFontFeatures());
    auto host = GetHost();
    CHECK_NULL_RETURN(host, eraseLength);
    auto uiNode = host->GetChildAtIndex(spanResult.GetSpanIndex());
    CHECK_NULL_RETURN(uiNode, eraseLength);
    auto spanNode = DynamicCast<SpanNode>(uiNode);
    CHECK_NULL_RETURN(spanNode, eraseLength);
    spanResult.SetTextStyle(GetTextStyleObject(spanNode));
    return eraseLength;
}

void RichEditorPattern::DeleteByDeleteValueInfo(const RichEditorDeleteValue& info)
{
    auto deleteSpans = info.GetRichEditorDeleteSpans();
    if (deleteSpans.empty()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::list<RefPtr<UINode>> deleteNode;
    std::set<int32_t, std::greater<int32_t>> deleteNodes;
    auto caretMoveLength = 0;
    for (const auto& it : deleteSpans) {
        caretMoveLength += it.GetEraseLength();
        switch (it.GetType()) {
            case SpanResultType::TEXT: {
                auto ui_node = host->GetChildAtIndex(it.GetSpanIndex());
                CHECK_NULL_VOID(ui_node);
                auto spanNode = DynamicCast<SpanNode>(ui_node);
                CHECK_NULL_VOID(spanNode);
                auto spanItem = spanNode->GetSpanItem();
                CHECK_NULL_VOID(spanItem);
                auto text = spanItem->content;
                std::wstring textTemp = StringUtils::ToWstring(text);
                textTemp.erase(it.OffsetInSpan(), it.GetEraseLength());
                if (textTemp.size() == 0) {
                    deleteNodes.emplace(it.GetSpanIndex());
                }
                text = StringUtils::ToString(textTemp);
                spanNode->UpdateContent(text);
                spanItem->position -= it.GetEraseLength();
                break;
            }
            case SpanResultType::IMAGE:
                deleteNodes.emplace(it.GetSpanIndex());
                break;
            case SpanResultType::SYMBOL:
                deleteNodes.emplace(it.GetSpanIndex());
                break;
            default:
                break;
        }
    }
    RemoveEmptySpan(deleteNodes);
    if (info.GetRichEditorDeleteDirection() == RichEditorDeleteDirection::BACKWARD) {
        SetCaretPosition(std::clamp(caretPosition_ - caretMoveLength, 0, static_cast<int32_t>(GetTextContentLength())));
    }
    UpdateSpanPosition();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
    OnModifyDone();
}

void RichEditorPattern::RemoveEmptySpan(std::set<int32_t, std::greater<int32_t>>& deleteSpanIndexs)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    for (auto index : deleteSpanIndexs) {
        host->RemoveChildAtIndex(index);
        auto it = spans_.begin();
        std::advance(it, index);
        if (it != spans_.end()) {
            spans_.erase(it);
        }
    }
}

RefPtr<GestureEventHub> RichEditorPattern::GetGestureEventHub() {
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    return host->GetOrCreateGestureEventHub();
}

bool RichEditorPattern::OnKeyEvent(const KeyEvent& keyEvent)
{
    return TextInputClient::HandleKeyEvent(keyEvent);
}

void RichEditorPattern::CursorMove(CaretMoveIntent direction)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "direction=%{public}d", direction);
    switch (direction) {
        case CaretMoveIntent::Left:
            CursorMoveLeft();
            break;
        case CaretMoveIntent::Right:
            CursorMoveRight();
            break;
        case CaretMoveIntent::Up:
            CursorMoveUp();
            break;
        case CaretMoveIntent::Down:
            CursorMoveDown();
            break;
        case CaretMoveIntent::LeftWord:
            CursorMoveLeftWord();
            break;
        case CaretMoveIntent::RightWord:
            CursorMoveRightWord();
            break;
        case CaretMoveIntent::ParagraghBegin:
            CursorMoveToParagraphBegin();
            break;
        case CaretMoveIntent::ParagraghEnd:
            CursorMoveToParagraphEnd();
            break;
        case CaretMoveIntent::Home:
            CursorMoveHome();
            break;
        case CaretMoveIntent::End:
            CursorMoveEnd();
            break;
        case CaretMoveIntent::LineBegin:
            CursorMoveLineBegin();
            break;
        case CaretMoveIntent::LineEnd:
            CursorMoveLineEnd();
            break;
        default:
            LOGW("Unsupported cursor move operation for rich editor");
    }
}

void RichEditorPattern::MoveCaretAfterTextChange()
{
    CHECK_NULL_VOID(isTextChange_);
    isTextChange_ = false;
    switch (moveDirection_) {
        case MoveDirection::BACKWARD:
            SetCaretPosition(
                std::clamp((caretPosition_ - moveLength_), 0, static_cast<int32_t>(GetTextContentLength())));
            break;
        case MoveDirection::FORWARD:
            SetCaretPosition(
                std::clamp((caretPosition_ + moveLength_), 0, static_cast<int32_t>(GetTextContentLength())));
            break;
        default:
            break;
    }
    moveLength_ = 0;
}

void RichEditorPattern::InitTouchEvent()
{
    CHECK_NULL_VOID(!touchListener_);
    auto gesture = GetGestureEventHub();
    CHECK_NULL_VOID(gesture);
    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleTouchEvent(info);
    };
    touchListener_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gesture->AddTouchEvent(touchListener_);
}

void RichEditorPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    auto touchInfo = info.GetTouches().front();
    auto touchType = touchInfo.GetTouchType();
    if (touchType == TouchType::DOWN) {
        HandleOnlyImageSelected(touchInfo.GetLocalLocation(), info.GetSourceTool() == SourceTool::FINGER);
        HandleTouchDown(info.GetTouches().front().GetLocalLocation());
    } else if (touchType == TouchType::UP) {
        isOnlyImageDrag_ = false;
        HandleTouchUp();
    } else if (touchType == TouchType::MOVE) {
        HandleTouchMove(info.GetTouches().front().GetLocalLocation());
    }
}

void RichEditorPattern::HandleTouchDown(const Offset& offset)
{
    CHECK_NULL_VOID(HasFocus());
    CHECK_NULL_VOID(overlayMod_);
    float caretHeight = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretHeight();
    OffsetF caretOffset = DynamicCast<RichEditorOverlayModifier>(overlayMod_)->GetCaretOffset();
    RectF lastCaretRect = RectF{ caretOffset.GetX() - caretHeight / 2, caretOffset.GetY(), caretHeight, caretHeight };
    isTouchCaret_ = RepeatClickCaret(offset, caretPosition_, lastCaretRect);
}

void RichEditorPattern::HandleTouchUp()
{
    if (isTouchCaret_ && selectOverlay_->IsSingleHandleShow()) {
        selectOverlay_->ShowMenu();
    }
    isTouchCaret_ = false;
    if (magnifierController_->GetShowMagnifier()) {
        magnifierController_->UpdateShowMagnifier();
    }
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    if (isLongPress_) {
        isLongPress_ = false;
    }
#endif
}

void RichEditorPattern::HandleTouchMove(const Offset& offset)
{
    CHECK_NULL_VOID(isTouchCaret_);
    Offset textOffset = ConvertTouchOffsetToTextOffset(offset);
    auto position = paragraphs_.GetIndex(textOffset);
    SetCaretPosition(position);
    CalcAndRecordLastClickCaretInfo(textOffset);
    if (selectOverlay_->IsSingleHandleShow()) {
        textSelector_.Update(caretPosition_);
        CalculateHandleOffsetAndShowOverlay();
        selectOverlay_->HideMenu();
        selectOverlay_->UpdateSecondHandleOffset();
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

bool RichEditorPattern::IsScrollBarPressed(const MouseInfo& info)
{
    auto scrollBar = GetScrollBar();
    Point point(info.GetLocalLocation().GetX(), info.GetLocalLocation().GetY());
    return scrollBar->InBarTouchRegion(point);
}

void RichEditorPattern::HandleMouseLeftButtonMove(const MouseInfo& info)
{
    if (blockPress_ || !leftMousePress_) {
        if (blockPress_) {
            dragBoxes_ = GetTextBoxes();
        }
        return;
    }

    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
        info.GetLocalLocation().GetY() - textPaintOffset.GetY() };
    if (dataDetectorAdapter_->pressedByLeftMouse_) {
        dataDetectorAdapter_->pressedByLeftMouse_ = false;
        MoveCaretAndStartFocus(textOffset);
    }

    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    if (!focusHub->IsCurrentFocus()) {
        return;
    }
    mouseStatus_ = MouseStatus::MOVE;
    if (isFirstMouseSelect_) {
        int32_t extend = paragraphs_.GetIndex(textOffset);
        UpdateSelector(textSelector_.baseOffset, extend);
        isFirstMouseSelect_ = false;
    } else {
        int32_t extend = paragraphs_.GetIndex(textOffset);
        UpdateSelector(textSelector_.baseOffset, extend);
        auto position = paragraphs_.GetIndex(textOffset);
        AdjustCursorPosition(position);
        SetCaretPosition(position);
        AutoScrollParam param = {
            .autoScrollEvent = AutoScrollEvent::MOUSE, .showScrollbar = true, .eventOffset = info.GetLocalLocation()
        };
        AutoScrollByEdgeDetection(param, OffsetF(info.GetLocalLocation().GetX(), info.GetLocalLocation().GetY()),
            EdgeDetectionStrategy::OUT_BOUNDARY);
        showSelect_ = true;
    }
    if (textSelector_.SelectNothing()) {
        if (!caretTwinkling_) {
            StartTwinkling();
        }
    } else {
        StopTwinkling();
    }
    isMouseSelect_ = true;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::HandleMouseLeftButtonPress(const MouseInfo& info)
{
    isMousePressed_ = true;
    if (IsScrollBarPressed(info) || BetweenSelectedPosition(info.GetGlobalLocation())) {
        blockPress_ = true;
        return;
    }
    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    if (!focusHub->IsFocusable()) {
        return;
    }
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
        info.GetLocalLocation().GetY() - textPaintOffset.GetY() };
    if (textSelector_.baseOffset != textSelector_.destinationOffset) {
        ResetSelection();
    }
    int32_t extend = paragraphs_.GetIndex(textOffset);
    textSelector_.Update(extend);
    leftMousePress_ = true;
    mouseStatus_ = MouseStatus::PRESSED;
    blockPress_ = false;
    caretUpdateType_ = CaretUpdateType::PRESSED;
    dataDetectorAdapter_->pressedByLeftMouse_ = false;
    HandleClickAISpanEvent(PointF(textOffset.GetX(), textOffset.GetY()));
    if (dataDetectorAdapter_->pressedByLeftMouse_) {
        return;
    }
    UseHostToUpdateTextFieldManager();
    MoveCaretAndStartFocus(textOffset);
    CalcCaretInfoByClick(info.GetLocalLocation());
}

void RichEditorPattern::HandleMouseLeftButtonRelease(const MouseInfo& info)
{
    blockPress_ = false;
    leftMousePress_ = false;
    auto oldMouseStatus = mouseStatus_;
    mouseStatus_ = MouseStatus::RELEASED;
    isMouseSelect_ = false;
    isMousePressed_ = false;
    isFirstMouseSelect_ = true;
    isOnlyImageDrag_ = false;
    if (dataDetectorAdapter_->pressedByLeftMouse_ && oldMouseStatus != MouseStatus::MOVE && !IsDragging()) {
        dataDetectorAdapter_->ResponseBestMatchItem(dataDetectorAdapter_->clickedAISpan_);
        dataDetectorAdapter_->pressedByLeftMouse_ = false;
        return;
    }

    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    if (selectStart != selectEnd) {
        FireOnSelect(selectStart, selectEnd);
    }
    StopAutoScroll();
    if (textSelector_.IsValid() && !textSelector_.StartEqualToDest() && IsSelectedBindSelectionMenu() &&
        oldMouseStatus == MouseStatus::MOVE) {
        auto offsetX = static_cast<float>(info.GetGlobalLocation().GetX());
        auto offsetY = static_cast<float>(info.GetGlobalLocation().GetY());
        selectionMenuOffsetByMouse_ = OffsetF(offsetX, offsetY);
        selectionMenuOffsetClick_ = OffsetF(offsetX, offsetY);
        ShowSelectOverlay(RectF(), RectF(), false, TextResponseType::SELECTED_BY_MOUSE);
    }
    if (HasFocus()) {
        HandleOnEditChanged(true);
    }
}

void RichEditorPattern::HandleMouseLeftButton(const MouseInfo& info)
{
    if (info.GetAction() == MouseAction::MOVE) {
        HandleMouseLeftButtonMove(info);
    } else if (info.GetAction() == MouseAction::PRESS) {
        HandleMouseLeftButtonPress(info);
    } else if (info.GetAction() == MouseAction::RELEASE) {
        HandleMouseLeftButtonRelease(info);
    }
}

void RichEditorPattern::HandleMouseRightButton(const MouseInfo& info)
{
    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    if (!focusHub->IsFocusable()) {
        return;
    }
    if (info.GetAction() == MouseAction::PRESS) {
        isMousePressed_ = true;
        usingMouseRightButton_ = true;
        CloseSelectionMenu();
    } else if (info.GetAction() == MouseAction::RELEASE) {
        auto offsetX = static_cast<float>(info.GetGlobalLocation().GetX());
        auto offsetY = static_cast<float>(info.GetGlobalLocation().GetY());
        selectionMenuOffsetByMouse_ = OffsetF(offsetX, offsetY);
        selectionMenuOffsetClick_ = OffsetF(offsetX, offsetY);
        if (textSelector_.IsValid() && BetweenSelectedPosition(info.GetGlobalLocation())) {
            ShowSelectOverlay(RectF(), RectF(), IsSelectAll(), TextResponseType::RIGHT_CLICK);
            isMousePressed_ = false;
            usingMouseRightButton_ = false;
            return;
        }
        auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0f, std::min(baselineOffset_, 0.0f));
        Offset textOffset = { info.GetLocalLocation().GetX() - textPaintOffset.GetX(),
            info.GetLocalLocation().GetY() - textPaintOffset.GetY() };
        HandleClickAISpanEvent(PointF(textOffset.GetX(), textOffset.GetY()));
        if (dataDetectorAdapter_->hasClickedAISpan_) {
            dataDetectorAdapter_->hasClickedAISpan_ = false;
            isMousePressed_ = false;
            usingMouseRightButton_ = false;
            return;
        }
        if (textSelector_.IsValid()) {
            CloseSelectOverlay();
            ResetSelection();
        }
        MouseRightFocus(info);
        ShowSelectOverlay(RectF(), RectF(), IsSelectAll(), TextResponseType::RIGHT_CLICK);
        isMousePressed_ = false;
        usingMouseRightButton_ = false;
    }
}

void RichEditorPattern::MouseRightFocus(const MouseInfo& info)
{
    Offset textOffset = ConvertTouchOffsetToTextOffset(info.GetLocalLocation());
    InitSelection(textOffset);
    auto selectStart = std::min(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto selectEnd = std::max(textSelector_.baseOffset, textSelector_.destinationOffset);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    SetCaretPosition(selectEnd);

    TextInsertValueInfo spanInfo;
    CalcInsertValueObj(spanInfo);
    auto spanNode = DynamicCast<FrameNode>(GetChildByIndex(spanInfo.GetSpanIndex() - 1));
    if (spanNode && spanNode->GetTag() == V2::IMAGE_ETS_TAG && spanInfo.GetOffsetInSpan() == 0 &&
        selectEnd == selectStart + 1 && BetweenSelectedPosition(info.GetGlobalLocation())) {
        selectedType_ = TextSpanType::IMAGE;
        FireOnSelect(selectStart, selectEnd);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        return;
    }
    if (textSelector_.IsValid()) {
        ResetSelection();
    }
    auto position = paragraphs_.GetIndex(textOffset);
    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    SetCaretPosition(position);
    selectedType_ = TextSpanType::TEXT;
    CHECK_NULL_VOID(overlayMod_);
    DynamicCast<RichEditorOverlayModifier>(overlayMod_)->SetCaretOffsetAndHeight(caretOffset, caretHeight);
    StartTwinkling();
}

void RichEditorPattern::FireOnSelect(int32_t selectStart, int32_t selectEnd)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    if (!textSelectInfo.GetSelection().resultObjects.empty()) {
        eventHub->FireOnSelect(&textSelectInfo);
    }
    UpdateSelectionType(textSelectInfo);
}

void RichEditorPattern::HandleMouseEvent(const MouseInfo& info)
{
    auto tmpHost = GetHost();
    CHECK_NULL_VOID(tmpHost);
    auto frameId = tmpHost->GetId();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto scrollBar = GetScrollBar();
    if (scrollBar && (scrollBar->IsHover() || scrollBar->IsPressed())) {
        pipeline->SetMouseStyleHoldNode(frameId);
        pipeline->ChangeMouseStyle(frameId, MouseFormat::DEFAULT);
        return;
    }

    caretUpdateType_ = CaretUpdateType::NONE;
    if (info.GetButton() == MouseButton::LEFT_BUTTON) {
        HandleMouseLeftButton(info);
    } else if (info.GetButton() == MouseButton::RIGHT_BUTTON) {
        HandleMouseRightButton(info);
    }
}

void RichEditorPattern::CopySelectionMenuParams(SelectOverlayInfo& selectInfo, TextResponseType responseType)
{
    auto selectType = selectedType_.value_or(TextSpanType::NONE);
    std::shared_ptr<SelectionMenuParams> menuParams = nullptr;
    menuParams = GetMenuParams(selectType, responseType);
    if (menuParams == nullptr) {
        return;
    }

    // long pressing on the image needs to set the position of the pop-up menu following the long pressing position
    if (selectType == TextSpanType::IMAGE && !selectInfo.isUsingMouse) {
        selectInfo.menuInfo.menuOffset = OffsetF(selectionMenuOffset_.GetX(), selectionMenuOffset_.GetY());
    }

    CopyBindSelectionMenuParams(selectInfo, menuParams);
}

void RichEditorPattern::ShowSelectOverlay(const RectF& firstHandle, const RectF& secondHandle, bool isCopyAll,
    TextResponseType responseType, bool handleReverse)
{
    textResponseType_ = responseType;
    selectOverlay_->ProcessOverlay({.animation = true});
}

void RichEditorPattern::CheckEditorTypeChange()
{
    CHECK_NULL_VOID(selectOverlayProxy_);
    CHECK_NULL_VOID(!selectOverlayProxy_->IsClosed());
    if (selectOverlayProxy_->GetSelectOverlayMangerInfo().menuInfo.editorType.value_or(static_cast<int32_t>(
            TextSpanType::NONE)) != static_cast<int32_t>(selectedType_.value_or(TextSpanType::NONE))) {
        CloseSelectionMenu();
    }
}

void RichEditorPattern::OnCopyOperation(bool isUsingExternalKeyboard)
{
    auto selectStart = textSelector_.GetTextStart();
    auto selectEnd = textSelector_.GetTextEnd();
    auto textSelectInfo = GetSpansInfo(selectStart, selectEnd, GetSpansMethod::ONSELECT);
    auto copyResultObjects = textSelectInfo.GetSelection().resultObjects;
    caretUpdateType_ = CaretUpdateType::NONE;
    if (copyResultObjects.empty()) {
        return;
    }
    RefPtr<PasteDataMix> pasteData = clipboard_->CreatePasteDataMix();
    auto resultProcessor = [weak = WeakClaim(this), pasteData, selectStart, selectEnd, clipboard = clipboard_](
                               const ResultObject& result) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (result.type == SelectSpanType::TYPESPAN) {
            auto data = pattern->GetSelectedSpanText(StringUtils::ToWstring(result.valueString),
                result.offsetInSpan[RichEditorSpanRange::RANGESTART],
                result.offsetInSpan[RichEditorSpanRange::RANGEEND]);
            clipboard->AddTextRecord(pasteData, data);
            return;
        }
        if (result.type == SelectSpanType::TYPEIMAGE) {
            if (result.valuePixelMap) {
                clipboard->AddPixelMapRecord(pasteData, result.valuePixelMap);
            } else {
                clipboard->AddImageRecord(pasteData, result.valueString);
            }
        }
    };
    for (auto resultObj = copyResultObjects.rbegin(); resultObj != copyResultObjects.rend(); ++resultObj) {
        resultProcessor(*resultObj);
    }
    clipboard_->SetData(pasteData, copyOption_);
}

void RichEditorPattern::HandleOnCopy(bool isUsingExternalKeyboard)
{
    CHECK_NULL_VOID(clipboard_);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "isUsingExternalKeyboard=%{public}d, copyOption=%{public}d",
        isUsingExternalKeyboard, copyOption_);
    if (copyOption_ == CopyOptions::None) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    TextCommonEvent event;
    eventHub->FireOnCopy(event);
    if (event.IsPreventDefault()) {
        CloseSelectOverlay();
        ResetSelection();
        StartTwinkling();
        return;
    }
    OnCopyOperation(isUsingExternalKeyboard);
    if (selectOverlay_->IsUsingMouse() || isUsingExternalKeyboard) {
        CloseSelectOverlay();
    } else {
        selectOverlay_->HideMenu();
    }
}

void RichEditorPattern::ResetAfterPaste()
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "ResetAfterPaste");
    auto pasteStr = GetPasteStr();
    SetCaretSpanIndex(-1);
    StartTwinkling();
    CloseSelectOverlay();
    InsertValueByPaste(pasteStr);
    ClearPasteStr();
}

void RichEditorPattern::InsertValueByPaste(const std::string& pasteStr)
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "InsertValueByPaste");
    InsertValue(pasteStr, false);
}

void RichEditorPattern::HandleOnPaste()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    TextCommonEvent event;
    eventHub->FireOnPaste(event);
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "HandleOnPaste, preventDefault=%{public}d", event.IsPreventDefault());
    if (event.IsPreventDefault()) {
        CloseSelectOverlay();
        ResetSelection();
        StartTwinkling();
        return;
    }
    CHECK_NULL_VOID(clipboard_);
    auto pasteCallback = [weak = WeakClaim(this)](const std::string& data) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "pasteCallback callback");
        auto richEditor = weak.Upgrade();
        CHECK_NULL_VOID(richEditor);
        if (data.empty()) {
            richEditor->ResetSelection();
            richEditor->StartTwinkling();
            richEditor->CloseSelectOverlay();
            return;
        }
        richEditor->AddPasteStr(data);
        richEditor->ResetAfterPaste();
    };
    clipboard_->GetData(pasteCallback);
}

void RichEditorPattern::SetCaretSpanIndex(int32_t index)
{
    caretSpanIndex_ = index;
}

void RichEditorPattern::HandleOnCut()
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "copyOption=%{public}d, textSelector_.IsValid()=%{public}d",
        copyOption_, textSelector_.IsValid());
    if (copyOption_ == CopyOptions::None) {
        return;
    }
    if (!textSelector_.IsValid()) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    TextCommonEvent event;
    eventHub->FireOnCut(event);
    if (event.IsPreventDefault()) {
        CloseSelectOverlay();
        ResetSelection();
        StartTwinkling();
        return;
    }

    caretUpdateType_ = CaretUpdateType::NONE;
    OnCopyOperation();
    DeleteBackward(1);
}

std::function<void(Offset)> RichEditorPattern::GetThumbnailCallback()
{
    return [wk = WeakClaim(this)](const Offset& point) {
        auto pattern = wk.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->BetweenSelectedPosition(point)) {
            return;
        }
        auto gesture = pattern->GetGestureEventHub();
        CHECK_NULL_VOID(gesture);
        auto isContentDraggable = pattern->JudgeContentDraggable();
        if (!isContentDraggable) {
            gesture->SetIsTextDraggable(false);
            return;
        }
        auto host = pattern->GetHost();
        auto children = host->GetChildren();
        std::list<RefPtr<FrameNode>> imageChildren;
        for (const auto& child : children) {
            auto node = DynamicCast<FrameNode>(child);
            if (!node) {
                continue;
            }
            auto tag = node->GetTag();
            if (tag == V2::IMAGE_ETS_TAG || tag == V2::PLACEHOLDER_SPAN_ETS_TAG) {
                imageChildren.emplace_back(node);
            }
        }
        RichEditorDragInfo info;
        info.selectedWidth = pattern->GetSelectedMaxWidth();
        info.handleColor = pattern->GetCaretColor();
        info.selectedBackgroundColor = pattern->GetSelectedBackgroundColor();
        pattern->CalculateHandleOffsetAndShowOverlay();
        auto firstHandleInfo = pattern->GetFirstHandleInfo();
        if (firstHandleInfo.has_value() && firstHandleInfo.value().isShow) {
            info.firstHandle = pattern->textSelector_.firstHandle;
        }
        auto secondHandleInfo = pattern->GetSecondHandleInfo();
        if (secondHandleInfo.has_value() && secondHandleInfo.value().isShow) {
            info.secondHandle = pattern->textSelector_.secondHandle;
        }
        pattern->dragNode_ = RichEditorDragPattern::CreateDragNode(host, imageChildren, info);
        auto textDragPattern = pattern->dragNode_->GetPattern<TextDragPattern>();
        if (textDragPattern) {
            auto option = host->GetDragPreviewOption();
            option.options.shadowPath = textDragPattern->GetBackgroundPath()->ConvertToSVGString();
            option.options.shadow = Shadow(RICH_DEFAULT_ELEVATION, {0.0, 0.0}, Color(RICH_DEFAULT_SHADOW_COLOR),
                ShadowStyle::OuterFloatingSM);
            host->SetDragPreviewOptions(option);
        }
        FrameNode::ProcessOffscreenNode(pattern->dragNode_);
    };
}

void RichEditorPattern::CreateHandles()
{
    if (IsDragging()) {
        TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "do not show handles when dragging");
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    CalculateHandleOffsetAndShowOverlay();
    selectOverlay_->ProcessOverlay({ .menuIsShow = selectOverlay_->IsCurrentMenuVisibile(), .animation = true });
}

void RichEditorPattern::ShowHandles(const bool isNeedShowHandles)
{
    if (!isNeedShowHandles) {
        auto info = GetSpansInfo(textSelector_.GetTextStart(), textSelector_.GetTextEnd(), GetSpansMethod::ONSELECT);
        auto selResult = info.GetSelection().resultObjects;
        if (selResult.size() == 1 && selResult.front().type == SelectSpanType::TYPEIMAGE) {
            textSelector_.Update(-1, -1);
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
            return;
        }
    }
    ShowHandles();
}

void RichEditorPattern::ShowHandles()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (!selectOverlay_->IsHandlesShow() && !selectOverlay_->SelectOverlayIsCreating()) {
        showSelect_ = true;
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        selectOverlay_->ProcessOverlay({.animation = false});
    }
}
void RichEditorPattern::OnAreaChangedInner()
{
    float selectLineHeight = 0.0f;
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto parentGlobalOffset = host->GetPaintRectOffset() - context->GetRootRect().GetOffset();
    if (parentGlobalOffset != parentGlobalOffset_) {
        parentGlobalOffset_ = parentGlobalOffset;
        UpdateTextFieldManager(Offset(parentGlobalOffset_.GetX(), parentGlobalOffset_.GetY()), frameRect_.Height());
        CHECK_NULL_VOID(SelectOverlayIsOn());
        textSelector_.selectionBaseOffset.SetX(
            CalcCursorOffsetByPosition(textSelector_.GetStart(), selectLineHeight).GetX());
        textSelector_.selectionDestinationOffset.SetX(
            CalcCursorOffsetByPosition(textSelector_.GetEnd(), selectLineHeight).GetX());
        CreateHandles();
        selectOverlay_->HideMenu(true);
    }
}

void RichEditorPattern::CloseSelectionMenu()
{
    CloseSelectOverlay();
}

void RichEditorPattern::CloseSelectOverlay()
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "CloseSelectOverlay");
    selectOverlay_->CloseOverlay(true, CloseReason::CLOSE_REASON_NORMAL);
}

void RichEditorPattern::CloseHandleAndSelect()
{
    selectOverlay_->CloseOverlay(false, CloseReason::CLOSE_REASON_NORMAL);
    showSelect_ = false;
}

void RichEditorPattern::CalculateHandleOffsetAndShowOverlay(bool isUsingMouse)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto rootOffset = pipeline->GetRootRect().GetOffset();
    auto offset = host->GetPaintRectOffset();
    auto textPaintOffset = offset - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    textSelector_.ReverseTextSelector();
    int32_t baseOffset = std::min(textSelector_.baseOffset, GetTextContentLength());
    int32_t destinationOffset = std::min(textSelector_.destinationOffset, GetTextContentLength());
    SizeF firstHandlePaintSize;
    SizeF secondHandlePaintSize;
    OffsetF firstHandleOffset;
    OffsetF secondHandleOffset;
    auto isSingleHandle = IsSingleHandle();
    selectOverlay_->SetIsSingleHandle(isSingleHandle);
    if (isSingleHandle) {
        auto [caretOffset, caretHeight] = CalculateCaretOffsetAndHeight();
        // only show the second handle.
        secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), caretHeight };
        secondHandleOffset = caretOffset + textPaintOffset - rootOffset;
    } else {
        float startSelectHeight = 0.0f;
        float endSelectHeight = 0.0f;
        auto startOffset = CalcCursorOffsetByPosition(baseOffset, startSelectHeight, true, false);
        auto endOffset = CalcCursorOffsetByPosition(destinationOffset, endSelectHeight, false, false);
        firstHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), startSelectHeight };
        secondHandlePaintSize = { SelectHandleInfo::GetDefaultLineWidth().ConvertToPx(), endSelectHeight };
        firstHandleOffset = startOffset + textPaintOffset - rootOffset;
        secondHandleOffset = endOffset + textPaintOffset - rootOffset;
    }
    textSelector_.selectionBaseOffset = firstHandleOffset;
    textSelector_.selectionDestinationOffset = secondHandleOffset;
    textSelector_.firstHandle = RectF{ firstHandleOffset, firstHandlePaintSize };
    textSelector_.secondHandle = RectF{ secondHandleOffset, secondHandlePaintSize };
}

bool RichEditorPattern::IsSingleHandle()
{
    return GetTextContentLength() == 0 || !IsSelected();
}

bool RichEditorPattern::IsHandlesShow()
{
    return selectOverlay_->IsHandlesShow();
}

void RichEditorPattern::ResetSelection()
{
    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "ResetSelection");
    bool selectNothing = textSelector_.SelectNothing();
    textSelector_.Update(-1, -1);
    if (!selectNothing) {
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto eventHub = host->GetEventHub<RichEditorEventHub>();
        CHECK_NULL_VOID(eventHub);
        auto textSelectInfo = GetSpansInfo(-1, -1, GetSpansMethod::ONSELECT);
        eventHub->FireOnSelect(&textSelectInfo);
        UpdateSelectionType(textSelectInfo);
        host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    }
}

bool RichEditorPattern::BetweenSelectedPosition(const Offset& globalOffset)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto offset = host->GetPaintRectOffset();
    auto localOffset = globalOffset - Offset(offset.GetX(), offset.GetY());
    auto eventHub = host->GetEventHub<EventHub>();
    if (copyOption_ != CopyOptions::None && GreatNotEqual(textSelector_.GetTextEnd(), textSelector_.GetTextStart())) {
        // Determine if the pan location is in the selected area
        auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
        auto panOffset = OffsetF(localOffset.GetX(), localOffset.GetY()) - GetTextRect().GetOffset() +
                         OffsetF(0.0, std::min(baselineOffset_, 0.0f));
        for (const auto& selectedRect : selectedRects) {
            if (selectedRect.IsInRegion(PointF(panOffset.GetX(), panOffset.GetY()))) {
                return true;
            }
        }
    }
    return false;
}

void RichEditorPattern::HandleSurfaceChanged(int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight)
{
    if (newWidth != prevWidth || newHeight != prevHeight) {
        TextPattern::HandleSurfaceChanged(newWidth, newHeight, prevWidth, prevHeight);
        UpdateOriginIsMenuShow(false);
    }
    UpdateCaretInfoToController();
}

void RichEditorPattern::HandleSurfacePositionChanged(int32_t posX, int32_t posY)
{
    UpdateCaretInfoToController();
}

void RichEditorPattern::DumpInfo()
{
    if (customKeyboardBuilder_) {
        DumpLog::GetInstance().AddDesc(std::string("CustomKeyboard: true")
                                           .append(", Attached: ")
                                           .append(std::to_string(isCustomKeyboardAttached_)));
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = host->GetContext();
    CHECK_NULL_VOID(context);
    auto richEditorTheme = context->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richEditorTheme);
    DumpLog::GetInstance().AddDesc(std::string("caret offset: ").append(GetCaretRect().GetOffset().ToString()));
    DumpLog::GetInstance().AddDesc(
        std::string("caret height: ")
            .append(std::to_string(NearZero(GetCaretRect().Height())
                                       ? richEditorTheme->GetDefaultCaretHeight().ConvertToPx()
                                       : GetCaretRect().Height())));
}

bool RichEditorPattern::HasFocus() const
{
    auto focusHub = GetFocusHub();
    CHECK_NULL_RETURN(focusHub, false);
    return focusHub->IsCurrentFocus();
}

void RichEditorPattern::UpdateTextFieldManager(const Offset& offset, float height)
{
    if (!HasFocus()) {
        return;
    }
    auto context = GetHost()->GetContext();
    CHECK_NULL_VOID(context);
    auto richEditorTheme = context->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(richEditorTheme);
    auto textFieldManager = DynamicCast<TextFieldManagerNG>(context->GetTextFieldManager());
    CHECK_NULL_VOID(textFieldManager);
    auto [caretOffset, caretHeight] = CalculateCaretOffsetAndHeight();
    textFieldManager->SetClickPosition({ offset.GetX() + caretOffset.GetX(), offset.GetY() + caretOffset.GetY() });
    textFieldManager->SetHeight(NearZero(caretHeight)
                                    ? richEditorTheme->GetDefaultCaretHeight().ConvertToPx()
                                    : caretHeight);
    textFieldManager->SetOnFocusTextField(WeakClaim(this));

    if (!isTextChange_) {
        return;
    }
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    taskExecutor->PostTask(
        [weak = WeakClaim(this)] {
            auto pattern = weak.Upgrade();
            CHECK_NULL_VOID(pattern);
            pattern->ScrollToSafeArea();
        },
        TaskExecutor::TaskType::UI, "ArkUIRichEditorScrollToSafeArea");
}

bool RichEditorPattern::IsDisabled() const
{
    auto eventHub = GetHost()->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, true);
    return !eventHub->IsEnabled();
}

void RichEditorPattern::InitSelection(const Offset& pos)
{
    int32_t currentPosition = paragraphs_.GetIndex(pos);
    currentPosition = std::min(currentPosition, GetTextContentLength());
    if (IsTouchBeforeCaret(currentPosition, pos)) {
        currentPosition = std::max(0, currentPosition - 1);
    }
    int32_t nextPosition = currentPosition + GetGraphemeClusterLength(GetWideText(), currentPosition);
    nextPosition = std::min(nextPosition, GetTextContentLength());
    adjusted_ = AdjustWordSelection(currentPosition, nextPosition);
    textSelector_.Update(currentPosition, nextPosition);
    auto selectedRects = paragraphs_.GetRects(currentPosition, nextPosition);
    bool isSelectEmpty = selectedRects.empty() || (selectedRects.size() == 1 && NearZero((selectedRects[0].Width())));
    if (isSelectEmpty) {
        textSelector_.Update(currentPosition, currentPosition);
    }
}

void RichEditorPattern::HandleSelectOverlayWithOptions(const SelectionOptions& options)
{
    if (options.menuPolicy == MenuPolicy::SHOW) {
        if (isMousePressed_ || sourceType_ == SourceType::MOUSE) {
            selectionMenuOffsetByMouse_ = selectionMenuOffsetClick_;
        }
        if (SelectOverlayIsOn()) {
            selectOverlay_->ProcessOverlay({.animation = true, .requestCode = REQUEST_RECREATE});
        } else {
            ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle, IsSelectAll());
        }
    } else if (options.menuPolicy == MenuPolicy::HIDE) {
        if (SelectOverlayIsOn()) {
            CloseSelectOverlay();
        }
    }
}

bool RichEditorPattern::ResetOnInvalidSelection(int32_t start, int32_t end)
{
    if (start < end) {
        return false;
    }
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "SetSelection failed, the selected area is empty.");
    CloseSelectOverlay();
    ResetSelection();
    StartTwinkling();
    return true;
}

void RichEditorPattern::RefreshSelectOverlay(bool isMousePressed, bool selectedTypeChange)
{
    if (isMousePressed && !selectedTypeChange) {
        return;
    }
    CloseSelectOverlay();
    auto responseType = static_cast<TextResponseType>(
        selectOverlayProxy_->GetSelectOverlayMangerInfo().menuInfo.responseType.value_or(0));
    ShowSelectOverlay(textSelector_.firstHandle, textSelector_.secondHandle, IsSelectAll(), responseType);
}

bool RichEditorPattern::IsShowHandle()
{
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_RETURN(richEditorTheme, false);
    return !richEditorTheme->IsRichEditorShowHandle();
}

void RichEditorPattern::UpdateSelectionInfo(int32_t start, int32_t end)
{
    UpdateSelectionType(GetSpansInfo(start, end, GetSpansMethod::ONSELECT));
    auto selectOverlayInfo = selectOverlay_->GetSelectOverlayInfo();
    textResponseType_ = selectOverlayInfo
                        ? static_cast<TextResponseType>(selectOverlayInfo->menuInfo.responseType.value_or(0))
                        : TextResponseType::LONG_PRESS;
    if (IsShowHandle() && !selectOverlay_->IsUsingMouse()) {
        ResetIsMousePressed();
        sourceType_ = SourceType::TOUCH;
    } else {
        isMousePressed_ = true;
    }
}

void RichEditorPattern::SetSelection(int32_t start, int32_t end, const std::optional<SelectionOptions>& options,
    bool isForward)
{
    bool hasFocus = HasFocus();
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "range=[%{public}d,%{public}d], hasFocus=%{public}d", start, end, hasFocus);
    CHECK_NULL_VOID(hasFocus);
    if (start == -1 && end == -1) {
        start = 0;
        end = GetTextContentLength();
    } else {
        start = std::clamp(start, 0, GetTextContentLength());
        end = std::clamp(end, 0, GetTextContentLength());
    }
    if (ResetOnInvalidSelection(start, end)) {
        return;
    }
    UpdateSelector(start, end);

    if (textSelector_.IsValid() && !textSelector_.StartEqualToDest()) {
        StopTwinkling();
        if (start != textSelector_.GetTextStart() || end != textSelector_.GetTextEnd()) {
            FireOnSelect(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
        }
    }
    SetCaretPosition(isForward ? textSelector_.GetTextStart() : textSelector_.GetTextEnd());
    MoveCaretToContentRect();
    CalculateHandleOffsetAndShowOverlay();
    UpdateSelectionInfo(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    if (!IsShowHandle()) {
        CloseSelectOverlay();
    } else if (!options.has_value() || options.value().menuPolicy == MenuPolicy::DEFAULT) {
        selectOverlay_->ProcessOverlay({ .menuIsShow = selectOverlay_->IsCurrentMenuVisibile(),
           .animation = true, .requestCode = REQUEST_RECREATE });
    } else if (options.value().menuPolicy == MenuPolicy::HIDE) {
        if (selectOverlay_->IsUsingMouse()) {
            CloseSelectOverlay();
        } else {
            selectOverlay_->ProcessOverlay({ .menuIsShow = false, .animation = true });
        }
    } else if (options.value().menuPolicy == MenuPolicy::SHOW) {
        if (selectOverlay_->IsUsingMouse() || sourceType_ == SourceType::MOUSE) {
            selectionMenuOffsetByMouse_ = selectionMenuOffsetClick_;
        }
        selectOverlay_->ProcessOverlay({ .animation = true, .requestCode = REQUEST_RECREATE });
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::BindSelectionMenu(TextResponseType type, TextSpanType richEditorType,
    std::function<void()>& menuBuilder, std::function<void(int32_t, int32_t)>& onAppear,
    std::function<void()>& onDisappear)
{
    TextPattern::BindSelectionMenu(richEditorType, type, menuBuilder, onAppear, onDisappear);
}

RefPtr<NodePaintMethod> RichEditorPattern::CreateNodePaintMethod()
{
    if (!contentMod_) {
        contentMod_ = MakeRefPtr<RichEditorContentModifier>(textStyle_, &paragraphs_, WeakClaim(this));
    }
    if (!overlayMod_) {
        auto scrollBar = GetScrollBar();
        if (scrollBar) {
            auto scrollBarModifier = AceType::MakeRefPtr<ScrollBarOverlayModifier>();
            scrollBarModifier->SetRect(scrollBar->GetActiveRect());
            scrollBarModifier->SetPositionMode(scrollBar->GetPositionMode());
            SetScrollBarOverlayModifier(scrollBarModifier);
        }
        SetEdgeEffect(EdgeEffect::FADE, GetAlwaysEnabled());
        SetEdgeEffect();
        overlayMod_ = AceType::MakeRefPtr<RichEditorOverlayModifier>(
            WeakClaim(this), GetScrollBarOverlayModifier(), GetScrollEdgeEffect());
    }

    if (GetIsCustomFont()) {
        contentMod_->SetIsCustomFont(true);
    }
    return MakeRefPtr<RichEditorPaintMethod>(WeakClaim(this), &paragraphs_, baselineOffset_, contentMod_, overlayMod_);
}

int32_t RichEditorPattern::GetHandleIndex(const Offset& offset) const
{
    CHECK_NULL_RETURN(!isShowPlaceholder_, 0);
    return paragraphs_.GetIndex(Offset(offset.GetX() + contentRect_.GetX() - richTextRect_.GetX(),
        offset.GetY() + contentRect_.GetY() - richTextRect_.GetY()));
}

std::vector<RectF> RichEditorPattern::GetTextBoxes()
{
    auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    std::vector<RectF> res;
    res.reserve(selectedRects.size());
    for (auto&& rect : selectedRects) {
        res.emplace_back(rect);
    }
    if (!res.empty() && paragraphs_.IsSelectLineHeadAndUseLeadingMargin(textSelector_.GetTextStart())) {
        // To make drag screenshot include LeadingMarginPlaceholder when not single line
        if (res.front().GetY() != res.back().GetY()) {
            res.front().SetLeft(0.0f);
        }
    }
    return res;
}

float RichEditorPattern::GetLineHeight() const
{
    auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    CHECK_NULL_RETURN(selectedRects.size(), 0.0f);
    return selectedRects.front().Height();
}

float RichEditorPattern::GetLetterSpacing() const
{
    auto selectedRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    CHECK_NULL_RETURN(!selectedRects.empty(), 0.0f);
    return selectedRects.front().Width();
}

void RichEditorPattern::UpdateSelectMenuInfo(SelectMenuInfo& menuInfo)
{
    bool isSupportCameraInput = false;
#if defined(ENABLE_STANDARD_INPUT)
    auto inputMethod = MiscServices::InputMethodController::GetInstance();
    isSupportCameraInput =
        inputMethod && inputMethod->IsInputTypeSupported(MiscServices::InputType::CAMERA_INPUT);
#endif
    menuInfo.showCameraInput = !IsSelected() && isSupportCameraInput && !customKeyboardBuilder_;
}

RectF RichEditorPattern::GetCaretRect() const
{
    RectF rect;
    CHECK_NULL_RETURN(overlayMod_, rect);
    auto richEditorOverlay = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    CHECK_NULL_RETURN(richEditorOverlay, rect);
    rect.SetOffset(richEditorOverlay->GetCaretOffset());
    rect.SetHeight(richEditorOverlay->GetCaretHeight());
    return rect;
}

void RichEditorPattern::ScrollToSafeArea() const
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto textFieldManager = DynamicCast<TextFieldManagerNG>(pipeline->GetTextFieldManager());
    CHECK_NULL_VOID(textFieldManager);
    textFieldManager->ScrollTextFieldToSafeArea();
}

void RichEditorPattern::InitScrollablePattern()
{
    if (GetScrollableEvent()) {
        return;
    }
    SetAxis(Axis::VERTICAL);
    AddScrollEvent();
    SetScrollBar(DisplayMode::AUTO);
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
        CHECK_NULL_VOID(richEditorTheme);
        scrollBar->SetMinHeight(richEditorTheme->GetScrollbarMinHeight());
    }
    if (overlayMod_) {
        UpdateScrollBarOffset();
    }
    auto layoutProperty = GetLayoutProperty<RichEditorLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto& paddingProperty = layoutProperty->GetPaddingProperty();
    if (paddingProperty) {
        auto offsetY = paddingProperty->top.has_value() ? paddingProperty->top->GetDimension().ConvertToPx() : 0.0f;
        auto offsetX = paddingProperty->left.has_value() ? paddingProperty->left->GetDimension().ConvertToPx() : 0.0f;
        richTextRect_.SetOffset(OffsetF(offsetX, offsetY));
    }
}

void RichEditorPattern::ProcessInnerPadding()
{
    auto context = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto theme = context->GetTheme<RichEditorTheme>();
    CHECK_NULL_VOID(theme);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<RichEditorLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    auto themePadding = theme->GetPadding();
    auto& paddingProp = layoutProperty->GetPaddingProperty();
    auto left = !paddingProp ? CalcLength(themePadding.Left()).GetDimension()
                             : paddingProp->left.value_or(CalcLength(themePadding.Left())).GetDimension();
    auto top = !paddingProp ? CalcLength(themePadding.Top()).GetDimension()
                            : paddingProp->top.value_or(CalcLength(themePadding.Top())).GetDimension();
    auto bottom = !paddingProp ? CalcLength(themePadding.Bottom()).GetDimension()
                               : paddingProp->bottom.value_or(CalcLength(themePadding.Bottom())).GetDimension();
    auto right = !paddingProp ? CalcLength(themePadding.Right()).GetDimension()
                              : paddingProp->right.value_or(CalcLength(themePadding.Right())).GetDimension();
    PaddingProperty paddings;
    paddings.top = NG::CalcLength(top);
    paddings.bottom = NG::CalcLength(bottom);
    paddings.left = NG::CalcLength(left);
    paddings.right = NG::CalcLength(right);
    layoutProperty->UpdatePadding(paddings);
}

void RichEditorPattern::UpdateScrollStateAfterLayout(bool shouldDisappear)
{
    bool hasTextOffsetChanged = false;
    if (GreatNotEqual(richTextRect_.GetY(), contentRect_.GetY())) {
        auto offset = richTextRect_.GetOffset();
        offset.AddY(contentRect_.GetY() - richTextRect_.GetY());
        richTextRect_.SetOffset(offset);
        hasTextOffsetChanged = true;
    }
    if (GreatNotEqual(richTextRect_.Height(), contentRect_.Height()) &&
        LessNotEqual(richTextRect_.Bottom(), contentRect_.Bottom())) {
        auto offset = richTextRect_.GetOffset();
        offset.AddY(contentRect_.Bottom() - richTextRect_.Bottom());
        richTextRect_.SetOffset(offset);
        hasTextOffsetChanged = true;
    }
    if (LessOrEqual(richTextRect_.Height(), contentRect_.Height()) &&
        LessNotEqual(richTextRect_.GetY(), contentRect_.GetY())) {
        richTextRect_.SetOffset(contentRect_.GetOffset());
        hasTextOffsetChanged = true;
    }
    if (hasTextOffsetChanged) {
        UpdateChildrenOffset();
    }
    StopScrollable();
    CheckScrollable();
    if (overlayMod_) {
        UpdateScrollBarOffset();
    }
    if (!GetScrollBar()) {
        return;
    }
    if (isFirstCallOnReady_) {
        isFirstCallOnReady_ = false;
        GetScrollBar()->ScheduleDisappearDelayTask();
        return;
    }
    if (shouldDisappear) {
        GetScrollBar()->ScheduleDisappearDelayTask();
    }
}

bool RichEditorPattern::OnScrollCallback(float offset, int32_t source)
{
    if (source == SCROLL_FROM_START) {
        auto scrollBar = GetScrollBar();
        if (scrollBar) {
            scrollBar->PlayScrollBarAppearAnimation();
        }
        if (SelectOverlayIsOn()) {
            selectOverlay_->HideMenu(true);
        }
        return true;
    }
    if (IsReachedBoundary(offset)) {
        return false;
    }
    auto newOffset = MoveTextRect(offset);
    MoveFirstHandle(newOffset);
    MoveSecondHandle(newOffset);
    return true;
}

float RichEditorPattern::GetCrossOverHeight() const
{
    if (!keyboardAvoidance_ || !contentChange_) {
        return 0.0f;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0.0f);
    auto rootHeight = pipeline->GetRootHeight();
    auto keyboardY = rootHeight - pipeline->GetSafeAreaManager()->GetKeyboardInset().Length();
    if (GreatOrEqual(keyboardY, rootHeight)) {
        return 0.0f;
    }
    float height = contentRect_.Bottom();
    float frameY = parentGlobalOffset_.GetY() + contentRect_.GetY();
    float bottom = frameY + height;
    auto crossOverHeight = bottom - keyboardY;
    if (LessOrEqual(crossOverHeight, 0.0f)) {
        return 0.0f;
    }
    return crossOverHeight;
}

float RichEditorPattern::MoveTextRect(float offset)
{
    auto keyboardOffset = GetCrossOverHeight();
    if (GreatNotEqual(richTextRect_.Height(), contentRect_.Height() - keyboardOffset)) {
        if (GreatNotEqual(richTextRect_.GetY() + offset, contentRect_.GetY())) {
            offset = contentRect_.GetY() - richTextRect_.GetY();
        } else if (LessNotEqual(richTextRect_.Bottom() + offset, contentRect_.Bottom() - keyboardOffset)) {
            offset = contentRect_.Bottom() - keyboardOffset - richTextRect_.Bottom();
        }
    } else if (!NearEqual(richTextRect_.GetY(), contentRect_.GetY())) {
        offset = contentRect_.GetY() - richTextRect_.GetY();
    } else {
        return 0.0f;
    }
    if (NearEqual(offset, 0.0f)) {
        return offset;
    }
    scrollOffset_ = richTextRect_.GetY() + offset;
    richTextRect_.SetOffset(OffsetF(richTextRect_.GetX(), scrollOffset_));
    UpdateScrollBarOffset();
    UpdateChildrenOffset();
    return offset;
}

void RichEditorPattern::MoveFirstHandle(float offset)
{
    if (SelectOverlayIsOn() && !NearEqual(offset, 0.0f)) {
        textSelector_.selectionBaseOffset.AddY(offset);
        auto firstHandleOffset = textSelector_.firstHandle.GetOffset();
        firstHandleOffset.AddY(offset);
        textSelector_.firstHandle.SetOffset(firstHandleOffset);
        selectOverlay_->UpdateFirstHandleOffset();
    }
}

void RichEditorPattern::MoveSecondHandle(float offset)
{
    if (SelectOverlayIsOn() && !NearEqual(offset, 0.0f)) {
        textSelector_.selectionDestinationOffset.AddY(offset);
        auto secondHandleOffset = textSelector_.secondHandle.GetOffset();
        secondHandleOffset.AddY(offset);
        textSelector_.secondHandle.SetOffset(secondHandleOffset);
        selectOverlay_->UpdateSecondHandleOffset();
    }
}

void RichEditorPattern::MoveCaretToContentRect(bool downStreamFirst)
{
    float caretHeight = 0.0f;
    auto caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight, downStreamFirst);
    MoveCaretToContentRect(caretOffset, caretHeight);
}

void RichEditorPattern::MoveCaretToContentRect(const OffsetF& caretOffset, float caretHeight)
{
    auto keyboardOffset = GetCrossOverHeight();
    auto contentRect = GetTextContentRect();
    auto textRect = GetTextRect();
    if (LessOrEqual(textRect.Height(), contentRect.Height() - keyboardOffset) || isShowPlaceholder_) {
        return;
    }
    if (LessNotEqual(contentRect.GetSize().Height(), caretHeight) &&
        !NearEqual(caretOffset.GetY() + caretHeight, contentRect.Bottom() - keyboardOffset)) {
        OnScrollCallback(contentRect.Bottom() - keyboardOffset - caretOffset.GetY() - caretHeight, SCROLL_FROM_NONE);
    }
    if (LessNotEqual(contentRect.GetSize().Height(), caretHeight)) {
        return;
    }
    if (LessNotEqual(caretOffset.GetY(), contentRect.GetY())) {
        if (LessOrEqual(caretOffset.GetX(), GetTextRect().GetX())) {
            OnScrollCallback(contentRect.GetY() - caretOffset.GetY() + caretHeight, SCROLL_FROM_NONE);
        } else {
            OnScrollCallback(contentRect.GetY() - caretOffset.GetY(), SCROLL_FROM_NONE);
        }
    } else if (GreatNotEqual(caretOffset.GetY() + caretHeight, contentRect.Bottom() - keyboardOffset)) {
        auto distance = contentRect.Bottom() - keyboardOffset - caretOffset.GetY() - caretHeight -
            CARET_BOTTOM_DISTANCE.ConvertToPx();
        OnScrollCallback(distance, SCROLL_FROM_NONE);
    }
}

void RichEditorPattern::UpdateScrollBarOffset()
{
    if (!GetScrollBar() && !GetScrollBarProxy()) {
        return;
    }
    Size size(frameRect_.Width(), frameRect_.Height());
    auto verticalGap = frameRect_.Height() - contentRect_.Height();
    UpdateScrollBarRegion(
        contentRect_.GetY() - richTextRect_.GetY(), richTextRect_.Height() + verticalGap, size, Offset(0.0, 0.0));
    auto tmpHost = GetHost();
    CHECK_NULL_VOID(tmpHost);
    tmpHost->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::OnScrollEndCallback()
{
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        scrollBar->ScheduleDisappearDelayTask();
    }
    if (IsSelectAreaVisible()) {
        selectOverlay_->UpdateMenuOffset();
        selectOverlay_->ShowMenu();
    }
}

bool RichEditorPattern::IsSelectAreaVisible()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    RectF visibleContentRect(contentRect_.GetOffset() + parentGlobalOffset_, contentRect_.GetSize());
    auto parent = host->GetAncestorNodeOfFrame();
    visibleContentRect = GetVisibleContentRect(parent, visibleContentRect);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto safeAreaManager = pipeline->GetSafeAreaManager();
    CHECK_NULL_RETURN(safeAreaManager, false);
    auto keyboardInsert = safeAreaManager->GetKeyboardInset();
    auto selectArea = GetSelectArea();
    if (GreatOrEqual(selectArea.Top(), visibleContentRect.Bottom()) ||
        GreatOrEqual(selectArea.Top(), keyboardInsert.start) ||
        LessOrEqual(selectArea.Bottom(), visibleContentRect.Top())) {
        return false;
    }
    return true;
}

bool RichEditorPattern::IsReachedBoundary(float offset)
{
    auto keyboardOffset = GetCrossOverHeight();
    return (NearEqual(richTextRect_.GetY(), contentRect_.GetY()) && GreatNotEqual(offset, 0.0f)) ||
           (NearEqual(richTextRect_.GetY() + richTextRect_.Height(),
                contentRect_.GetY() + contentRect_.Height() - keyboardOffset) &&
               LessNotEqual(offset, 0.0f));
}

void RichEditorPattern::CheckScrollable()
{
    auto gestureHub = GetGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    scrollable_ = GetTextContentLength() > 0 && GreatNotEqual(richTextRect_.Height(), contentRect_.Height());
    SetScrollEnabled(scrollable_);
}

void RichEditorPattern::UpdateChildrenOffset()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    std::vector<int32_t> placeholderIndex;
    for (const auto& child : spans_) {
        if (!child) {
            continue;
        }
        if (AceType::InstanceOf<ImageSpanItem>(child) || AceType::InstanceOf<PlaceholderSpanItem>(child)) {
            placeholderIndex.emplace_back(child->placeholderIndex);
        }
    }
    if (spans_.empty() || placeholderIndex.empty()) {
        return;
    }
    size_t index = 0;
    std::vector<RectF> rectsForPlaceholders = paragraphs_.GetPlaceholderRects();
    auto childrenNodes = host->GetChildren();
    auto textOffset = GetTextRect().GetOffset();
    for (const auto& child : childrenNodes) {
        auto childNode = AceType::DynamicCast<FrameNode>(child);
        if (!childNode) {
            continue;
        }
        if (!(childNode->GetPattern<ImagePattern>() || childNode->GetPattern<PlaceholderSpanPattern>())) {
            continue;
        }
        if (index >= rectsForPlaceholders.size()) {
            break;
        }
        auto rect = rectsForPlaceholders.at(index);
        auto geometryNode = childNode->GetGeometryNode();
        if (geometryNode) {
            geometryNode->SetMarginFrameOffset(textOffset + OffsetF(rect.Left(), rect.Top()));
            childNode->ForceSyncGeometryNode();
            childNode->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
        }
        ++index;
    }
}

void RichEditorPattern::AutoScrollByEdgeDetection(AutoScrollParam param, OffsetF offset, EdgeDetectionStrategy strategy)
{
    if (NearEqual(prevAutoScrollOffset_.GetY(), offset.GetY())) {
        return;
    }
    prevAutoScrollOffset_ = offset;
    auto contentRect = GetTextContentRect();
    auto isDragging = param.autoScrollEvent == AutoScrollEvent::DRAG;
    float edgeThreshold = isDragging ? AUTO_SCROLL_DRAG_EDGE_DISTANCE.ConvertToPx()
                                     : AUTO_SCROLL_EDGE_DISTANCE.ConvertToPx();
    auto maxHeight = isDragging ? frameRect_.Height() : contentRect.Height();
    if (GreatNotEqual(edgeThreshold * 2, maxHeight)) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "AutoScrollByEdgeDetection: hot area height is great than max height.");
        return;
    }
    float topEdgeThreshold = isDragging ? edgeThreshold : edgeThreshold + contentRect.GetY();
    float bottomThreshold = isDragging ? frameRect_.Height() - edgeThreshold : contentRect.Bottom() - edgeThreshold;
    if (param.autoScrollEvent == AutoScrollEvent::HANDLE) {
        auto handleTopOffset = offset;
        auto handleBottomOffset = OffsetF(offset.GetX(), offset.GetY() + param.handleRect.Height());
        if (GreatNotEqual(handleBottomOffset.GetY(), bottomThreshold)) {
            param.offset = bottomThreshold - handleBottomOffset.GetY();
            ScheduleAutoScroll(param);
        } else if (LessNotEqual(handleTopOffset.GetY(), topEdgeThreshold)) {
            param.offset = topEdgeThreshold - handleTopOffset.GetY();
            ScheduleAutoScroll(param);
        } else {
            StopAutoScroll();
        }
        return;
    }
    // drag and mouse
    if (GreatNotEqual(offset.GetY(), bottomThreshold)) {
        param.offset = isDragging ? -CalcDragSpeed(bottomThreshold, frameRect_.Height(), offset.GetY())
                                  : bottomThreshold - offset.GetY();
        ScheduleAutoScroll(param);
    } else if (LessNotEqual(offset.GetY(), topEdgeThreshold)) {
        param.offset = isDragging ? CalcDragSpeed(topEdgeThreshold, 0, offset.GetY())
                                  : topEdgeThreshold - offset.GetY();
        ScheduleAutoScroll(param);
    } else {
        StopAutoScroll();
    }
}

float RichEditorPattern::CalcDragSpeed(float hotAreaStart, float hotAreaEnd, float point)
{
    auto distanceRatio = (point - hotAreaStart) / (hotAreaEnd - hotAreaStart);
    auto speedFactor = Curves::SHARP->MoveInternal(distanceRatio);
    return ((MAX_DRAG_SCROLL_SPEED * speedFactor) / TIME_UNIT) * AUTO_SCROLL_INTERVAL;
}

void RichEditorPattern::ScheduleAutoScroll(AutoScrollParam param)
{
    if (GreatNotEqual(param.offset, 0.0f) && IsReachTop()) {
        return;
    }
    if (LessNotEqual(param.offset, 0.0f) && IsReachBottom()) {
        return;
    }
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    if (param.isFirstRun_) {
        param.isFirstRun_ = false;
        currentScrollParam_ = param;
        if (isAutoScrollRunning_) {
            return;
        }
    }
    autoScrollTask_.Reset([weak = WeakClaim(this)]() {
        auto client = weak.Upgrade();
        CHECK_NULL_VOID(client);
        client->OnAutoScroll(client->currentScrollParam_);
        if (client->IsReachTop() || client->IsReachBottom()) {
            client->StopAutoScroll();
        }
    });
    isAutoScrollRunning_ = true;
    taskExecutor->PostDelayedTask(autoScrollTask_, TaskExecutor::TaskType::UI, AUTO_SCROLL_INTERVAL,
        "ArkUIRichEditorScheduleAutoScroll");
}

void RichEditorPattern::OnAutoScroll(AutoScrollParam param)
{
    if (param.showScrollbar) {
        auto scrollBar = GetScrollBar();
        if (scrollBar) {
            scrollBar->PlayScrollBarAppearAnimation();
        }
        param.showScrollbar = false;
    }

    if (param.autoScrollEvent == AutoScrollEvent::HANDLE) {
        auto newOffset = MoveTextRect(param.offset);
        if (param.isFirstHandle) {
            MoveSecondHandle(newOffset);
        } else {
            MoveFirstHandle(newOffset);
        }
        selectOverlay_->OnHandleMove(param.handleRect, param.isFirstHandle);
        if (NearEqual(newOffset, 0.0f)) {
            return;
        }
        ScheduleAutoScroll(param);
        return;
    }

    if (param.autoScrollEvent == AutoScrollEvent::DRAG) {
        auto newOffset = MoveTextRect(param.offset);
        if (NearEqual(newOffset, 0.0f)) {
            return;
        }
        ScheduleAutoScroll(param);
        return;
    }

    if (param.autoScrollEvent == AutoScrollEvent::MOUSE) {
        auto newOffset = MoveTextRect(param.offset);
        auto textOffset =
            Offset(param.eventOffset.GetX() - GetTextRect().GetX(), param.eventOffset.GetY() - GetTextRect().GetY());
        int32_t extend = paragraphs_.GetIndex(textOffset);
        UpdateSelector(textSelector_.baseOffset, extend);
        SetCaretPosition(std::max(textSelector_.baseOffset, extend));
        if (NearEqual(newOffset, 0.0f)) {
            return;
        }
        ScheduleAutoScroll(param);
    }
}

void RichEditorPattern::StopAutoScroll()
{
    isAutoScrollRunning_ = false;
    autoScrollTask_.Cancel();
    prevAutoScrollOffset_ = OffsetF(0.0f, 0.0f);
    auto scrollBar = GetScrollBar();
    if (scrollBar) {
        scrollBar->PlayScrollBarDisappearAnimation();
    }
}

bool RichEditorPattern::NeedAiAnalysis(
    const CaretUpdateType targeType, const int32_t pos, const int32_t& spanStart, const std::string& content)
{
    if (spanStart < 0) {
        TAG_LOGW(AceLogTag::ACE_RICH_TEXT, "NeedAiAnalysis -spanStart:%{public}d, return!", spanStart);
        return false;
    }

    if (!InputAIChecker::NeedAIAnalysis(content, targeType, lastClickTimeStamp_ - lastAiPosTimeStamp_)) {
        return false;
    }

    if (IsClickBoundary(pos) && targeType == CaretUpdateType::PRESSED) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "NeedAiAnalysis IsClickBoundary, return!");
        return false;
    }

    if (IsIndexAfterOrInSymbolOrEmoji(pos)) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "NeedAiAnalysis IsIndexAfterOrInSymbolOrEmoji, return!");
        return false;
    }

    return true;
}

bool RichEditorPattern::IsIndexAfterOrInSymbolOrEmoji(int32_t index) {
    auto it = GetSpanIter(index);
    CHECK_NULL_RETURN((it != spans_.end()), false);
    auto spanItem = *it;
    CHECK_NULL_RETURN(spanItem, false);
    auto spanEnd = spanItem->position;
    auto spanStart = spanEnd - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length());
    bool isIndexAfterOrInSymbol = spanItem->unicode != 0 && index > spanStart && index <= spanEnd;
    bool isIndexAfterOrInEmoji = TextEmojiProcessor::IsIndexAfterOrInEmoji(index - spanStart, spanItem->content);
    return isIndexAfterOrInSymbol || isIndexAfterOrInEmoji;
}

void RichEditorPattern::AdjustCursorPosition(int32_t& pos)
{
    // the rich text has some spans, the pos is belong to the whole richtext content, should use (pos - spanStarint)
    int32_t spanStart = -1;
    // get the span text by the position, maybe text is empty
    std::string content = GetPositionSpansText(pos, spanStart);

    if (NeedAiAnalysis(CaretUpdateType::PRESSED, pos, spanStart, content)) {
        int32_t aiPos = pos - spanStart;
        DataDetectorMgr::GetInstance().AdjustCursorPosition(aiPos, content, lastAiPosTimeStamp_, lastClickTimeStamp_);
        if (aiPos < 0) {
            return;
        }
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get ai pos:%{public}d--spanStart%{public}d", aiPos, spanStart);
        pos = aiPos + spanStart;
    }
}

bool RichEditorPattern::AdjustWordSelection(int32_t& start, int32_t& end)
{
    // the rich text has some spans, the pos is belong to the whole richtext content, should use (pos - spanStarint)
    int32_t spanStart = -1;
    // get the span text by the position, maybe text is empty
    std::string content = GetPositionSpansText(start, spanStart);
    if (NeedAiAnalysis(CaretUpdateType::DOUBLE_CLICK, start, spanStart, content)) {
        int32_t aiPosStart = start - spanStart;
        int32_t aiPosEnd = end - spanStart;
        DataDetectorMgr::GetInstance().AdjustWordSelection(aiPosStart, content, aiPosStart, aiPosEnd);
        if (aiPosStart < 0 || aiPosEnd < 0) {
            return false;
        }

        start = std::min(aiPosStart + spanStart, GetTextContentLength());
        end = std::min(aiPosEnd + spanStart, GetTextContentLength());
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get ai selector [%{public}d--%{public}d]", start, end);
        AdjustSelector(start, end);
        return true;
    }
    return false;
}

void RichEditorPattern::AdjustPlaceholderSelection(int32_t& start, int32_t& end, const Offset& touchPos)
{
    CHECK_NULL_VOID(!spans_.empty());
    if (!IsTouchBeforeCaret(start, touchPos)) {
        return;
    }
    auto it = std::find_if(spans_.begin(), spans_.end(), [start](const RefPtr<SpanItem>& spanItem) {
        return spanItem->position == start;
    });
    if (it != spans_.end()) {
        // adjust selection if touch right of image or placeholder
        auto spanIndex = std::distance(spans_.begin(), it);
        auto spanNodeBefore = DynamicCast<FrameNode>(GetChildByIndex(spanIndex));
        if (spanNodeBefore && (spanNodeBefore->GetTag() == V2::IMAGE_ETS_TAG ||
            spanNodeBefore->GetTag() == V2::PLACEHOLDER_SPAN_ETS_TAG)) {
            end = start;
            --start;
            TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "get placeholder selector [%{public}d--%{public}d]", start, end);
        }
    }
}

bool RichEditorPattern::IsTouchBeforeCaret(int32_t caretPos, const Offset& touchPos) {
    CHECK_NULL_RETURN(!spans_.empty(), false);
    float selectLineHeight = 0.0f;
    OffsetF caretOffsetUp = paragraphs_.ComputeCursorOffset(caretPos, selectLineHeight);
    auto needAdjustRect = RectF{ 0, caretOffsetUp.GetY(), caretOffsetUp.GetX(), selectLineHeight };
    return needAdjustRect.IsInRegion(PointF{ touchPos.GetX(), touchPos.GetY() });
}

bool RichEditorPattern::IsClickBoundary(const int32_t position)
{
    if (InputAIChecker::IsSingleClickAtBoundary(position, GetTextContentLength())) {
        return true;
    }

    float height = 0;
    auto handleOffset = CalcCursorOffsetByPosition(position, height);
    if (InputAIChecker::IsMultiClickAtBoundary(handleOffset, TextPattern::GetTextRect())) {
        return true;
    }
    return false;
}

std::string RichEditorPattern::GetPositionSpansText(int32_t position, int32_t& startSpan)
{
    int32_t start = position - AI_TEXT_RANGE_LEFT;
    int32_t end = position + AI_TEXT_RANGE_RIGHT;

    start = std::clamp(start, 0, GetTextContentLength());
    end = std::clamp(end, 0, GetTextContentLength());

    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "caret=%{public}d, range=[%{public}d,%{public}d]", position, start, end);

    // get all the spans between start and end, then filter the valid text
    auto infos = GetSpansInfo(start, end, GetSpansMethod::ONSELECT);
    if (infos.GetSelection().resultObjects.empty()) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "get spans text is null pos:%{public}d,return", position);
        return "";
    }
    auto list = infos.GetSelection().resultObjects;

    std::stringstream sstream;
    for (const auto& obj : list) {
        if (obj.type == SelectSpanType::TYPEIMAGE || obj.type == SelectSpanType::TYPESYMBOLSPAN) {
            if (obj.spanPosition.spanRange[1] <= position) {
                sstream.str("");
                startSpan = -1;
            } else {
                break;
            }
        } else if (obj.type == SelectSpanType::TYPESPAN) {
            if (startSpan < 0) {
                startSpan = obj.spanPosition.spanRange[0] + obj.offsetInSpan[0];
            }
            // we should use the wide string deal to avoid crash
            auto wideText = StringUtils::ToWstring(obj.valueString);
            sstream << StringUtils::ToString(
                wideText.substr(obj.offsetInSpan[0], obj.offsetInSpan[1] - obj.offsetInSpan[0]));
        }
    }

    TAG_LOGD(AceLogTag::ACE_RICH_TEXT, "get spans text ret spanStart:%{public}d", startSpan);
    return sstream.str();
}

bool RichEditorPattern::IsShowSelectMenuUsingMouse()
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto selectOverlayManager = pipeline->GetSelectOverlayManager();
    CHECK_NULL_RETURN(selectOverlayManager, false);
    return selectOverlayManager->GetSelectOverlayInfo().isUsingMouse;
}

RefPtr<FocusHub> RichEditorPattern::GetFocusHub() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    auto focusHub = host->GetOrCreateFocusHub();
    return focusHub;
}

void RichEditorPattern::HandleCursorOnDragMoved(const RefPtr<NotifyDragEvent>& notifyDragEvent)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    if (isCursorAlwaysDisplayed_) {
        if (SystemProperties::GetDebugEnabled()) {
            TAG_LOGI(AceLogTag::ACE_TEXT_FIELD,
                "In OnDragMoved, the cursor has always Displayed in the textField, id:%{public}d", host->GetId());
        }
        return;
    }
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT,
        "In OnDragMoved, the dragging node is moving in the richEditor, id:%{public}d", host->GetId());
    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->RequestFocusImmediately();
    isCursorAlwaysDisplayed_ = true;
    StartTwinkling();
};

void RichEditorPattern::HandleCursorOnDragLeaved(const RefPtr<NotifyDragEvent>& notifyDragEvent)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT,
        "In OnDragLeaved, the dragging node has left from richEditor, id:%{public}d", host->GetId());
    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    focusHub->LostFocus();
    isCursorAlwaysDisplayed_ = false;
    StopTwinkling();
};

void RichEditorPattern::HandleCursorOnDragEnded(const RefPtr<NotifyDragEvent>& notifyDragEvent)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = GetFocusHub();
    CHECK_NULL_VOID(focusHub);
    StopAutoScroll();
    if (!isCursorAlwaysDisplayed_) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "In OnDragEnded,"
            " the released location is not in the current richEditor, id:%{public}d", host->GetId());
        focusHub->LostFocus();
        StopTwinkling();
        return;
    }
    TAG_LOGI(AceLogTag::ACE_RICH_TEXT,
        "In OnDragEnded, the released location is in the current richEditor, id:%{public}d", host->GetId());
    if (HasFocus()) {
        RequestKeyboard(false, true, true);
    } else {
        focusHub->RequestFocusImmediately();
    }
    isCursorAlwaysDisplayed_ = false;
    StartTwinkling();
};

void RichEditorPattern::HandleOnDragStatusCallback(
    const DragEventType& dragEventType, const RefPtr<NotifyDragEvent>& notifyDragEvent)
{
    ScrollablePattern::HandleOnDragStatusCallback(dragEventType, notifyDragEvent);
    switch (dragEventType) {
        case DragEventType::MOVE:
            isDragging_ = true;
            HandleCursorOnDragMoved(notifyDragEvent);
            break;
        case DragEventType::LEAVE:
            HandleCursorOnDragLeaved(notifyDragEvent);
            break;
        case DragEventType::DROP:
            isDragging_ = false;
            HandleCursorOnDragEnded(notifyDragEvent);
            break;
        default:
            break;
    }
}

void RichEditorPattern::HandleOnCameraInput()
{
#if defined(ENABLE_STANDARD_INPUT)
    if (richEditTextChangeListener_ == nullptr) {
        richEditTextChangeListener_ = new OnTextChangedListenerImpl(WeakClaim(this));
    }
    auto inputMethod = MiscServices::InputMethodController::GetInstance();
    if (!inputMethod) {
        return;
    }
    StartTwinkling();
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    if (imeShown_) {
        inputMethod->StartInputType(MiscServices::InputType::CAMERA_INPUT);
    } else {
        auto optionalTextConfig = GetMiscTextConfig();
        CHECK_NULL_VOID(optionalTextConfig.has_value());
        MiscServices::TextConfig textConfig = optionalTextConfig.value();
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "HandleOnCameraInput set calling window id is : %{public}u",
            textConfig.windowId);
#ifdef WINDOW_SCENE_SUPPORTED
        auto systemWindowId = GetSCBSystemWindowId();
        if (systemWindowId) {
            TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "windowId From %{public}u to %{public}u.", textConfig.windowId,
                systemWindowId);
            textConfig.windowId = systemWindowId;
        }
#endif
        inputMethod->Attach(richEditTextChangeListener_, false, textConfig);
        inputMethod->StartInputType(MiscServices::InputType::CAMERA_INPUT);
        inputMethod->ShowTextInput();
    }
#endif
#endif
}

bool RichEditorPattern::CanStartAITask()
{
    return TextPattern::CanStartAITask() && !HasFocus();
}

bool RichEditorPattern::NeedShowAIDetect()
{
    return TextPattern::NeedShowAIDetect() && !HasFocus();
}

void RichEditorPattern::ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const
{
    json->PutExtAttr("enableDataDetector", textDetectEnable_ ? "true" : "false", filter);
    auto jsonValue = JsonUtil::Create(true);
    jsonValue->Put("types", "");
    json->PutExtAttr("dataDetectorConfig", jsonValue->ToString().c_str(), filter);
    json->PutExtAttr("placeholder", GetPlaceHolderInJson().c_str(), filter);
}

std::string RichEditorPattern::GetPlaceHolderInJson() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, "");
    auto layoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    bool hasPlaceHolder = layoutProperty && layoutProperty->HasPlaceholder()
        && !layoutProperty->GetPlaceholder().value().empty();
    CHECK_NULL_RETURN(hasPlaceHolder, "");
    auto jsonValue = JsonUtil::Create(true);
    jsonValue->Put("value", layoutProperty->GetPlaceholderValue("").c_str());
    auto jsonFont = JsonUtil::Create(true);
    jsonFont->Put("size", GetFontSizeInJson(layoutProperty->GetPlaceholderFontSize()).c_str());
    jsonFont->Put("weight", GetFontWeightInJson(layoutProperty->GetPlaceholderFontWeight()).c_str());
    jsonFont->Put("family", GetFontFamilyInJson(layoutProperty->GetPlaceholderFontFamily()).c_str());
    jsonFont->Put("style", GetFontStyleInJson(layoutProperty->GetPlaceholderItalicFontStyle()).c_str());
    auto jsonStyle = JsonUtil::Create(true);
    jsonStyle->Put("font", jsonFont->ToString().c_str());
    jsonStyle->Put("fontColor", GetTextColorInJson(layoutProperty->GetPlaceholderTextColor()).c_str());
    jsonValue->Put("style", jsonStyle->ToString().c_str());
    return StringUtils::RestoreBackslash(jsonValue->ToString());
}

std::string RichEditorPattern::GetTextColorInJson(const std::optional<Color>& value) const
{
    CHECK_NULL_RETURN(!value, value->ColorToString());
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, "");
    auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_RETURN(richEditorTheme, "");
    Color textColor = richEditorTheme->GetTextStyle().GetTextColor();
    return textColor.ColorToString();
}

void RichEditorPattern::GetCaretMetrics(CaretMetricsF& caretCaretMetric)
{
    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto offset = host->GetPaintRectOffset();
    caretOffset += offset;
    caretCaretMetric.offset = caretOffset;
    caretCaretMetric.height = caretHeight;
}

void RichEditorPattern::OnVirtualKeyboardAreaChanged()
{
    CHECK_NULL_VOID(SelectOverlayIsOn());
    float selectLineHeight = 0.0f;
    textSelector_.selectionBaseOffset.SetX(
        CalcCursorOffsetByPosition(textSelector_.GetStart(), selectLineHeight).GetX());
    textSelector_.selectionDestinationOffset.SetX(
        CalcCursorOffsetByPosition(textSelector_.GetEnd(), selectLineHeight).GetX());
    CreateHandles();
    selectOverlay_->HideMenu(true);
}

void RichEditorPattern::ResetDragOption()
{
    auto gestureEventHub = GetGestureEventHub();
    CHECK_NULL_VOID(gestureEventHub);
    if (gestureEventHub->GetIsTextDraggable()) {
        CloseSelectOverlay();
        ResetSelection();
    }
}

RectF RichEditorPattern::GetSelectArea()
{
    auto selectRects = paragraphs_.GetRects(textSelector_.GetTextStart(), textSelector_.GetTextEnd());
    if (selectRects.empty()) {
        float caretHeight = 0.0f;
        auto caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight);
        if (isShowPlaceholder_) {
            caretOffset = CalculateEmptyValueCaretRect();
        }
        auto caretWidth = Dimension(1.5f, DimensionUnit::VP).ConvertToPx();
        return RectF(caretOffset + parentGlobalOffset_, SizeF(caretWidth, caretHeight));
    }
    auto frontRect = selectRects.front();
    auto backRect = selectRects.back();
    RectF res;
    if (GreatNotEqual(backRect.Bottom(), frontRect.Bottom())) {
        res.SetRect(contentRect_.GetX() + parentGlobalOffset_.GetX(),
            frontRect.GetY() + richTextRect_.GetY() + parentGlobalOffset_.GetY(), contentRect_.Width(),
            backRect.Bottom() - frontRect.Top());
    } else {
        res.SetRect(frontRect.GetX() + richTextRect_.GetX() + parentGlobalOffset_.GetX(),
            frontRect.GetY() + richTextRect_.GetY() + parentGlobalOffset_.GetY(), backRect.Right() - frontRect.Left(),
            backRect.Bottom() - frontRect.Top());
    }
    auto contentRect = contentRect_;
    contentRect.SetOffset(contentRect.GetOffset() + parentGlobalOffset_);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, RectF(0, 0, 0, 0));
    auto parent = host->GetAncestorNodeOfFrame();
    contentRect = GetVisibleContentRect(parent, contentRect);
    return res.IntersectRectT(contentRect);
}

bool RichEditorPattern::IsTouchInFrameArea(const PointF& touchPoint)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto viewPort = RectF(parentGlobalOffset_, frameRect_.GetSize());
    auto parent = host->GetAncestorNodeOfFrame();
    viewPort = GetVisibleContentRect(parent, viewPort);
    return viewPort.IsInRegion(touchPoint);
}

void RichEditorPattern::SetPlaceholder(std::vector<std::list<RefPtr<SpanItem>>>& spanItemList)
{
    if (!spanItemList.empty() || !spans_.empty()) {
        isShowPlaceholder_ = false;
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto layoutProperty = host->GetLayoutProperty<TextLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    if (!layoutProperty->HasPlaceholder() || layoutProperty->GetPlaceholder().value().empty()) {
        isShowPlaceholder_ = false;
        return;
    }
    auto placeholderValue = layoutProperty->GetPlaceholder().value();
    auto* stack = ViewStackProcessor::GetInstance();
    CHECK_NULL_VOID(stack);
    auto nodeId = stack->ClaimNodeId();
    auto placeholderNode = SpanNode::GetOrCreateSpanNode(nodeId);
    CHECK_NULL_VOID(placeholderNode);
    if (layoutProperty->HasPlaceholderFontSize()) {
        placeholderNode->UpdateFontSize(layoutProperty->GetPlaceholderFontSize().value());
    }
    if (layoutProperty->HasPlaceholderFontWeight()) {
        placeholderNode->UpdateFontWeight(layoutProperty->GetPlaceholderFontWeight().value());
    }
    if (layoutProperty->HasPlaceholderFontFamily()) {
        placeholderNode->UpdateFontFamily(layoutProperty->GetPlaceholderFontFamily().value());
    }
    if (layoutProperty->HasPlaceholderItalicFontStyle()) {
        placeholderNode->UpdateItalicFontStyle(layoutProperty->GetPlaceholderItalicFontStyle().value());
    }
    if (layoutProperty->HasPlaceholderTextColor()) {
        placeholderNode->UpdateTextColor(layoutProperty->GetPlaceholderTextColor().value());
    }

    auto spanItem = placeholderNode->GetSpanItem();
    CHECK_NULL_VOID(spanItem);
    spanItem->content = placeholderValue;
    std::list<RefPtr<SpanItem>> newGroup;
    newGroup.push_back(spanItem);
    spanItemList.push_back(std::move(newGroup));
    isShowPlaceholder_ = true;
}

Color RichEditorPattern::GetCaretColor()
{
    if (caretColor_.has_value()) {
        return caretColor_.value();
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, SYSTEM_CARET_COLOR);
    auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
    CHECK_NULL_RETURN(richEditorTheme, SYSTEM_CARET_COLOR);
    return richEditorTheme->GetCaretColor();
}

Color RichEditorPattern::GetSelectedBackgroundColor()
{
    Color selectedBackgroundColor;
    if (selectedBackgroundColor_.has_value()) {
        selectedBackgroundColor = selectedBackgroundColor_.value();
    } else {
        auto pipeline = PipelineContext::GetCurrentContext();
        CHECK_NULL_RETURN(pipeline, SYSTEM_SELECT_BACKGROUND_COLOR);
        auto richEditorTheme = pipeline->GetTheme<RichEditorTheme>();
        CHECK_NULL_RETURN(richEditorTheme, SYSTEM_SELECT_BACKGROUND_COLOR);
        selectedBackgroundColor = richEditorTheme->GetSelectedBackgroundColor();
    }
    // Alpha == 255 Means completely opaque
    if (selectedBackgroundColor.GetAlpha() == COLOR_OPAQUE) {
        selectedBackgroundColor = selectedBackgroundColor.ChangeOpacity(DEFAILT_OPACITY);
    }
    return selectedBackgroundColor;
}

void RichEditorPattern::HandleOnDragDrop(const RefPtr<OHOS::Ace::DragEvent>& event)
{
    CHECK_NULL_VOID(!isSpanStringMode_);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    TextCommonEvent textCommonEvent;
    if (textCommonEvent.IsPreventDefault()) {
        CloseSelectOverlay();
        ResetSelection();
        StartTwinkling();
        return;
    }

    auto data = event->GetData();
    CHECK_NULL_VOID(data);

    auto records = UdmfClient::GetInstance()->GetPlainTextRecords(data);
    if (records.empty()) {
        return;
    }

    std::string str;
    for (const auto& record : records) {
        str += record;
    }

    if (str.empty()) {
        TAG_LOGI(AceLogTag::ACE_RICH_TEXT, "text is empty.");
        return;
    }

    HandleOnDragDropTextOperation(str, isDragSponsor_);

    if (textSelector_.IsValid()) {
        CloseSelectOverlay();
        ResetSelection();
    }
    auto focusHub = GetHost()->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    if (focusHub->IsCurrentFocus()) {
        StartTwinkling();
    }
}

void RichEditorPattern::DeleteForward(int32_t currentPosition, int32_t length)
{
    RichEditorDeleteValue info;
    info.SetOffset(currentPosition);
    info.SetRichEditorDeleteDirection(RichEditorDeleteDirection::FORWARD);
    info.SetLength(length);
    CalcDeleteValueObj(currentPosition, length, info);
    DeleteByDeleteValueInfo(info);
}

int32_t RichEditorPattern::HandleOnDragDeleteForward()
{
    int32_t allDelLength = 0;
    SelectionInfo textSelectInfo = GetSpansInfo(dragRange_.first, dragRange_.second, GetSpansMethod::ONSELECT);
    std::list<ResultObject> dragResultObjects = textSelectInfo.GetSelection().resultObjects;
    for (auto ri = dragResultObjects.rbegin(); ri != dragResultObjects.rend(); ++ri) {
        if (SelectSpanType::TYPESPAN == ri->type || (SelectSpanType::TYPEIMAGE == ri->type && ri->valueString != " ")) {
            int32_t spanStart = ri->offsetInSpan[RichEditorSpanRange::RANGESTART];
            int32_t spanEnd = ri->offsetInSpan[RichEditorSpanRange::RANGEEND];
            int32_t reStart = ri->spanPosition.spanRange[RichEditorSpanRange::RANGESTART];
            int32_t delStart = reStart;
            if (spanStart > 0) {
                delStart += spanStart;
            }
            int32_t delLength = spanEnd - spanStart;
            DeleteForward(delStart, delLength);
            allDelLength += delLength;
        }
    }
    return allDelLength;
}

void RichEditorPattern::HandleOnDragDropTextOperation(const std::string& insertValue, bool isDeleteSelect)
{
    int32_t currentPosition = caretPosition_;
    int32_t strLength = static_cast<int32_t>(StringUtils::ToWstring(insertValue).length());
    OperationRecord record;
    record.addText = insertValue;
    record.beforeCaretPosition = dragRange_.first;
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, record, RecordType::DRAG));
    if (isDeleteSelect) {
        if (currentPosition < dragRange_.first) {
            HandleOnDragInsertValue(insertValue);
            dragRange_.first += strLength;
            dragRange_.second += strLength;
            HandleOnDragDeleteForward();
            caretPosition_ += strLength;
        } else if (currentPosition > dragRange_.second) {
            HandleOnDragInsertValue(insertValue);
            int32_t delLength = HandleOnDragDeleteForward();
            caretPosition_ -= (delLength - strLength);
        }
    } else {
        HandleOnDragInsertValue(insertValue);
    }
    AfterChangeText(changeValue);
}

void RichEditorPattern::UndoDrag(const OperationRecord& record)
{
    if (!record.addText.has_value() || record.deleteCaretPostion == -1) {
        return;
    }
    const std::string& str = record.addText.value();
    int32_t length = static_cast<int32_t>(StringUtils::ToWstring(str).length());
    DeleteForward(record.beforeCaretPosition, length);

    caretPosition_ = record.deleteCaretPostion;
    InsertValueOperation(str, nullptr, false);
}

void RichEditorPattern::RedoDrag(const OperationRecord& record)
{
    if (!record.addText.has_value() || record.deleteCaretPostion == -1) {
        return;
    }
    RichEditorChangeValue changeValue;
    CHECK_NULL_VOID(BeforeChangeText(changeValue, record, RecordType::REDO));
    const std::string& str = record.addText.value();
    int32_t length = static_cast<int32_t>(StringUtils::ToWstring(str).length());
    DeleteForward(record.deleteCaretPostion, length);
    caretPosition_ = record.beforeCaretPosition;
    InsertValueOperation(str, nullptr, false);
    AfterChangeText(changeValue);
}

void RichEditorPattern::HandleOnDragInsertValueOperation(const std::string& insertValue)
{
    TextSpanOptions options;
    options.offset = caretPosition_;
    options.value = insertValue;
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        options.style = typingTextStyle_.value();
    }
    AddTextSpanOperation(options, false, -1, false, false);
}

void RichEditorPattern::HandleOnDragInsertValue(const std::string& insertValue)
{
    OperationRecord record;
    record.beforeCaretPosition = caretPosition_ + moveLength_;
    if (textSelector_.IsValid()) {
        record.beforeCaretPosition = textSelector_.GetTextStart();
    }
    record.addText = insertValue;
    ClearRedoOperationRecords();
    HandleOnDragInsertValueOperation(insertValue);
    int32_t length = dragRange_.second - dragRange_.first;
    record.afterCaretPosition = record.beforeCaretPosition + length;
    record.deleteCaretPostion = dragRange_.first;
    AddOperationRecord(record);
}

bool RichEditorPattern::IsEditing()
{
    return isEditing_;
}

void RichEditorPattern::HandleOnEditChanged(bool isEditing)
{
    if (isEditing_ == isEditing) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    isEditing_ = isEditing;
    eventHub->FireOnEditingChange(isEditing);
}

void RichEditorPattern::ResetKeyboardIfNeed()
{
    bool needToResetKeyboard = false;
    auto currentAction = GetTextInputActionValue(GetDefaultTextInputAction());
    // When the enter key type changes, the keyboard needs to be reset.
    if (action_ != TextInputAction::UNSPECIFIED) {
        needToResetKeyboard = action_ != currentAction;
    }
    action_ = currentAction;
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    if (needToResetKeyboard) {
        // if keyboard attached or keyboard is shown, pull up keyboard again
        if (imeShown_ || isCustomKeyboardAttached_) {
            if (HasFocus()) {
                RequestKeyboard(false, true, true);
            }
            return;
        }
#if defined(ENABLE_STANDARD_INPUT)
        auto inputMethod = MiscServices::InputMethodController::GetInstance();
        CHECK_NULL_VOID(inputMethod);
        MiscServices::Configuration config;
        config.SetEnterKeyType(static_cast<MiscServices::EnterKeyType>(action_));
        config.SetTextInputType(static_cast<MiscServices::TextInputType>(keyboard_));
        inputMethod->OnConfigurationChange(config);
#endif
    }
#else
    if (needToResetKeyboard && HasConnection()) {
        CloseKeyboard(true);
        RequestKeyboard(false, true, true);
    }
#endif
}

void RichEditorPattern::OnTextInputActionUpdate(TextInputAction value) {}

void RichEditorPattern::PerformAction(TextInputAction action, bool forceCloseKeyboard)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    // When the Enter key is triggered, perform a line feed operation.
    // It will not exit the editing state, nor will it trigger the Enter key type callback.
    if (action == TextInputAction::NEW_LINE) {
        InsertValue("\n");
        return;
    }
    // Enter key type callback
    TextFieldCommonEvent event;
    auto eventHub = host->GetEventHub<RichEditorEventHub>();
    eventHub->FireOnSubmit(static_cast<int32_t>(action), event);
    // If the developer wants to keep editing, editing will not stop
    if (event.IsKeepEditable()) {
        return;
    }
    // Exit the editing state
    StopEditing();
}

void RichEditorPattern::StopEditing()
{
    if (!HasFocus()) {
        return;
    }

    // The selection status disappears, the cursor is hidden, and the soft keyboard is exited
    HandleBlurEvent();
    // In order to avoid the physical keyboard being able to type, you need to make sure that you lose focus
    FocusHub::LostFocusToViewRoot();
}

TextInputAction RichEditorPattern::GetDefaultTextInputAction() const
{
    // As with TextInput, it is a line break by default
    return TextInputAction::NEW_LINE;
}

void RichEditorPattern::GetChangeSpanStyle(RichEditorChangeValue& changeValue, std::optional<TextStyle>& spanTextStyle,
    std::optional<struct UpdateParagraphStyle>& spanParaStyle, const RefPtr<SpanNode>& spanNode, int32_t spanIndex)
{
    auto originalSpans = changeValue.GetRichEditorOriginalSpans();
    if (spanIndex == 0 && originalSpans.size()) {
        const RichEditorAbstractSpanResult& firstInfo = originalSpans.front();
        const RichEditorAbstractSpanResult& lastInfo = originalSpans.back();
        int32_t firstLength = static_cast<int32_t>(StringUtils::ToWstring(firstInfo.GetValue()).length());
        int32_t lastLength = static_cast<int32_t>(StringUtils::ToWstring(lastInfo.GetValue()).length());
        if (firstInfo.GetEraseLength() == firstLength && lastInfo.GetEraseLength() == lastLength) {
            if (spans_.size() == originalSpans.size() ||
                static_cast<int32_t>(spans_.size()) == (lastInfo.GetSpanIndex() + 1)) {
                return; // all spanNode be deleted, set default style
            }
            spanIndex = lastInfo.GetSpanIndex() + 1;
        } else if (firstInfo.GetEraseLength() == firstLength) {
            spanIndex = lastInfo.GetSpanIndex();
        }
        auto it = spans_.begin();
        std::advance(it, spanIndex);
        if ((*it)->unicode != 0 || DynamicCast<PlaceholderSpanItem>(*it)) {
            return; // is not a textSpan(Image/Symbol/other)
        }
        spanTextStyle = (*it)->GetTextStyle();
        struct UpdateParagraphStyle paraStyle;
        paraStyle.textAlign = (*it)->textLineStyle->GetTextAlign();
        paraStyle.leadingMargin = (*it)->textLineStyle->GetLeadingMargin();
        spanParaStyle = paraStyle;
    } else if (spanNode && spanNode->GetSpanItem()) {
        spanTextStyle = spanNode->GetSpanItem()->GetTextStyle();
        struct UpdateParagraphStyle paraStyle;
        paraStyle.textAlign = spanNode->GetTextAlign();
        paraStyle.leadingMargin = spanNode->GetLeadingMarginValue({});
        spanParaStyle = paraStyle;
    }
}

void RichEditorPattern::GetReplacedSpan(RichEditorChangeValue& changeValue, int32_t& innerPosition,
    const std::string& insertValue, int32_t textIndex, std::optional<TextStyle> textStyle,
    std::optional<struct UpdateParagraphStyle> paraStyle, bool isCreate, bool fixDel)
{
    std::string originalStr;
    int32_t originalPos = 0;
    RefPtr<SpanItem> spanItem = fixDel ? GetDelPartiallySpanItem(changeValue, originalStr, originalPos) : nullptr;
    TextInsertValueInfo info;
    CalcInsertValueObj(info, textIndex, isCreate);
    int32_t spanIndex = info.GetSpanIndex();
    int32_t offsetInSpan = info.GetOffsetInSpan();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto uiNode = host->GetChildAtIndex(spanIndex);
    RefPtr<SpanNode> spanNode = DynamicCast<SpanNode>(uiNode);
    if (uiNode && uiNode->GetTag() != V2::SPAN_ETS_TAG) {
        spanNode = nullptr;
        ++spanIndex; // create a new span When the span is not a textSpan(Image/Symbol/other)
        offsetInSpan = 0;
    }

    auto wInsertValue = StringUtils::ToWstring(insertValue);
    changeValue.SetRangeAfter({ innerPosition, innerPosition + wInsertValue.length()});
    std::wstring textTemp = wInsertValue;
    if (!textStyle) {
        if (typingStyle_.has_value() && spanNode && !HasSameTypingStyle(spanNode)) {
            textStyle = typingTextStyle_;
            ++spanIndex; // create a new span When have a different typingStyle
            offsetInSpan = 0;
        } else if (!isCreate && spanNode && spanNode->GetSpanItem()) {
            textTemp = StringUtils::ToWstring(spanNode->GetSpanItem()->content);
            textTemp.insert(offsetInSpan, wInsertValue);
        }
    }

    auto it = textTemp.find(lineSeparator);
    bool containNextLine = it != std::wstring::npos && it != static_cast<int32_t>(textTemp.length()) - 1;
    auto content = StringUtils::ToString(textTemp);

    if (textStyle || containNextLine) { // SpanNode Fission
        GetReplacedSpanFission(changeValue, innerPosition, content, spanIndex, offsetInSpan, textStyle, paraStyle);
    } else {
        std::optional<TextStyle> spanTextStyle = textStyle;
        std::optional<struct UpdateParagraphStyle> spanParaStyle = paraStyle;
        GetChangeSpanStyle(changeValue, spanTextStyle, spanParaStyle, spanNode, spanIndex);
        CreateSpanResult(changeValue, innerPosition, spanIndex, offsetInSpan, offsetInSpan + wInsertValue.length(),
            content, spanTextStyle, spanParaStyle);
        innerPosition += wInsertValue.length();
    }

    if (spanItem) {
        spanItem->content = originalStr;
        spanItem->position = originalPos;
    }
}

void RichEditorPattern::GetReplacedSpanFission(RichEditorChangeValue& changeValue, int32_t& innerPosition,
    std::string& content, int32_t startSpanIndex, int32_t offsetInSpan, std::optional<TextStyle> textStyle,
    std::optional<struct UpdateParagraphStyle> paraStyle)
{
    std::vector<RichEditorAbstractSpanResult> ret;
    int spanIndex = startSpanIndex;
    auto wContent = StringUtils::ToWstring(content);

    auto index = wContent.find(lineSeparator);
    while (index != std::wstring::npos) {
        auto textAfter = wContent.substr(index + 1);
        if (textAfter.empty()) {
            break;
        }
        auto textBefore = wContent.substr(0, index + 1);
        if (offsetInSpan != static_cast<int32_t>(textBefore.length())) {
            CreateSpanResult(changeValue, innerPosition, spanIndex, offsetInSpan, textBefore.length(),
                StringUtils::ToString(textBefore), textStyle, paraStyle);
            innerPosition += textBefore.length() - offsetInSpan;
        }
        wContent = textAfter;
        index = wContent.find(lineSeparator);
        offsetInSpan = 0;
        ++spanIndex;
    }
    CreateSpanResult(changeValue, innerPosition, spanIndex, offsetInSpan, wContent.length(),
        StringUtils::ToString(wContent), textStyle, paraStyle);
    innerPosition += wContent.length();
}

void RichEditorPattern::CreateSpanResult(RichEditorChangeValue& changeValue, int32_t& innerPosition, int32_t spanIndex,
    int32_t offsetInSpan, int32_t endInSpan, std::string content, std::optional<TextStyle> textStyle,
    std::optional<struct UpdateParagraphStyle> paraStyle)
{
    RichEditorAbstractSpanResult retInfo;
    if (textStyle) {
        SetTextStyleToRet(retInfo, *textStyle);
    } else {
        retInfo.SetFontColor((Color::BLACK).ColorToString());
        retInfo.SetFontSize(Dimension(16.0f, DimensionUnit::VP).ConvertToVp());
        retInfo.SetFontStyle(OHOS::Ace::FontStyle::NORMAL);
        retInfo.SetFontWeight(static_cast<int32_t>(FontWeight::NORMAL));
        retInfo.SetTextDecoration(TextDecoration::NONE);
        retInfo.SetColor((Color::BLACK).ColorToString());
        retInfo.SetFontFamily("HarmonyOS Sans");
    }
    retInfo.SetSpanIndex(spanIndex);
    retInfo.SetValue(content);
    int32_t rangStart = std::max(0, innerPosition - offsetInSpan);
    retInfo.SetSpanRangeStart(rangStart);
    retInfo.SetSpanRangeEnd(rangStart + StringUtils::ToWstring(content).length());
    retInfo.SetOffsetInSpan(offsetInSpan);
    retInfo.SetEraseLength(endInSpan - offsetInSpan);
    if (paraStyle) {
        TextStyleResult textStyleResult = retInfo.GetTextStyle();
        textStyleResult.textAlign = static_cast<int32_t>(paraStyle->textAlign.value_or(TextAlign::START));
        if (paraStyle->leadingMargin) {
            textStyleResult.leadingMarginSize[0] = paraStyle->leadingMargin->size.Width().ToString();
            textStyleResult.leadingMarginSize[1] = paraStyle->leadingMargin->size.Height().ToString();
        }
        retInfo.SetTextStyle(textStyleResult);
    }
    changeValue.SetRichEditorReplacedSpans(retInfo);
}

void RichEditorPattern::SetTextStyleToRet(RichEditorAbstractSpanResult& retInfo, const TextStyle& textStyle)
{
    retInfo.SetTextDecoration(textStyle.GetTextDecoration());
    retInfo.SetFontColor(textStyle.GetTextColor().ColorToString());
    retInfo.SetColor(textStyle.GetTextDecorationColor().ColorToString());
    retInfo.SetFontSize(textStyle.GetFontSize().ConvertToVp());
    retInfo.SetFontStyle(textStyle.GetFontStyle());
    TextStyleResult textStyleResult;
    textStyleResult.lineHeight = textStyle.GetLineHeight().ConvertToVp();
    textStyleResult.letterSpacing = textStyle.GetLetterSpacing().ConvertToVp();
    retInfo.SetTextStyle(textStyleResult);
    retInfo.SetLineHeight(textStyle.GetLineHeight().ConvertToVp());
    retInfo.SetLetterspacing(textStyle.GetLetterSpacing().ConvertToVp());
    retInfo.SetFontFeature(textStyle.GetFontFeatures());
    std::string fontFamilyValue;
    auto fontFamily = textStyle.GetFontFamilies();
    for (const auto& str : fontFamily) {
        fontFamilyValue += str;
    }
    retInfo.SetFontFamily(fontFamilyValue);
    retInfo.SetFontWeight((int32_t)textStyle.GetFontWeight());
}

void RichEditorPattern::CalcInsertValueObj(TextInsertValueInfo& info, int textIndex, bool isCreate)
{
    if (spans_.empty()) {
        info.SetSpanIndex(0);
        info.SetOffsetInSpan(0);
        return;
    }
    auto it = std::find_if(
        spans_.begin(), spans_.end(), [caretPosition = textIndex](const RefPtr<SpanItem>& spanItem) {
            auto spanLength = static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length());
            if (spanLength == 0) {
                return spanItem->position == caretPosition;
            }
            return (spanItem->position - spanLength <= caretPosition) && (caretPosition <= spanItem->position);
        });
    if (it == spans_.end()) {
        info.SetSpanIndex(static_cast<int32_t>(spans_.size()) - 1);
        info.SetOffsetInSpan(StringUtils::ToWstring((*spans_.rbegin())->content).length());
        return;
    }
    if (textIndex && isCreate) {
        info.SetSpanIndex(std::distance(spans_.begin(), it) + 1);
        info.SetOffsetInSpan(0);
        return;
    }
    if ((*it)->content.back() == '\n' && (*it)->position == textIndex) { // next line/span begin
        info.SetSpanIndex(std::distance(spans_.begin(), it) + 1);
        info.SetOffsetInSpan(0);
    } else {
        info.SetSpanIndex(std::distance(spans_.begin(), it));
        int32_t spanStart = (*it)->position - StringUtils::ToWstring((*it)->content).length();
        info.SetOffsetInSpan(textIndex - spanStart);
    }
}

void RichEditorPattern::GetDeletedSpan(
    RichEditorChangeValue& changeValue, int32_t& innerPosition, int32_t length, RichEditorDeleteDirection direction)
{
    RichEditorDeleteValue info;
    if (!textSelector_.SelectNothing()) {
        length = textSelector_.GetTextEnd() - textSelector_.GetTextStart();
        innerPosition = std::min(textSelector_.GetStart(), textSelector_.GetEnd());
    } else {
        int32_t emojiLength = CalculateDeleteLength(length, (direction == RichEditorDeleteDirection::BACKWARD));
        if (direction == RichEditorDeleteDirection::BACKWARD) {
            innerPosition -= emojiLength;
        }
        if (length < emojiLength) {
            length = emojiLength;
        }
    }

    info.SetOffset(innerPosition);
    info.SetRichEditorDeleteDirection(direction);
    info.SetLength(length);
    if (!spans_.empty()) {
        CalcDeleteValueObj(innerPosition, length, info);
        changeValue.SetRangeBefore({ innerPosition, innerPosition + length });
    }
    const std::list<RichEditorAbstractSpanResult>& resultList = info.GetRichEditorDeleteSpans();
    for (auto& it : resultList) {
        if (it.GetType() == SpanResultType::TEXT) {
            changeValue.SetRichEditorOriginalSpans(it);
        }
    }
}

RefPtr<SpanItem> RichEditorPattern::GetDelPartiallySpanItem(
    RichEditorChangeValue& changeValue, std::string& originalStr, int32_t& originalPos)
{
    RefPtr<SpanItem> retItem = nullptr;
    if (changeValue.GetRichEditorOriginalSpans().size() == 0) {
        return retItem;
    }
    std::wstring textTemp;
    auto originalSpans = changeValue.GetRichEditorOriginalSpans();
    const RichEditorAbstractSpanResult& firstResult = originalSpans.front();
    auto it = spans_.begin();
    std::advance(it, firstResult.GetSpanIndex());
    retItem = *it;
    originalStr = retItem->content;
    originalPos = retItem->position;
    textTemp = StringUtils::ToWstring(originalStr).erase(firstResult.OffsetInSpan(), firstResult.GetEraseLength());
    retItem->content = StringUtils::ToString(textTemp);
    retItem->position -= firstResult.GetEraseLength();
    if (firstResult.GetEraseLength() != static_cast<int32_t>(StringUtils::ToWstring(firstResult.GetValue()).length())) {
        return retItem;
    }

    if (firstResult.GetSpanIndex() == 0) {
        int32_t spanIndex = 0;
        for (auto& orgIt : originalSpans) {
            spanIndex = orgIt.GetSpanIndex();
            if (orgIt.GetEraseLength() != static_cast<int32_t>(StringUtils::ToWstring(orgIt.GetValue()).length())) {
                // find the deleted(Partially) spanItem
                auto findIt = spans_.begin();
                std::advance(findIt, spanIndex);
                textTemp = StringUtils::ToWstring((*findIt)->content);
                textTemp.erase(orgIt.OffsetInSpan(), orgIt.GetEraseLength());
                retItem->content = StringUtils::ToString(textTemp);
                retItem->position = textTemp.length();
                return retItem;
            }
        }
        if (spans_.size() == originalSpans.size() || static_cast<int32_t>(spans_.size()) == (spanIndex + 1)) {
            return retItem; // all spanNode be deleted
        }
        auto nextIt = spans_.begin();
        std::advance(nextIt, spanIndex + 1);
        if ((*nextIt)->unicode != 0 || DynamicCast<PlaceholderSpanItem>(*nextIt)) {
            return retItem; // is not a textSpan(Image/Symbol/other)
        }
        retItem->content = (*nextIt)->content;
        retItem->position = StringUtils::ToWstring(retItem->content).length();
    }
    return retItem;
}

bool RichEditorPattern::BeforeChangeText(RichEditorChangeValue& changeValue, const TextSpanOptions& options)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    if (!eventHub->HasOnWillChange() && !eventHub->HasOnDidChange()) {
        return true;
    }
    int32_t innerPosition = caretPosition_;

   // AddTextSpan
    std::optional<TextStyle> textStyle = std::nullopt;
    if (options.style.has_value()) {
        textStyle = options.style;
    }
    if (options.offset.has_value()) {
        if (spans_.empty() || options.offset.value() < 0) {
            innerPosition = 0;
        } else if (options.offset.value() > GetTextContentLength()) {
            innerPosition = GetTextContentLength();
        } else {
            innerPosition = options.offset.value();
        }
    } else {
        innerPosition = GetTextContentLength();
    }
    // only add, do not delete
    changeValue.SetRangeBefore({ innerPosition, innerPosition });
    GetReplacedSpan(changeValue, innerPosition, options.value, innerPosition, textStyle, options.paraStyle, true);

    CHECK_NULL_RETURN(eventHub->HasOnWillChange(), true);
    auto ret = eventHub->FireOnWillChange(changeValue);
    return ret;
}

bool RichEditorPattern::BeforeAddImage(RichEditorChangeValue& changeValue,
    const ImageSpanOptions& options, int32_t insertIndex)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    if (!eventHub->HasOnWillChange() && !eventHub->HasOnDidChange()) {
        return true;
    }
    changeValue.SetRangeBefore({ insertIndex, insertIndex });
    changeValue.SetRangeAfter({ insertIndex, insertIndex + 1});
    RichEditorAbstractSpanResult retInfo;
    TextInsertValueInfo info;
    CalcInsertValueObj(info, insertIndex, true);
    int32_t spanIndex = info.GetSpanIndex();
    retInfo.SetSpanIndex(spanIndex);
    if (options.image) {
        retInfo.SetValueResourceStr(*options.image);
    }
    if (options.imagePixelMap) {
        retInfo.SetValuePixelMap(*options.imagePixelMap);
    }
    if (options.imageAttribute.has_value()) {
        auto imgAttr = options.imageAttribute.value();
        if (imgAttr.size.has_value()) {
            retInfo.SetSizeWidth(imgAttr.size->width.value_or(CalcDimension()).ConvertToPx());
            retInfo.SetSizeHeight(imgAttr.size->height.value_or(CalcDimension()).ConvertToPx());
        }
        if (imgAttr.verticalAlign.has_value()) {
            retInfo.SetVerticalAlign(imgAttr.verticalAlign.value());
        }
        if (imgAttr.objectFit.has_value()) {
            retInfo.SetImageFit(imgAttr.objectFit.value());
        }
        if (imgAttr.marginProp.has_value()) {
            retInfo.SetMargin(imgAttr.marginProp.value().ToString());
        }
        if (imgAttr.borderRadius.has_value()) {
            retInfo.SetBorderRadius(imgAttr.borderRadius.value().ToString());
        }
    }
    retInfo.SetOffsetInSpan(0);
    retInfo.SetEraseLength(1);
    retInfo.SetSpanRangeStart(insertIndex);
    retInfo.SetSpanRangeEnd(insertIndex + 1);
    retInfo.SetSpanType(SpanResultType::IMAGE);
    changeValue.SetRichEditorReplacedImageSpans(retInfo);
    CHECK_NULL_RETURN(eventHub->HasOnWillChange(), true);
    auto ret = eventHub->FireOnWillChange(changeValue);
    return ret;
}

void RichEditorPattern::AfterAddImage(RichEditorChangeValue& changeValue)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    CHECK_NULL_VOID(eventHub->HasOnDidChange());
    eventHub->FireOnDidChange(changeValue);
}

void RichEditorPattern::FixMoveDownChange(RichEditorChangeValue& changeValue, int32_t delLength)
{
    int32_t delSpanCount = 0;
    for (auto& it : changeValue.GetRichEditorOriginalSpans()) {
        if (it.GetEraseLength() == static_cast<int32_t>(StringUtils::ToWstring(it.GetValue()).length())) {
            ++delSpanCount;
        }
    }
    for (auto& it : const_cast<std::vector<RichEditorAbstractSpanResult>&>(changeValue.GetRichEditorReplacedSpans())) {
        if (delSpanCount) {
            it.SetSpanIndex(it.GetSpanIndex() - delSpanCount);
        }
    }
}

void RichEditorPattern::BeforeUndo(
    RichEditorChangeValue& changeValue, int32_t& innerPosition, const OperationRecord& record)
{
    innerPosition = record.afterCaretPosition;
    if (record.addText.has_value() && record.deleteCaretPostion != -1) { // UndoDrag
        GetDeletedSpan(changeValue, innerPosition, StringUtils::ToWstring(record.addText.value_or("")).length(),
            RichEditorDeleteDirection::FORWARD);
        innerPosition = record.deleteCaretPostion;
        GetReplacedSpan(changeValue, innerPosition, record.addText.value(), innerPosition, std::nullopt, std::nullopt);
    } else if (record.addText.has_value() && record.deleteText.has_value()) {
        GetDeletedSpan(changeValue, innerPosition, StringUtils::ToWstring(record.addText.value_or("")).length(),
            RichEditorDeleteDirection::BACKWARD);
        GetReplacedSpan(
            changeValue, innerPosition, record.deleteText.value(), innerPosition, std::nullopt, std::nullopt);
    } else if (record.deleteText.has_value()) {
        GetReplacedSpan(
            changeValue, innerPosition, record.deleteText.value(), innerPosition, std::nullopt, std::nullopt);
    } else if (record.addText.has_value()) {
        GetDeletedSpan(changeValue, innerPosition, StringUtils::ToWstring(record.addText.value_or("")).length(),
            RichEditorDeleteDirection::BACKWARD);
    }
}

void RichEditorPattern::BeforeRedo(
    RichEditorChangeValue& changeValue, int32_t& innerPosition, const OperationRecord& record)
{
    innerPosition = record.beforeCaretPosition - StringUtils::ToWstring(record.addText.value_or("")).length();
    if (record.addText.has_value() && record.deleteCaretPostion != -1) { // RedoDrag
        innerPosition = record.deleteCaretPostion;
        GetDeletedSpan(changeValue, innerPosition, StringUtils::ToWstring(record.addText.value_or("")).length(),
            RichEditorDeleteDirection::FORWARD);
        innerPosition = record.beforeCaretPosition;
        GetReplacedSpan(changeValue, innerPosition, record.addText.value(), innerPosition, std::nullopt, std::nullopt);
    } else if (record.addText.has_value() && record.deleteText.has_value()) {
        GetDeletedSpan(changeValue, innerPosition, StringUtils::ToWstring(record.deleteText.value_or("")).length(),
            RichEditorDeleteDirection::FORWARD);
        GetReplacedSpan(changeValue, innerPosition, record.addText.value(), innerPosition, std::nullopt, std::nullopt);
    } else if (record.deleteText.has_value()) {
        GetDeletedSpan(changeValue, innerPosition, StringUtils::ToWstring(record.deleteText.value_or("")).length(),
            RichEditorDeleteDirection::FORWARD);
    } else if (record.addText.has_value()) {
        innerPosition = std::min(innerPosition, record.afterCaretPosition);
        GetReplacedSpan(changeValue, innerPosition, record.addText.value(), innerPosition, std::nullopt, std::nullopt);
    }
}

void RichEditorPattern::BeforeDrag(
    RichEditorChangeValue& changeValue, int32_t& innerPosition, const OperationRecord& record)
{
    int length = StringUtils::ToWstring(record.addText.value_or("")).length();
    int32_t nowPosition = innerPosition;
    std::optional<TextStyle> style = std::nullopt;
    if (typingStyle_.has_value() && typingTextStyle_.has_value()) {
        style = typingTextStyle_.value();
    }
    if (!isDragSponsor_) { // drag from outside
        GetReplacedSpan(
            changeValue, innerPosition, record.addText.value(), innerPosition, style, std::nullopt, true, false);
    } else if (nowPosition < record.beforeCaretPosition + length) { // move up
        innerPosition = record.beforeCaretPosition;
        GetDeletedSpan(changeValue, innerPosition, length, RichEditorDeleteDirection::FORWARD);
        innerPosition = nowPosition;
        GetReplacedSpan(
            changeValue, innerPosition, record.addText.value(), nowPosition, style, std::nullopt, true, false);
    } else { // move down
        innerPosition = record.beforeCaretPosition;
        GetDeletedSpan(changeValue, innerPosition, length, RichEditorDeleteDirection::FORWARD);
        innerPosition = nowPosition - length;
        GetReplacedSpan(
            changeValue, innerPosition, record.addText.value(), nowPosition, style, std::nullopt, true, false);
        FixMoveDownChange(changeValue, length);
    }
}

bool RichEditorPattern::BeforeChangeText(
    RichEditorChangeValue& changeValue, const OperationRecord& record, RecordType type, int32_t delLength)
{
    int32_t innerPosition = caretPosition_;
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_RETURN(eventHub, false);
    if (!eventHub->HasOnWillChange() && !eventHub->HasOnDidChange()) {
        return true;
    }

    if (RecordType::INSERT == type) {
        if (textSelector_.IsValid()) {
            GetDeletedSpan(changeValue, innerPosition,
                static_cast<int32_t>(textSelector_.GetTextEnd() - textSelector_.GetTextStart()));
        }
        GetReplacedSpan(changeValue, innerPosition, record.addText.value(), innerPosition, std::nullopt, std::nullopt);
    }
    if (RecordType::DEL_FORWARD == type) {
        innerPosition = record.beforeCaretPosition;
        GetDeletedSpan(changeValue, innerPosition, delLength, RichEditorDeleteDirection::FORWARD);
    }
    if (RecordType::DEL_BACKWARD == type) {
        innerPosition = record.beforeCaretPosition;
        GetDeletedSpan(changeValue, innerPosition, delLength, RichEditorDeleteDirection::BACKWARD);
    }
    if (RecordType::UNDO == type) {
        BeforeUndo(changeValue, innerPosition, record);
    }
    if (RecordType::REDO == type) {
        BeforeRedo(changeValue, innerPosition, record);
    }
    if (RecordType::DRAG == type) {
        BeforeDrag(changeValue, innerPosition, record);
    }

    if (changeValue.GetRichEditorOriginalSpans().empty()) {
        // only add, do not delete
        changeValue.SetRangeBefore({ caretPosition_, caretPosition_ });
    }

    CHECK_NULL_RETURN(eventHub->HasOnWillChange(), true);
    auto ret = eventHub->FireOnWillChange(changeValue);
    return ret;
}

void RichEditorPattern::AfterChangeText(RichEditorChangeValue& changeValue)
{
    auto eventHub = GetEventHub<RichEditorEventHub>();
    CHECK_NULL_VOID(eventHub);
    CHECK_NULL_VOID(eventHub->HasOnDidChange());

    eventHub->FireOnDidChange(changeValue);
}

OffsetF RichEditorPattern::GetTextPaintOffset() const
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, OffsetF(0.0f, 0.0f));
    auto pipeline = host->GetContext();
    CHECK_NULL_RETURN(pipeline, OffsetF(0.0f, 0.0f));
    auto rootOffset = pipeline->GetRootRect().GetOffset();
    auto textPaintOffset = host->GetPaintRectOffset();
    return textPaintOffset - rootOffset;
}

int32_t RichEditorPattern::CalcMoveUpPos(OffsetF& caretOffsetUp, OffsetF& caretOffsetDown)
{
    int32_t caretPosition;
    float caretHeightUp = 0.0f;
    float caretHeightDown = 0.0f;
    Offset textOffset;

    OffsetF caretOffsetCalcUp = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeightUp, false, false);
    OffsetF caretOffsetCalcDown = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeightDown, true, false);
    caretOffsetUp = caretOffsetCalcUp;
    caretOffsetDown = caretOffsetCalcDown;
    auto overlayMod = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    auto caretOffsetOverlay = overlayMod->GetCaretOffset();
    float textOffsetY = GetTextRect().GetY() + caretHeightDown / 2.0; // 2.0 Cursor one half at the center position
    float textOffsetDownY = caretOffsetOverlay.GetY() - textOffsetY;
    textOffset = Offset(caretOffsetOverlay.GetX() - GetTextRect().GetX(), textOffsetDownY);
    caretPosition = paragraphs_.GetIndex(textOffset);
    return caretPosition;
}

int32_t RichEditorPattern::CalcLineBeginPosition()
{
    float caretHeight = 0.0f;
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight, false, false);
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    auto textOffsetY = caretOffset.GetY() - textPaintOffset.GetY() + caretHeight / 2.0;
    Offset textOffset = { 0, textOffsetY };
    auto newPos = paragraphs_.GetIndex(textOffset);
    return newPos;
}

int32_t RichEditorPattern::CalcLineEndPosition()
{
    float caretHeightDown = 0.0f;

    OffsetF CaretOffsetDown = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeightDown, true, false);
    auto textPaintOffset = GetTextContentRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    Offset textOffset;
    float textWidth = GetTextRect().Width();
    float textPaintOffsetY = textPaintOffset.GetY() - caretHeightDown / 2.0;
    float textOffsetClickY = lastClickOffset_.GetY() - textPaintOffsetY;
    float textOffsetDownY = CaretOffsetDown.GetY() - textPaintOffsetY;
    if (lastClickOffset_.NonNegative()) {
        textOffset = { textWidth, textOffsetClickY };
    } else {
        textOffset = { textWidth, textOffsetDownY };
    }
    auto position = paragraphs_.GetIndex(textOffset);
    return position;
}

bool RichEditorPattern::CursorMoveLineEndPos(OffsetF& caretOffsetUp, OffsetF& caretOffsetDown, OffsetF& nextCaretOffset)
{
    float caretHeightEnd = 0.0f;
    auto overlayMod = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    auto caretOffsetOverlay = overlayMod->GetCaretOffset();
    auto caretOffsetWidth = overlayMod->GetCaretWidth();
    auto caretOffsetUpDisY = caretOffsetOverlay.GetY() - caretOffsetUp.GetY();
    auto caretOffsetUpDisX = caretOffsetOverlay.GetX() - GetTextRect().GetX();
    auto caretOffsetDownDisX = caretOffsetDown.GetX() - GetTextRect().GetX();
    if ((caretOffsetUpDisY == 0 && caretOffsetDownDisX == 0 && caretOffsetUpDisX != 0) ||
        caretOffsetUpDisX == caretOffsetWidth || caretOffsetOverlay.GetX() > nextCaretOffset.GetX()) {
        // line end
        OffsetF caretOffsetEnd = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeightEnd, false, false);
        overlayMod->SetCaretOffsetAndHeight(caretOffsetEnd, caretHeightEnd);
        SetLastClickOffset(caretOffsetEnd);
    }
    return true;
}

bool RichEditorPattern::CursorMoveLineBegin()
{
    float caretHeightDown = 0.0f;
    Offset textOffset;

    if (0 == caretPosition_) {
        return false;
    }
    OffsetF caretOffsetDown = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeightDown, true, false);
    auto textPaintOffset = GetTextRect().GetOffset() - OffsetF(0.0, std::min(baselineOffset_, 0.0f));
    float textPaintOffsetY = textPaintOffset.GetY() - caretHeightDown / 2.0;
    if (lastClickOffset_.NonNegative()) {
        textOffset = { 0, lastClickOffset_.GetY() - textPaintOffsetY };
    } else {
        textOffset = { 0, caretOffsetDown.GetY() - textPaintOffsetY };
    }
    auto position = paragraphs_.GetIndex(textOffset);
    AdjustCursorPosition(position);
    SetCaretPosition(position);
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

bool RichEditorPattern::CursorMoveLineEnd()
{
    float caretHeight = 0.0f;
    auto position = CalcLineEndPosition();
    auto overlayMod = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    SetCaretPosition(position);
    StartTwinkling();
    OffsetF caretOffset = CalcCursorOffsetByPosition(GetCaretPosition(), caretHeight, false, false);
    overlayMod->SetCaretOffsetAndHeight(caretOffset, caretHeight);
    SetLastClickOffset(caretOffset);
    MoveCaretToContentRect(caretOffset, caretHeight);
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

void RichEditorPattern::HandleSelectFontStyle(KeyCode code)
{
    if (!textSelector_.IsValid() && !SelectOverlayIsOn()) {
        return;
    }
    TextStyle spanStyle;
    struct UpdateSpanStyle updateSpanStyle;
    ImageSpanAttribute imageStyle;
    int32_t position;
    if (textSelector_.GetEnd() > textSelector_.GetStart()) {
        position = caretPosition_;
    } else {
        position = textSelector_.GetStart();
    }
    position = std::clamp(position, 0, GetTextContentLength());
    auto it = std::find_if(spans_.begin(), spans_.end(), [position](const RefPtr<SpanItem>& spanItem) {
        return (spanItem->position - static_cast<int32_t>(StringUtils::ToWstring(spanItem->content).length()) <=
                   position) &&
               (position <= spanItem->position);
    });
    auto spanIndex = std::distance(spans_.begin(), it);
    auto host = GetHost();
    auto nextit = host->GetChildren().begin();
    std::advance(nextit, spanIndex);
    auto spanNode = DynamicCast<SpanNode>(*nextit);
    CHECK_NULL_VOID(spanNode);
    std::optional<TextStyle> spanStyle_t = (spanNode->GetSpanItem())->GetTextStyle();
    spanStyle = spanStyle_t.value();
    HandleSelectFontStyleWrapper(code, spanStyle);
    updateSpanStyle.updateTextColor = spanStyle.GetTextColor();
    updateSpanStyle.updateFontSize = spanStyle.GetFontSize();
    updateSpanStyle.updateItalicFontStyle = spanStyle.GetFontStyle();
    updateSpanStyle.updateFontWeight = spanStyle.GetFontWeight();
    updateSpanStyle.updateFontFamily = spanStyle.GetFontFamilies();
    updateSpanStyle.updateTextDecoration = spanStyle.GetTextDecoration();
    richEditorController_->SetUpdateSpanStyle(updateSpanStyle);
    richEditorController_->UpdateSpanStyle(
        textSelector_.GetTextStart(), textSelector_.GetTextEnd(), spanStyle, imageStyle);
    StartTwinkling();
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
}

void RichEditorPattern::HandleOnShowMenu()
{
    auto overlayMod = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    auto caretOffsetOverlay = overlayMod->GetCaretOffset();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto focusHub = host->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    selectionMenuOffsetByMouse_ = OffsetF(
        parentGlobalOffset_.GetX() + caretOffsetOverlay.GetX(), parentGlobalOffset_.GetY() + caretOffsetOverlay.GetY());
    focusHub->RequestFocusImmediately();
    StartTwinkling();
    ShowSelectOverlay(RectF(), RectF(), IsSelectAll(), TextResponseType::RIGHT_CLICK);
}

int32_t RichEditorPattern::HandleSelectWrapper(CaretMoveIntent direction)
{
    switch (direction) {
        case CaretMoveIntent::Left:
            return CaretPositionSelectEmoji(CaretMoveIntent::Left);
        case CaretMoveIntent::Right:
            return CaretPositionSelectEmoji(CaretMoveIntent::Right);
        case CaretMoveIntent::Up:
            return HandleSelectPosition(true);
        case CaretMoveIntent::Down:
            return HandleSelectPosition(false);
        case CaretMoveIntent::LeftWord:
            return GetLeftWordPosition(caretPosition_);
        case CaretMoveIntent::RightWord:
            return GetRightWordPosition(caretPosition_);
        case CaretMoveIntent::ParagraghBegin:
            return HandleSelectParagraghPos(true);
        case CaretMoveIntent::ParagraghEnd:
            return HandleSelectParagraghPos(false);
        case CaretMoveIntent::LineBegin:
            return CalcLineBeginPosition();
        case CaretMoveIntent::LineEnd:
            return CalcLineEndPosition();
        default:
            return NONE_SELECT_TYPE;
    }
}

int32_t RichEditorPattern::HandleSelectPosition(bool isForward)
{
    float caretHeight = 0.0f;
    float careOffsetY = 0.0f;
    int32_t newPos;
    OffsetF caretOffset = CalcCursorOffsetByPosition(caretPosition_, caretHeight);
    auto minDet = paragraphs_.minParagraphFontSize.value() / 2.0;
    if (isForward) {
        careOffsetY = caretOffset.GetY() - GetTextRect().GetY() - minDet;
    } else {
        careOffsetY = caretOffset.GetY() - GetTextRect().GetY() + caretHeight + minDet / 2.0;
    }
    newPos = paragraphs_.GetIndex(Offset(caretOffset.GetX() - GetTextRect().GetX(), careOffsetY), true);
    return newPos;
}

int32_t RichEditorPattern::HandleSelectParagraghPos(bool direction)
{
    int32_t newPos = 0;
    CloseSelectOverlay();
    ResetSelection();
    if (direction) {
        newPos = GetParagraphBeginPosition(caretPosition_);
        if (newPos == caretPosition_ && caretPosition_ > 0) {
            newPos = GetParagraphBeginPosition(caretPosition_ - 1);
        }
    } else {
        newPos = GetParagraphEndPosition(caretPosition_);
        if (newPos == caretPosition_ && caretPosition_ < static_cast<int32_t>(GetTextContentLength())) {
            newPos = GetParagraphEndPosition(caretPosition_ + 1);
        }
    }
    return newPos;
}

void RichEditorPattern::HandleSelectFontStyleWrapper(KeyCode code, TextStyle& spanStyle)
{
    switch (code) {
        case KeyCode::KEY_B:
            if (spanStyle.GetFontWeight() == Ace::FontWeight::BOLD) {
                spanStyle.SetFontWeight(Ace::FontWeight::NORMAL);
            } else {
                spanStyle.SetFontWeight(Ace::FontWeight::BOLD);
            }
            break;
        case KeyCode::KEY_I:
            if (spanStyle.GetFontStyle() == OHOS::Ace::FontStyle::ITALIC) {
                spanStyle.SetFontStyle(OHOS::Ace::FontStyle::NORMAL);
            } else {
                spanStyle.SetFontStyle(OHOS::Ace::FontStyle::ITALIC);
            }
            break;
        case KeyCode::KEY_U:
            if (spanStyle.GetTextDecoration() == TextDecoration::UNDERLINE) {
                spanStyle.SetTextDecoration(TextDecoration::NONE);
            } else {
                spanStyle.SetTextDecoration(TextDecoration::UNDERLINE);
            }
            break;
        default:
            LOGW("Unsupported select operation for HandleSelectFrontStyle");
            return;
    }
}

bool RichEditorPattern::HandleOnDeleteComb(bool backward)
{
    CloseSelectOverlay();
    ResetSelection();
    int32_t newPos;
    if (backward) {
        newPos = GetLeftWordPosition(caretPosition_);
        if (newPos == caretPosition_) {
            return false;
        }
        DeleteBackward(caretPosition_ - newPos);
        SetCaretPosition(newPos);
    } else {
        newPos = GetRightWordPosition(caretPosition_);
        if (newPos == caretPosition_) {
            return false;
        }
        DeleteForward(newPos - caretPosition_);
    }
    MoveCaretToContentRect();
    StartTwinkling();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->MarkDirtyNode(PROPERTY_UPDATE_RENDER);
    return true;
}

float RichEditorPattern::GetSelectedMaxWidth()
{
    std::vector<RectF> selecedRect;
    auto overlayMod = DynamicCast<RichEditorOverlayModifier>(overlayMod_);
    CHECK_NULL_RETURN(overlayMod, 0.0f);
    auto selectedRects = overlayMod->GetSelectedRects();
    auto tempWidth = 0.0f;
    auto top = 0.0f;
    auto firstLineLeft = 0.0f;
    auto firstLineWidth = 0.0f;
    auto selectedWidth = 0.0f;
    auto index = 0;
    bool isCalculate = false;
    auto left = 0.0f;
    for (const auto& rect : selectedRects) {
        if (NearZero(tempWidth) || !NearEqual(top, rect.GetY())) {
            if (index == FIRST_LINE) {
                selectedWidth = firstLineLeft - left + firstLineWidth;
            }
            selectedWidth = std::max(selectedWidth, tempWidth);
            if (isCalculate && NearZero(firstLineWidth)) {
                firstLineWidth = tempWidth;
                firstLineLeft = left;
            }
            top = rect.GetY();
            left = rect.GetX();
            tempWidth = 0.0f;
            index++;
            isCalculate = true;
        }
        if (NearEqual(top, rect.GetY())) {
            tempWidth += rect.Width();
        }
    }
    if (index == FIRST_LINE) {
        return tempWidth;
    } else if (index == SECOND_LINE) {
        return std::max(firstLineWidth + firstLineLeft - left, tempWidth);
    } else {
        return std::max(selectedWidth, tempWidth);
    }
}

const std::list<RefPtr<UINode>>& RichEditorPattern::GetAllChildren() const
{
    childNodes_.clear();
    auto host = GetHost();
    CHECK_NULL_RETURN(host, childNodes_);
    return host->GetChildren();
}
} // namespace OHOS::Ace::NG
