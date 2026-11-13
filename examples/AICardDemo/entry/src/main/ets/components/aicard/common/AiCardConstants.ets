/**
 * AiCard 全量常量。UX 样式值/阈值/时长在此收敛，UX 值到位仅改本文件。
 * @since AICardDemo-v1.0
 */

import { AiSearchErrorCode } from './AiCardTypes';

export class AiCardConstants {
  static readonly STREAM_TICK_MS_DEFAULT: number = 40;
  static readonly FIRST_PACKET_TIMEOUT_MS: number = 60000;
  static readonly STREAM_GAP_TIMEOUT_MS: number = 30000;
  static readonly MARKDOWN_COLLAPSE_MAX_LINES: number = 3;
  static readonly CARD_ANIM_DURATION_MS: number = 3000;
  static readonly RECOMMEND_ANIM_DURATION_MS: number = 3000;
  static readonly RECOMMEND_FALLBACK_TIMEOUT_MS: number = 3500;
  static readonly ERROR_CODE_CONNECT_FAILED: AiSearchErrorCode = 1001;
  static readonly ERROR_CODE_FIRST_PACKET_TIMEOUT: AiSearchErrorCode = 1002;
  static readonly ERROR_CODE_STREAM_GAP_TIMEOUT: AiSearchErrorCode = 1003;
  static readonly ERROR_CODE_PROVIDER_ERROR: AiSearchErrorCode = 1004;
  static readonly ERROR_MSG_CONNECT_FAILED: string = 'connect to search provider failed';
  static readonly ERROR_MSG_FIRST_PACKET_TIMEOUT: string = 'first packet timeout after 60s';
  static readonly ERROR_MSG_STREAM_GAP_TIMEOUT: string = 'stream gap timeout after 30s';
  static readonly ERROR_MSG_PROVIDER_ERROR: string = 'provider transport error';
  static readonly AICARD_DOMAIN_CONTROLLER: number = 0x0A02;
  static readonly AICARD_DOMAIN_UI: number = 0x0A03;
  static readonly TAG_CONTROLLER: string = 'AiCardController';
  static readonly TAG_AI_CARD: string = 'AiCard';
  static readonly SKELETON_ROWS: number = 4;
  static readonly SKELETON_ROW_HEIGHT: number = 16;
  static readonly SKELETON_RADIUS: Resource = $r('sys.float.corner_radius_level2');
  static readonly ICON_PLACEHOLDER_SIZE: number = 24;
  static readonly ICON_PLACEHOLDER_RADIUS: number = 6;
  static readonly COLOR_ICON_PLACEHOLDER: Resource = $r('sys.color.comp_background_tertiary');
  static readonly COLOR_SKELETON: Resource = $r('sys.color.comp_background_tertiary');
  static readonly COLOR_TEXT_PRIMARY: Resource = $r('sys.color.font_primary');
  static readonly COLOR_TEXT_SECONDARY: Resource = $r('sys.color.icon_secondary');
  static readonly COLOR_CARD_BG: ResourceColor = Color.Red;
  static readonly FONT_SIZE_TITLE: number = 16;
  static readonly FONT_SIZE_BODY: number = 14;
  static readonly LINE_HEIGHT: number = 22;
  static readonly SPACING_PADDING: number = 12;
  static readonly SPACING_GAP: number = 8;
  static readonly SKELETON_CONTENT_HEIGHT: number = AiCardConstants.SKELETON_ROWS * AiCardConstants.SKELETON_ROW_HEIGHT + (AiCardConstants.SKELETON_ROWS - 1) * AiCardConstants.SPACING_GAP;
  static readonly CONTENT_ERROR_PADDING_V_SMALL: number = 8;
  static readonly CONTENT_ERROR_PADDING_V_LARGE: number = 12;
  static readonly FONT_SCALE_THRESHOLD_MEDIUM: number = 2;
  static readonly FONT_SCALE_THRESHOLD_LARGE: number = 3.2;
  static readonly FONT_SIZE_HEADER1: number = 18;
  static readonly FONT_SIZE_HEADER3: number = 15;
  static readonly RECOMMEND_ANIM_OFFSET_Y: number = 100;
  static readonly CONTENT_MAX_HEIGHT_UNLIMITED: number = 10000;
  static readonly DIALOG_COUNTER_MARGIN_BOTTOM: number = 4;
  static readonly DIALOG_COLUMNS_PER_ROW: number = 2;
  static readonly TEXT_TITLE_THINKING: Resource = $r('app.string.titlebar_thinking');
  static readonly TEXT_TITLE_GENERATING: Resource = $r('app.string.titlebar_generating');
  static readonly TEXT_TITLE_FOUND: Resource = $r('app.string.titlebar_found');
  static readonly TEXT_TITLE_ERROR: Resource = $r('app.string.titlebar_error');
  static readonly TEXT_TITLE_STOPPED: Resource = $r('app.string.titlebar_stopped');
  static readonly TEXT_COMPLIANCE: Resource = $r('app.string.compliance_text');
  static readonly GROUNDING_OPEN_TAG: string = '<grounding>';
  static readonly GROUNDING_CLOSE_TAG: string = '</grounding>';
  static readonly GRADIENT_BORDER_WIDTH: number = 0.5;
  static readonly GRADIENT_BORDER_RADIUS: number = 16;
  static readonly GRADIENT_INITIAL_ANGLE: number = 135;
  static readonly GRADIENT_COLORS: Array<[Resource, number]> = [
    [$r('app.color.aicard_gradient_0'), 0.0],
    [$r('app.color.aicard_gradient_1'), 0.25],
    [$r('app.color.aicard_gradient_2'), 0.5],
    [$r('app.color.aicard_gradient_3'), 0.75],
    [$r('app.color.aicard_gradient_4'), 1.0]
  ];
  static readonly CARD_BORDER_RADIUS: number = 16;
  static readonly TITLEBAR_HEIGHT: number = 40;
  static readonly ICON_SIZE_AI: number = 18;
  static readonly AI_ICON_SEARCH_RES: Resource = $r('sys.media.AI_search');
  static readonly SPACING_ICON_TITLE: number = 8;
  static readonly SPACING_TITLE_BUTTON: number = 8;
  static readonly SPACING_RIGHT_MARGIN: number = 12;
  static readonly FONT_SIZE_STOP: number = 24;
  static readonly TITLEBAR_FONT_SIZE_TITLE: number = 14;
  static readonly BUTTON_SIZE: number = 32;
  static readonly BUTTON_RIGHT_MARGIN: number = 8;
  static readonly TITLEBAR_GRADIENT_COLORS: Array<[Resource, number]> = [
    [$r('app.color.titlebar_gradient_start'), 0.0],
    [$r('app.color.titlebar_gradient_mid'), 0.5],
    [$r('app.color.titlebar_gradient_end'), 1.0]
  ];
  static readonly TITLEBAR_GRADIENT_COLORS_FOUR: Array<[Resource, number]> = [
    [$r('app.color.titlebar_gradient_start'), 0.0],
    [$r('app.color.titlebar_gradient_mid'), 0.3],
    [$r('app.color.titlebar_gradient_end'), 0.7],
    [$r('app.color.titlebar_gradient_extra'), 1.0]
  ];
  static readonly TITLEBAR_PADDING_FONTSCALE_SMALL: number = 4;
  static readonly TITLEBAR_PADDING_FONTSCALE_MEDIUM: number = 12;
  static readonly TITLEBAR_PADDING_FONTSCALE_LARGE: number = 16;
  static readonly TITLEBAR_GRADIENT_WIDTH_THRESHOLD: number = 100;
  static readonly COMPLIANCE_SPACING_NO_FEEDBACK: number = 12;
  static readonly COMPLIANCE_SPACING_WITH_FEEDBACK: number = 8;
  static readonly COMPLIANCE_DIVIDER_WIDTH: number = 1;
  static readonly COMPLIANCE_ICON_HOT_SIZE: number = 36;
  static readonly COMPLIANCE_ICON_SIZE: number = 20;
  static readonly COMPLIANCE_ICON_GAP: number = 8;
  static readonly COMPLIANCE_ICON_BG_RADIUS: number = 18;
  static readonly COMPLIANCE_ICON_BG: Resource = $r('sys.color.comp_background_tertiary');
  static readonly COMPLIANCE_SPACING_FONTSCALE_MEDIUM: number = 12;
  static readonly COMPLIANCE_SPACING_FONTSCALE_LARGE: number = 16;
  static readonly COMPLIANCE_PADDING_FONTSCALE_MEDIUM: number = 16;
  static readonly COMPLIANCE_PADDING_FONTSCALE_LARGE: number = 24;
  static readonly DIALOG_PADDING_HORIZONTAL: number = 24;
  static readonly DIALOG_MARGIN_HORIZONTAL: number = 16;
  static readonly DIALOG_MAX_WIDTH: number = 400;
  static readonly DIALOG_TITLE_PADDING_TOP: number = 32;
  static readonly DIALOG_TITLE_PADDING_TOP_FONTSCALE: number = 16;
  static readonly DIALOG_TITLE_PADDING_BOTTOM: number = 8;
  static readonly DIALOG_CHIP_MARGIN_TOP: number = 8;
  static readonly DIALOG_CHIP_PADDING_V_SMALL: number = 8;
  static readonly DIALOG_CHIP_PADDING_V_MEDIUM: number = 12;
  static readonly DIALOG_CHIP_PADDING_V_LARGE: number = 16;
  static readonly DIALOG_CHIP_COL_GAP: number = 16;
  static readonly DIALOG_BUTTON_PADDING_H: number = 12;
  static readonly DIALOG_BUTTON_PADDING_V: number = 24;
  static readonly DIALOG_INPUT_MIN_HEIGHT: number = 72;
  static readonly DIALOG_INPUT_MAX_HEIGHT: number = 228;
  static readonly DIALOG_INPUT_PADDING_H: number = 12;
  static readonly DIALOG_INPUT_PADDING_TOP: number = 12;
  static readonly DIALOG_INPUT_PADDING_BOTTOM: number = 8;
  static readonly DIALOG_BUTTON_AREA_PADDING_V: number = 8;
  static readonly DIALOG_DESC_MAX_LENGTH: number = 500;
  static readonly DIALOG_BODY_FONT_SIZE: number = 14;
  static readonly DIALOG_COUNTER_FONT_SIZE: number = 12;
  static readonly TEXT_RECOMMEND_TITLE: Resource = $r('app.string.recommend_title');
  static readonly RECOMMEND_CHIP_MAX_DEFAULT: number = 3;
  static readonly RECOMMEND_CHIP_PADDING_V_SMALL: number = 8;
  static readonly RECOMMEND_CHIP_PADDING_V_MEDIUM: number = 12;
  static readonly RECOMMEND_CHIP_PADDING_V_LARGE: number = 16;
  static readonly RECOMMEND_ICON_SIZE: number = 16;
  static readonly RECOMMEND_SPACING_TITLE_CHIPS: number = 12;
  static readonly CONTENT_COLLAPSE_HEIGHT: number = 200;
  static readonly MARKDOWN_BOTTOM_PADDING: number = 8;
  static readonly MARKDOWN_BOTTOM_PADDING_FONTSCALE_MEDIUM: number = 12;
  static readonly MARKDOWN_BOTTOM_PADDING_FONTSCALE_LARGE: number = 16;
  static readonly CARD_BOTTOM_PADDING: number = 8;
  static readonly RECOMMEND_BOTTOM_PADDING: number = 12;
  static readonly CARD_TOGGLE_BUTTON_PADDING_V_SMALL: number = 8;
  static readonly CARD_TOGGLE_BUTTON_PADDING_V_MEDIUM: number = 16;
  static readonly CARD_TOGGLE_BUTTON_PADDING_V_LARGE: number = 20;
  static readonly CARD_TOGGLE_ROW_PADDING_VERTICAL: number = 4;
  static readonly CARD_TOGGLE_ICON_SIZE: number = 24;
  static readonly TEXT_CARD_EXPAND: Resource = $r('app.string.card_expand');
  static readonly TEXT_CARD_COLLAPSE: Resource = $r('app.string.card_collapse');
  static readonly ELLIPSIS_TEXT: string = '...';
  static readonly COLLAPSE_ICON_FONT_SIZE: number = 24;
  static readonly COLLAPSE_ICON_SIZE: number = 24;
  static readonly COLLAPSE_BUTTON_GAP: number = 8;
  static readonly COLLAPSE_BUTTON_RADIUS: number = 12;
  static readonly ICON_EXPAND: Resource = $r('sys.symbol.chevron_down');
  static readonly ICON_COLLAPSE: Resource = $r('sys.symbol.chevron_up');
  static readonly COLLAPSE_BUTTON_BG: Resource = $r('sys.color.comp_background_tertiary');
}
