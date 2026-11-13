/**
 * Markdown 测算器：逐字测算截断逻辑，确保折叠态不超 maxLines 行。
 * 纯逻辑，countLines 函数注入，可本地 UT。
 * @since AICardDemo-v1.3
 */

import { MeasureResult } from './AiCardTypes';

export class MarkdownMeasurer {

  public static measure(
    text: string,
    maxWidth: number,
    maxLines: number,
    ellipsis: string,
    countLines: (text: string, maxWidth: number) => number
  ): MeasureResult {
    if (text.length === 0 || maxWidth <= 0) {
      const empty: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
      return empty;
    }

    const estimatedThreshold: number = Math.floor(maxWidth / 20 * maxLines);
    if (text.length < estimatedThreshold) {
      const short: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
      return short;
    }

    let fullLineCount: number;
    try {
      fullLineCount = countLines(text, maxWidth);
    } catch (e) {
      const fallback: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
      return fallback;
    }

    if (fullLineCount <= maxLines) {
      const ok: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
      return ok;
    }

    let low: number = 0;
    let high: number = text.length;
    let bestIndex: number = 0;

    while (low <= high) {
      const mid: number = Math.floor((low + high) / 2);
      const truncated: string = text.substring(0, mid) + ellipsis;
      let lineCount: number;
      try {
        lineCount = countLines(truncated, maxWidth);
      } catch (e) {
        lineCount = maxLines + 1;
      }
      if (lineCount <= maxLines) {
        bestIndex = mid;
        low = mid + 1;
      } else {
        high = mid - 1;
      }
    }

    if (bestIndex === 0) {
      const noTruncate: MeasureResult = { displayText: text, isCollapsed: false, truncationIndex: -1 };
      return noTruncate;
    }

    const displayText: string = text.substring(0, bestIndex) + ellipsis;
    const result: MeasureResult = { displayText: displayText, isCollapsed: true, truncationIndex: bestIndex };
    return result;
  }
}
