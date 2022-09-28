//
//  NSObject+KBEngine_h.m
//  Test
//
//  Created by tamtt5 on 24/09/2022.
//
//#import <Foundation/Foundation.h>
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
    
    NSMutableArray *res = [[NSMutableArray alloc]init];
    
    for(auto it = output.begin(); it != output.end(); it ++) {
//        NSLog(@"%i ", *it);
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
@end
