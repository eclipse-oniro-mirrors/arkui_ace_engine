import { AiCardConstants } from '../common/AiCardConstants';
import { MarkdownParser, MarkdownStyleType, ParsedMarkdown, MarkdownStyleRange } from '../common/MarkdownParser';
import { LengthMetrics } from '@kit.ArkUI';

@Component
export struct MarkdownRender {
  @Prop @Watch('onTextChanged') text: string = '';

  private textController: TextController = new TextController();

  aboutToAppear(): void {
    this.textController.setStyledString(this.buildStyledString());
  }

  build() {
    Text('', { controller: this.textController })
      .width('100%')
  }

  onTextChanged(): void {
    this.textController.setStyledString(this.buildStyledString());
  }

  private buildStyledString(): StyledString {
    if (this.text.length === 0) {
      return new StyledString('', []);
    }

    const parsed: ParsedMarkdown = MarkdownParser.parse(this.text);
    const textLength: number = parsed.plainText.length;

    const baseFont: TextStyle = new TextStyle({
      fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_BODY)
    });

    const options: StyleOptions[] = [
      { start: 0, length: textLength, styledKey: StyledStringKey.FONT, styledValue: baseFont }
    ];

    for (let i: number = 0; i < parsed.styles.length; i++) {
      const s: MarkdownStyleRange = parsed.styles[i];
      if (s.type === MarkdownStyleType.BOLD) {
        const style: TextStyle = new TextStyle({ fontWeight: FontWeight.Bold });
        const opt: StyleOptions = {
          start: s.start, length: s.length,
          styledKey: StyledStringKey.FONT, styledValue: style
        };
        options.push(opt);
      } else if (s.type === MarkdownStyleType.ITALIC) {
        const style: TextStyle = new TextStyle({ fontStyle: FontStyle.Italic });
        const opt: StyleOptions = {
          start: s.start, length: s.length,
          styledKey: StyledStringKey.FONT, styledValue: style
        };
        options.push(opt);
      } else if (s.type === MarkdownStyleType.CODE) {
        const style: TextStyle = new TextStyle({ fontFamily: 'monospace' });
        const opt: StyleOptions = {
          start: s.start, length: s.length,
          styledKey: StyledStringKey.FONT, styledValue: style
        };
        options.push(opt);
      } else if (s.type === MarkdownStyleType.HEADER1) {
        const style: TextStyle = new TextStyle({ fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_HEADER1) });
        const opt: StyleOptions = {
          start: s.start, length: s.length,
          styledKey: StyledStringKey.FONT, styledValue: style
        };
        options.push(opt);
      } else if (s.type === MarkdownStyleType.HEADER2) {
        const style: TextStyle = new TextStyle({ fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_TITLE) });
        const opt: StyleOptions = {
          start: s.start, length: s.length,
          styledKey: StyledStringKey.FONT, styledValue: style
        };
        options.push(opt);
      } else if (s.type === MarkdownStyleType.HEADER3) {
        const style: TextStyle = new TextStyle({ fontSize: LengthMetrics.fp(AiCardConstants.FONT_SIZE_HEADER3) });
        const opt: StyleOptions = {
          start: s.start, length: s.length,
          styledKey: StyledStringKey.FONT, styledValue: style
        };
        options.push(opt);
      }
    }

    return new StyledString(parsed.plainText, options);
  }
}
