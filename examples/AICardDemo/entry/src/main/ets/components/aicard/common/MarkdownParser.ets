/**
 * Markdown 语法解析器：将 markdown 文本解析为纯文本 + 样式区间描述。
 * 纯逻辑，无 UI/hilog 依赖，可本地 UT。
 * @since AICardDemo-v1.3
 */

export enum MarkdownStyleType {
  BOLD = 0,
  ITALIC = 1,
  CODE = 2,
  HEADER1 = 3,
  HEADER2 = 4,
  HEADER3 = 5
}

export interface MarkdownStyleRange {
  start: number;
  length: number;
  type: MarkdownStyleType;
}

export interface ParsedMarkdown {
  plainText: string;
  styles: MarkdownStyleRange[];
}

export class MarkdownParser {

  public static parse(text: string): ParsedMarkdown {
    if (text.length === 0) {
      const empty: ParsedMarkdown = { plainText: '', styles: [] };
      return empty;
    }

    const lines: string[] = text.split('\n');
    let plainText: string = '';
    const styles: MarkdownStyleRange[] = [];

    for (let i: number = 0; i < lines.length; i++) {
      const lineResult: ParsedMarkdown = MarkdownParser.parseLine(lines[i]);
      const offset: number = plainText.length;
      plainText += lineResult.plainText;
      for (let j: number = 0; j < lineResult.styles.length; j++) {
        const s: MarkdownStyleRange = lineResult.styles[j];
        const adjusted: MarkdownStyleRange = {
          start: s.start + offset,
          length: s.length,
          type: s.type
        };
        styles.push(adjusted);
      }
      if (i < lines.length - 1) {
        plainText += '\n';
      }
    }

    const result: ParsedMarkdown = { plainText: plainText, styles: styles };
    return result;
  }

  private static parseLine(line: string): ParsedMarkdown {
    let remaining: string = line;
    let headerLevel: number = 0;
    let listPrefix: string = '';

    let hashCount: number = 0;
    while (hashCount < remaining.length && remaining.charAt(hashCount) === '#') {
      hashCount++;
    }
    if (hashCount >= 1 && hashCount <= 3 && hashCount < remaining.length &&
      remaining.charAt(hashCount) === ' ') {
      headerLevel = hashCount;
      remaining = remaining.substring(hashCount + 1);
    } else if (remaining.startsWith('- ') || remaining.startsWith('* ')) {
      listPrefix = '• ';
      remaining = remaining.substring(2);
    }

    let plainText: string = listPrefix;
    const styles: MarkdownStyleRange[] = [];

    let i: number = 0;
    while (i < remaining.length) {
      const ch: string = remaining.charAt(i);
      const nextCh: string = i + 1 < remaining.length ? remaining.charAt(i + 1) : '';

      if (ch === '*' && nextCh === '*') {
        const closeIdx: number = remaining.indexOf('**', i + 2);
        if (closeIdx >= 0 && closeIdx > i + 1) {
          const inner: string = remaining.substring(i + 2, closeIdx);
          const start: number = plainText.length;
          plainText += inner;
          const range: MarkdownStyleRange = {
            start: start,
            length: inner.length,
            type: MarkdownStyleType.BOLD
          };
          styles.push(range);
          i = closeIdx + 2;
        } else {
          plainText += ch;
          i++;
        }
      } else if (ch === '_' && nextCh === '_') {
        const closeIdx: number = remaining.indexOf('__', i + 2);
        if (closeIdx >= 0 && closeIdx > i + 1) {
          const inner: string = remaining.substring(i + 2, closeIdx);
          const start: number = plainText.length;
          plainText += inner;
          const range: MarkdownStyleRange = {
            start: start,
            length: inner.length,
            type: MarkdownStyleType.BOLD
          };
          styles.push(range);
          i = closeIdx + 2;
        } else {
          plainText += ch;
          i++;
        }
      } else if (ch === '*') {
        const closeIdx: number = MarkdownParser.findSingleClose(remaining, i + 1, '*');
        if (closeIdx >= 0 && closeIdx > i) {
          const inner: string = remaining.substring(i + 1, closeIdx);
          const start: number = plainText.length;
          plainText += inner;
          const range: MarkdownStyleRange = {
            start: start,
            length: inner.length,
            type: MarkdownStyleType.ITALIC
          };
          styles.push(range);
          i = closeIdx + 1;
        } else {
          plainText += ch;
          i++;
        }
      } else if (ch === '_') {
        const closeIdx: number = MarkdownParser.findSingleClose(remaining, i + 1, '_');
        if (closeIdx >= 0 && closeIdx > i) {
          const inner: string = remaining.substring(i + 1, closeIdx);
          const start: number = plainText.length;
          plainText += inner;
          const range: MarkdownStyleRange = {
            start: start,
            length: inner.length,
            type: MarkdownStyleType.ITALIC
          };
          styles.push(range);
          i = closeIdx + 1;
        } else {
          plainText += ch;
          i++;
        }
      } else if (ch === '`') {
        const closeIdx: number = remaining.indexOf('`', i + 1);
        if (closeIdx >= 0) {
          const inner: string = remaining.substring(i + 1, closeIdx);
          const start: number = plainText.length;
          plainText += inner;
          const range: MarkdownStyleRange = {
            start: start,
            length: inner.length,
            type: MarkdownStyleType.CODE
          };
          styles.push(range);
          i = closeIdx + 1;
        } else {
          plainText += ch;
          i++;
        }
      } else {
        plainText += ch;
        i++;
      }
    }

    if (headerLevel > 0) {
      const headerType: MarkdownStyleType = headerLevel === 1 ? MarkdownStyleType.HEADER1 :
        headerLevel === 2 ? MarkdownStyleType.HEADER2 : MarkdownStyleType.HEADER3;
      const range: MarkdownStyleRange = {
        start: 0,
        length: plainText.length,
        type: headerType
      };
      styles.push(range);
    }

    const result: ParsedMarkdown = { plainText: plainText, styles: styles };
    return result;
  }

  private static findSingleClose(text: string, fromIdx: number, marker: string): number {
    let i: number = fromIdx;
    while (i < text.length) {
      if (text.charAt(i) === marker) {
        if (i + 1 < text.length && text.charAt(i + 1) === marker) {
          i += 2;
        } else {
          return i;
        }
      } else {
        i++;
      }
    }
    return -1;
  }
}
