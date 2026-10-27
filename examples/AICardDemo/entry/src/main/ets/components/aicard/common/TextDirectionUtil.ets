/**
 * 文本书写方向判定：首个强方向字符判定 RTL/LTR，与系统 locale 无关。
 * 纯逻辑，可本地 UT。用于 MarkdownCollapseIcon 左右侧自适应（渲染对齐跟随内容字符方向）。
 * @since AICardDemo-v1.4
 */

export class TextDirectionUtil {

  private static isRtlChar(code: number): boolean {
    return (code >= 0x0590 && code <= 0x05FF) || (code >= 0xFB1D && code <= 0xFB4F)
      || (code >= 0x0600 && code <= 0x06FF) || (code >= 0x0750 && code <= 0x077F)
      || (code >= 0x08A0 && code <= 0x08FF) || (code >= 0xFB50 && code <= 0xFDFF)
      || (code >= 0xFE70 && code <= 0xFEFF);
  }

  private static isNeutralChar(code: number): boolean {
    if (code === 0x09 || code === 0x0A || code === 0x0D || code === 0x20) {
      return true;
    }
    if (code >= 0x21 && code <= 0x40) {
      return true;
    }
    if (code >= 0x5B && code <= 0x60) {
      return true;
    }
    if (code >= 0x7B && code <= 0x7E) {
      return true;
    }
    if (code >= 0xA0 && code <= 0xBF) {
      return true;
    }
    if (code >= 0x2000 && code <= 0x206F) {
      return true;
    }
    if (code >= 0x3000 && code <= 0x303F) {
      return true;
    }
    if (code >= 0xFF00 && code <= 0xFFEF) {
      return true;
    }
    return false;
  }

  public static isRtlText(text: string): boolean {
    for (let i: number = 0; i < text.length; i++) {
      const code: number = text.charCodeAt(i);
      if (TextDirectionUtil.isNeutralChar(code)) {
        continue;
      }
      return TextDirectionUtil.isRtlChar(code);
    }
    return false;
  }
}
