import localUnitTest from './LocalUnit.test';
import streamEngineTest from './StreamEngine.test';
import timeoutManagerTest from './TimeoutManager.test';
import stateMachineTest from './AiCardStateMachine.test';
import sessionControllerTest from './AiCardSessionController.test';
import facadeTest from './AiCardController.test';
import testProviderTest from './TestSearchProvider.test';
import markdownParserTest from './MarkdownParser.test';
import markdownMeasureTest from './MarkdownMeasure.test';
import textDirectionTest from './TextDirection.test';

export default function testsuite() {
  localUnitTest();
  streamEngineTest();
  timeoutManagerTest();
  stateMachineTest();
  sessionControllerTest();
  facadeTest();
  testProviderTest();
  markdownParserTest();
  markdownMeasureTest();
  textDirectionTest();
}