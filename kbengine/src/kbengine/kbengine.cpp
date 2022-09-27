/*
 * kbengine.cpp
 *
 *  Created on: Sep 23, 2022
 *      Author: tamtt5
 */
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "macrologger.h"
#include "kbengine.h"
#include "vnlexi.h"
#include "utils.h"

#define ACTION_NOT_PROCESSED(ret) (ret < 0)
#define ACTION_PROCESSED(ret) (ret >= 0)
#define UNICODE_MASK 0x00010000

enum InputMethodType {
    VNI,
    SimpleTelex,
    Telex,
    
    InputMethodTypeCount
};

static vector<UInt8> _wordBreakCode = {
    KEY_BACKQUOTE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS,
    KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACK_SLASH,
    KEY_SEMICOLON, KEY_QUOTE, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_SPACE
};

struct KeyMapping {
	UInt16 key;
	int action;
};

static map<UInt16, int> InputMethodMapping[] = {
    {   // VNI
        {KEY_0, Tone0},
        {KEY_1, Tone1},
        {KEY_2, Tone2},
        {KEY_3, Tone3},
        {KEY_4, Tone4},
        {KEY_5, Tone5},
        {KEY_6, RoofAll},
        {KEY_7, HookAll},
        {KEY_8, Bowl},
        {KEY_9, Dd},
        {KEY_ESC, EscChar},
        {0, Normal}
    },
    {   // Simple Telex
        {KEY_Z, Tone0},
        {KEY_S, Tone1},
        {KEY_F, Tone2},
        {KEY_R, Tone3},
        {KEY_X, Tone4},
        {KEY_J, Tone5},
        {KEY_W, HookAll},
        {KEY_A, RoofA},
        {KEY_E, RoofE},
        {KEY_O, RoofO},
        {KEY_D, Dd},
        {KEY_ESC, EscChar},
        {0, Normal}
    },
    {   // Telex
        {KEY_Z, Tone0},
        {KEY_S, Tone1},
        {KEY_F, Tone2},
        {KEY_R, Tone3},
        {KEY_X, Tone4},
        {KEY_J, Tone5},
        {KEY_W, HookAll},
        {KEY_A, RoofA},
        {KEY_E, RoofE},
        {KEY_O, RoofO},
        {KEY_LEFT_BRACKET, HookO},
        {KEY_RIGHT_BRACKET, HookU},
        {KEY_D, Dd},
        {KEY_ESC, EscChar},
        {0, Normal}
    }
};

// map tone to key mapping, will be init when we change input method
static map<int, UInt16> ToneToKeyCodeMapping = {};

/**
 * codeTableIndex: bang ma can dung. 0 = unicode
 */
UInt32 _getCharacterCode(int codeTableIndex, const BufferEntry& entry ) {
    UInt16 keyCode = entry.keyCode;
    
    if (entry.roofType == ROOF)
        keyCode = keyCode | MASK_ROOF;
    else if (entry.roofType == HOOK)
        keyCode = keyCode | MASK_BOWLHOOK;
    
    if (codeTable[codeTableIndex].find(keyCode) != codeTable[codeTableIndex].end()) {
        vector<UInt16> charList = codeTable[codeTableIndex][keyCode];
        int index = (entry.tone - KeyEvent::Tone0) * 2 + (entry.cap ? 0 : 1);
        
        UInt32 result = index >= 0 && charList.size() > 0 ? charList[index] | UNICODE_MASK : keyCode;
        
        return result;
    }
    
    return entry.keyCode;
}


kbengine::kbengine()
{
    this->resetBuffer();
    this->setInputMethod(InputMethodType::VNI);
}

kbengine::~kbengine()
{
    // TODO Auto-generated destructor stub
}

