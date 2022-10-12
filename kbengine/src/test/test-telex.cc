#include <gtest/gtest.h>
#include "fixture/kbengine-test-telex.cc"

// Test input ] --> ư
TEST_F(KbEngineTestTelex, TestTelex_Input_LEFT_BRACKET_Get_U_Hook) {
  vector<UInt16> charCodes = { ']' };

  vector<UInt32> expectedOut = {
                                 getVNCharAt(KEY_U | MASK_HOOK, 1),   // add ư
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

// Test input ]] --> ]
TEST_F(KbEngineTestTelex, TestTelex_Input_LEFT_BRACKET_Get_Right_Bracket) {
  vector<UInt16> charCodes = { ']' };

  vector<UInt32> expectedOut = {
                                KEY_DELETE,
                                KEY_RIGHT_BRACKET,
                              };

  engine.resetBuffer();
  
  // send first ']'
  sendCharCodes(charCodes);
  // send next ']'
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

// Test input [ --> o+
TEST_F(KbEngineTestTelex, TestTelex_Input_RIGHT_BRACKET_Get_O_Hook) {
  vector<UInt16> charCodes = { '[' };

  vector<UInt32> expectedOut = {
                                 getVNCharAt(KEY_O | MASK_HOOK, 1),   // add o+
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}