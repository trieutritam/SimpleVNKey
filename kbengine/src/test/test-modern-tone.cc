#include <gtest/gtest.h>
#include "fixture/kbengine-test.h"

TEST_F(KbEngineTest, TestModernTone_Input_viet65) {
  vector<UInt16> charCodes = { 'v', 'i', 'e', 't', '6', '5' };

  useModernToneMethod();

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
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}


TEST_F(KbEngineTest, TestModernTone_Rule1_Input_a1) {
  vector<UInt16> charCodes = { 'a', '1' };

  useModernToneMethod();

  vector<UInt32> expectedOut = {
                                  KEY_DELETE,
                                  getVNCharAt(KEY_A, 3)
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule1_Input_ach5) {
  vector<UInt16> charCodes = { 'a', 'c', 'h', '5' };

  useModernToneMethod();

  vector<UInt32> expectedOut =  {
                                  KEY_DELETE, KEY_DELETE, KEY_DELETE,
                                  getVNCharAt(KEY_A, 11), KEY_C, KEY_H
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule1_Input_giuc5) {
  vector<UInt16> charCodes = { 'g', 'i', 'u', 'c', '5' };

  useModernToneMethod();

  vector<UInt32> expectedOut =  {
                                  KEY_DELETE, KEY_DELETE,
                                  getVNCharAt(KEY_U, 11), KEY_C
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule2_Input_quyet65) {
  vector<UInt16> charCodes = { 'q', 'u', 'y', 'e', 't', '6', '5' };

  useModernToneMethod();

  vector<UInt32> expectedOut =  {
                                  KEY_DELETE, KEY_DELETE, KEY_DELETE,
                                  KEY_Y,
                                  getVNCharAt(KEY_E | MASK_ROOF, 1), KEY_T
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule2_Input_cuu72) {
  vector<UInt16> charCodes = { 'c', 'u', 'u', '7', '2' };

  useModernToneMethod();

  vector<UInt32> expectedOut =  { // cuu7
                                  KEY_DELETE, KEY_DELETE,
                                  getVNCharAt(KEY_U | MASK_HOOK, 1),
                                  getVNCharAt(KEY_U, 1),
                                  KEY_DELETE, KEY_DELETE,
                                  getVNCharAt(KEY_U | MASK_HOOK, 5),
                                  getVNCharAt(KEY_U, 1),
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule3_Input_choang2) {
  vector<UInt16> charCodes = { 'c', 'h', 'o', 'a', 'n', 'g', '2'};

  useModernToneMethod();

  vector<UInt32> expectedOut =  { 
                                  KEY_DELETE, KEY_DELETE, KEY_DELETE,
                                  getVNCharAt(KEY_A , 5),
                                  KEY_N, KEY_G
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule4_Input_thuy3) {
  vector<UInt16> charCodes = { 't', 'h', 'u', 'y', '3'};

  useModernToneMethod();

  vector<UInt32> expectedOut =  { 
                                  KEY_DELETE,
                                  getVNCharAt(KEY_Y, 7),
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule4_Input_hoa2) {
  vector<UInt16> charCodes = { 'h', 'o', 'a', '2'};

  useModernToneMethod();

  vector<UInt32> expectedOut =  { 
                                  KEY_DELETE,
                                  getVNCharAt(KEY_A, 5),
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}


TEST_F(KbEngineTest, TestModernTone_Rule5_Input_ngoeo3) {
  vector<UInt16> charCodes = { 'n', 'g', 'o', 'e', 'o', '3'};

  useModernToneMethod();

  vector<UInt32> expectedOut =  { 
                                  KEY_DELETE, KEY_DELETE,
                                  getVNCharAt(KEY_E, 7),
                                  getVNCharAt(KEY_O, 1)
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

TEST_F(KbEngineTest, TestModernTone_Rule5_Input_giui1) {
  vector<UInt16> charCodes = { 'g', 'i', 'u', 'i', '1' };

  useModernToneMethod();

  vector<UInt32> expectedOut =  { 
                                  KEY_DELETE, KEY_DELETE,
                                  getVNCharAt(KEY_U, 3),
                                  getVNCharAt(KEY_I, 1)
                                };

  engine.resetBuffer();
  
  vector<UInt32> output = sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}