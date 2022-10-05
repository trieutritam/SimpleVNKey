/*
 * vnlexi.h
 *
 *  Created on: Sep 24, 2022
 *      Author: tamtt5
 */
#ifndef KBENGINE_VNLEXI_H_
#define KBENGINE_VNLEXI_H_

#include <map>
#include <vector>
#include <string>
using namespace std;

#include "datatypes.h"

#define MASK_ORIGIN   0x0100    // 00000001 00000000
#define MASK_ROOF     0x0200    // 00000010 00000000
#define MASK_HOOK     0x0400    // 00000100 00000000

#define MASK_EXTRA_MARK   0x0100


enum CodeTable {
    UNICODE
};

extern map<UInt8, vector<vector<UInt16>>> syllableTable;
extern vector<map<UInt32, vector<UInt16>>> codeTableList;

#endif /* KBENGINE_VNLEXI_H_ */
