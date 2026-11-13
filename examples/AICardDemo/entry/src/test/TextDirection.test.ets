import { describe, it, expect } from '@ohos/hypium';
import { TextDirectionUtil } from '../main/ets/components/aicard/common/TextDirectionUtil';

export default function textDirectionTest() {
  describe('TextDirectionUtil', () => {
    it('empty_string_is_ltr', 0, () => {
      expect(TextDirectionUtil.isRtlText('')).assertFalse();
    });

    it('chinese_is_ltr', 0, () => {
      expect(TextDirectionUtil.isRtlText('这是一段中文文本，用于验证折叠图标的侧别判定')).assertFalse();
    });

    it('english_is_ltr', 0, () => {
      expect(TextDirectionUtil.isRtlText('The quick brown fox jumps')).assertFalse();
    });

    it('arabic_is_rtl', 0, () => {
      expect(TextDirectionUtil.isRtlText('مرحبا بالعالم')).assertTrue();
    });

    it('hebrew_is_rtl', 0, () => {
      expect(TextDirectionUtil.isRtlText('שלום עולם')).assertTrue();
    });

    it('neutral_prefix_then_arabic_is_rtl', 0, () => {
      expect(TextDirectionUtil.isRtlText('123 مرحبا')).assertTrue();
    });

    it('neutral_prefix_then_chinese_is_ltr', 0, () => {
      expect(TextDirectionUtil.isRtlText('... 你好世界')).assertFalse();
    });

    it('chinese_then_arabic_word_is_ltr', 0, () => {
      expect(TextDirectionUtil.isRtlText('这是 Arabic مرحبا 混排测试')).assertFalse();
    });

    it('arabic_then_chinese_word_is_rtl', 0, () => {
      expect(TextDirectionUtil.isRtlText('مرحبا 中文测试 مرحبا')).assertTrue();
    });

    it('digits_only_defaults_ltr', 0, () => {
      expect(TextDirectionUtil.isRtlText('123 456')).assertFalse();
    });
  });
}
