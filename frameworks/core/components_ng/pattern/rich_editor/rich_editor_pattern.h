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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_RICH_EDITOR_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_RICH_EDITOR_PATTERN_H

#include <cstdint>
#include <optional>
#include <set>
#include <string>

#include "core/common/ime/text_edit_controller.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_client.h"
#include "core/common/ime/text_input_configuration.h"
#include "core/common/ime/text_input_connection.h"
#include "core/common/ime/text_input_formatter.h"
#include "core/common/ime/text_input_proxy.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/ime/text_selection.h"
#include "core/components_ng/pattern/rich_editor/paragraph_manager.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_accessibility_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_content_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_controller.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_event_hub.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_algorithm.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_layout_property.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_overlay_modifier.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_paint_method.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_select_overlay.h"
#include "core/components_ng/pattern/rich_editor/rich_editor_styled_string_controller.h"
#include "core/components_ng/pattern/rich_editor/selection_info.h"
#include "core/components_ng/pattern/scrollable/scrollable_pattern.h"
#include "core/components_ng/pattern/text_field/text_field_model.h"
#include "core/components_ng/pattern/select_overlay/magnifier.h"
#include "core/components_ng/pattern/select_overlay/magnifier_controller.h"
#include "core/components_ng/pattern/text/span_node.h"
#include "core/components_ng/pattern/text/text_base.h"
#include "core/components_ng/pattern/text/text_pattern.h"

#if not defined(ACE_UNITTEST)
#if defined(ENABLE_STANDARD_INPUT)
#include "commonlibrary/c_utils/base/include/refbase.h"

namespace OHOS::MiscServices {
class OnTextChangedListener;
struct TextConfig;
} // namespace OHOS::MiscServices
#endif
#endif

namespace OHOS::Ace::NG {
class InspectorFilter;

// TextPattern is the base class for text render node to perform paint text.
enum class MoveDirection { FORWARD, BACKWARD };

enum class AutoScrollEvent { HANDLE, DRAG, MOUSE, NONE };
enum class EdgeDetectionStrategy { OUT_BOUNDARY, IN_BOUNDARY, DISABLE };
struct AutoScrollParam {
    AutoScrollEvent autoScrollEvent = AutoScrollEvent::NONE;
    RectF handleRect;
    bool isFirstHandle = false;
    float offset = 0.0f;
    bool showScrollbar = false;
    Offset eventOffset;
    bool isFirstRun_ = true;
};
enum class RecordType {
    DEL_FORWARD = 0,
    DEL_BACKWARD = 1,
    INSERT = 2,
    UNDO = 3,
    REDO = 4,
    DRAG = 5
};

class RichEditorPattern
    : public TextPattern, public ScrollablePattern, public TextInputClient, public Magnifier, public SpanWatcher {
    DECLARE_ACE_TYPE(RichEditorPattern, TextPattern, ScrollablePattern, TextInputClient, Magnifier, SpanWatcher);

public:
    RichEditorPattern();
    ~RichEditorPattern() override;

    struct OperationRecord {
        OperationRecord() : deleteCaretPostion(-1) {}
        std::optional<std::string> addText;
        std::optional<std::string> deleteText;
        int32_t beforeCaretPosition;
        int32_t afterCaretPosition;
        int32_t deleteCaretPostion;
    };

    struct PreviewTextRecord {
        int32_t startOffset = INVALID_VALUE;
        int32_t endOffset = INVALID_VALUE;
        int32_t spanIndex = INVALID_VALUE;
        int32_t currentClickedPosition = INVALID_VALUE;
        bool isPreviewTextInputting = false;
        std::string deltaStr;
        RefPtr<SpanItem> previewTextSpan;

        std::string ToString() const
        {
            auto jsonValue = JsonUtil::Create(true);
            if (previewTextSpan) {
                JSON_STRING_PUT_STRING(jsonValue, previewTextSpan->content);
            }
            JSON_STRING_PUT_BOOL(jsonValue, isPreviewTextInputting);
            JSON_STRING_PUT_INT(jsonValue, startOffset);
            JSON_STRING_PUT_INT(jsonValue, endOffset);
            JSON_STRING_PUT_INT(jsonValue, spanIndex);
            JSON_STRING_PUT_INT(jsonValue, currentClickedPosition);

            return jsonValue->ToString();
        }

        void Reset()
        {
            startOffset = INVALID_VALUE;
            endOffset = INVALID_VALUE;
            spanIndex = INVALID_VALUE;
            currentClickedPosition = INVALID_VALUE;
            isPreviewTextInputting = false;
            deltaStr.clear();
            previewTextSpan = nullptr;
        }

        bool IsValid() const
        {
            return previewTextSpan && startOffset >= 0 && endOffset >= startOffset;
        }
    };

    int32_t SetPreviewText(const std::string& previewTextValue, const PreviewRange range) override;

    bool InitPreviewText(const std::string& previewTextValue, const PreviewRange range);

    bool UpdatePreviewText(const std::string& previewTextValue, const PreviewRange range);

    bool CallbackBeforeSetPreviewText(
        int32_t& delta, const std::string& previewTextValue, const PreviewRange& range, bool isReplaceAll);

    bool CallbackAfterSetPreviewText(int32_t& delta);

    void FinishTextPreview() override;

    void ReceivePreviewTextStyle(const std::string& style) override
    {
        ACE_UPDATE_LAYOUT_PROPERTY(RichEditorLayoutProperty, PreviewTextStyle, style);
    }

    const Color& GetPreviewTextDecorationColor() const;

    bool IsPreviewTextInputting()
    {
        return previewTextRecord_.IsValid();
    }

    std::vector<RectF> GetPreviewTextRects();

    float GetPreviewTextUnderlineWidth() const;

    PreviewTextStyle GetPreviewTextStyle() const;

    void InsertValueInPreview(const std::string& insertValue);

    bool BetweenPreviewTextPosition(const Offset& globalOffset);

    void SetSupportPreviewText(bool isTextPreviewSupported)
    {
        isTextPreviewSupported_ = isTextPreviewSupported;
    }

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(TextInputAction, TextInputAction)
    TextInputAction GetDefaultTextInputAction() const;

    // RichEditor needs softkeyboard, override function.
    bool NeedSoftKeyboard() const override
    {
        return true;
    }

    BlurReason GetBlurReason();

    uint32_t GetSCBSystemWindowId();

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<RichEditorEventHub>();
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<RichEditorLayoutProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<RichEditorLayoutAlgorithm>(spans_, &paragraphs_);
    }

