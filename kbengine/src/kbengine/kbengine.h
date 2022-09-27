/*
 * kbengine.h
 *
 *  Created on: Sep 23, 2022
 *      Author: tamtt5
 */

#ifndef KBENGINE_KBENGINE_H_
#define KBENGINE_KBENGINE_H_

#include <vector>
using namespace std;

#include "datatypes.h"

#define MAX_BUFF 10

enum RoofType {
    ORIGIN,
    ROOF, ROOF_A, ROOF_E, ROOF_O,
    HOOK        // in the case 'a' this is indicate ă
};

enum KeyEvent {
  RoofAll, RoofA, RoofE, RoofO,
  HookAll, Bowl, HookO, HookU,
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
    short currentCodeTable = 0; // 0 - unicode
    
    short _currentInputMethod =  -1;   //0 VNI, 1 Simple Telex, 2 Telex
        
	BufferEntry _buffer[MAX_BUFF];
    int _bufferStartWordIdx;    // index of begin of the last word in buffer
	int _bufferSize;
    
    // output after processing
    vector<UInt32> _keyCodeOutput;

    int _processMark(const UInt8 &keycode, const RoofType &roofType);
    int _processD(const UInt8 &keycode);
    int _processToneTraditional(const UInt8 &keycode, const KeyEvent &tone, const bool &fromSpelling);
    
    int _correctSpelling(const UInt8 &keycode);
    
    void _processKeyCodeOutput(int numDelete, int startPos, int endPos);
    void _processBackSpacePressed();
    void _startNewWord();
    void _addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap);
    
    int _findSyllable(vector<UInt16> &syllableCombine, const UInt16 &expectedType, const UInt16 &expectedKey);
public:
	kbengine();
	virtual ~kbengine();

	void resetBuffer();
	int process(const UInt16 &charCode, const UInt8 &keycode, const UInt8 &shiftCap, const bool &otherControl);
    vector<UInt32> getOutputBuffer();
    
    void setInputMethod(const UInt8 &inputMethod);
    UInt8 getInputMethod();

	// TODO: test only
	BufferEntry* getBuffer();
};

#endif /* KBENGINE_KBENGINE_H_ */
