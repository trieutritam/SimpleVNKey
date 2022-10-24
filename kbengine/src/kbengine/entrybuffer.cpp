#include "macrologger.h"

#include "entrybuffer.h"

void EntryBuffer::addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap)
{
    BufferEntry e = { keycode };
    e.cap = shiftCap > 0 ? true : false;

    _processedBuffer.addEntry(e);
    
    KeyStrokeEntry k = { keycode };
    k.cap = shiftCap > 0 ? true : false;
    this->_keystrokeBuffer.addEntry(k);
}


void EntryBuffer::resetBuffer()
{
    this->_keystrokeBuffer.resetBuffer();
    this->_processedBuffer.resetBuffer();
}
