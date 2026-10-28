import { describe, it, expect } from '@ohos/hypium';
import { Driver, ON } from '@ohos.UiTest';

async function textAppears(driver: Driver, text: string): Promise<boolean> {
  try {
    const comp = await driver.findComponent(ON.text(text));
    return comp !== null;
  } catch (e) {
    return false;
  }
}

async function clickText(driver: Driver, text: string): Promise<boolean> {
  try {
    const btn = await driver.findComponent(ON.text(text));
    if (btn !== null) {
      await btn.click();
      return true;
    }
    return false;
  } catch (e) {
    return false;
  }
}

async function waitText(driver: Driver, text: string, timeoutMs: number): Promise<boolean> {
  const start: number = Date.now();
  while (Date.now() - start < timeoutMs) {
    if (await textAppears(driver, text)) {
      return true;
    }
    await driver.delayMs(200);
  }
  return false;
}

export default function aiCardFlowTest() {
  describe('AiCardFlow', () => {
    it('page_loads_smoke', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(500);
      const start: boolean = await textAppears(driver, '开始');
      expect(start).assertTrue();
      await clickText(driver, '开始');
      const standby: boolean = await waitText(driver, '思考中...', 2000);
      expect(standby).assertTrue();
      await driver.delayMs(1500);
      const found: boolean = await textAppears(driver, '为你找到');
      expect(found).assertFalse();
      done();
    });

    it('active_flow_shows_found', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(300);
      await clickText(driver, '开始');
      await driver.delayMs(300);
      await clickText(driver, '重新搜索');
      await driver.delayMs(4000);
      const found: boolean = await textAppears(driver, '为你找到');
      expect(found).assertTrue();
      done();
    });

    it('pause_shows_stopped', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(300);
      await clickText(driver, '开始');
      await driver.delayMs(100);
      await clickText(driver, '暂停');
      await driver.delayMs(300);
      const stopped: boolean = await textAppears(driver, '已停止生成');
      expect(stopped).assertTrue();
      done();
    });

    it('terminal_request_researches', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(300);
      await clickText(driver, '开始');
      await driver.delayMs(300);
      await clickText(driver, '重新搜索');
      await waitText(driver, '为你找到', 8000);
      await clickText(driver, '重新搜索');
      const thinking: boolean = await waitText(driver, '思考中...', 2000);
      expect(thinking).assertTrue();
      const foundAgain: boolean = await waitText(driver, '为你找到', 8000);
      expect(foundAgain).assertTrue();
      done();
    });

    it('streaming_request_restarts', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(300);
      await clickText(driver, '开始');
      await driver.delayMs(300);
      await clickText(driver, '重新搜索');
      const generating: boolean = await waitText(driver, '生成中...', 5000);
      expect(generating).assertTrue();
      try {
        const input = await driver.findComponent(ON.type('TextInput'));
        if (input !== null) {
          await input.inputText('换个问题再搜');
        }
      } catch (e) {
      }
      await clickText(driver, '重新搜索');
      const thinking: boolean = await waitText(driver, '思考中...', 2000);
      expect(thinking).assertTrue();
      const found: boolean = await waitText(driver, '为你找到', 8000);
      expect(found).assertTrue();
      done();
    });

    it('cancel_during_searching_dismisses', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(300);
      await clickText(driver, '开始');
      await driver.delayMs(300);
      await clickText(driver, '重新搜索');
      await driver.delayMs(100);
      await clickText(driver, '取消');
      await driver.delayMs(500);
      const thinking: boolean = await textAppears(driver, '思考中...');
      expect(thinking).assertFalse();
      done();
    });

    it('cancel_during_streaming_shows_stopped', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(300);
      await clickText(driver, '开始');
      await driver.delayMs(300);
      await clickText(driver, '重新搜索');
      const generating: boolean = await waitText(driver, '生成中...', 5000);
      expect(generating).assertTrue();
      await clickText(driver, '取消');
      const stopped: boolean = await waitText(driver, '已停止生成', 2000);
      expect(stopped).assertTrue();
      done();
    });

    it('request_revives_dismissed_card', 0, async (done: Function) => {
      const driver: Driver = new Driver();
      await driver.delayMs(300);
      await clickText(driver, '开始');
      await driver.delayMs(300);
      await clickText(driver, '重新搜索');
      await driver.delayMs(100);
      await clickText(driver, '取消');
      await driver.delayMs(300);
      const dismissed: boolean = await textAppears(driver, '思考中...');
      expect(dismissed).assertFalse();
      await clickText(driver, '重新搜索');
      const found: boolean = await waitText(driver, '为你找到', 8000);
      expect(found).assertTrue();
      done();
    });
  });
}
