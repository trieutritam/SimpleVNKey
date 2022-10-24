/*
 * entrybuffer.h
 *
 *  Created on: Oct 23, 2022
 *      Author: tamtt5
 */
#ifndef KBENGINE_ENTRYBUFFER_H_
#define KBENGINE_ENTRYBUFFER_H_

#include "wordbuffer.h"


class EntryBuffer
{
    WordBuffer<BufferEntry> _processedBuffer;
    WordBuffer<KeyStrokeEntry> _keystrokeBuffer;

public:
    void addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap);
    void resetBuffer();
};


#endif /* KBENGINE_ENTRYBUFFER_H_ */
