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
#include "vnlexi.h"

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
    WordBuffer()
    {
        _maxSize = MAX_BUFF;
        _buffer.reserve(_maxSize);
        _beginWordIdx = 0;
    }
    
    void addEntry(T entry);
    void resetBuffer();
    void removeLastEntry();
    T    getLastEntry();
    int  getWordSize();

    
    void printBuffer();
};


template <class T>
void WordBuffer<T>::addEntry(T entry)
{
    if (_buffer.size() == _maxSize) {
        // need to remove all word except the last one
        UInt16 wordSize = _buffer.size() - _beginWordIdx;
        
        if (wordSize < _maxSize) {
            for(int i = _beginWordIdx; i < _buffer.size(); i++) {
                int buffIndex = i - _beginWordIdx;
                _buffer[buffIndex] = _buffer[i];
            }
        }
        else {
            _buffer.clear();
            _buffer.reserve(_maxSize);
        }
        
        _beginWordIdx = 0;
        
        LOG_DEBUG("Moved current: %hu, size: %lu", _beginWordIdx, _buffer.size());
    }
    
    this->_buffer.push_back(entry);
    
    LOG_DEBUG("Word Buffer size: %lu", _buffer.size());

    // Check if we start new word
    if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), entry.keyCode) != _wordBreakCode.end())
    {
        _beginWordIdx = _buffer.size();
    }
}

template <class T>
void WordBuffer<T>::resetBuffer()
{
    _buffer.clear();
    _buffer.reserve(_maxSize);
    _beginWordIdx = 0;
}

template <class T>
void WordBuffer<T>::removeLastEntry()
{
    if (_buffer.size() == 0)
        return;
    
    _buffer.pop_back();
    
    // Reupdate start position of word
    if (_buffer.size() <= _beginWordIdx) {
        _beginWordIdx = 0;

        // Find the start position of the word
        for (unsigned long i = _buffer.size() - 1; i > 0; i--) {
            if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), _buffer[i].keyCode) != _wordBreakCode.end()) {
                _beginWordIdx = i + 1;
                break;
            }
        }
    }
}

template <class T>
T WordBuffer<T>::getLastEntry()
{
    return _buffer[_buffer.size() - 1];
}

template <class T>
int  WordBuffer<T>::getWordSize()
{
    return _buffer.size() - _beginWordIdx;
}

template <class T>
void WordBuffer<T>::printBuffer()
{
    LOG_DEBUG("Print Buffer, beginIndex: %hu ", _beginWordIdx);
    for (unsigned long i = _beginWordIdx; i < _buffer.size(); i ++) {
        LOG_DEBUG("%2X ", _buffer[i].keyCode);
    }
}

#endif /* KBENGINE_WORDBUFFER_H_ */
