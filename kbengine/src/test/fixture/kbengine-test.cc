#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include <kbengine/kbengine.h>
#include <kbengine/platforms/macos.h>
#include <kbengine/vnlexi.h>

#include "keycode-map.h"

// this is unicode mask used by engine
#define UNICODE_MASK 0x00010000

class KbEngineTest: public ::testing::Test {
  protected:
    kbengine engine;
    map<UInt32, vector<UInt16>> charList;

    void SetUp() override {
      charList = codeTable[0];  // Unicode encoding
    }

    UInt32 getVNCharAt(UInt16 charCodeType, int index) {
      return UInt32(charList[charCodeType].at(index) | UNICODE_MASK);
    }

    vector<UInt32> sendKeyStrokes(const vector<UInt16> &charCodes) {
      vector<UInt32> output;
      for(int i=0; i < charCodes.size(); i++) {
        char temp = toupper(charCodes[i]);
        engine.process(charCodes[i], charToKeyCode[temp], 0, false);
        vector<UInt32> keystrokes = engine.getOutputBuffer();

        for(auto it = keystrokes.begin(); it != keystrokes.end(); it++) {
          output.push_back(*it);
        }
      }
      return output;
    }
};