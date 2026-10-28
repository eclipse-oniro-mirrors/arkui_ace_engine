import { hilog } from '@kit.PerformanceAnalysisKit';

/**
 * AiCard DFX 日志封装。hilog 分模块 domain、分级（DEBUG~FATAL）。
 * release/异常=ERROR 级，流程节点=DEBUG/INFO 级（R-19）。
 * @since AICardDemo-v1.0
 */
export class AiCardLogger {
  static debug(domain: number, tag: string, msg: string): void {
    hilog.debug(domain, tag, '%{public}s', msg);
  }

  static info(domain: number, tag: string, msg: string): void {
    hilog.info(domain, tag, '%{public}s', msg);
  }

  static warn(domain: number, tag: string, msg: string): void {
    hilog.warn(domain, tag, '%{public}s', msg);
  }

  static error(domain: number, tag: string, msg: string): void {
    hilog.error(domain, tag, '%{public}s', msg);
  }

  static fatal(domain: number, tag: string, msg: string): void {
    hilog.fatal(domain, tag, '%{public}s', msg);
  }
}
