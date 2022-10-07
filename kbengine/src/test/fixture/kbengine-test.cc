#include "kbengine-test.h"
#include "keycode-map.h"

void KbEngineTest::SetUp() {
  charList = codeTableList.at(0);  // Unicode encoding
}

UInt32 KbEngineTest::getVNCharAt(UInt16 charCodeType, int index) {
  return UInt32(charList[charCodeType].at(index) | UNICODE_MASK);
}

vector<UInt32> KbEngineTest::sendKeyStrokes(const vector<UInt16> &charCodes) {
  vector<UInt32> output;
  for(int i=0; i < charCodes.size(); i++) {
    char temp = toupper(charCodes[i]);
    engine.process(charCodes[i], UpperCharToKeyCode[temp], 0, false);
    vector<UInt32> keystrokes = engine.getOutputBuffer();

    for(auto it = keystrokes.begin(); it != keystrokes.end(); it++) {
      output.push_back(*it);
    }
  }
  return output;
}