int kbengine::_findSyllable(vector<UInt16> &syllableCombine, const UInt16 &expectedType, const UInt16 &expectedKey = KEY_EMPTY)
{
    int endIdx = this->_bufferSize;
    int curIdx = this->_bufferStartWordIdx;
    
    for(curIdx = this->_bufferStartWordIdx; curIdx < endIdx ; curIdx++)
    {
        UInt16 curKeyCode = this->_buffer[curIdx].keyCode;
        
        if (expectedKey != KEY_EMPTY && curKeyCode != expectedKey) {
            LOG_DEBUG("Ignore %d due to expected: %d", curKeyCode, expectedKey);
            continue;
        }
        auto findResult = syllableTable.find(curKeyCode);
        
        if (findResult == syllableTable.end())
            continue;
        
        vector<vector<UInt16>> syllableList = findResult->second;
        for (vector<vector<UInt16>>::iterator it = syllableList.begin(); it != syllableList.end(); ++it)
        {
            vector<UInt16> combine = *it;
            // this combine first item not match expectedType
            // this combine not match the end part of buffer
            //-> ignore
            if (!(combine[0] & expectedType) || (endIdx - curIdx) > combine.size() - 1) {
                continue;
            }
            
            bool found = true;
            for(auto ii=combine.size()-1; ii >= 1; ii--) {
                auto bufIdx = ii + curIdx - 1;
                UInt8 curCode = UInt8(combine[ii] ^ MASK_EXTRA_MARK);   // remove MASK_EXTRA_MARK if any
                if (curCode != (int)this->_buffer[bufIdx].keyCode) {
                    found = false;
                    break;
                }
            }
            
            if (found) {
                syllableCombine = combine;
                break;
            }
        }
        if (syllableCombine.size() > 0) break;
    }
    
    return curIdx;
}

int kbengine::_processMark(const UInt8 &keycode, const RoofType &roofType)
{
    vector<UInt16> matchCombine;
    auto maskType = MASK_ORIGIN;
    auto targetRoofType = RoofType::ORIGIN;
    
    UInt16 expectedKey = KEY_EMPTY;
    switch (roofType) {
        case ROOF_A:
            maskType = MASK_ROOF;
            targetRoofType = RoofType::ROOF;
            expectedKey = KEY_A;
            break;
        case ROOF_E:
            maskType = MASK_ROOF;
            targetRoofType = RoofType::ROOF;
            expectedKey = KEY_E;
            break;
        case ROOF_O:
            maskType = MASK_ROOF;
            targetRoofType = RoofType::ROOF;
            expectedKey = KEY_O;
            break;
        default:
            expectedKey = KEY_EMPTY;
            targetRoofType = roofType;
            maskType = roofType == ROOF ? MASK_ROOF : (roofType == HOOK ? MASK_BOWLHOOK : MASK_ORIGIN);
            break;
    }
    
    int foundIdx = this->_findSyllable(matchCombine, maskType, expectedKey);
    
    if (matchCombine.size() >= 0) {
        
        int endIdx = this->_bufferSize;
        
        int numBackSpaces = (endIdx - foundIdx);

        bool canSetHook = false;
        
        // ignore the first element since it is category of combine
        for (int i = 1; i < matchCombine.size(); i++) {
            
            if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                int buffIndex = (i-1) + foundIdx;   // (i-1) since we ignore the first value of combine
                if (this->_buffer[buffIndex].roofType == ORIGIN) {
                    this->_buffer[buffIndex].roofType = targetRoofType;
                    
                    canSetHook = true;
                }
            }
        }
        
        if (!canSetHook) {  // not set any hook, we reverse hook
            for (int i = 1; i < matchCombine.size(); i++) {
                if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                    int buffIndex = (i-1) + foundIdx;   // (i-1) since we ignore the first value of combine
                    this->_buffer[buffIndex].roofType = ORIGIN;
                }
            }
            // add the pressed keycode back to buffer
            _addKeyCode(keycode, 0);
            endIdx ++;
        }
        this->_processKeyCodeOutput(numBackSpaces, foundIdx, endIdx);
    }
    else {
        foundIdx = -1;
    }
    
    return foundIdx;
}

