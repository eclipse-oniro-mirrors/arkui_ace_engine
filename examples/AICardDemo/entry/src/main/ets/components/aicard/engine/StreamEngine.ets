import { AiCardConstants } from '../common/AiCardConstants';
import { SearchResult, StreamStatus } from '../common/AiCardTypes';

/**
 * 流式文本引擎：打字机推进、乱序重组、grounding 替换与进度保持、status=17 过滤。
 * 纯逻辑，无 UI 依赖（OH-ARCH-LAYERING）。DFX 埋点由 controller 在 onEvent 回调中完成。
 * @since AICardDemo-v1.0
 */
export class StreamEngine {
  private targetText: string = '';
  private displayedText: string = '';
  private cursor: number = 0;
  private readonly chunkBuffer: Map<number, string> = new Map<number, string>();
  private nextIndex: number = 0;
  private grounded: boolean = false;
  private streamEnded: boolean = false;
  private ended: boolean = false;
  private startEmitted: boolean = false;
  private playing: boolean = false;
  private intervalId: number = -1;

  private readonly streamTickMs: number;
  private readonly showMarkdown: boolean;
  private readonly onEvent: (status: StreamStatus, displayedText: string) => void;

  constructor(streamTickMs: number, showMarkdown: boolean,
    onEvent: (status: StreamStatus, displayedText: string) => void) {
    this.streamTickMs = streamTickMs > 0 ? streamTickMs : AiCardConstants.STREAM_TICK_MS_DEFAULT;
    this.showMarkdown = showMarkdown;
    this.onEvent = onEvent;
  }

  push(result: SearchResult): void {
    if (this.ended) {
      return;
    }
    if (this.grounded && result.status === 17) {
      return;
    }
    if (this.isGroundingBody(result.streamingText)) {
      this.targetText = this.extractGrounding(result.streamingText);
      this.grounded = true;
      if (this.cursor > this.targetText.length) {
        this.cursor = this.targetText.length;
      }
      this.displayedText = this.targetText.substring(0, this.cursor);
    } else {
      if (result.index >= this.nextIndex) {
        this.chunkBuffer.set(result.index, result.streamingText);
        this.popConsecutive();
      }
    }
    if (result.isFinal) {
      this.streamEnded = true;
    }
    this.advancePlayback();
  }

  private advancePlayback(): void {
    if (!this.showMarkdown) {
      if (this.cursor < this.targetText.length) {
        this.cursor = this.targetText.length;
        this.displayedText = this.targetText;
      }
      this.emitStartIfNeeded();
      if (this.streamEnded && this.cursor >= this.targetText.length) {
        this.emitEnd();
      }
    } else {
      this.emitStartIfNeeded();
      if (this.streamEnded && this.cursor >= this.targetText.length) {
        this.emitEnd();
      } else if (this.cursor < this.targetText.length && !this.playing && !this.ended) {
        this.startTick();
      }
    }
  }

  private popConsecutive(): void {
    while (this.chunkBuffer.has(this.nextIndex)) {
      const chunk: string | undefined = this.chunkBuffer.get(this.nextIndex);
      if (chunk !== undefined) {
        this.targetText += chunk;
      }
      this.chunkBuffer.delete(this.nextIndex);
      this.nextIndex++;
    }
  }

  private startTick(): void {
    if (this.playing) {
      return;
    }
    this.playing = true;
    const tickMs: number = this.streamTickMs;
    this.intervalId = setInterval(() => {
      this.tick();
    }, tickMs);
  }

  private tick(): void {
    if (this.ended) {
      this.stopTick();
      return;
    }
    if (this.cursor < this.targetText.length) {
      this.cursor++;
      this.displayedText = this.targetText.substring(0, this.cursor);
      this.emitStartIfNeeded();
      this.onEvent('PARTIAL', this.displayedText);
    }
    if (this.cursor >= this.targetText.length) {
      if (this.streamEnded) {
        this.stopTick();
        this.emitEnd();
      } else {
        this.stopTick();
      }
    }
  }

  private emitStartIfNeeded(): void {
    if (!this.startEmitted && this.cursor > 0) {
      this.startEmitted = true;
      this.onEvent('START', this.displayedText);
    }
  }

  private emitEnd(): void {
    if (this.ended) {
      return;
    }
    this.stopTick();
    this.ended = true;
    this.onEvent('END', this.displayedText);
  }

  stop(): void {
    this.stopTick();
  }

  skipToEnd(): void {
    if (this.ended) {
      return;
    }
    this.cursor = this.targetText.length;
    this.displayedText = this.targetText;
    this.stopTick();
    if (this.streamEnded) {
      this.emitEnd();
    }
  }

  private stopTick(): void {
    if (this.intervalId !== -1) {
      clearInterval(this.intervalId);
      this.intervalId = -1;
    }
    this.playing = false;
  }

  getTargetText(): string {
    return this.targetText;
  }

  getDisplayedText(): string {
    return this.displayedText;
  }

  getCursor(): number {
    return this.cursor;
  }

  isGrounded(): boolean {
    return this.grounded;
  }

  isStreamEnded(): boolean {
    return this.streamEnded;
  }

  isEnded(): boolean {
    return this.ended;
  }

  private isGroundingBody(text: string): boolean {
    return text.startsWith(AiCardConstants.GROUNDING_OPEN_TAG)
      && text.endsWith(AiCardConstants.GROUNDING_CLOSE_TAG);
  }

  private extractGrounding(text: string): string {
    const start: number = AiCardConstants.GROUNDING_OPEN_TAG.length;
    const end: number = text.length - AiCardConstants.GROUNDING_CLOSE_TAG.length;
    return text.substring(start, end);
  }
}
