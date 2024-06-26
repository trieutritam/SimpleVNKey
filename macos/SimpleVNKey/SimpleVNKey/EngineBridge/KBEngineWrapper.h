//
//  NSObject+KBEngine_h.h
//  Test
//
//  Created by tamtt5 on 24/09/2022.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface KBEngineWrapper : NSObject
- (NSArray*)process: (UInt16)charCode
                keycode: (UInt16)keycode
                shiftOrCapPressed: (UInt8)shiftCap
                otherControlPressed: (bool)otherControl;

- (void)setInputMethod: (UInt8) inputMethod;
- (UInt8)getInputMethod;
- (void)resetBuffer;
- (void)setActiveCodeTable: (UInt8) codeTableNumber;
- (UInt8)getCurrentCodeTable;
- (UInt8)getTotalCodeTable;
- (void)addCharacterEncoding: (NSDictionary*)characterEncoding charType: (UInt8) charType;
- (void)setUseModernToneMethod: (bool) isUse;
- (bool)isUseModernToneMethod;
- (void)setAutoRestoreWord: (bool) isEnabled;
@end

NS_ASSUME_NONNULL_END
