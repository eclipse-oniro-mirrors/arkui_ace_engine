import { describe, it, expect } from '@ohos/hypium';
import { MarkdownMeasurer } from '../main/ets/components/aicard/common/MarkdownMeasurer';

function makeLineCounter(charsPerLine: number): (text: string, maxWidth: number) => number {
  return (text: string, _: number) => {
    if (text.length === 0) {
      return 0;
    }
    return Math.ceil(text.length / charsPerLine);
  };
}

export default function markdownMeasureTest() {
  describe('MarkdownMeasurer', () => {
    it('short_text_no_truncation', 0, () => {
      const counter = makeLineCounter(50);
      const result = MarkdownMeasurer.measure('short', 300, 3, '...', counter);
      expect(result.isCollapsed).assertFalse();
      expect(result.displayText).assertEqual('short');
      expect(result.truncationIndex).assertEqual(-1);
    });

    it('text_within_max_lines_no_truncation', 0, () => {
      const counter = makeLineCounter(20);
      const result = MarkdownMeasurer.measure('a'.repeat(60), 300, 3, '...', counter);
      expect(result.isCollapsed).assertFalse();
      expect(result.displayText).assertEqual('a'.repeat(60));
    });

    it('text_exceeding_max_lines_truncated', 0, () => {
      const counter = makeLineCounter(20);
      const result = MarkdownMeasurer.measure('a'.repeat(100), 300, 3, '...', counter);
      expect(result.isCollapsed).assertTrue();
      expect(result.displayText).assertContain('...');
      expect(result.displayText.length).assertLess(100);
    });

    it('truncated_text_does_not_exceed_max_lines', 0, () => {
      const counter = makeLineCounter(20);
      const result = MarkdownMeasurer.measure('a'.repeat(100), 300, 3, '...', counter);
      const lineCount = counter(result.displayText, 300);
      expect(lineCount).assertLessOrEqual(3);
    });

    it('threshold_skip_short_text', 0, () => {
      const counter = makeLineCounter(1000);
      const result = MarkdownMeasurer.measure('short text', 300, 3, '...', counter);
      expect(result.isCollapsed).assertFalse();
    });

    it('zero_width_fallback', 0, () => {
      const counter = makeLineCounter(20);
      const result = MarkdownMeasurer.measure('some text', 0, 3, '...', counter);
      expect(result.isCollapsed).assertFalse();
      expect(result.displayText).assertEqual('some text');
    });

    it('empty_text_fallback', 0, () => {
      const counter = makeLineCounter(20);
      const result = MarkdownMeasurer.measure('', 300, 3, '...', counter);
      expect(result.displayText).assertEqual('');
      expect(result.isCollapsed).assertFalse();
    });

    it('countLines_exception_fallback', 0, () => {
      const throwingCounter = (_text: string, _w: number): number => {
        throw new Error('measure failed');
      };
      const result = MarkdownMeasurer.measure('a'.repeat(100), 300, 3, '...', throwingCounter);
      expect(result.isCollapsed).assertFalse();
      expect(result.displayText).assertEqual('a'.repeat(100));
    });

    it('binary_search_finds_truncation', 0, () => {
      let callCount = 0;
      const counter = (text: string, _: number) => {
        callCount++;
        return Math.ceil(text.length / 20);
      };
      const result = MarkdownMeasurer.measure('a'.repeat(100), 300, 3, '...', counter);
      expect(result.isCollapsed).assertTrue();
      expect(result.truncationIndex).assertLarger(0);
      expect(callCount).assertLess(30);
    });
  });
}