int kbengine::_processD(const UInt8 &keycode) {
    int foundIdx = -1;
    int endIdx = this->_bufferSize;
    
    vector<UInt16> syllableCombine;
    // check the previous keycode is d or not
    if (this->_buffer[endIdx-1].keyCode == KEY_D) {
        foundIdx = endIdx;
    }
    else {
        foundIdx = this->_findSyllable(syllableCombine, MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK);
    }
    
    if (foundIdx >= 0) {
        if (this->_buffer[foundIdx-1].keyCode == KEY_D) {
            int numBackSpaces = (endIdx - foundIdx) + 1; // delete vowel & d character
            
            if (this->_buffer[foundIdx-1].roofType == ROOF) {
                this->_buffer[foundIdx-1].roofType = ORIGIN;
                _addKeyCode(keycode, 0);
                endIdx ++;
            }
            else
                this->_buffer[foundIdx-1].roofType = ROOF;
            
            this->_processKeyCodeOutput(numBackSpaces, foundIdx-1, endIdx);
        }
        else {
            foundIdx = -1;
        }
    }
    
    return foundIdx;
}

/**
 * In the case of correct spelling tone, we need previousTonePosition to start from there
 */
int kbengine::_processToneTraditional(const UInt8 &keycode, const KeyEvent &tone, const bool &fromSpelling = false)
{
    vector<UInt16> syllableCombine;
    int foundIdx = this->_findSyllable(syllableCombine, MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK);
    
    if (syllableCombine.size() >= 0) {
        // count vowel, ignore first element
        int tonePosition = -1;
        // if buffer has ê or ơ, tone alway there
        for(int i = this->_bufferStartWordIdx; i < this->_bufferSize; i++) {
            if ((this->_buffer[i].keyCode == KEY_E && this->_buffer[i].roofType == ROOF)
                || (this->_buffer[i].keyCode == KEY_O && this->_buffer[i].roofType == HOOK))
            {
                tonePosition = i;
                break;
            }
        }
        
        // not found ê or ơ -> check number of vowel
        if (tonePosition < 0) {
            int vowelCount = 0;
            bool lastIsVowel = true;   // keep track the last char is vowel or not
            for(int i = 1; i < syllableCombine.size(); i++) {
                auto keyCode = EXCLUDE_MARK(syllableCombine[i]);
                
                if (IS_VOWEL(keyCode)) vowelCount++;
                lastIsVowel = IS_VOWEL(keyCode);
            }
            // Nếu có một nguyên âm thì dấu đặt ở nguyên âm: á, tã, nhà, nhãn, gánh, ngáng
            // Nếu là tập hợp hai (2) nguyên âm (nguyên âm đôi) thì đánh dấu ở nguyên âm đầu.
            if (vowelCount == 1 || (vowelCount == 2 && lastIsVowel)) tonePosition = foundIdx;
            
            // Tập hợp ba (3) nguyên âm (nguyên âm ba) hoặc
            // hai nguyên âm + phụ âm cuối thì vị trí dấu chuyển đến nguyên âm thứ nhì.
            if (vowelCount == 3 || (vowelCount == 2 && !lastIsVowel)) tonePosition = foundIdx + 1;
        }

        if (tonePosition >= 0) {
            int endIdx = this->_bufferSize;
            int numBackSpaces = (endIdx - tonePosition);
            // if not call from correctSpelling, we reverse tone
            if (this->_buffer[tonePosition].tone == tone && !fromSpelling) {
                this->_buffer[tonePosition].tone = KeyEvent::Tone0;
                _addKeyCode(keycode, 0);
                endIdx ++;
            }
            else
                this->_buffer[tonePosition].tone = tone;
            

            int previousTonePos = -1;
            for (int i = this->_bufferStartWordIdx; i < tonePosition; i++) {
                LOG_DEBUG("Buff tone: %d, tone: %d", i, this->_buffer[i].tone);
                if (this->_buffer[i].tone != KeyEvent::Tone0) {
                    this->_buffer[i].tone = KeyEvent::Tone0;
                    previousTonePos = i;
                    break;
                }
            }
            
            LOG_DEBUG("Previous tone: %d , endIdx: %d", previousTonePos, endIdx);
            if (previousTonePos >= 0) {
                tonePosition = previousTonePos;
                numBackSpaces = (endIdx - previousTonePos);
            }
            
            // call from spelling, decrease backspace
            if (fromSpelling)
                numBackSpaces -= 1;

            LOG_DEBUG("tone Pos: %d , endIdx: %d", tonePosition, endIdx);
            LOG_DEBUG("Num Backspaces: %d", numBackSpaces);
            this->_processKeyCodeOutput(numBackSpaces, tonePosition, endIdx);
        }
    }
    else {
        foundIdx = -1;
    }
    
    return foundIdx;
}

