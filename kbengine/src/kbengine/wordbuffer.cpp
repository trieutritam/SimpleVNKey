#include "macrologger.h"
#include "wordbuffer.h"
#include "utils.h"

KeyStrokeBuffer::KeyStrokeBuffer()
{
    _maxSize = 5;    // max 5 wordsa
}

KeyStrokeBuffer::~KeyStrokeBuffer()
{
}

UInt16 KeyStrokeBuffer::getLastWordSize()
{
    auto word = _buffer.top();
    return word.size();
}

void KeyStrokeBuffer::resetBuffer()
{
    while(!_buffer.empty()) {
        _buffer.pop();
    }
}

void KeyStrokeBuffer::addKeyStroke(UInt8 keycode, bool shiftCap)
{
    LOG_DEBUG("AddKey, Buffer size = %lu", _buffer.size());
    
    if (_buffer.size() == 0)
        startNewWord();
    
    auto word = _buffer.top();
    BufferEntry e = { keycode };
    e.cap = (shiftCap > 0);
    word.push_back(e);
    
    LOG_DEBUG("AddKey, Word size = %lu", word.size());
}

void KeyStrokeBuffer::startNewWord()
{
    vector<BufferEntry> word;
    word.reserve(MAX_BUFF);

    _buffer.push(word);
}

void KeyStrokeBuffer::deleteLastKey()
{
    if (_buffer.size() == 0)
        return;
    
    auto word = _buffer.top();
    
    while(word.size() == 0) {
        _buffer.pop();
        
        // there is nothing in buffer, return.
        if (_buffer.size() == 0)
            return;
        
        word = _buffer.top();
        LOG_DEBUG("Word is empty, delete char of previous word, size = %lu", word.size());
    }
    
    word.pop_back();
    
    LOG_DEBUG("DeleteKey, Word size = %lu", word.size());
}
