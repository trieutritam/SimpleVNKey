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

#define MAX_BUFF 32

#define UNICODE_MASK 0x00010000

enum RoofType {
    ORIGIN,
    ROOF, ROOF_A, ROOF_E, ROOF_O,
    HOOK,
    BREVE
};

enum KeyEvent {
  RoofAll, RoofA, RoofE, RoofO,
  HookAll, Breve, HookO, HookU, // Breve is top of ă
  Dd,
  Tone0, Tone1, Tone2, Tone3, Tone4, Tone5,
  EscChar,
  Normal, //does not belong to any of the above categories
  KeyEventCount //just to count how many event types there are
};

struct BufferEntry {
    UInt8 keyCode;
    bool cap;    // capitalize or not
    RoofType roofType = ORIGIN;
    short tone = KeyEvent::Tone0;    // KeyEvent enum tone: 0 = none, 1 = sac, 2 = huyen, 3 = hoi, 4 = nga, 5 = nang
};

struct ModifiedKeyInfo {
    UInt16 keyCode;
    bool isUniChar;
};

class kbengine {
    UInt8 currentCodeTable = 0; // 0 - unicode
    
    short _currentInputMethod =  -1;   //0 VNI, 1 Simple Telex, 2 Telex
        
	BufferEntry _buffer[MAX_BUFF];
    int _bufferStartWordIdx;    // index of begin of the last word in buffer
	int _bufferSize;
    
    // output after processing
    vector<UInt32> _keyCodeOutput;

    int _processMark(const UInt8 &keycode, const RoofType &roofType, const bool &fromCorrectFunc = false);
    int _processD(const UInt8 &keycode);
    int _placeToneTraditionalRule(int foundIdx, vector<UInt16> syllableCombine);
    int _processToneTraditional(const UInt8 &keycode, const KeyEvent &tone, const bool &fromCorrectFunc = false);
    int _processHookOU(const UInt8 &keycode, const UInt16 &expectedKey);
    
    int _correctTone(const UInt8 &keycode);
    int _correctMark(const UInt8 &keycode);
    
    void _processKeyCodeOutput(int numDelete, int startPos, int endPos);
    int _calculateNumberOfBackSpace(int startIdx, int endIdx);
    void _processBackSpacePressed();
    void _startNewWord();
    void _addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap);
    
    int _findSyllable(vector<UInt16> &syllableCombine, const UInt16 &expectedType, const UInt16 &expectedKey = KEY_EMPTY);
    
    UInt32 _getCharacterCode(const BufferEntry& entry );
    UInt8 _getCurrentCodeTableCharType();
public:
	kbengine();
	virtual ~kbengine();

	void resetBuffer();
    vector<UInt32> getOutputBuffer();
    
    // Main Process
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
