import { describe, it, expect } from '@ohos/hypium';
import { MarkdownParser, MarkdownStyleType } from '../main/ets/components/aicard/common/MarkdownParser';

export default function markdownParserTest() {
  describe('MarkdownParser', () => {
    it('plainText_no_markers', 0, () => {
      const result = MarkdownParser.parse('hello world');
      expect(result.plainText).assertEqual('hello world');
      expect(result.styles.length).assertEqual(0);
    });

    it('bold_double_asterisk', 0, () => {
      const result = MarkdownParser.parse('**bold**');
      expect(result.plainText).assertEqual('bold');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].start).assertEqual(0);
      expect(result.styles[0].length).assertEqual(4);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.BOLD);
    });

    it('bold_double_underscore', 0, () => {
      const result = MarkdownParser.parse('__bold__');
      expect(result.plainText).assertEqual('bold');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.BOLD);
    });

    it('italic_single_asterisk', 0, () => {
      const result = MarkdownParser.parse('*italic*');
      expect(result.plainText).assertEqual('italic');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].start).assertEqual(0);
      expect(result.styles[0].length).assertEqual(6);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.ITALIC);
    });

    it('italic_single_underscore', 0, () => {
      const result = MarkdownParser.parse('_italic_');
      expect(result.plainText).assertEqual('italic');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.ITALIC);
    });

    it('header_h1', 0, () => {
      const result = MarkdownParser.parse('# Header');
      expect(result.plainText).assertEqual('Header');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].start).assertEqual(0);
      expect(result.styles[0].length).assertEqual(6);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.HEADER1);
    });

    it('header_h2', 0, () => {
      const result = MarkdownParser.parse('## Header');
      expect(result.plainText).assertEqual('Header');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.HEADER2);
    });

    it('header_h3', 0, () => {
      const result = MarkdownParser.parse('### Header');
      expect(result.plainText).assertEqual('Header');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.HEADER3);
    });

    it('bullet_list_dash', 0, () => {
      const result = MarkdownParser.parse('- item');
      expect(result.plainText).assertEqual('• item');
      expect(result.styles.length).assertEqual(0);
    });

    it('bullet_list_asterisk', 0, () => {
      const result = MarkdownParser.parse('* item');
      expect(result.plainText).assertEqual('• item');
      expect(result.styles.length).assertEqual(0);
    });

    it('ordered_list', 0, () => {
      const result = MarkdownParser.parse('1. item');
      expect(result.plainText).assertEqual('1. item');
      expect(result.styles.length).assertEqual(0);
    });

    it('inline_code', 0, () => {
      const result = MarkdownParser.parse('`code`');
      expect(result.plainText).assertEqual('code');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].start).assertEqual(0);
      expect(result.styles[0].length).assertEqual(4);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.CODE);
    });

    it('line_break', 0, () => {
      const result = MarkdownParser.parse('line1\nline2');
      expect(result.plainText).assertEqual('line1\nline2');
      expect(result.styles.length).assertEqual(0);
    });

    it('paragraph_break', 0, () => {
      const result = MarkdownParser.parse('para1\n\npara2');
      expect(result.plainText).assertEqual('para1\n\npara2');
      expect(result.styles.length).assertEqual(0);
    });

    it('unclosed_bold_literal', 0, () => {
      const result = MarkdownParser.parse('**bold');
      expect(result.plainText).assertEqual('**bold');
      expect(result.styles.length).assertEqual(0);
    });

    it('unclosed_italic_literal', 0, () => {
      const result = MarkdownParser.parse('*italic');
      expect(result.plainText).assertEqual('*italic');
      expect(result.styles.length).assertEqual(0);
    });

    it('unclosed_code_literal', 0, () => {
      const result = MarkdownParser.parse('`code');
      expect(result.plainText).assertEqual('`code');
      expect(result.styles.length).assertEqual(0);
    });

    it('unsupported_table_literal', 0, () => {
      const result = MarkdownParser.parse('| col1 | col2 |');
      expect(result.plainText).assertEqual('| col1 | col2 |');
      expect(result.styles.length).assertEqual(0);
    });

    it('header_with_bold', 0, () => {
      const result = MarkdownParser.parse('# **bold**');
      expect(result.plainText).assertEqual('bold');
      expect(result.styles.length).assertEqual(2);
      expect(result.styles[0].start).assertEqual(0);
      expect(result.styles[0].length).assertEqual(4);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.BOLD);
      expect(result.styles[1].start).assertEqual(0);
      expect(result.styles[1].length).assertEqual(4);
      expect(result.styles[1].type).assertEqual(MarkdownStyleType.HEADER1);
    });

    it('bold_and_italic_mixed', 0, () => {
      const result = MarkdownParser.parse('**bold** and *italic*');
      expect(result.plainText).assertEqual('bold and italic');
      expect(result.styles.length).assertEqual(2);
      expect(result.styles[0].start).assertEqual(0);
      expect(result.styles[0].length).assertEqual(4);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.BOLD);
      expect(result.styles[1].start).assertEqual(9);
      expect(result.styles[1].length).assertEqual(6);
      expect(result.styles[1].type).assertEqual(MarkdownStyleType.ITALIC);
    });

    it('empty_text', 0, () => {
      const result = MarkdownParser.parse('');
      expect(result.plainText).assertEqual('');
      expect(result.styles.length).assertEqual(0);
    });

    it('h4_not_supported_literal', 0, () => {
      const result = MarkdownParser.parse('#### Header');
      expect(result.plainText).assertEqual('#### Header');
      expect(result.styles.length).assertEqual(0);
    });

    it('hash_no_space_not_header', 0, () => {
      const result = MarkdownParser.parse('#NoSpace');
      expect(result.plainText).assertEqual('#NoSpace');
      expect(result.styles.length).assertEqual(0);
    });

    it('multiline_with_styles', 0, () => {
      const result = MarkdownParser.parse('# Title\n**bold**\n`code`');
      expect(result.plainText).assertEqual('Title\nbold\ncode');
      expect(result.styles.length).assertEqual(3);
      expect(result.styles[0].start).assertEqual(0);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.HEADER1);
      expect(result.styles[1].start).assertEqual(6);
      expect(result.styles[1].type).assertEqual(MarkdownStyleType.BOLD);
      expect(result.styles[2].start).assertEqual(11);
      expect(result.styles[2].type).assertEqual(MarkdownStyleType.CODE);
    });

    it('list_with_italic', 0, () => {
      const result = MarkdownParser.parse('- *italic*');
      expect(result.plainText).assertEqual('• italic');
      expect(result.styles.length).assertEqual(1);
      expect(result.styles[0].start).assertEqual(2);
      expect(result.styles[0].length).assertEqual(6);
      expect(result.styles[0].type).assertEqual(MarkdownStyleType.ITALIC);
    });
  });
}