    FocusPattern GetFocusPattern() const override
    {
        FocusPattern focusPattern = { FocusType::NODE, true, FocusStyleType::INNER_BORDER };
        focusPattern.SetIsFocusActiveWhenFocused(true);
        return focusPattern;
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    const RefPtr<RichEditorController>& GetRichEditorController()
    {
        return richEditorController_;
    }

    void SetRichEditorController(const RefPtr<RichEditorController>& controller)
    {
        richEditorController_ = controller;
    }

    const RefPtr<RichEditorStyledStringController>& GetRichEditorStyledStringController()
    {
        return richEditorStyledStringController_;
    }

    void SetRichEditorStyledStringController(const RefPtr<RichEditorStyledStringController>& controller)
    {
        richEditorStyledStringController_ = controller;
    }

    long long GetTimestamp() const
    {
        return timestamp_;
    }

    void UpdateSpanPosition()
    {
        uint32_t spanTextLength = 0;
        for (auto& span : spans_) {
            spanTextLength += StringUtils::ToWstring(span->content).length();
            span->position = static_cast<int32_t>(spanTextLength);
        }
    }

    void RegisterCaretChangeListener(std::function<void(int32_t)>&& listener)
    {
        caretChangeListener_ = listener;
    }

    void SetStyledString(const RefPtr<SpanString>& value);

    RefPtr<MutableSpanString> GetStyledString() const
    {
        return styledString_;
    }

    void UpdateSpanItems(const std::list<RefPtr<NG::SpanItem>>& spanItems) override;
    void ProcessStyledString();
    void MountImageNode(const RefPtr<ImageSpanItem>& imageItem);
    void SetImageLayoutProperty(RefPtr<ImageSpanNode> imageNode, const ImageSpanOptions& options);
    void InsertValueInStyledString(const std::string& insertValue);
    RefPtr<SpanString> CreateStyledStringByTextStyle(
        const std::string& insertValue, const struct UpdateSpanStyle& updateSpanStyle, const TextStyle& textStyle);
    RefPtr<FontSpan> CreateFontSpanByTextStyle(
        const struct UpdateSpanStyle& updateSpanStyle, const TextStyle& textStyle, int32_t length);
    RefPtr<DecorationSpan> CreateDecorationSpanByTextStyle(
        const struct UpdateSpanStyle& updateSpanStyle, const TextStyle& textStyle, int32_t length);
    void DeleteBackwardInStyledString(int32_t length);
    void DeleteForwardInStyledString(int32_t length, bool isIME = true);

    bool BeforeStyledStringChange(int32_t start, int32_t length, const std::string& string);
    bool BeforeStyledStringChange(int32_t start, int32_t length, const RefPtr<SpanString>& styledString);
    void AfterStyledStringChange(int32_t start, int32_t length, const std::string& string);

    void ResetBeforePaste();
    void ResetAfterPaste();

    void OnVisibleChange(bool isVisible) override;
    void OnModifyDone() override;
    void BeforeCreateLayoutWrapper() override;
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;
    void MoveCaretOnLayoutSwap(bool isReduceSize);

    void UpdateEditingValue(const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent = true) override;
    void PerformAction(TextInputAction action, bool forceCloseKeyboard = true) override;
    void InsertValue(const std::string& insertValue) override;
    void InsertValue(const std::string& insertValue, bool isIME);
    void InsertValueOperation(
        const std::string& insertValue, OperationRecord* const record = nullptr, bool isIME = true);
    void DeleteSelectOperation(OperationRecord* const record);
    void InsertValueAfterBeforeSpan(RefPtr<SpanNode>& spanNodeBefore, RefPtr<SpanNode>& spanNode,
        const TextInsertValueInfo& info, const std::string& insertValue, bool isIME = true);
    void InsertDiffStyleValueInSpan(
        RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME = true);
    void InsertValueWithoutSpan(
        RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME = true);
    void InsertValueByPaste(const std::string& insertValue);
    bool IsLineSeparatorInLast(RefPtr<SpanNode>& spanNode);
    void InsertValueToSpanNode(
        RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info);
    void SpanNodeFission(RefPtr<SpanNode>& spanNode, const std::string& insertValue, const TextInsertValueInfo& info);
    void SpanNodeFission(RefPtr<SpanNode>& spanNode);
    void CreateTextSpanNode(
        RefPtr<SpanNode>& spanNode, const TextInsertValueInfo& info, const std::string& insertValue, bool isIME = true);
    void SetDefaultColor(RefPtr<SpanNode>& spanNode);
    void HandleOnDelete(bool backward) override;
    std::pair<bool, bool> IsEmojiOnCaretPosition(int32_t& emojiLength, bool isBackward, int32_t length);
    int32_t CalculateDeleteLength(int32_t length, bool isBackward);
    void DeleteBackward(int32_t length = 0) override;
    std::wstring DeleteBackwardOperation(int32_t length = 0);
    void DeleteForward(int32_t length) override;
    std::wstring DeleteForwardOperation(int32_t length);
    void SetInputMethodStatus(bool keyboardShown) override;
    bool ClickAISpan(const PointF& textOffset, const AISpan& aiSpan) override;
    void NotifyKeyboardClosedByUser() override
    {
        FocusHub::LostFocusToViewRoot();
    }
    void ClearOperationRecords();
    void ClearRedoOperationRecords();
    void AddOperationRecord(const OperationRecord& record);
    bool HandleOnEscape() override;
    void HandleOnUndoAction() override;
    void HandleOnRedoAction() override;
    void CursorMove(CaretMoveIntent direction) override;
    bool CursorMoveLeft();
    bool CursorMoveRight();
    bool CursorMoveUp();
    bool CursorMoveDown();
    bool CursorMoveLeftWord();
    bool CursorMoveRightWord();
    bool CursorMoveToParagraphBegin();
    bool CursorMoveToParagraphEnd();
    bool CursorMoveHome();
    bool CursorMoveEnd();
    int32_t CalcMoveUpPos(OffsetF& caretOffsetUp, OffsetF& caretOffsetDown);
    int32_t CalcLineBeginPosition();
    int32_t CalcLineEndPosition();
    bool CursorMoveLineEndPos(OffsetF& caretOffsetUp, OffsetF& caretOffsetDown, OffsetF& nextCaretOffset);
    bool CursorMoveLineBegin();
    bool CursorMoveLineEnd();
    void HandleSelectFontStyle(KeyCode code) override;
    void HandleSelectFontStyleWrapper(KeyCode code, TextStyle& spanStyle);
    void HandleOnShowMenu() override;
    int32_t HandleSelectPosition(bool isForward);
    int32_t HandleSelectParagraghPos(bool direction);
    int32_t HandleSelectWrapper(CaretMoveIntent direction);
    bool HandleOnDeleteComb(bool backward) override;
    int32_t GetLeftWordPosition(int32_t caretPosition);
    int32_t GetRightWordPosition(int32_t caretPosition);
    int32_t GetParagraphBeginPosition(int32_t caretPosition);
    int32_t GetParagraphEndPosition(int32_t caretPosition);
    int32_t CaretPositionSelectEmoji(CaretMoveIntent direction);
    void HandleSelect(CaretMoveIntent direction) override;
    bool SetCaretPosition(int32_t pos);
    int32_t GetCaretPosition();
    int32_t GetTextContentLength() override;
    bool GetCaretVisible() const;
    OffsetF CalcCursorOffsetByPosition(int32_t position, float& selectLineHeight,
        bool downStreamFirst = false, bool needLineHighest = true);
    void CopyTextSpanStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target, bool needLeadingMargin = false);
    void CopyTextSpanFontStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target);
    void CopyTextSpanLineStyle(RefPtr<SpanNode>& source, RefPtr<SpanNode>& target, bool needLeadingMargin = false);
    int32_t TextSpanSplit(int32_t position, bool needLeadingMargin = false);
    SpanPositionInfo GetSpanPositionInfo(int32_t position);
    std::function<ImageSourceInfo()> CreateImageSourceInfo(const ImageSpanOptions& options);
    void DeleteSpans(const RangeOptions& options);
    void DeleteSpanByRange(int32_t start, int32_t end, SpanPositionInfo info);
    void DeleteSpansByRange(int32_t start, int32_t end, SpanPositionInfo startInfo, SpanPositionInfo endInfo);
    void ClearContent(const RefPtr<UINode>& child);
    void CloseSelectionMenu();
    bool SetCaretOffset(int32_t caretPosition);
    void ResetFirstNodeStyle();
    void FireOnDeleteComplete(const RichEditorDeleteValue& info);

    void UpdateSpanStyle(int32_t start, int32_t end, const TextStyle& textStyle, const ImageSpanAttribute& imageStyle);
    bool SymbolSpanUpdateStyle(
        RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle);
    void SetUpdateSpanStyle(struct UpdateSpanStyle updateSpanStyle);
    void UpdateParagraphStyle(int32_t start, int32_t end, const struct UpdateParagraphStyle& style);
    void UpdateParagraphStyle(RefPtr<SpanNode> spanNode, const struct UpdateParagraphStyle& style);
    std::vector<ParagraphInfo> GetParagraphInfo(int32_t start, int32_t end);
    void SetTypingStyle(struct UpdateSpanStyle typingStyle, TextStyle textStyle);
    int32_t AddImageSpan(const ImageSpanOptions& options, bool isPaste = false, int32_t index = -1,
        bool updateCaret = true);
    void DisableDrag(RefPtr<ImageSpanNode> imageNode);
    void SetGestureOptions(UserGestureOptions userGestureOptions, RefPtr<SpanItem> spanItem);
    int32_t AddTextSpan(const TextSpanOptions& options, bool isPaste = false, int32_t index = -1);
    int32_t AddTextSpanOperation(const TextSpanOptions& options, bool isPaste = false, int32_t index = -1,
        bool needLeadingMargin = false, bool updateCaretPosition = true);
    int32_t AddSymbolSpan(const SymbolSpanOptions& options, bool isPaste = false, int32_t index = -1);
    int32_t AddSymbolSpanOperation(const SymbolSpanOptions& options, bool isPaste = false, int32_t index = -1);
    void AddSpanItem(const RefPtr<SpanItem>& item, int32_t offset);
    int32_t AddPlaceholderSpan(const RefPtr<UINode>& customNode, const SpanOptionBase& options);
    void HandleSelectOverlayWithOptions(const SelectionOptions& options);
    void SetSelection(int32_t start, int32_t end, const std::optional<SelectionOptions>& options = std::nullopt,
        bool isForward = false);
    bool ResetOnInvalidSelection(int32_t start, int32_t end);
    void RefreshSelectOverlay(bool isMousePressed, bool selectedTypeChange);
    bool IsShowHandle();
    void UpdateSelectionInfo(int32_t start, int32_t end);
    bool IsEditing();
    std::u16string GetLeftTextOfCursor(int32_t number) override;
    std::u16string GetRightTextOfCursor(int32_t number) override;
    int32_t GetTextIndexAtCursor() override;
    void ShowSelectOverlay(const RectF& firstHandle, const RectF& secondHandle, bool isCopyAll = false,
        TextResponseType responseType = TextResponseType::LONG_PRESS, bool handlReverse = false);
    void CheckEditorTypeChange();
    int32_t GetHandleIndex(const Offset& offset) const override;
    void OnAreaChangedInner() override;
    void CreateHandles() override;
    void ShowHandles(const bool isNeedShowHandles) override;
    void ShowHandles() override;
    void HandleMenuCallbackOnSelectAll();
    void HandleOnSelectAll() override;
    void OnCopyOperation(bool isUsingExternalKeyboard = false);
    void HandleOnCopy(bool isUsingExternalKeyboard = false) override;
    void HandleDraggableFlag(GestureEvent& info, bool& isInterceptEvent);
    bool JudgeContentDraggable();
    std::pair<OffsetF, float> CalculateCaretOffsetAndHeight();
    OffsetF CalculateEmptyValueCaretRect();
    void RemoveEmptySpan(std::set<int32_t, std::greater<int32_t>>& deleteSpanIndexs);
    void RemoveEmptySpanItems();
    RefPtr<GestureEventHub> GetGestureEventHub();
    float GetSelectedMaxWidth();
    void AfterAddImage(RichEditorChangeValue& changeValue);
    bool BeforeAddImage(RichEditorChangeValue& changeValue, const ImageSpanOptions& options, int32_t insertIndex);

    bool IsUsingMouse() const
    {
        return isMousePressed_;
    }

    void ResetIsMousePressed()
    {
        isMousePressed_ = false;
    }

    OffsetF GetSelectionMenuOffset() const
    {
        return selectionMenuOffsetByMouse_;
    }

    void SetLastClickOffset(const OffsetF& lastClickOffset)
    {
        lastClickOffset_ = lastClickOffset;
    }

    void ResetLastClickOffset()
    {
        lastClickOffset_.SetX(-1);
        lastClickOffset_.SetY(-1);
    }

    int32_t GetCaretSpanIndex()
    {
        return caretSpanIndex_;
    }

    std::list<ParagraphManager::ParagraphInfo> GetParagraphs() const override
    {
        return paragraphs_.GetParagraphs();
    }

    std::optional<SelectHandleInfo> GetFirstHandleInfo() const
    {
        return selectOverlay_->GetFirstHandleInfo();
    }

    std::optional<SelectHandleInfo> GetSecondHandleInfo() const
    {
        return selectOverlay_->GetSecondHandleInfo();
    }

    RectF GetCaretRect() const override;
    void CloseSelectOverlay() override;
    void CloseHandleAndSelect() override;
    void CalculateHandleOffsetAndShowOverlay(bool isUsingMouse = false);
    bool IsSingleHandle();
    bool IsHandlesShow() override;
    void CopySelectionMenuParams(SelectOverlayInfo& selectInfo, TextResponseType responseType);
    std::function<void(Offset)> GetThumbnailCallback() override;
    void HandleOnDragStatusCallback(
        const DragEventType& dragEventType, const RefPtr<NotifyDragEvent>& notifyDragEvent) override;
    void ResetSelection();
    bool BetweenSelectedPosition(const Offset& globalOffset) override;
    void HandleSurfaceChanged(int32_t newWidth, int32_t newHeight, int32_t prevWidth, int32_t prevHeight) override;
    void HandleSurfacePositionChanged(int32_t posX, int32_t posY) override;
    bool RequestCustomKeyboard();
    bool CloseCustomKeyboard();
    const std::string& GetPasteStr() const
    {
        return pasteStr_;
    }
    void AddPasteStr(const std::string& addedStr)
    {
        pasteStr_.append(addedStr);
    }
    void ClearPasteStr()
    {
        pasteStr_.clear();
    }
    void SetCustomKeyboard(const std::function<void()>&& keyboardBuilder)
    {
        if (customKeyboardBuilder_ && isCustomKeyboardAttached_ && !keyboardBuilder) {
            // close customKeyboard and request system keyboard
            CloseCustomKeyboard();
            customKeyboardBuilder_ = keyboardBuilder; // refresh current keyboard
            RequestKeyboard(false, true, true);
            return;
        }
        if (!customKeyboardBuilder_ && keyboardBuilder) {
            // close system keyboard and request custom keyboard
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
            if (imeShown_) {
                CloseKeyboard(true);
                customKeyboardBuilder_ = keyboardBuilder; // refresh current keyboard
                RequestKeyboard(false, true, true);
                return;
            }
#endif
        }
        customKeyboardBuilder_ = keyboardBuilder;
    }
    void BindSelectionMenu(TextResponseType type, TextSpanType richEditorType, std::function<void()>& menuBuilder,
        std::function<void(int32_t, int32_t)>& onAppear, std::function<void()>& onDisappear);
    void ClearSelectionMenu()
    {
        selectionMenuMap_.clear();
    }
    void DumpInfo() override;
    void InitSelection(const Offset& pos);
    bool HasFocus() const;
    void OnColorConfigurationUpdate() override;
    bool IsDisabled() const;
    float GetLineHeight() const override;
    float GetLetterSpacing() const;
    std::vector<RectF> GetTextBoxes() override;
    bool OnBackPressed() override;

    // Add for Scroll

    void OnAttachToFrameNode() override
    {
        TextPattern::OnAttachToFrameNode();
        richEditorInstanceId_ = Container::CurrentIdSafely();
    }

    void OnDetachFromFrameNode(FrameNode* node) override
    {
        TextPattern::OnDetachFromFrameNode(node);
        ScrollablePattern::OnDetachFromFrameNode(node);
    }

    bool IsAtBottom() const override
    {
        return true;
    }

    bool IsAtTop() const override
    {
        return true;
    }

    bool UpdateCurrentOffset(float offset, int32_t source) override
    {
        return true;
    }

    const RectF& GetTextRect() override
    {
        return richTextRect_;
    }

    float GetScrollOffset() const
    {
        return scrollOffset_;
    }

    RefPtr<ScrollBar> GetScrollControllerBar()
    {
        return GetScrollBar();
    }

    bool OnScrollCallback(float offset, int32_t source) override;
    void OnScrollEndCallback() override;
    bool IsScrollable() const override
    {
        return scrollable_;
    }

    bool IsAtomicNode() const override
    {
        return true;
    }

    bool IsShowSelectMenuUsingMouse();

    bool IsShowPlaceholder() const
    {
        return isShowPlaceholder_;
    }

    bool IsSelectAreaVisible();

    void SetPlaceholder(std::vector<std::list<RefPtr<SpanItem>>>& spanItemList);

    void HandleOnCameraInput() override;

    RefPtr<FocusHub> GetFocusHub() const;
    void ResetDragOption() override;
    bool NeedShowAIDetect() override;

    TextSpanType GetEditorType() const
    {
        return selectedType_.value_or(TextSpanType::NONE);
    }
    void GetCaretMetrics(CaretMetricsF& caretCaretMetric) override;

    void SetShowSelect(bool isShowSelect)
    {
        showSelect_ = isShowSelect;
    }

    const std::list<RefPtr<UINode>>& GetAllChildren() const override;

    void OnVirtualKeyboardAreaChanged() override;

    void SetCaretColor(const Color& caretColor)
    {
        caretColor_ = caretColor;
    }

    Color GetCaretColor();

    void SetSelectedBackgroundColor(const Color& selectedBackgroundColor)
    {
        selectedBackgroundColor_ = selectedBackgroundColor;
    }

    Color GetSelectedBackgroundColor();

    void SetCustomKeyboardOption(bool supportAvoidance);
    void StopEditing();
    void ResetKeyboardIfNeed();

    void HandleOnEnter() override
    {
        PerformAction(GetTextInputActionValue(GetDefaultTextInputAction()), false);
    }

    RefPtr<Clipboard> GetClipboard() override
    {
        return clipboard_;
    }

    int32_t GetCaretIndex() const override
    {
        return caretPosition_;
    }

    int32_t GetContentWideTextLength() override
    {
        return GetTextContentLength();
    }

    OffsetF GetCaretOffset() const override
    {
        return GetCaretRect().GetOffset();
    }

    OffsetF GetParentGlobalOffset() const override
    {
        return parentGlobalOffset_;
    }

    OffsetF GetFirstHandleOffset() const override
    {
        return textSelector_.firstHandle.GetOffset();
    }

    OffsetF GetSecondHandleOffset() const override
    {
        return textSelector_.secondHandle.GetOffset();
    }

    OffsetF GetTextPaintOffset() const override;

    float GetCrossOverHeight() const;

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<RichEditorAccessibilityProperty>();
    }

    void AdjustSelector(int32_t& index, bool isFirst);
    void AdjustSelector(int32_t& start, int32_t& end);
    void UpdateSelector(int32_t start, int32_t end);
    std::list<RefPtr<SpanItem>>::iterator GetSpanIter(int32_t index);

    void SetContentChange(bool onChange)
    {
        contentChange_ = onChange;
    }

