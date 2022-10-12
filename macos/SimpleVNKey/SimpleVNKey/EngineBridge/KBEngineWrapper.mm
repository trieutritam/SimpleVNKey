//
//  NSObject+KBEngine_h.m
//  Test
//
//  Created by tamtt5 on 24/09/2022.
//
//#import <Foundation/Foundation.h>
#include <string>

#import "KBEngineWrapper.h"
#include "kbengine.h"


static kbengine engine;

@implementation KBEngineWrapper
- (NSArray*)process: (UInt16)charCode
                      keycode: (UInt16)keycode
                      shiftOrCapPressed: (UInt8)shiftCap
                      otherControlPressed: (bool)otherControl
{
    engine.process(charCode, keycode, shiftCap, otherControl);
    std::vector<UInt32> output = engine.getOutputBuffer();


    NSMutableArray *res = [NSMutableArray new];

    for(auto it = output.begin(); it != output.end(); it ++) {
        [res addObject: [NSNumber numberWithInt: (*it)]];
    }

    return [NSArray arrayWithArray:res];
}

- (void)setInputMethod: (UInt8) inputMethod
{
    engine.setInputMethod(inputMethod);
}

- (UInt8)getInputMethod
{
    return engine.getInputMethod();
}

- (void)resetBuffer
{
    engine.resetBuffer();
}

- (void)setActiveCodeTable: (UInt8) codeTableNumber
{
    engine.setActiveCodeTable(codeTableNumber);
}


- (UInt8)getCurrentCodeTable
{
    return engine.getCurrentCodeTable();
}

- (UInt8)getTotalCodeTable
{
    return engine.getTotalCodeTable();
}

- (void)addCharacterEncoding: (NSDictionary*)characterEncoding charType: (UInt8) charType
{
    std::map<std::string, vector<UInt16>> codeTable;
    
    for(NSString *key in [characterEncoding allKeys]) {
        NSArray *values = [characterEncoding valueForKey:key];
        vector<UInt16> arr;

        std::vector<UInt16> vectorList;
        vectorList.reserve([values count]);
        
        for (id value in values) {
            vectorList.push_back([value unsignedShortValue]);
        }
        
        std::string stdKey = std::string([key UTF8String]);
        codeTable.insert(std::pair<std::string, std::vector<UInt16>>(stdKey, vectorList));
    }
    
    engine.addCodeTable(charType, codeTable);
}

- (void)setUseModernToneMethod: (bool) isUse
{
    engine.setUseModernTone(isUse);
}

- (bool)isUseModernToneMethod
{
    return engine.isUseModernTone();
}
@end
