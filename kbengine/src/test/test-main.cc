#include <gtest/gtest.h>

#include "fixture/kbengine-test.cc"

TEST_F(KbEngineTest, TestInput_viet65) {
  vector<UInt16> keyCodes = { KEY_V, KEY_I, KEY_E, KEY_T, KEY_6, KEY_5 };
  vector<UInt16> charCodes = { 'v', 'i', 'e', 't' };

  // expect 2 delete keystore + unicode-ê, KEY_T
  vector<UInt32> expectedOut = { 
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE), 
                                 getVNCharAt(KEY_E | MASK_ROOF, 11), UInt32(KEY_T)
                                };

  engine.resetBuffer();
  
  for(int i=0; i < keyCodes.size(); i++) {
    engine.process(charCodes[i], keyCodes[i], 0, false);
  }

  vector<UInt32> keystrokes = engine.getOutputBuffer();
  
  EXPECT_EQ(keystrokes, expectedOut);
}
