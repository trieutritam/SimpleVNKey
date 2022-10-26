/*
 * kbengine.h
 *
 *  Created on: Sep 23, 2022
 *      Author: tamtt5
 */

#ifndef KBENGINE_KBENGINE_H_
#define KBENGINE_KBENGINE_H_

#include <vector>
#include <map>
using namespace std;

#include "datatypes.h"
#include "wordbuffer.h"

#define MAX_BUFF 32

#define UNICODE_MASK 0x00010000


struct ModifiedKeyInfo {
    UInt16 keyCode;
    bool isUniChar;
};

struct ProcessResult {
    int startPosition = -1;
    bool processed = false;
    bool ignoreKeyCode = false;
    int adjustDelete = 0;
};

class kbengine {
    UInt8 currentCodeTable = 0; // 0 - unicode
    
    short _currentInputMethod =  -1;   //0 VNI, 1 Simple Telex, 2 Telex
    short _useModernTone = false;
        
	BufferEntry _buffer[MAX_BUFF];
    int _bufferStartWordIdx;    // index of begin of the last word in buffer
	int _bufferSize;

    // output after processing
    vector<UInt32> _keyCodeOutput;
    
    int _calculateNumberOfBackSpace(int startIdx, int endIdx);
    
    int _processMark(const UInt8 &keycode, const RoofType &roofType, const bool &fromCorrectFunc = false);
    int _processD(const UInt8 &keycode);
    int _placeToneTraditionalRule(int foundIdx, vector<UInt16> syllableCombine);
    int _placeToneModernRule(int foundIdx, vector<UInt16> syllableCombine);
    int _processTone(const UInt8 &keycode, const KeyEvent &tone, const bool &fromCorrectFunc = false);
    int _processHookOU(const UInt8 &keycode, const UInt16 &expectedKey);
    void _processBackSpacePressed();

    int _correctTone(const UInt8 &keycode);
    int _correctMark(const UInt8 &keycode);
    
    void _processKeyCodeOutput(int numDelete, int startPos);
    
    void _startNewWord();
    void _addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap, const bool processed = false);
    
    int _findSyllable(vector<UInt16> &syllableCombine, const UInt16 &expectedType, const UInt16 &expectedKey = KEY_EMPTY);
    
    UInt32 _getCharacterCode(const BufferEntry& entry );
    UInt8 _getCurrentCodeTableCharType();
    
    vector<BufferEntry*> extractWord();
    
    //V2
    ProcessResult _processWord(vector<BufferEntry*> word, const UInt16 &keycode, const UInt8 &shiftCap, const bool &otherControl);
    ProcessResult _processMarkV2(const vector<BufferEntry*> &word,
                       const UInt16 &keycode,
                       const RoofType &roofType, const bool &fromCorrectFunc = false);
    
    int _findSyllableV2(const vector<BufferEntry*> &word,
                        vector<UInt16> &syllableCombine,
                        const UInt16 &expectedType, const UInt16 &expectedKey = KEY_EMPTY);
    
    int _calculateNumberOfBackSpaceV2(const vector<BufferEntry*> &word, int startIdx);
    void _processKeyCodeOutputV2(const vector<BufferEntry*> &word, int numDelete, int startPos);
    void _addKeyCodeV2(const UInt16 &keycode, const UInt8 &shiftCap, const bool processed = false);
    BufferEntry* _processBackSpacePressedV2();
    void _processResult(vector<BufferEntry*> word, ProcessResult result, int adjustDelete);
    ProcessResult _processDV2(const vector<BufferEntry*> &word, const UInt8 &keycode);
    ProcessResult _processToneV2(const vector<BufferEntry*> &word,
                                 const UInt8 &keycode,
                                 const KeyEvent &tone,
                                 const bool &fromCorrectFunc = false);
    int _placeToneTraditionalRuleV2(const vector<BufferEntry*> &word, int foundIdx, vector<UInt16> syllableCombine);
    int _placeToneModernRuleV2(const vector<BufferEntry*> &word, int foundIdx, vector<UInt16> syllableCombine);
    ProcessResult _processHookOUV2(const vector<BufferEntry*> word, const UInt8 &keycode, const UInt8 &shiftCap, const UInt16 &expectedKey);

public:
	kbengine();
	virtual ~kbengine();

	void resetBuffer();
    vector<UInt32> getOutputBuffer();
    
    void setUseModernTone(bool isUse);
    bool isUseModernTone();
    
    /* Main Process, after process keycode
     * consumer need call getOutputBuffer() to check whether keycode processed and send keycodes to target app
     */
	int process(const UInt16 &charCode, const UInt16 &keycode, const UInt8 &shiftCap, const bool &otherControl);
    
    void setInputMethod(const UInt8 &inputMethod);
    UInt8 getInputMethod();

	// TODO: test only
	BufferEntry* getBuffer();
    
    UInt8 getCurrentCodeTable();
    void setActiveCodeTable(const UInt8 &codeTableNumber);
    UInt8 getTotalCodeTable();
    void addCodeTable(const unsigned short &charType, const map<std::string, vector<UInt16>> &codeTableRaw);
};

#endif /* KBENGINE_KBENGINE_H_ */
