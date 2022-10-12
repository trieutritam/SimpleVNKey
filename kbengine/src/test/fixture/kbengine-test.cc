#include "kbengine-test.h"
#include "keycode-map.h"

KbEngineTest::KbEngineTest() {
  map<std::string, vector<UInt16>> vniChars = {
      { "a"   ,  {0x0041, 0x0061, 0xd941, 0xf961, 0xd841, 0xf861, 0xdb41, 0xfb61, 0xd541, 0xf561, 0xcf41, 0xef6 }},
      { "a1"  ,  {0xc241, 0xe261, 0xc141, 0xe161, 0xc041, 0xe061, 0xc541, 0xe561, 0xc341, 0xe361, 0xc441, 0xe46 }},
      //            Ă       ă       Ắ       ắ       Ằ       ằ       Ẳ       ẳ       Ẵ       ẵ       Ặ       ặ
      { "a2"  ,  {0xca41, 0xea61, 0xc941, 0xe961, 0xc841, 0xe861, 0xda41, 0xfa61, 0xdc41, 0xfc61, 0xcb41, 0xeb6 }},
      { "e"   ,  {0x0045, 0x0065, 0xd945, 0xf965, 0xd845, 0xf865, 0xdb45, 0xfb65, 0xd545, 0xf565, 0xcf45, 0xef6 }},
      { "e1"  ,  {0xc245, 0xe265, 0xc145, 0xe165, 0xc045, 0xe065, 0xc545, 0xe565, 0xc345, 0xe365, 0xc445, 0xe46 }},
      { "i"   ,  {0x0049, 0x0069, 0x00cd, 0x00ed, 0x00cc, 0x00ec, 0x00c6, 0x00e6, 0x00d3, 0x00f3, 0x00d2, 0x00f }},
      { "y"   ,  {0x0059, 0x0079, 0xd959, 0xf979, 0xd859, 0xf879, 0xdb59, 0xfb79, 0xd559, 0xf579, 0x00ce, 0x00e }},
      { "o"   ,  {0x004f, 0x006f, 0xd94f, 0xf96f, 0xd84f, 0xf86f, 0xdb4f, 0xfb6f, 0xd54f, 0xf56f, 0xcf4f, 0xef6 }},
      { "o1"  ,  {0xc24f, 0xe26f, 0xc14f, 0xe16f, 0xc04f, 0xe06f, 0xc54f, 0xe56f, 0xc34f, 0xe36f, 0xc44f, 0xe46 }},
      { "o2"  ,  {0x00d4, 0x00f4, 0xd9d4, 0xf9f4, 0xd8d4, 0xf8f4, 0xdbd4, 0xfbf4, 0xd5d4, 0xf5f4, 0xcfd4, 0xeff }},
      { "u"   ,  {0x0055, 0x0075, 0xd955, 0xf975, 0xd855, 0xf875, 0xdb55, 0xfb75, 0xd555, 0xf575, 0xcf55, 0xef7 }},
      { "u2"  ,  {0x00d6, 0x00f6, 0xd9d6, 0xf9f6, 0xd8d6, 0xf8f6, 0xdbd6, 0xfbf6, 0xd5d6, 0xf5f6, 0xcfd6, 0xeff }},
      { "d"   ,  {0x0044, 0x0064 }},
      { "d1"  ,  {0x00d1, 0x00f1 }}
  };
  engine.addCodeTable(2, vniChars);
}

void KbEngineTest::SetUp() {
  charList = codeTableList.at(0);  // Unicode encoding

  engine.setActiveCodeTable(0);
  engine.setUseModernTone(false);
}

void KbEngineTest::activateVNIWindowsMethod()
{
  engine.setActiveCodeTable(1);
  charList = codeTableList.at(1);
}

void KbEngineTest::useModernToneMethod()
{
  engine.setUseModernTone(true);
}

UInt32 KbEngineTest::getVNCharAt(UInt16 charCodeType, int index) {
  return UInt32(charList[charCodeType].at(index) | UNICODE_MASK);
}

UInt32 KbEngineTest::getLowChar(UInt16 charCodeType, int index) {
  return UInt32(BYTE_LOW(charList[charCodeType].at(index))| UNICODE_MASK);
}

UInt32 KbEngineTest::getHighChar(UInt16 charCodeType, int index) {
  return UInt32(BYTE_HIGH(charList[charCodeType].at(index))| UNICODE_MASK);
}

vector<UInt32> KbEngineTest::sendCharCodes(const vector<UInt16> &charCodes) {
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

vector<UInt32> KbEngineTest::sendKeyStrokes(const vector<UInt16> &keyCodes) {
  vector<UInt32> output;
  for(int i=0; i < keyCodes.size(); i++) {
    engine.process(0, keyCodes[i], 0, false);
    vector<UInt32> keystrokes = engine.getOutputBuffer();

    for(auto it = keystrokes.begin(); it != keystrokes.end(); it++) {
      output.push_back(*it);
    }
  }
  return output;
}
