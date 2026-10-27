import { AiSearchStatus } from '../common/AiCardTypes';

type AiCardInternalState = 'idle' | 'searching' | 'streaming' | 'completed' | 'error' | 'stopped';

/**
 * 会话状态机：内部 6 态 idle→searching→streaming→{completed,error,stopped}；
 * 对外 AiSearchStatus 5 值映射（stopped→'completed'，ADR-6）；终态幂等守卫（R-12/R-14）；
 * 同状态重复迁移不重发（R-20）。
 * @since AICardDemo-v1.0
 */
export class AiCardStateMachine {
  private state: AiCardInternalState = 'idle';
  private finalized: boolean = false;
  private lastEmitted: AiSearchStatus = 'idle';
  private readonly onStatusChange: (status: AiSearchStatus) => void;

  constructor(onStatusChange: (status: AiSearchStatus) => void) {
    this.onStatusChange = onStatusChange;
  }

  getStatus(): AiSearchStatus {
    return AiCardStateMachine.toExternal(this.state);
  }

  isFinalized(): boolean {
    return this.finalized;
  }

  toSearching(): void {
    if (this.finalized || this.state !== 'idle') {
      return;
    }
    this.state = 'searching';
    this.emit();
  }

  toStreaming(): void {
    if (this.finalized) {
      return;
    }
    if (this.state !== 'searching' && this.state !== 'streaming') {
      return;
    }
    this.state = 'streaming';
    this.emit();
  }

  toCompleted(): void {
    this.finalize('completed');
  }

  toError(): void {
    this.finalize('error');
  }

  toStopped(): void {
    this.finalize('stopped');
  }

  private finalize(target: AiCardInternalState): void {
    if (this.finalized) {
      return;
    }
    this.state = target;
    this.finalized = true;
    this.emit();
  }

  private emit(): void {
    const ext: AiSearchStatus = this.getStatus();
    if (ext !== this.lastEmitted) {
      this.lastEmitted = ext;
      this.onStatusChange(ext);
    }
  }

  private static toExternal(internal: AiCardInternalState): AiSearchStatus {
    switch (internal) {
      case 'idle':
        return 'idle';
      case 'searching':
        return 'searching';
      case 'streaming':
        return 'streaming';
      case 'completed':
        return 'completed';
      case 'error':
        return 'error';
      case 'stopped':
        return 'completed';
    }
  }
}