protected:
    bool CanStartAITask() override;

private:
    friend class RichEditorSelectOverlay;
    RefPtr<RichEditorSelectOverlay> selectOverlay_;
    void UpdateSelectMenuInfo(SelectMenuInfo& selectInfo);
    void HandleOnPaste() override;
    void HandleOnCut() override;
    void InitClickEvent(const RefPtr<GestureEventHub>& gestureHub) override;
    void InitFocusEvent(const RefPtr<FocusHub>& focusHub);
    void HandleBlurEvent();
    void HandleFocusEvent();
    void HandleClickEvent(GestureEvent& info);
    void HandleSingleClickEvent(GestureEvent& info);
    Offset ConvertTouchOffsetToTextOffset(const Offset& touchOffset);
    bool RepeatClickCaret(const Offset& offset, int32_t lastCaretPosition, const RectF& lastCaretRect);
    void CreateAndShowSingleHandle(GestureEvent& info);
    void MoveCaretAndStartFocus(const Offset& offset);
    void HandleDoubleClickEvent(GestureEvent& info);
    bool HandleUserClickEvent(GestureEvent& info);
    bool HandleUserLongPressEvent(GestureEvent& info);
    bool HandleUserGestureEvent(
        GestureEvent& info, std::function<bool(RefPtr<SpanItem> item, GestureEvent& info)>&& gestureFunc);
    void HandleOnlyImageSelected(const Offset& globalOffset, const bool isFingerSelected);
    void CalcCaretInfoByClick(const Offset& touchOffset);
    std::pair<OffsetF, float> CalcAndRecordLastClickCaretInfo(const Offset& textOffset);
    void HandleEnabled();
    void InitMouseEvent();
    void ScheduleCaretTwinkling();
    void OnCaretTwinkling();
    void StartTwinkling();
    void StopTwinkling();
    void UpdateFontFeatureTextStyle(
        RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle& updateSpanStyle, TextStyle& textStyle);
    void UpdateTextStyle(RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle);
    void UpdateSymbolStyle(RefPtr<SpanNode>& spanNode, struct UpdateSpanStyle updateSpanStyle, TextStyle textStyle);
    void UpdateImageStyle(RefPtr<FrameNode>& imageNode, const ImageSpanAttribute& imageStyle);
    void InitTouchEvent();
    bool SelectOverlayIsOn();
    void HandleLongPress(GestureEvent& info);
    void HandleDoubleClickOrLongPress(GestureEvent& info);
    void HandleDoubleClickOrLongPress(GestureEvent& info, RefPtr<FrameNode> host);
    std::string GetPositionSpansText(int32_t position, int32_t& startSpan);
    void FireOnSelect(int32_t selectStart, int32_t selectEnd);
    void FireOnSelectionChange(const int32_t caretPosition);
    void FireOnSelectionChange(const TextSelector& selector);
    void FireOnSelectionChange(int32_t selectStart, int32_t selectEnd);
    void MouseRightFocus(const MouseInfo& info);
    bool IsScrollBarPressed(const MouseInfo& info);
    void HandleMouseLeftButtonMove(const MouseInfo& info);
    void HandleMouseLeftButtonPress(const MouseInfo& info);
    void HandleMouseLeftButtonRelease(const MouseInfo& info);
    void HandleMouseLeftButton(const MouseInfo& info);
    void HandleMouseRightButton(const MouseInfo& info);
    void HandleMouseEvent(const MouseInfo& info);
    void HandleTouchEvent(const TouchEventInfo& info);
    void HandleTouchDown(const Offset& offset);
    void HandleTouchUp();
    void HandleTouchMove(const Offset& offset);
    void InitLongPressEvent(const RefPtr<GestureEventHub>& gestureHub);
    void UseHostToUpdateTextFieldManager();
    void UpdateTextFieldManager(const Offset& offset, float height);
    void ScrollToSafeArea() const override;
    void InitDragDropEvent();
    void OnDragStartAndEnd();
    void onDragDropAndLeave();
    void ClearDragDropEvent();
    void OnDragMove(const RefPtr<OHOS::Ace::DragEvent>& event);
    void OnDragEnd(const RefPtr<Ace::DragEvent>& event);
    void ToJsonValue(std::unique_ptr<JsonValue>& json, const InspectorFilter& filter) const override;
    std::string GetPlaceHolderInJson() const;
    std::string GetTextColorInJson(const std::optional<Color>& value) const;

    void AddDragFrameNodeToManager(const RefPtr<FrameNode>& frameNode)
    {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto dragDropManager = context->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->AddDragFrameNode(frameNode->GetId(), AceType::WeakClaim(AceType::RawPtr(frameNode)));
    }

    void RemoveDragFrameNodeFromManager(const RefPtr<FrameNode>& frameNode)
    {
        auto context = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(context);
        auto dragDropManager = context->GetDragDropManager();
        CHECK_NULL_VOID(dragDropManager);
        dragDropManager->RemoveDragFrameNode(frameNode->GetId());
    }

    void HandleCursorOnDragMoved(const RefPtr<NotifyDragEvent>& notifyDragEvent);
    void HandleCursorOnDragLeaved(const RefPtr<NotifyDragEvent>& notifyDragEvent);
    void HandleCursorOnDragEnded(const RefPtr<NotifyDragEvent>& notifyDragEvent);

    int32_t GetParagraphLength(const std::list<RefPtr<UINode>>& spans) const;
    // REQUIRES: 0 <= start < end
    std::vector<RefPtr<SpanNode>> GetParagraphNodes(int32_t start, int32_t end) const;
    void OnHover(bool isHover);
    bool RequestKeyboard(bool isFocusViewChanged, bool needStartTwinkling, bool needShowSoftKeyboard);
    void UpdateCaretInfoToController();
