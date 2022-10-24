#include "macrologger.h"

#include "wordbuffer.h"


static vector<UInt8> _wordBreakCode = {
    KEY_BACKQUOTE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS,
    KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACK_SLASH,
    KEY_SEMICOLON, KEY_QUOTE, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_SPACE
};

template <class T> WordBuffer<T>::WordBuffer()
{
    _maxSize = MAX_BUFF;
    _buffer.reserve(_maxSize);
}

template <class T> void WordBuffer<T>::addEntry(T entry)
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
    
    // Check if we start new word
    if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), entry.keyCode) != _wordBreakCode.end())
    {
        _beginWordIdx = _buffer.size();
    }
}

template <class T> void WordBuffer<T>::resetBuffer()
{
    _buffer.clear();
    _buffer.reserve(_maxSize);
    _beginWordIdx = 0;
}
