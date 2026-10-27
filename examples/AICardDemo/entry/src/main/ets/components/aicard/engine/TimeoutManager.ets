import { AiCardConstants } from '../common/AiCardConstants';

/**
 * 双定时器管理：首包超时（60s）与流中断超时（30s）。
 * 阈值经构造参数注入（UT 用小值验证逻辑而非 mock 时钟）。超时回调由 controller 记日志。
 * @since AICardDemo-v1.0
 */
export class TimeoutManager {
  private firstPacketTimer: number = -1;
  private streamGapTimer: number = -1;
  private readonly firstPacketMs: number;
  private readonly streamGapMs: number;
  private readonly onFirstPacketTimeout: () => void;
  private readonly onStreamGapTimeout: () => void;

  constructor(onFirstPacketTimeout: () => void, onStreamGapTimeout: () => void,
    firstPacketMs: number = AiCardConstants.FIRST_PACKET_TIMEOUT_MS,
    streamGapMs: number = AiCardConstants.STREAM_GAP_TIMEOUT_MS) {
    this.onFirstPacketTimeout = onFirstPacketTimeout;
    this.onStreamGapTimeout = onStreamGapTimeout;
    this.firstPacketMs = firstPacketMs;
    this.streamGapMs = streamGapMs;
  }

  startFirstPacket(): void {
    this.stopFirstPacket();
    const ms: number = this.firstPacketMs;
    this.firstPacketTimer = setTimeout(() => {
      this.fireFirstPacket();
    }, ms);
  }

  stopFirstPacket(): void {
    if (this.firstPacketTimer !== -1) {
      clearTimeout(this.firstPacketTimer);
      this.firstPacketTimer = -1;
    }
  }

  resetStreamGap(): void {
    this.stopStreamGap();
    const ms: number = this.streamGapMs;
    this.streamGapTimer = setTimeout(() => {
      this.fireStreamGap();
    }, ms);
  }

  stopStreamGap(): void {
    if (this.streamGapTimer !== -1) {
      clearTimeout(this.streamGapTimer);
      this.streamGapTimer = -1;
    }
  }

  stopAll(): void {
    this.stopFirstPacket();
    this.stopStreamGap();
  }

  private fireFirstPacket(): void {
    this.firstPacketTimer = -1;
    this.onFirstPacketTimeout();
  }

  private fireStreamGap(): void {
    this.streamGapTimer = -1;
    this.onStreamGapTimeout();
  }
}
