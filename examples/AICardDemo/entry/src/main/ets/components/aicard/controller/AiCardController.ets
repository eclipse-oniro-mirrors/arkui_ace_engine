import { AiCardConstants } from '../common/AiCardConstants';
import { AiCardLogger } from '../common/AiCardLogger';
import { CardRequestOptions } from '../common/AiCardTypes';

/** 组件侧实现的目标接口（UI→controller 方向依赖，合法）。@internal 宿主不应使用。 */
export interface AiCardControlTarget {
  request(query: string, requestOptions?: CardRequestOptions): void;
  cancel(): void;
}

/** @internal 绑定注册表（模块级单例），仅绑定通道使用。 */
const targetRegistry: Map<AiCardController, AiCardControlTarget> = new Map();

/**
 * AiCard 公共控制器：开发者持有，经 AiCard controller 参数注入。
 * 对外仅 request(query, requestOptions?)（重新搜索；requestOptions 为空接口预留扩展位，
 * 透传至组件、本版本无字段）与 cancel()（停止生成，与停止按钮效果一致）。
 * 返回值维持 void，会话结果经既有生命周期回调（onAnimatedStart/onAnimatedEnd/onError 等）通知。
 * 绑定通道为模块级 @internal 函数，不在类表面。
 * @since AICardDemo-v1.2
 */
export class AiCardController {
  request(query: string, requestOptions?: CardRequestOptions): void {
    const target: AiCardControlTarget | null = targetRegistry.get(this) ?? null;
    if (target === null) {
      AiCardLogger.warn(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER,
        'request ignored: controller not bound');
      return;
    }
    target.request(query, requestOptions);
  }

  cancel(): void {
    const target: AiCardControlTarget | null = targetRegistry.get(this) ?? null;
    if (target === null) {
      AiCardLogger.debug(AiCardConstants.AICARD_DOMAIN_CONTROLLER, AiCardConstants.TAG_CONTROLLER,
        'cancel ignored: controller not bound');
      return;
    }
    target.cancel();
  }
}

/** @internal 仅供 AiCard 组件调用：绑定（覆盖式）。 */
export function bindAiCardController(controller: AiCardController, target: AiCardControlTarget): void {
  targetRegistry.set(controller, target);
}

/** @internal 仅供 AiCard 组件调用：解绑（校验式，防 ForEach 重建竞态误杀新绑定）。 */
export function unbindAiCardController(controller: AiCardController, target: AiCardControlTarget): void {
  if (targetRegistry.get(controller) === target) {
    targetRegistry.delete(controller);
  }
}
