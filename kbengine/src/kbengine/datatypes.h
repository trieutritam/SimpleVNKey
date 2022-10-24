/*
 * DataTypes.h
 *
 *  Created on: Sep 24, 2022
 *      Author: tamtt5
 */



#ifndef KBENGINE_DATATYPES_H_
#define KBENGINE_DATATYPES_H_

//
//#ifdef LINUX
//#include "platforms/linux.h"
//#elif _WIN32
//#include "platforms/win32.h"
//#else
#include "platforms/macos.h"
//#endif



typedef unsigned char Byte;
typedef signed char Int8;
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;
//typedef unsigned long int UInt64;


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


#endif /* KBENGINE_DATATYPES_H_ */