/*
 * - get the tone of last word.
 * - call _processToneTraditional or _processToneNew to reprocess tone
 * Notes: only call this func if new char is Vowel
 */
int kbengine::_correctSpelling(const UInt8 &keycode)
{
//    bool isVowel = IS_VOWEL(keycode);
//    if (isVowel) {
//        LOG_DEBUG("Ignore Spelling, is vowel inputted: %d", isVowel);
//        return -1;
//    }
    
    short currentTone = KeyEvent::Tone0;
    
    for (int i = this->_bufferStartWordIdx; i < this->_bufferSize; i++) {
        if (this->_buffer[i].tone != KeyEvent::Tone0) {
            currentTone = this->_buffer[i].tone;
            break;
        }
    }
    if (currentTone != KeyEvent::Tone0) {
        UInt8 keyCode = ToneToKeyCodeMapping[currentTone];
        LOG_DEBUG("Check spelling, keyCode = %d, curTone: %d", keyCode, currentTone);
        
        // TODO: call new process Tone if support
        this->_processToneTraditional(keyCode, static_cast<KeyEvent>(currentTone), true);
    }
    
    return 0;
}


void kbengine::_processKeyCodeOutput(int numDelete, int startPos, int endPos)
{
    // prepare keystroke to be sent
    for(int i = 0; i < numDelete; i ++)
        this->_keyCodeOutput.push_back(KEY_DELETE);
    
    for (int i = startPos; i < endPos; i ++) {
        this->_keyCodeOutput.push_back(_getCharacterCode(this->currentCodeTable, this->_buffer[i]));
    }
}

vector<UInt32> kbengine::getOutputBuffer()
{
    return this->_keyCodeOutput;
}

void kbengine::_addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap) {
    if (this->_bufferSize == MAX_BUFF) {
        // need to remove all word except the last one
        UInt16 wordSize = this->_bufferSize - this->_bufferStartWordIdx;
        
        for(int i = this->_bufferStartWordIdx; i < this->_bufferSize; i ++) {
            int buffIndex = i - this->_bufferStartWordIdx;
            this->_buffer[buffIndex] = this->_buffer[i];
        }
        this->_bufferStartWordIdx = 0;
        this->_bufferSize = wordSize;
        
        LOG_DEBUG("Moved current: %d, size: %d", this->_bufferStartWordIdx, wordSize);
    }
    
    BufferEntry e = { keycode };
    e.cap = shiftCap > 0 ? true : false;
    this->_buffer[this->_bufferSize] = e;
    this->_bufferSize++;
    
    // Check if we start new word
    if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), keycode) != _wordBreakCode.end())
    {
        this->_startNewWord();
    }
}

