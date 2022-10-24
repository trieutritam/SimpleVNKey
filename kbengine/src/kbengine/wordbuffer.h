/*
 * wordbuffer.h
 *
 *  Created on: Oct 23, 2022
 *      Author: tamtt5
 */

#ifndef KBENGINE_WORDBUFFER_H_
#define KBENGINE_WORDBUFFER_H_

#include <vector>
#include <string>
using namespace std;

#include "datatypes.h"


#define MAX_BUFF 32


struct BufferEntry {
    UInt8 keyCode;
    bool cap;    // capitalize or not
    RoofType roofType = ORIGIN;
    short tone = KeyEvent::Tone0;    // KeyEvent enum tone: 0 = none, 1 = sac, 2 = huyen, 3 = hoi, 4 = nga, 5 = nang
};

struct KeyStrokeEntry {
    UInt8 keyCode;
    bool cap;    // capitalize or not
};

template <class T> class WordBuffer
{
    vector<T> _buffer;
    
    UInt16 _beginWordIdx;
    UInt16 _maxSize;
    
public:
    WordBuffer();
    
    void addEntry(T entry);
    void resetBuffer();
};

#endif /* KBENGINE_WORDBUFFER_H_ */
