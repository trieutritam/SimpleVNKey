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

#define MAX_BUFF 64

#define UNICODE_MASK 0x00010000

struct BufferEntry {
    UInt16 keyCode;
    bool cap;    // capitalize or not
    RoofType roofType = ORIGIN;
    short tone = KeyEvent::Tone0;    // KeyEvent enum tone: 0 = none, 1 = sac, 2 = huyen, 3 = hoi, 4 = nga, 5 = nang
    bool processed = false;
    bool isIMCode = false;
    bool isBreakCode = false;
};

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
    bool  isEnableAutoRestoreWord = false;  // auto restore keystroke for invalid VN word
        
	BufferEntry _buffer[MAX_BUFF];
    int _bufferStartWordIdx;    // index of begin of the last word in buffer
	int _bufferSize;

    // output after processing
    vector<UInt32> _keyCodeOutput;
    
    UInt32 _getCharacterCode(const BufferEntry& entry );
    UInt8 _getCurrentCodeTableCharType();
    
    vector<BufferEntry*> getCurrentWord();
    vector<BufferEntry>  extractCurrentWord();

    
    //V2
    void _startNewWord();
    ProcessResult _processWord(vector<BufferEntry*> word, const UInt16 &keycode, const UInt8 &shiftCap, const bool &otherControl);
    ProcessResult _processMarkV2(const vector<BufferEntry*> &word,
                       const UInt16 &keycode,
                       const RoofType &roofType, const bool &fromCorrectFunc = false);
    
    int _findSyllableV2(const vector<BufferEntry*> &word,
                        vector<UInt16> &syllableCombine,
                        const UInt16 &expectedType, const UInt16 &expectedKey = KEY_EMPTY);
    
    int _calculateNumberOfBackSpaceV2(const vector<BufferEntry*> &word, int startIdx);
    void _processKeyCodeOutputV2(const vector<BufferEntry*> &word, int numDelete, int startPos);
    void _addKeyCodeV2(const UInt16 &keycode, const UInt8 &shiftCap,
                       const bool &processed = false,
                       const bool &isBreakCode = false);
    BufferEntry* _processBackSpacePressedV2();
    
    void _processResult(vector<BufferEntry*> pOldWord, vector<BufferEntry*> word,
                        ProcessResult result, int adjustDelete);
    
    ProcessResult _processDV2(const vector<BufferEntry*> &word, const UInt8 &keycode);
    ProcessResult _processToneV2(const vector<BufferEntry*> &word,
                                 const UInt8 &keycode,
                                 const KeyEvent &tone,
                                 const bool &fromCorrectFunc = false);
    int _placeToneTraditionalRuleV2(const vector<BufferEntry*> &word, int foundIdx, vector<UInt16> syllableCombine);
    int _placeToneModernRuleV2(const vector<BufferEntry*> &word, int foundIdx, vector<UInt16> syllableCombine);
    ProcessResult _processHookOUV2(const vector<BufferEntry*> word, const UInt8 &keycode, const UInt8 &shiftCap, const UInt16 &expectedKey);
    
    ProcessResult _correctUO(const vector<BufferEntry*> &word, const UInt8 &keycode);
    ProcessResult _correctToneV2(const vector<BufferEntry*> &word, const UInt8 &keycode);
    
    BufferEntry* processDelete1(const vector<BufferEntry*> &pOrigWord);
    void processDelete2(const vector<BufferEntry*> &pOrigWord);
    void _restoreWordIfAny();
public:
	kbengine();
	virtual ~kbengine();

	void resetBuffer();
    vector<UInt32> getOutputBuffer();
    
    void setUseModernTone(bool isUse);
    bool isUseModernTone();
    void setAutoRestoreWord(bool isEnable);
    
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
