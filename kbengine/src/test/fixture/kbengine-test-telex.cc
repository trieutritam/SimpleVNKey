#ifndef TEST_KBENGINE_TEST_TELEX_H_
#define TEST_KBENGINE_TEST_TELEX_H_

#include "kbengine-test.h"

class KbEngineTestTelex: public KbEngineTest {
  protected:
    void SetUp() override {
      KbEngineTest::SetUp();
      // TELEX
      engine.setInputMethod(UInt8(2));
    }
};

#endif