#if defined(ENABLE_STANDARD_INPUT)
    bool EnableStandardInput(bool needShowSoftKeyboard);
    std::optional<MiscServices::TextConfig> GetMiscTextConfig();
#else
    bool UnableStandardInput(bool isFocusViewChanged);
#endif

    bool HasConnection() const;
    bool CloseKeyboard(bool forceClose) override;
    void CalcInsertValueObj(TextInsertValueInfo& info);
    void CalcDeleteValueObj(int32_t currentPosition, int32_t length, RichEditorDeleteValue& info);
    RefPtr<SpanNode> GetSpanNodeBySpanItem(const RefPtr<SpanItem> spanItem);
    int32_t DeleteValueSetBuilderSpan(const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult);
    int32_t DeleteValueSetImageSpan(const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult);
    int32_t DeleteValueSetSymbolSpan(const RefPtr<SpanItem>& spanItem, RichEditorAbstractSpanResult& spanResult);
    int32_t DeleteValueSetTextSpan(const RefPtr<SpanItem>& spanItem, int32_t currentPosition, int32_t length,
        RichEditorAbstractSpanResult& spanResult);
    void DeleteByDeleteValueInfo(const RichEditorDeleteValue& info);
    bool OnKeyEvent(const KeyEvent& keyEvent);
    void MoveCaretAfterTextChange();
    bool BeforeIMEInsertValue(const std::string& insertValue);
    void AfterInsertValue(
        const RefPtr<SpanNode>& spanNode, int32_t insertValueLength, bool isCreate, bool isIme = true);
    bool AfterIMEInsertValue(const RefPtr<SpanNode>& spanNode, int32_t moveLength, bool isCreate);
    RefPtr<SpanNode> InsertValueToBeforeSpan(RefPtr<SpanNode>& spanNodeBefore, const std::string& insertValue);
    void SetCaretSpanIndex(int32_t index);
    bool HasSameTypingStyle(const RefPtr<SpanNode>& spanNode);

    void GetChangeSpanStyle(RichEditorChangeValue& changeValue, std::optional<TextStyle>& spanTextStyle,
        std::optional<struct UpdateParagraphStyle>& spanParaStyle, const RefPtr<SpanNode>& spanNode, int32_t spanIndex);
    void GetReplacedSpan(RichEditorChangeValue& changeValue, int32_t& innerPosition, const std::string& insertValue,
        int32_t textIndex, std::optional<TextStyle> textStyle, std::optional<struct UpdateParagraphStyle> paraStyle,
        bool isCreate = false, bool fixDel = true);
    void GetReplacedSpanFission(RichEditorChangeValue& changeValue, int32_t& innerPosition, std::string& content,
        int32_t startSpanIndex, int32_t offsetInSpan, std::optional<TextStyle> textStyle,
        std::optional<struct UpdateParagraphStyle> paraStyle);
    void CreateSpanResult(RichEditorChangeValue& changeValue, int32_t& innerPosition, int32_t spanIndex,
        int32_t offsetInSpan, int32_t endInSpan, std::string content, std::optional<TextStyle> textStyle,
        std::optional<struct UpdateParagraphStyle> paraStyle);
    void SetTextStyleToRet(RichEditorAbstractSpanResult& retInfo, const TextStyle& textStyle);
    void CalcInsertValueObj(TextInsertValueInfo& info, int textIndex, bool isCreate = false);
    void GetDeletedSpan(RichEditorChangeValue& changeValue, int32_t& innerPosition, int32_t length,
        RichEditorDeleteDirection direction = RichEditorDeleteDirection::FORWARD);
    RefPtr<SpanItem> GetDelPartiallySpanItem(
        RichEditorChangeValue& changeValue, std::string& originalStr, int32_t& originalPos);
    void FixMoveDownChange(RichEditorChangeValue& changeValue, int32_t delLength);
    bool BeforeChangeText(
        RichEditorChangeValue& changeValue, const OperationRecord& record, RecordType type, int32_t delLength = 0);
    void BeforeUndo(RichEditorChangeValue& changeValue, int32_t& innerPosition, const OperationRecord& record);
    void BeforeRedo(RichEditorChangeValue& changeValue, int32_t& innerPosition, const OperationRecord& record);
    void BeforeDrag(RichEditorChangeValue& changeValue, int32_t& innerPosition, const OperationRecord& record);
    bool BeforeChangeText(RichEditorChangeValue& changeValue, const TextSpanOptions& options);
    void AfterChangeText(RichEditorChangeValue& changeValue);

    // add for scroll.
    void UpdateChildrenOffset();
    void MoveFirstHandle(float offset);
    void MoveSecondHandle(float offset);
    void InitScrollablePattern();
    bool IsReachedBoundary(float offset);
    void UpdateScrollBarOffset() override;
    void CheckScrollable();
    void UpdateScrollStateAfterLayout(bool shouldDisappear);
    void ScheduleAutoScroll(AutoScrollParam param);
    void OnAutoScroll(AutoScrollParam param);
    void StopAutoScroll();
    void AutoScrollByEdgeDetection(AutoScrollParam param, OffsetF offset, EdgeDetectionStrategy strategy);
    float CalcDragSpeed(float hotAreaStart, float hotAreaEnd, float point);
    float MoveTextRect(float offset);
    void MoveCaretToContentRect(bool downStreamFirst = true);
    void MoveCaretToContentRect(const OffsetF& caretOffset, float caretHeight);
    bool IsTextArea() const override
    {
        return true;
    }
    void ProcessInnerPadding();
    bool IsReachTop()
    {
        return NearEqual(richTextRect_.GetY(), contentRect_.GetY());
    }

    bool IsReachBottom()
    {
        return NearEqual(richTextRect_.Bottom(), contentRect_.Bottom());
    }
    // ai analysis fun
    bool NeedAiAnalysis(
        const CaretUpdateType targeType, const int32_t pos, const int32_t& spanStart, const std::string& content);
    bool IsIndexAfterOrInSymbolOrEmoji(int32_t index);
    void AdjustCursorPosition(int32_t& pos);
    void AdjustPlaceholderSelection(int32_t& start, int32_t& end, const Offset& pos);
    bool AdjustWordSelection(int32_t& start, int32_t& end);
    bool IsTouchBeforeCaret(int32_t caretPos, const Offset& touchPos);
    bool IsClickBoundary(const int32_t position);
    void InsertValueInSpanOffset(const TextInsertValueInfo& info, std::wstring& text, const std::wstring& insertValue);
    void SetSelfAndChildDraggableFalse(const RefPtr<UINode>& customNode);

    RectF GetSelectArea();
    bool IsTouchInFrameArea(const PointF& touchPoint);
    void HandleOnDragDrop(const RefPtr<OHOS::Ace::DragEvent>& event);
    void DeleteForward(int32_t currentPosition, int32_t length);
    int32_t HandleOnDragDeleteForward();
    void HandleOnDragDropTextOperation(const std::string& insertValue, bool isDeleteSelect);
    void UndoDrag(const OperationRecord& record);
    void RedoDrag(const OperationRecord& record);
    void HandleOnDragInsertValueOperation(const std::string& insertValue);
    void HandleOnDragInsertValue(const std::string& str);
    void HandleOnEditChanged(bool isEditing);
    void OnTextInputActionUpdate(TextInputAction value);
    void CloseSystemMenu();
    void SetAccessibilityAction();

