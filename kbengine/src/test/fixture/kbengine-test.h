#ifndef TEST_KBENGINE_TEST_H_
#define TEST_KBENGINE_TEST_H_

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
    kbengine engine;
    map<UInt32, vector<UInt16>> charList;

    void SetUp() override;
    UInt32 getVNCharAt(UInt16 charCodeType, int index);
    vector<UInt32> sendKeyStrokes(const vector<UInt16> &charCodes);
};

#endif