int kbengine::process(const UInt16 &charCode, const UInt8 &keycode, const UInt8 &shiftCap, const bool &otherControl)
{
    this->_keyCodeOutput.clear();
    
    auto action = InputMethodMapping[this->_currentInputMethod].find(keycode);
    
    KeyEvent result = Normal;
    int actionResult = -1;
    if (shiftCap == 0 && action != InputMethodMapping[_currentInputMethod].end() && this->_bufferSize > 0)
    {
        result = (KeyEvent) action->second;
        switch (result) {
            case RoofAll:
                actionResult = this->_processMark(keycode, RoofType::ROOF);
                break;
            case RoofA:
                actionResult = this->_processMark(keycode, RoofType::ROOF_A);
                break;
            case RoofO:
                actionResult = this->_processMark(keycode, RoofType::ROOF_O);
                break;
            case RoofE:
                actionResult = this->_processMark(keycode, RoofType::ROOF_E);
                break;
            case HookAll:
            case Bowl:
                actionResult = this->_processMark(keycode, RoofType::HOOK);
                break;
            case HookO:
            case HookU:
                LOG_DEBUG("Process HookO, hookU in Telex");
                break;
            case Dd:
                actionResult = this->_processD(keycode);
                break;
            case Tone0:
            case Tone1:
            case Tone2:
            case Tone3:
            case Tone4:
            case Tone5:
                actionResult = this->_processToneTraditional(keycode, result);
                break;
            case EscChar:
                this->resetBuffer();
                break;
            default:
                result = Normal;
                break;
        }
    }
    
    // keyCode already processed
    if (result != Normal && ACTION_PROCESSED(actionResult)
        && this->_keyCodeOutput.size() > 0) {
        LOG_DEBUG("Processed, Begin Idx: %d, EndIdx: %d", this->_bufferStartWordIdx, this->_bufferSize);
        return 1;
    }
    
    
    bool printable = charCode >= 32 && charCode <= 127;
    
    LOG_DEBUG("Character Printable: %d", printable);
    if (printable) {
        _addKeyCode(keycode, shiftCap);
        _correctSpelling(keycode);
    }
    else {
        // Check delete key
        if (keycode == KEY_DELETE) {
            if (shiftCap > 0 || otherControl) {
                this->resetBuffer();
            }
            else {
                // TODO: Implement replace the tone if any
                this->_processBackSpacePressed();
            }
        }
        else if (keycode == KEY_ENTER) {
            this->resetBuffer();
        }
    }
    
    LOG_DEBUG("Begin Idx: %d, EndIdx: %d", this->_bufferStartWordIdx, this->_bufferSize);

	return 0;
}


void kbengine::resetBuffer()
{
    memset(this->_buffer, MAX_BUFF, sizeof(BufferEntry));
    this->_bufferStartWordIdx = 0;
    this->_bufferSize = 0;
}

// Recalculate startIndex of a word & buffer when user press delete
void kbengine::_processBackSpacePressed() {
    this->_bufferSize--;
    this->_bufferStartWordIdx = 0;
    if (this->_bufferSize < 0) this->_bufferSize = 0;
    
    for (int i = this->_bufferSize - 1; i > 0; i--) {
        if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), this->_buffer[i].keyCode) != _wordBreakCode.end()) {
            this->_bufferStartWordIdx = i;
            break;
        }
    }
}

// Recalculate startIndex of a word when user start new word
void kbengine::_startNewWord() {
    this->_bufferStartWordIdx = this->_bufferSize - 1;
}


BufferEntry* kbengine::getBuffer()
{
	return this->_buffer;
}

void kbengine::setInputMethod(const UInt8 &inputMethod)
{
    if (inputMethod >= InputMethodType::InputMethodTypeCount)
        this->_currentInputMethod = InputMethodType::VNI;
    else
        this->_currentInputMethod = inputMethod;
    
    // reset tone to keycode mapping
    ToneToKeyCodeMapping.clear();
    auto inputMapping = InputMethodMapping[_currentInputMethod];
    for (auto it = inputMapping.begin(); it != inputMapping.end(); it++) {
        ToneToKeyCodeMapping[it->second] = it->first;
    }
}

UInt8 kbengine::getInputMethod() {
    return this->_currentInputMethod;
}
