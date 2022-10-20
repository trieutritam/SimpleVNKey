#include <gtest/gtest.h>
#include "fixture/kbengine-test.h"

TEST_F(KbEngineTest, TestVNIWindows_Input_viet65) {
  vector<UInt16> charCodes = { 'v', 'i', 'e', 't', '6', '5' };

  activateVNIWindowsMethod();
  // expect 2 delete keystore, unicode-ê, KEY_T
  vector<UInt32> expectedOut = {
                                 // receive key 6
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE), UInt32(KEY_DELETE),      // delete 3 chars
                                 UInt32(KEY_I), 
                                 getLowChar(KEY_E | MASK_ROOF, 1), getHighChar(KEY_E | MASK_ROOF, 1),   
                                 UInt32(KEY_T), // add iêt
                                 // receive key 5
                                 // delete 3 chars (include circumflex)
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE), UInt32(KEY_DELETE),
                                 getLowChar(KEY_E | MASK_ROOF, 11), getHighChar(KEY_E | MASK_ROOF, 11), 
                                 UInt32(KEY_T),               // add ệt
                                };

  engine.resetBuffer();
  
  vector<UInt32> output =sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

// Test uo7 --> uơ
TEST_F(KbEngineTest,TestVNIWindows_Input_uo7) {
  vector<UInt16> charCodes = { 'u', 'o', '7' };

  activateVNIWindowsMethod();
  vector<UInt32> expectedOut = {
                                // receive key 7
                                 UInt32(KEY_DELETE),      // delete 1 chars
                                 getVNCharAt(KEY_O | MASK_HOOK, 1), // add ơ
                                };

  engine.resetBuffer();
  
  vector<UInt32> output =sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

// Test uo7n --> ươn
TEST_F(KbEngineTest,TestVNIWindows_Input_uo7n) {
  vector<UInt16> charCodes = { 'u', 'o', '7', 'n' };

  activateVNIWindowsMethod();

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
  
  vector<UInt32> output =sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}


// Test uon7 --> ươn
TEST_F(KbEngineTest,TestVNIWindows_Input_uon7) {
  vector<UInt16> charCodes = { 'u', 'o', 'n', '7' };

  activateVNIWindowsMethod();
  vector<UInt32> expectedOut = {
                                 // receive 'n'
                                 UInt32(KEY_DELETE), UInt32(KEY_DELETE), UInt32(KEY_DELETE), // delete 3 chars
                                 getVNCharAt(KEY_U | MASK_HOOK, 1),   // add ư
                                 getVNCharAt(KEY_O| MASK_HOOK, 1),    // add ơ
                                 UInt32(KEY_N)                        // add n
                                };

  engine.resetBuffer();
  
  vector<UInt32> output =sendCharCodes(charCodes);
  
  EXPECT_EQ(output, expectedOut);
}

// Test tướng , send DELETE -> output tướn
TEST_F(KbEngineTest,TestVNIWindows_DELETE_With_Input_tuong71) {
  vector<UInt16> charCodes = { 't', 'u', 'o', 'n', 'g', '7', '1' };

  activateVNIWindowsMethod();


  vector<UInt32> expectedOut = {}; // this case there is no output since we delete normal char

  engine.resetBuffer();
  
  // send key first
 sendCharCodes(charCodes);

  // send DELETE
  vector<UInt32> output = sendKeyStrokes({ KEY_DELETE });
  
  EXPECT_EQ(output, expectedOut);
}

// Test tướn , send 2xDELETE -> output tươ
TEST_F(KbEngineTest,TestVNIWindows_With_Input_tuon71_2xDELETE) {
  vector<UInt16> charCodes = { 't', 'u', 'o', 'n', '7', '1' };

  activateVNIWindowsMethod();

  vector<UInt32> expectedOut = {
    KEY_DELETE, KEY_DELETE,
    // getVNCharAt(KEY_O| MASK_HOOK, 1),    // add ơ
  };

  engine.resetBuffer();
  
  // send tướn
 sendCharCodes(charCodes);

  // delete 'n'
 auto out1 = sendKeyStrokes({ KEY_DELETE });
 vector<UInt32> expOut1 = {};
 EXPECT_EQ(out1, expOut1);


  // second delete -> remove tone mark
  vector<UInt32> output = sendKeyStrokes({ KEY_DELETE });
  
  EXPECT_EQ(output, expectedOut);
}

// Test tướn , send 3xDELETE -> output tưo
TEST_F(KbEngineTest,TestVNIWindows_With_Input_tuon71_3xDELETE) {
  vector<UInt16> charCodes = { 't', 'u', 'o', 'n', '7', '1' };

  activateVNIWindowsMethod();

  vector<UInt32> expectedOut = {
    // KEY_DELETE,
    // getVNCharAt(KEY_O, 1),    // add o
  };

  engine.resetBuffer();
  
  // send tướn
  sendCharCodes(charCodes);

  // delete 'n', tone mark
  sendKeyStrokes({ KEY_DELETE, KEY_DELETE });

  // last delete -> remove hook's o
  vector<UInt32> output = sendKeyStrokes({ KEY_DELETE });
  
  EXPECT_EQ(output, expectedOut);
}