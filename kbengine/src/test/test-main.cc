#include <gtest/gtest.h>

#include "fixture/kbengine-test.cc"

TEST_F(KbEngineTest, TestKeyStroke_Input_viet65) {
  vector<UInt16> charCodes = { 'v', 'i', 'e', 't', '6', '5' };

  // expect 2 delete keystore, unicode-ê, KEY_T
  vector<UInt32> expectedOut = {
                                // receive key 6
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE), UInt32(KEY_DELETE),      // delete 3 chars
                                 UInt32(KEY_I), getVNCharAt(KEY_E | MASK_ROOF, 1), UInt32(KEY_T), // add iêt
                                 // receive key 5
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE),                          // delete 2 chars
                                 getVNCharAt(KEY_E | MASK_ROOF, 11), UInt32(KEY_T),               // add ệt
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendKeyStrokes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

// Test uo7 --> uơ
TEST_F(KbEngineTest, TestKeyStroke_Input_uo7) {
  vector<UInt16> charCodes = { 'u', 'o', '7' };

  vector<UInt32> expectedOut = {
                                // receive key 7
                                 UInt32(KEY_DELETE),      // delete 1 chars
                                 getVNCharAt(KEY_O | MASK_HOOK, 1), // add ơ
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendKeyStrokes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

// Test uo7n --> ươn
TEST_F(KbEngineTest, TestKeyStroke_Input_uo7n) {
  vector<UInt16> charCodes = { 'u', 'o', '7', 'n' };

  vector<UInt32> expectedOut = {
                                  // receive key 7
                                 UInt32(KEY_DELETE),      // delete 1 chars
                                 getVNCharAt(KEY_O | MASK_HOOK, 1), // add ơ
                                 // receive 'n'
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE), // delete 2 chars
                                 getVNCharAt(KEY_U | MASK_HOOK, 1),   // add ư
                                 getVNCharAt(KEY_O| MASK_HOOK, 1),    // add ơ
                                 UInt32(KEY_N)                        // add n
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendKeyStrokes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}


// Test uon7 --> ươn
TEST_F(KbEngineTest, TestKeyStroke_Input_uon7) {
  vector<UInt16> charCodes = { 'u', 'o', 'n', '7' };

  vector<UInt32> expectedOut = {
                                 // receive 'n'
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE), UInt32(KEY_DELETE), // delete 3 chars
                                 getVNCharAt(KEY_U | MASK_HOOK, 1),   // add ư
                                 getVNCharAt(KEY_O| MASK_HOOK, 1),    // add ơ
                                 UInt32(KEY_N)                        // add n
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendKeyStrokes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}