#if defined(ENABLE_STANDARD_INPUT)
    sptr<OHOS::MiscServices::OnTextChangedListener> richEditTextChangeListener_;
#else
    RefPtr<TextInputConnection> connection_ = nullptr;
#endif
    bool isMouseSelect_ = false;
    bool isMousePressed_ = false;
    bool isFirstMouseSelect_ = true;
    bool leftMousePress_ = false;
    bool isLongPress_ = false;
    bool isDragging_ = false;
#if defined(OHOS_STANDARD_SYSTEM) && !defined(PREVIEW)
    bool imeAttached_ = false;
    bool imeShown_ = false;
#endif
    bool isTextChange_ = false;
    bool caretVisible_ = false;
    bool caretTwinkling_ = false;
    bool isRichEditorInit_ = false;
    bool clickEventInitialized_ = false;
    bool focusEventInitialized_ = false;
    bool blockPress_ = false;
    bool isCustomKeyboardAttached_ = false;
    bool usingMouseRightButton_ = false;
    bool isCursorAlwaysDisplayed_ = false;
    bool isClickOnAISpan_ = false;

    int32_t moveLength_ = 0;
    int32_t caretPosition_ = 0;
    int32_t caretSpanIndex_ = -1;
    long long timestamp_ = 0;
    OffsetF selectionMenuOffsetByMouse_;
    OffsetF selectionMenuOffsetClick_;
    OffsetF lastClickOffset_;
    std::string pasteStr_;

    // still in progress
    ParagraphManager paragraphs_;

    std::vector<OperationRecord> operationRecords_;
    std::vector<OperationRecord> redoOperationRecords_;

    RefPtr<TouchEventImpl> touchListener_;
    struct UpdateSpanStyle updateSpanStyle_;
    CancelableCallback<void()> caretTwinklingTask_;
    RefPtr<RichEditorController> richEditorController_;
    RefPtr<RichEditorStyledStringController> richEditorStyledStringController_;
    RefPtr<MutableSpanString> styledString_;
    MoveDirection moveDirection_ = MoveDirection::FORWARD;
    RectF frameRect_;
    std::optional<struct UpdateSpanStyle> typingStyle_;
    std::optional<TextStyle> typingTextStyle_;
    std::list<ResultObject> dragResultObjects_;
    std::optional<Color> caretColor_;
    std::optional<Color> selectedBackgroundColor_;
    std::function<void()> customKeyboardBuilder_;
    std::function<void(int32_t)> caretChangeListener_;
    RefPtr<OverlayManager> keyboardOverlay_;
    Offset selectionMenuOffset_;
    // add for scroll
    RectF richTextRect_;
    float scrollOffset_ = 0.0f;
    bool isFirstCallOnReady_ = false;
    bool scrollable_ = true;
    CancelableCallback<void()> autoScrollTask_;
    OffsetF prevAutoScrollOffset_;
    AutoScrollParam currentScrollParam_;
    bool isAutoScrollRunning_ = false;
    // add for ai input analysis
    bool hasClicked_ = false;
    CaretUpdateType caretUpdateType_ = CaretUpdateType::NONE;
    TimeStamp lastClickTimeStamp_;
    TimeStamp lastAiPosTimeStamp_;
    bool adjusted_ = false;
    bool isShowMenu_ = true;
    bool isOnlyImageDrag_ = false;
    bool isShowPlaceholder_ = false;
    bool isSingleHandle_ = false;
    bool isTouchCaret_ = false;
    SelectionRangeInfo lastSelectionRange_{-1, -1};
    bool isDragSponsor_ = false;
    std::pair<int32_t, int32_t> dragRange_ { 0, 0 };
    bool isEditing_ = false;
    int32_t dragPosition_ = 0;
    // Action when "enter" pressed.
    TextInputAction action_ = TextInputAction::NEW_LINE;
    // What the keyboard appears.
    TextInputType keyboard_ = TextInputType::UNSPECIFIED;
    ACE_DISALLOW_COPY_AND_MOVE(RichEditorPattern);
    bool keyboardAvoidance_ = false;
    int32_t richEditorInstanceId_ = -1;
    bool contentChange_ = false;
    PreviewTextRecord previewTextRecord_;
    bool isTextPreviewSupported_ = true;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_RICH_EDITOR_RICH_EDITOR_PATTERN_H
