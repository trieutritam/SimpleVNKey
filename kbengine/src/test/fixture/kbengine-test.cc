#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include <kbengine/kbengine.h>
#include <kbengine/platforms/macos.h>
#include <kbengine/vnlexi.h>

// this is unicode mask used by engine
#define UNICODE_MASK 0x00010000

class KbEngineTest: public ::testing::Test {
  protected:
    void SetUp() override {
      charList = codeTable[0];  // Unicode encoding
    }

    UInt32 getVNCharAt(UInt16 charCodeType, int index) {
      return UInt32(charList[charCodeType].at(index) | UNICODE_MASK);
    }

    kbengine engine;
    map<UInt32, vector<UInt16>> charList;
};