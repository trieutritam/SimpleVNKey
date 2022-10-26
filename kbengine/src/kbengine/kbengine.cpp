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

#define  BYTE_HIGH(x)   ((x >> 8) & 0xff)
#define  BYTE_LOW(x)    (x & 0xff)

enum InputMethodType {
    VNI,
    SimpleTelex,
    Telex,
    
    InputMethodTypeCount
};

//
//static vector<UInt8> _wordBreakCode = {
//    KEY_BACKQUOTE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS,
//    KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACK_SLASH,
//    KEY_SEMICOLON, KEY_QUOTE, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_SPACE
//};

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
        {KEY_8, Breve},
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

bool checkWord(BufferEntry* pEntryList, int start, int end)
{
    // find the fist vowel
    int startVowel = -1;
    LOG_DEBUG("Start: %d, end: %d", start, end);
    
    for(int i = start; i < end; i++) {
        BufferEntry *pEntry = (pEntryList + i);
        if (IS_VOWEL(pEntry->keyCode)) {
            startVowel = i;
            break;
        }
    }
    
    LOG_DEBUG("Start vowel: %d", startVowel);

    
    if (startVowel == -1)
        return true;
    
    BufferEntry *firstEntry = (pEntryList + startVowel);
    int expectedSize = end - startVowel;
    
    LOG_DEBUG("Start Vowel: %d, size: %d", startVowel, expectedSize);
    
    bool isValid = false;
    for(auto it = syllableTable.begin(); it != syllableTable.end(); it++) {
        vector<vector<UInt16>> syllableList = it->second;
        
        if (firstEntry->keyCode != it->first)
            continue;
        
        for(auto it2 = syllableList.begin(); it2 != syllableList.end(); it2++) {
            vector<UInt16> syllable = *it2;

            if (expectedSize == (syllable.size() -1)) {
                isValid = true;
                for(int i = 1; i < syllable.size(); i++) {
                    LOG_DEBUG("expexted keycode %dX, syllabel: %X", (firstEntry + (i-1))->keyCode, syllable[i]);
                    if ((firstEntry + (i-1))->keyCode != syllable[i]) {
                        isValid = false;
                        break;
                    }
                }
            }
            if (isValid)
                break;
        }
    }
    return isValid;
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

vector<BufferEntry*> kbengine::extractWord()
{
    vector<BufferEntry*> word;
    
    int endIdx = this->_bufferSize;
    for(int i = this->_bufferStartWordIdx; i < endIdx ; i++)
    {
        if (this->_buffer[i].processed) continue;
        word.push_back(&(this->_buffer[i]));
    }
    
    LOG_DEBUG("ExtractWord, size: %lu", word.size());
    
    return word;
}

int kbengine::_findSyllableV2(vector<UInt16> &syllableCombine,
                    const vector<BufferEntry*> &word, const UInt16 &expectedType, const UInt16 &expectedKey)
{
    int curIdx = 0;
    UInt16 wordSize = word.size();
    
    for(curIdx = 0; curIdx < word.size(); curIdx++) {
        UInt16 curKeyCode = word[curIdx]->keyCode;
        
        if (expectedKey != KEY_EMPTY && curKeyCode != expectedKey) {
            LOG_DEBUG("Ignore %d due to expected: %d", curKeyCode, expectedKey);
            continue;
        }
        auto findResult = syllableTable.find(curKeyCode);
        
        if (findResult == syllableTable.end())
            continue;
        
        LOG_DEBUG("Find syllableCombine, found num of syllable! %lu", findResult->second.size());
        
        vector<vector<UInt16>> syllableList = findResult->second;
        for (vector<vector<UInt16>>::iterator it = syllableList.begin(); it != syllableList.end(); ++it)
        {
            vector<UInt16> combine = *it;
            // this combine first item not match expectedType
            // this combine not match the end part of buffer
            //-> ignore
            if (!(combine[0] & expectedType) || (wordSize - curIdx) != combine.size() - 1) {
                continue;
            }
            
            bool found = true;
            for(auto ii=combine.size()-1; ii >= 1; ii--) {
                auto buffIdx = ii + curIdx - 1;
                UInt8 curCode = UInt8(combine[ii] ^ MASK_EXTRA_MARK);   // remove MASK_EXTRA_MARK if any
                if (curCode != (int)word[buffIdx]->keyCode) {
                    found = false;
                    break;
                }
            }
            
            if (found) {
                PRINT_VECTOR(combine);
                syllableCombine = combine;
                break;
            }
        }
        if (syllableCombine.size() > 0) break;
    }
    
    return curIdx < word.size() ? curIdx : -1;
}

int kbengine::_findSyllable(vector<UInt16> &syllableCombine, const UInt16 &expectedType, const UInt16 &expectedKey)
{
    vector<BufferEntry*> word = extractWord();
    
    int curIdx = 0;
    UInt16 wordSize = word.size();
    
    for(curIdx = 0; curIdx < word.size(); curIdx++) {
        UInt16 curKeyCode = word[curIdx]->keyCode;
        
        if (expectedKey != KEY_EMPTY && curKeyCode != expectedKey) {
            LOG_DEBUG("Ignore %d due to expected: %d", curKeyCode, expectedKey);
            continue;
        }
        auto findResult = syllableTable.find(curKeyCode);
        
        if (findResult == syllableTable.end())
            continue;
        
        LOG_DEBUG("Find syllableCombine, found num of syllable! %lu", findResult->second.size());
        
        vector<vector<UInt16>> syllableList = findResult->second;
        for (vector<vector<UInt16>>::iterator it = syllableList.begin(); it != syllableList.end(); ++it)
        {
            vector<UInt16> combine = *it;
            // this combine first item not match expectedType
            // this combine not match the end part of buffer
            //-> ignore
            if (!(combine[0] & expectedType) || (wordSize - curIdx) != combine.size() - 1) {
                continue;
            }
            
            bool found = true;
            for(auto ii=combine.size()-1; ii >= 1; ii--) {
                auto buffIdx = ii + curIdx - 1;
                UInt8 curCode = UInt8(combine[ii] ^ MASK_EXTRA_MARK);   // remove MASK_EXTRA_MARK if any
                if (curCode != (int)word[buffIdx]->keyCode) {
                    found = false;
                    break;
                }
            }
            
            if (found) {
                PRINT_VECTOR(combine);
                syllableCombine = combine;
                break;
            }
        }
        if (syllableCombine.size() > 0) break;
    }
    
    return curIdx < word.size() ? curIdx : -1;  // + this->_bufferStartWordIdx;
    
//    int endIdx = this->_bufferSize;
//    int curIdx = this->_bufferStartWordIdx;
//
//    for(curIdx = this->_bufferStartWordIdx; curIdx < endIdx ; curIdx++)
//    {
//        UInt16 curKeyCode = this->_buffer[curIdx].keyCode;
//
//        if (expectedKey != KEY_EMPTY && curKeyCode != expectedKey) {
//            LOG_DEBUG("Ignore %d due to expected: %d", curKeyCode, expectedKey);
//            continue;
//        }
//        auto findResult = syllableTable.find(curKeyCode);
//
//        if (findResult == syllableTable.end())
//            continue;
//
//        LOG_DEBUG("Find syllableCombine, found! %lu", findResult->second.size());
//
//        vector<vector<UInt16>> syllableList = findResult->second;
//        for (vector<vector<UInt16>>::iterator it = syllableList.begin(); it != syllableList.end(); ++it)
//        {
//            vector<UInt16> combine = *it;
//            // this combine first item not match expectedType
//            // this combine not match the end part of buffer
//            //-> ignore
//            if (!(combine[0] & expectedType) || (endIdx - curIdx) != combine.size() - 1) {
//                continue;
//            }
//
//            bool found = true;
//            for(auto ii=combine.size()-1; ii >= 1; ii--) {
//                auto bufIdx = ii + curIdx - 1;
//                UInt8 curCode = UInt8(combine[ii] ^ MASK_EXTRA_MARK);   // remove MASK_EXTRA_MARK if any
//                if (curCode != (int)this->_buffer[bufIdx].keyCode) {
//                    found = false;
//                    break;
//                }
//            }
//
//            if (found) {
//                PRINT_VECTOR(combine);
//                syllableCombine = combine;
//                break;
//            }
//        }
//        if (syllableCombine.size() > 0) break;
//    }
    
    return curIdx;
}

int kbengine::_processMarkV2(const vector<BufferEntry*> &word, BufferEntry *lastChar, const RoofType &roofType, const bool &fromCorrectFunc)
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
        case BREVE: // only a have breve on top of it
            maskType = MASK_HOOK;
            targetRoofType = RoofType::HOOK;
            expectedKey = KEY_A;
            break;
        default:
            expectedKey = KEY_EMPTY;
            targetRoofType = roofType;
            maskType = roofType == ROOF ? MASK_ROOF : (roofType == HOOK ? MASK_HOOK :  MASK_ORIGIN);
            break;
    }
    
    int foundIdx = this->_findSyllableV2(matchCombine, word, maskType, expectedKey);
    
    LOG_DEBUG("FoundIdx: %d, matchCombine.size: %lu", foundIdx, matchCombine.size());
    
    if (matchCombine.size() >= 0 && foundIdx >= 0) {
        PRINT_VECTOR(matchCombine);

        bool canSetHook = false;
        
        // ignore the first element since it is category of combine
        for (int i = 1; i < matchCombine.size(); i++) {
            if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                int buffIndex = (i-1) + foundIdx;   // (i-1) since we ignore the first value of combine
                if (word[buffIndex]->roofType != targetRoofType) {
                    word[buffIndex]->roofType = targetRoofType;
                    canSetHook = true;
                }
            }
        }
        
        // not set any hook, we reverse hook if this process is not from correct function
        // in correct function we don't reverse hook
        if (!canSetHook && !fromCorrectFunc) {
            for (int i = 1; i < matchCombine.size(); i++) {
                if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                    int buffIndex = (i-1) + foundIdx;
                    word[buffIndex]->roofType = ORIGIN;
                }
            }
        }
        
        lastChar->processed = canSetHook;
//        if (canSetHook) {
//            word[word.size() - 1]->processed = true;
        //}
        
//        int numBackSpaces = _calculateNumberOfBackSpaceV2(word, foundIdx);
//
//        LOG_DEBUG("Process Mark, Can set hook: %d, From Correct Func: %d", canSetHook, fromCorrectFunc);
//        LOG_DEBUG("Process Mark, numBackSpaces: %d, foundIdx: %d", numBackSpaces, foundIdx);
//
//        if (fromCorrectFunc)
//            numBackSpaces --;
//
//        this->_processKeyCodeOutputV2(word, numBackSpaces, foundIdx);
    }
    else {
        foundIdx = -1;
    }
    
    return foundIdx;
}

int kbengine::_processMark(const UInt8 &keycode, const RoofType &roofType, const bool &fromCorrectFunc)
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
        case BREVE: // only a have breve on top of it
            maskType = MASK_HOOK;
            targetRoofType = RoofType::HOOK;
            expectedKey = KEY_A;
            break;
        default:
            expectedKey = KEY_EMPTY;
            targetRoofType = roofType;
            maskType = roofType == ROOF ? MASK_ROOF : (roofType == HOOK ? MASK_HOOK :  MASK_ORIGIN);
            break;
    }
    
    int foundIdx = this->_findSyllable(matchCombine, maskType, expectedKey);
    
    vector<BufferEntry*> word = extractWord();
    
    if (matchCombine.size() > 0) {
        PRINT_VECTOR(matchCombine);
        
        int endIdx = this->_bufferSize;
        
        int numBackSpaces = _calculateNumberOfBackSpace(foundIdx, endIdx);

        bool canSetHook = false;
        
        // ignore the first element since it is category of combine
        for (int i = 1; i < matchCombine.size(); i++) {
            if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                int buffIndex = (i-1) + foundIdx;   // (i-1) since we ignore the first value of combine
                if (word[buffIndex]->roofType != targetRoofType) {
                    word[buffIndex]->roofType = targetRoofType;
                    canSetHook = true;
                }
            }
        }
        
        // not set any hook, we reverse hook if this process is not from correct function
        // in correct function we don't reverse hook
        if (!canSetHook && !fromCorrectFunc) {
            for (int i = 1; i < matchCombine.size(); i++) {
                if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                    int buffIndex = (i-1) + foundIdx;
                    word[buffIndex]->roofType = ORIGIN;
                }
            }
            // add the pressed keycode back to buffer
            _addKeyCode(keycode, 0);
            endIdx ++;
        }
        
        if (canSetHook) {
            _addKeyCode(keycode, 0, true);
        }
        
        LOG_DEBUG("Process Mark, Can set hook: %d, From Correct Func: %d", canSetHook, fromCorrectFunc);
        LOG_DEBUG("Process Mark, numBackSpaces: %d, foundIdx: %d", numBackSpaces, foundIdx);

        if (fromCorrectFunc)
            numBackSpaces --;
        
        this->_processKeyCodeOutput(numBackSpaces, foundIdx);
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
    
    vector<BufferEntry*> word = extractWord();
    
    // check the previous keycode is d or not
    for(int i = (int)word.size() - 1; i >= 0; i --) {
        BufferEntry *entry = word[i];
        
        if (entry->keyCode == KEY_D) {
            foundIdx = i;
            break;
        }
    }
    
    if (foundIdx >= 0) {
        int numBackSpaces = _calculateNumberOfBackSpace(foundIdx, endIdx);
        
        if (word[foundIdx]->roofType == ROOF) {
            word[foundIdx]->roofType = ORIGIN;
            _addKeyCode(keycode, 0);
        }
        else {
            word[foundIdx]->roofType = ROOF;
            _addKeyCode(keycode, 0, true);
        }
        
        this->_processKeyCodeOutput(numBackSpaces, foundIdx);
    }
    
    return foundIdx;
}

int kbengine::_placeToneTraditionalRule(int foundIdx, vector<UInt16> syllableCombine)
{
    int tonePosition = -1;
    
    vector<BufferEntry*> word = extractWord();
    UInt16 wordSize = word.size();
    
    // if buffer has ê or ơ, tone alway there
    for (UInt16 i = 0; i < wordSize; i++) {
        BufferEntry *entry = word[i];
        if ((entry->keyCode == KEY_E && entry->roofType == ROOF) || (entry->keyCode == KEY_O && entry->roofType == HOOK)) {
            tonePosition = i;
            break;
        }
    }
    
    // not found ê or ơ -> check number of vowel
    if (tonePosition < 0) {
        int vowelCount = 0;
        bool lastIsVowel = true;   // keep track the last char is vowel or not
        PRINT_VECTOR(syllableCombine);
        for(int i = 1; i < syllableCombine.size(); i++) {
            auto keyCode = EXCLUDE_MARK(syllableCombine[i]);
            
            if (IS_VOWEL(keyCode)) vowelCount++;
            lastIsVowel = IS_VOWEL(keyCode);
        }
        // Nếu có một nguyên âm thì dấu đặt ở nguyên âm: á, tã, nhà, nhãn, gánh, ngáng
        // Nếu là tập hợp hai (2) nguyên âm (nguyên âm đôi) thì đánh dấu ở nguyên âm đầu.
        if (vowelCount == 1 || (vowelCount == 2 && lastIsVowel)) tonePosition = foundIdx;
        
        // Kiểu cũ dựa trên những từ điển từ trước năm 1950 nên "gi" và "qu" được coi là một mẫu tự riêng.
        // Vì vậy "già" và "quạ" không phải là nguyên âm đôi "ia" hay "ua" mà là "gi" + "à"; và "qu" + "ạ".
        // => This case we already count 2 vowels, and we need to adjust
        if ((vowelCount == 2 && lastIsVowel) && foundIdx > this->_bufferStartWordIdx) {
            LOG_DEBUG("Special: %d %d", word[foundIdx - 1]->keyCode, word[foundIdx]->keyCode);
            if ((word[foundIdx - 1]->keyCode == KEY_Q && word[foundIdx]->keyCode == KEY_U)
                || (word[foundIdx - 1]->keyCode == KEY_G && word[foundIdx]->keyCode == KEY_I))
            {
                tonePosition += 1;
            }
        }
        
        
        // Tập hợp ba (3) nguyên âm (nguyên âm ba) hoặc
        // hai nguyên âm + phụ âm cuối thì vị trí dấu chuyển đến nguyên âm thứ nhì.
        if (vowelCount == 3 || (vowelCount == 2 && !lastIsVowel)) tonePosition = foundIdx + 1;
    }
    
    return tonePosition;
}


int kbengine::_placeToneModernRule(int foundIdx, vector<UInt16> syllableCombine)
{
    int tonePosition = -1;
    
    PRINT_VECTOR(syllableCombine);
    
    int vowelCount = 0;
    int firstVowelPos = -1;
    int lastVowelPos = -1;
    bool lastIsVowel = false;
    
    bool hasCircumflex = false;
    int circumflexPos = -1;
    
    //int endIdx = this->_bufferSize - 1;
    
    vector<BufferEntry*> word = extractWord();
    
    for(int i = 0; i < word.size(); i++) {
        BufferEntry *pEntry = word[i];
        
        auto keyCode = pEntry->keyCode;
        
        if (IS_VOWEL(keyCode)) {
            vowelCount++;
            if (firstVowelPos == -1) firstVowelPos = i;
            lastVowelPos = i;
        }
        
        lastIsVowel = IS_VOWEL(keyCode);
        hasCircumflex = pEntry->roofType != RoofType::ORIGIN;
        if (hasCircumflex) {
            circumflexPos = i;
        }
    }

    //Rule 1:
    LOG_DEBUG("vowelCount: %d - firstVowelPos: %d - lastVowelPos: %d", vowelCount, firstVowelPos, lastVowelPos);
    if (vowelCount == 1) {
        tonePosition = firstVowelPos;
    }
    else if (vowelCount == 2 && lastIsVowel == false) {
        // Rule 3
        LOG_DEBUG("vowelCount: %d - pos: %d - lastIsVowel: %d", vowelCount, firstVowelPos, lastIsVowel);
        
        tonePosition = lastVowelPos;
    }
    
    // Rule 2
    LOG_DEBUG("hasCircumflex: %d - pos: %d", hasCircumflex, circumflexPos);
    if (hasCircumflex) {
        tonePosition = circumflexPos;
    }

    // Rule 4
    if (vowelCount >= 2 && lastIsVowel) {
        auto *beforeLast = word[word.size() - 2];
        auto *last = word[word.size() - 1];
        if ((beforeLast->keyCode == KEY_O && last->keyCode == KEY_A)
            || (beforeLast->keyCode == KEY_O && last->keyCode == KEY_E)
            || (beforeLast->keyCode == KEY_U && last->keyCode == KEY_Y)
            ){
            LOG_DEBUG("Tone: oa,oe,uy");
            tonePosition = lastVowelPos;
        }
        else {
            LOG_DEBUG("Tone which is not: oa,oe,uy");
            tonePosition = lastVowelPos - 1;
            
            // check start with qu, gi
            // if beforeLast = u or i && beforeLast's before = g or q
            if (word.size() >= 3) {
                auto beforeBeforeLast = word[word.size() - 3];
                if ((beforeLast->keyCode == KEY_U || beforeLast->keyCode == KEY_I)
                    && (beforeBeforeLast->keyCode == KEY_G || beforeBeforeLast->keyCode == KEY_Q)) {
                    tonePosition = lastVowelPos;
                }
            }
        }
    }
    
    LOG_DEBUG("Tone position: %d", tonePosition);
    
    
    return tonePosition;
}

/**
 * In the case of correct spelling tone, we need previousTonePosition to start from there
 */
int kbengine::_processTone(const UInt8 &keycode, const KeyEvent &tone, const bool &fromCorrectFunc)
{
    vector<UInt16> syllableCombine;
    int foundIdx = this->_findSyllable(syllableCombine, MASK_ORIGIN | MASK_ROOF | MASK_HOOK);
    
    if (syllableCombine.size() >= 0) {
        
        int tonePosition = _useModernTone ? _placeToneModernRule(foundIdx, syllableCombine)
                                            : _placeToneTraditionalRule(foundIdx, syllableCombine);

        vector<BufferEntry*> word = extractWord();
        if (tonePosition >= 0) {
//            tonePosition += _bufferStartWordIdx;
            LOG_DEBUG("Target tone posision: %d", tonePosition);
            int endIdx = this->_bufferSize;
            int numBackSpaces = 0; //_calculateNumberOfBackSpace(tonePosition, endIdx); //(endIdx - tonePosition);
            
            KeyEvent targetTone = tone;
            bool isCaseReset = false;
            
            // check target tonePosision have tone or not
            //if (this->_buffer[tonePosition].tone == tone && !fromCorrectFunc) {
            if (word[tonePosition]->tone == tone && !fromCorrectFunc) {
                //this->_buffer[tonePosition].tone = KeyEvent::Tone0;
                LOG_DEBUG("Found previous tone at target posision: %d", tonePosition);

                targetTone = KeyEvent::Tone0;
                isCaseReset = true;
            }
            
            int previousTonePos = -1;
            // check whole word to detect previous tone, we have this case is move tone
            //for (int i = this->_bufferStartWordIdx; i < tonePosition; i++) {
//                if(this->_buffer[i].processed)
//                    continue;
            for (auto i = 0; i < word.size(); i++) {
                LOG_DEBUG("Buffer index: %d, tone: %d", i, word[i]->tone);
                if (word[i]->tone != KeyEvent::Tone0) {
                    previousTonePos = i;
                    break;
                }
            }
            
            // calculate number of backspace
            numBackSpaces = _calculateNumberOfBackSpace(previousTonePos >= 0 ? previousTonePos : tonePosition, endIdx);
            
            if (isCaseReset) {
                //this->_buffer[tonePosition].tone = KeyEvent::Tone0;
                word[tonePosition]->tone = KeyEvent::Tone0;
                _addKeyCode(keycode, 0);
                endIdx ++;
            }
            else {
                //this->_buffer[tonePosition].tone = targetTone;
                word[tonePosition]->tone = targetTone;
                _addKeyCode(keycode, 0, true);
            }
            
            if (previousTonePos >= 0 && previousTonePos != tonePosition) {
                //this->_buffer[previousTonePos].tone = KeyEvent::Tone0;
                word[previousTonePos]->tone = KeyEvent::Tone0;
                tonePosition = previousTonePos;
            }
            
            if (fromCorrectFunc) {
                LOG_DEBUG("Call from CorrectFunc, numBackSpace before adjusted: %d", numBackSpaces);
                numBackSpaces -= 1;
            }
            
        
            LOG_DEBUG("tone Pos: %d , endIdx: %d, Num Backspaces: %d", tonePosition, endIdx, numBackSpaces);
            this->_processKeyCodeOutput(numBackSpaces, tonePosition);
        }
    }
    else {
        foundIdx = -1;
    }
    
    return foundIdx;
}

// This handle hook O/U, specialize for Telex
int kbengine::_processHookOU(const UInt8 &keycode, const UInt16 &expectedKey)
{
    LOG_DEBUG("_processHookOU, keycode: %d, buffSize: %d", keycode, this->_bufferSize);
    int numBackSpaces = 0;
    
    vector<BufferEntry*> word = extractWord();
    
    int foundIdx = (int) word.size();
    
    // find previous char have keycode or not
    if (word.size() > 0) {
        int curIndex = (int)word.size() - 1;
        BufferEntry *bufferEntry = word[curIndex];
        
        LOG_DEBUG("Previous code: %d", bufferEntry->keyCode);
        
        // if previous already have u+/o+, we need to remove it and replace with  ']' or '['
        if (bufferEntry->keyCode == expectedKey && bufferEntry->roofType == RoofType::HOOK) {
            bufferEntry->keyCode = keycode;
            bufferEntry->roofType = RoofType::ORIGIN;
            bufferEntry->processed = false;
            numBackSpaces = 1;
            foundIdx = curIndex;
        }
    }
    
    // there is no previous HookOU, just add HoolOU keycode
    if (numBackSpaces == 0) {
        this->_addKeyCode(expectedKey, 0);
        this->_buffer[this->_bufferSize-1].roofType = RoofType::HOOK;
    }
    
    LOG_DEBUG("Num backspace: %d, foundIdx: %d", numBackSpaces, foundIdx);
    this->_processKeyCodeOutput(numBackSpaces, foundIdx);
    
    return foundIdx;
}

/*
 * - get the tone of last word.
 * - call _processToneTraditional or _processToneNew to reprocess tone
 * Notes: only call this func if new char is Vowel
 */
int kbengine::_correctTone(const UInt8 &keycode)
{
//    bool isVowel = IS_VOWEL(keycode);
//    if (isVowel) {
//        LOG_DEBUG("Ignore Spelling, is vowel inputted: %d", isVowel);
//        return -1;
//    }
    
    short currentTone = KeyEvent::Tone0;
    LOG_DEBUG("Correct Tone - start: %d, end: %d", this->_bufferStartWordIdx, this->_bufferSize);
    for (int i = this->_bufferStartWordIdx; i < this->_bufferSize; i++) {
        BufferEntry *entry = &(this->_buffer[i]);
        
        if(entry->processed) continue;
        
        if (entry->tone != KeyEvent::Tone0) {
            currentTone = entry->tone;
            break;
        }
    }
    if (currentTone != KeyEvent::Tone0) {
        UInt8 keyCode = ToneToKeyCodeMapping[currentTone];
        LOG_DEBUG("Check spelling, keyCode = %d, curTone: %d", keyCode, currentTone);
        
        this->_processTone(keyCode, static_cast<KeyEvent>(currentTone), true);
    }
    
    return 0;
}

// Only correct for the case uo
int kbengine::_correctMark(const UInt8 &keycode)
{
    bool foundPreviousHook = false;
    
    for (int i = this->_bufferStartWordIdx; i < this->_bufferSize; i++) {
        BufferEntry *entry = &(this->_buffer[i]);
        if (entry->processed)
            continue;

        if (entry->roofType == RoofType::HOOK) {
            foundPreviousHook = true;
            break;
        }
    }
    
    LOG_DEBUG("Correct uo+ -> u+o+, foundPreviousHook: %d", foundPreviousHook);
    
    vector<BufferEntry*> word = extractWord();
    
    int foundIdx = -1;
    if (foundPreviousHook) {
        vector<UInt16> matchCombine;
        foundIdx = this->_findSyllable(matchCombine, MASK_HOOK, KEY_U);
        
        // only process if match uo and a <char>
        if (matchCombine.size() >= 4) {
            PRINT_VECTOR(matchCombine);
            
            int endIdx = (int) word.size();//this->_bufferSize;
            int numBackSpaces = _calculateNumberOfBackSpace(foundIdx, endIdx) - 1;

            bool canSetHook = false;
            
            // ignore the first element since it is category of combine
            for (int i = 1; i < matchCombine.size(); i++) {
                if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                    int buffIndex = (i-1) + foundIdx;   // (i-1) since we ignore the first value of combine
                    if (word[buffIndex]->roofType == ORIGIN) {
                        word[buffIndex]->roofType = RoofType::HOOK;
                        
                        canSetHook = true;
                    }
                }
            }
            LOG_DEBUG("Can set hook: %d", canSetHook);
            LOG_DEBUG("numBackSpaces: %d, foundIdx: %d", numBackSpaces, foundIdx);
            
            if (canSetHook)
                this->_processKeyCodeOutput(numBackSpaces, foundIdx);
        }
    }
    
    return foundIdx;
}

void kbengine::_processKeyCodeOutputV2(const vector<BufferEntry*> &word, int numDelete, int startPos)
{
    this->_keyCodeOutput.clear();
    
    // prepare keystroke to be sent
    for(int i = 0; i < numDelete; i ++)
        this->_keyCodeOutput.push_back(KEY_DELETE);
    
    for (int i = startPos; i < word.size(); i ++) {
        
        if(word[i]->processed)
            continue;
        
        auto charCode = _getCharacterCode(*word[i]);
        //LOG_DEBUG("_getCharacterCode: %d", charCode);

        auto charType = codeTableList[this->currentCodeTable][CODE_TABLE_CHAR_TYPE][0];

        if (charCode > 0) {
            if (charType == 2) {
                charCode = charCode ^ UNICODE_MASK;

                auto highByte = BYTE_HIGH(charCode);
                auto lowByte = BYTE_LOW(charCode);

                LOG_DEBUG("process high: %20X, low: %20X", highByte, lowByte);

                this->_keyCodeOutput.push_back(lowByte | UNICODE_MASK);

                if (highByte > 0) this->_keyCodeOutput.push_back(highByte  | UNICODE_MASK);
            }
            else {
                this->_keyCodeOutput.push_back(charCode);
            }
        }
        else {
            this->_keyCodeOutput.push_back(word[i]->keyCode);
        }
    }
}

void kbengine::_processKeyCodeOutput(int numDelete, int startPos)
{
    this->_keyCodeOutput.clear();
    vector<BufferEntry*> word = extractWord();
    
    // prepare keystroke to be sent
    for(int i = 0; i < numDelete; i ++)
        this->_keyCodeOutput.push_back(KEY_DELETE);
    
    for (int i = startPos; i < word.size(); i ++) {
        auto charCode = _getCharacterCode(*word[i]);
        //LOG_DEBUG("_getCharacterCode: %d", charCode);

        auto charType = codeTableList[this->currentCodeTable][CODE_TABLE_CHAR_TYPE][0];

        if (charCode > 0) {
            if (charType == 2) {
                charCode = charCode ^ UNICODE_MASK;

                auto highByte = BYTE_HIGH(charCode);
                auto lowByte = BYTE_LOW(charCode);

                LOG_DEBUG("process high: %20X, low: %20X", highByte, lowByte);

                this->_keyCodeOutput.push_back(lowByte | UNICODE_MASK);

                if (highByte > 0) this->_keyCodeOutput.push_back(highByte  | UNICODE_MASK);
            }
            else {
                this->_keyCodeOutput.push_back(charCode);
            }
        }
        else {
            this->_keyCodeOutput.push_back(word[i]->keyCode);
        }
    }
    
    // prepare keystroke to be sent
//    for(int i = 0; i < numDelete; i ++)
//        this->_keyCodeOutput.push_back(KEY_DELETE);
//
//    for (int i = startPos; i < endPos; i ++) {
//        if (this->_buffer[i].processed)
//            continue;
//
//        auto charCode = _getCharacterCode(this->_buffer[i]);
//        //LOG_DEBUG("_getCharacterCode: %d", charCode);
//
//        auto charType = codeTableList[this->currentCodeTable][CODE_TABLE_CHAR_TYPE][0];
//
//        if (charCode > 0) {
//            if (charType == 2) {
//                charCode = charCode ^ UNICODE_MASK;
//
//                auto highByte = BYTE_HIGH(charCode);
//                auto lowByte = BYTE_LOW(charCode);
//
//                LOG_DEBUG("process high: %20X, low: %20X", highByte, lowByte);
//
//                this->_keyCodeOutput.push_back(lowByte | UNICODE_MASK);
//
//                if (highByte > 0) this->_keyCodeOutput.push_back(highByte  | UNICODE_MASK);
//            }
//            else {
//                this->_keyCodeOutput.push_back(charCode);
//            }
//        }
//        else {
//            this->_keyCodeOutput.push_back(this->_buffer[i].keyCode);
//        }
//    }
}

int kbengine::_calculateNumberOfBackSpaceV2(const vector<BufferEntry*> &word, int startIdx)
{
    unsigned long wordSize = word.size();
    
    LOG_DEBUG("calculateNumberOfBackSpace - start %d, end %lu", startIdx, wordSize);

    int numBackSpace = 0;
    for (unsigned long i = startIdx; i < wordSize; i++) {
        BufferEntry *entry = word[i];
        
        if (entry->processed)
            continue;
        
        numBackSpace += 1;
        
        auto charCode = _getCharacterCode(*entry);
        
        if (charCode > 0) {
            charCode = charCode ^ UNICODE_MASK;  //remove unicode mask
            
            auto charType = codeTableList[this->currentCodeTable][CODE_TABLE_CHAR_TYPE][0];
            UInt8 highByte = BYTE_HIGH(charCode);
            
            LOG_DEBUG("CharCode: %04X, %02X - charType: %d", charCode, highByte, charType);

            //currentCodeTable is double byte
            if ( charType == 2 && highByte > 0) {
                LOG_DEBUG("Add more backspace");
                numBackSpace += 1;
            }
        }
    }
    LOG_DEBUG("calculateNumberOfBackSpace, result: %d", numBackSpace);
    return numBackSpace;
}

int kbengine::_calculateNumberOfBackSpace(int startIdx, int endIdx)
{
    vector<BufferEntry*> word = extractWord();
    
    unsigned long wordSize = word.size();
    
    LOG_DEBUG("calculateNumberOfBackSpace - start %d, end %lu", startIdx, wordSize);

    int numBackSpace = 0;
    for (unsigned long i = startIdx; i < wordSize; i++) {
        BufferEntry *entry = word[i];
        
        if (entry->processed)
            continue;
        
        numBackSpace += 1;
        
        auto charCode = _getCharacterCode(*entry);
        
        if (charCode > 0) {
            charCode = charCode ^ UNICODE_MASK;  //remove unicode mask
            
            auto charType = codeTableList[this->currentCodeTable][CODE_TABLE_CHAR_TYPE][0];
            UInt8 highByte = BYTE_HIGH(charCode);
            
            LOG_DEBUG("CharCode: %04X, %02X - charType: %d", charCode, highByte, charType);

            //currentCodeTable is double byte
            if ( charType == 2 && highByte > 0) {
                LOG_DEBUG("Add more backspace");
                numBackSpace += 1;
            }
        }
    }
    LOG_DEBUG("calculateNumberOfBackSpace, result: %d", numBackSpace);
    return numBackSpace;
}

vector<UInt32> kbengine::getOutputBuffer()
{
    return this->_keyCodeOutput;
}

void kbengine::setUseModernTone(bool isUse)
{
    _useModernTone = isUse;
}

bool kbengine::isUseModernTone()
{
    return _useModernTone;
}

void kbengine::_addKeyCodeV2(const UInt16 &keycode, const UInt8 &shiftCap, const bool processed)
{
    if (this->_bufferSize == MAX_BUFF) {
        // need to remove all word except the last one
        UInt16 wordSize = this->_bufferSize - this->_bufferStartWordIdx;
        
        if (wordSize < MAX_BUFF) {
            for(int i = this->_bufferStartWordIdx; i < this->_bufferSize; i ++) {
                int buffIndex = i - this->_bufferStartWordIdx;
                this->_buffer[buffIndex] = this->_buffer[i];
            }
        }
        else {
            wordSize = 0;
        }
        this->_bufferStartWordIdx = 0;
        this->_bufferSize = wordSize;
        
        LOG_DEBUG("Moved current: %d, size: %d", this->_bufferStartWordIdx, wordSize);
    }
    
    BufferEntry e = { keycode };
    e.cap = shiftCap > 0 ? true : false;
    e.processed = processed;
    this->_buffer[this->_bufferSize] = e;
    this->_bufferSize++;
}

void kbengine::_addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap, bool processed) {
    if (this->_bufferSize == MAX_BUFF) {
        // need to remove all word except the last one
        UInt16 wordSize = this->_bufferSize - this->_bufferStartWordIdx;
        
        if (wordSize < MAX_BUFF) {
            for(int i = this->_bufferStartWordIdx; i < this->_bufferSize; i ++) {
                int buffIndex = i - this->_bufferStartWordIdx;
                this->_buffer[buffIndex] = this->_buffer[i];
            }
        }
        else {
            wordSize = 0;
        }
        this->_bufferStartWordIdx = 0;
        this->_bufferSize = wordSize;
        
        LOG_DEBUG("Moved current: %d, size: %d", this->_bufferStartWordIdx, wordSize);
    }
    
    BufferEntry e = { keycode };
    e.cap = shiftCap > 0 ? true : false;
    e.processed = processed;
    this->_buffer[this->_bufferSize] = e;
    this->_bufferSize++;
    
    // Check if we start new word
    if (!processed && std::find (_wordBreakCode.begin(), _wordBreakCode.end(), keycode) != _wordBreakCode.end())
    {
        this->_startNewWord();
    }
}

int kbengine::process(const UInt16 &charCode, const UInt16 &keycode, const UInt8 &shiftCap, const bool &otherControl)
{
    this->_keyCodeOutput.clear();
    bool printable = charCode >= 32 && charCode <= 127;
    
    LOG_DEBUG("KeyCode: %d - charCode: %c, shiftCap: %d, otherControl: %d ", keycode, UInt8(charCode), shiftCap, otherControl);
    
    BufferEntry *lastEntry = NULL;
    vector<BufferEntry*> word;
    if (printable) {
        _addKeyCodeV2(keycode, shiftCap);
        lastEntry = &(_buffer[_bufferSize-1]);
        word = extractWord();
        word.pop_back();
    }
    else {
        if (keycode == KEY_DELETE) {
            if (shiftCap > 0 || otherControl) {
                this->resetBuffer();
            }
            else {
                //lastEntry = &(_buffer[_bufferSize-1]);
                this->_processBackSpacePressedV2();
                
                lastEntry = &(_buffer[_bufferSize-1]);
                word = extractWord();
            }
        }
    }
    LOG_DEBUG("Buffer beginIdx: %d, size: %d,", _bufferStartWordIdx, _bufferSize);
    
//    for (int i = _bufferStartWordIdx; keycode == KEY_DELETE && i < _bufferSize; i++) {
//        BufferEntry *entry = &(_buffer[i]);
//        entry->roofType = RoofType::ORIGIN;
//        entry->tone = Tone0;
//    }

    if (lastEntry != NULL) {
        _processWord(word, lastEntry, shiftCap, otherControl);
        if (keycode == KEY_DELETE) {
            _keyCodeOutput.insert(_keyCodeOutput.begin(), KEY_DELETE);
        }
    }
    
    //TODO: start new word?
    return 0;
}

void kbengine::_processWord(vector<BufferEntry*> word, BufferEntry* lastEntry, const UInt8 &shiftCap, const bool &otherControl)
{
    int actionResult = -1;
    
    if (word.size() > 0) {
        //BufferEntry *lastEntry = word[word.size() - 1];
        UInt16 keycode = lastEntry->keyCode;
        
        auto action = InputMethodMapping[this->_currentInputMethod].find(keycode);
        LOG_DEBUG("keycode: %d, action found: %d", keycode, action != InputMethodMapping[_currentInputMethod].end());
        
        KeyEvent result = Normal;
        if (shiftCap == 0 && !otherControl && action != InputMethodMapping[_currentInputMethod].end())
        {
            result = (KeyEvent) action->second;
            LOG_DEBUG("Action: %d", result);
            
            switch (result) {
                case RoofAll:
                    actionResult = this->_processMarkV2(word, lastEntry, RoofType::ROOF);
                    break;
                default:
                    break;
            }
        }
    }
    if (actionResult >= 0) {
        auto foundIdx = actionResult >= 0 ? actionResult : 0;
        int numBackSpaces = _calculateNumberOfBackSpaceV2(word, foundIdx);

        LOG_DEBUG("Process Mark, numBackSpaces: %d, foundIdx: %d", numBackSpaces, foundIdx);

//        if (fromCorrectFunc)
//            numBackSpaces --;
        auto newWord = extractWord();
        this->_processKeyCodeOutputV2(newWord, numBackSpaces, foundIdx);
    }
}

//int kbengine::process(const UInt16 &charCode, const UInt16 &keycode, const UInt8 &shiftCap, const bool &otherControl)
//{
//    this->_keyCodeOutput.clear();
//
//    LOG_DEBUG("KeyCode: %d - charCode: %c, shiftCap: %d, otherControl: %d ", keycode, UInt8(charCode), shiftCap, otherControl);
//
//    auto action = InputMethodMapping[this->_currentInputMethod].find(keycode);
//
//    KeyEvent result = Normal;
//    int actionResult = -1;
//    if (shiftCap == 0 && !otherControl && action != InputMethodMapping[_currentInputMethod].end())// && this->_bufferSize > 0)
//    {
//        result = (KeyEvent) action->second;
//        LOG_DEBUG("Action: %d", result);
//
//        switch (result) {
//            case RoofAll:
//                actionResult = this->_processMark(keycode, RoofType::ROOF);
//                break;
//            case RoofA:
//                actionResult = this->_processMark(keycode, RoofType::ROOF_A);
//                break;
//            case RoofO:
//                actionResult = this->_processMark(keycode, RoofType::ROOF_O);
//                break;
//            case RoofE:
//                actionResult = this->_processMark(keycode, RoofType::ROOF_E);
//                break;
//            case HookAll:
//                actionResult = this->_processMark(keycode, RoofType::HOOK);
//                break;
//            case Breve:
//                actionResult = this->_processMark(keycode, RoofType::BREVE);
//                break;
//            case HookO:
//                actionResult = this->_processHookOU(keycode, KEY_O);
//                break;
//            case HookU:
//                actionResult = this->_processHookOU(keycode, KEY_U);
//                break;
//            case Dd:
//                actionResult = this->_processD(keycode);
//                break;
//            case Tone0:
//            case Tone1:
//            case Tone2:
//            case Tone3:
//            case Tone4:
//            case Tone5:
//                actionResult = this->_processTone(keycode, result);
//                break;
//            case EscChar:
//                this->resetBuffer();
//                break;
//            default:
//                result = Normal;
//                break;
//        }
//    }
//
//    bool printable = charCode >= 32 && charCode <= 127;
//
//    // keyCode already processed
//    if (result != Normal && ACTION_PROCESSED(actionResult) && this->_keyCodeOutput.size() > 0) {
//        LOG_DEBUG("Processed, Begin Idx: %d, EndIdx: %d", this->_bufferStartWordIdx, this->_bufferSize);
//        return 1;
//    }
//
//    LOG_DEBUG("Character Printable: %d", printable);
//    if (printable && !otherControl) {
//        _addKeyCode(keycode, shiftCap);
//        _correctTone(keycode);
//        _correctMark(keycode);
//    }
//    else {
//        // Check delete key
//        if (keycode == KEY_DELETE) {
//            if (shiftCap > 0 || otherControl) {
//                this->resetBuffer();
//            }
//            else {
//                this->_processBackSpacePressed();
//            }
//        }
//        else if (std::find(_wordBreakCode.begin(), _wordBreakCode.end(), keycode) == _wordBreakCode.end()) {
//            // if current keycode is not in list allow new word, we reset buffer
//            LOG_DEBUG("Ignore process keycode, reset buffer");
//            this->resetBuffer();
//        }
//    }
//
//    LOG_DEBUG("Begin Idx: %d, EndIdx: %d", this->_bufferStartWordIdx, this->_bufferSize);
//
//	return 0;
//}


void kbengine::resetBuffer()
{
    memset(this->_buffer, MAX_BUFF, sizeof(BufferEntry));
    this->_bufferStartWordIdx = 0;
    this->_bufferSize = 0;
}

UInt32 kbengine::_getCharacterCode(const BufferEntry& entry )
{
    UInt16 keyCode = entry.keyCode;
    
    if (entry.roofType == ROOF)
        keyCode = keyCode | MASK_ROOF;
    else if (entry.roofType == HOOK)
        keyCode = keyCode | MASK_HOOK;

    auto codeTable = codeTableList.at(this->currentCodeTable);

    if (codeTable.find(keyCode) != codeTable.end()) {
        vector<UInt16> charList = codeTable[keyCode];
        int index = (entry.tone - KeyEvent::Tone0) * 2 + (entry.cap ? 0 : 1);
        
        UInt32 result = index >= 0 && charList.size() > 0 ? charList[index] | UNICODE_MASK : keyCode;
        
        return result;
    }
    
    return 0;
}

UInt8 kbengine::_getCurrentCodeTableCharType()
{
    return codeTableList[this->currentCodeTable][CODE_TABLE_CHAR_TYPE][0];
}

void kbengine::_processBackSpacePressedV2()
{
    if (this->_bufferSize == 0) return;
    this->_bufferSize--;
    
    // Reupdate start position of word
    if (this->_bufferSize <= this->_bufferStartWordIdx) {
        this->_bufferStartWordIdx = 0;

        // Find the start position of the word
        for (int i = this->_bufferSize - 1; i > 0; i--) {
            if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), this->_buffer[i].keyCode) != _wordBreakCode.end()) {
                this->_bufferStartWordIdx = i + 1;
                break;
            }
        }
    }
}

void kbengine::_processBackSpacePressed()
{
    // remove all keycode marked processed (key tone, roof, hook,...)
    while(this->_buffer[_bufferSize-1].processed && this->_bufferSize > 0) {
        this->_bufferSize--;
    }
    
    if (this->_bufferSize == 0) return;
    
    // create output to send delete in case 2 bytes
    // case 1 byte we let system process
    auto charCode = this->_getCharacterCode(this->_buffer[this->_bufferSize - 1]);
    
    int addDelete = 0;
    if (_getCurrentCodeTableCharType() == 2 && (BYTE_HIGH(charCode) > 0)) {
        addDelete = 2;
    }
    
    this->_bufferSize--;
    
    // Reupdate start position of word
    if (this->_bufferSize <= this->_bufferStartWordIdx) {
        this->_bufferStartWordIdx = 0;

        // Find the start position of the word
        for (int i = this->_bufferSize - 1; i > 0; i--) {
            if (this->_buffer[i].processed)
                continue;
            if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), this->_buffer[i].keyCode) != _wordBreakCode.end()) {
                this->_bufferStartWordIdx = i + 1;
                break;
            }
        }
    }
    
    LOG_DEBUG("Number Delete: %d", addDelete);
    if (addDelete > 0)
        _processKeyCodeOutput(addDelete, _bufferSize); //we pass _bufferSize to process delete only
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

UInt8 kbengine::getInputMethod()
{
    return this->_currentInputMethod;
}

void kbengine::setActiveCodeTable(const UInt8 &codeTableNumber)
{
    if (codeTableNumber >= codeTableList.size()) {
        this->currentCodeTable = 0;
    }
    else {
        this->currentCodeTable = codeTableNumber;
    }
    LOG_DEBUG("Selected Code Table Index: %d", codeTableNumber);
}

UInt8 kbengine::getCurrentCodeTable()
{
    return this->currentCodeTable;
}

UInt8 kbengine::getTotalCodeTable()
{
    return codeTableList.size();
}

void kbengine::addCodeTable(const unsigned short &charType, const map<std::string, vector<UInt16>> &codeTableRaw)
{
    LOG_DEBUG("codeTable items: %ld, type: %d", codeTableRaw.size(), charType);
    map<UInt32, vector<UInt16>> codeTable;
    
    for (auto const& mapItem : codeTableRaw) {
        auto key = mapItem.first;
        auto values = mapItem.second;
        //PRINT_VECTOR(values);
        auto keyChar = key[0];
        auto keyNum = '0';
        if (key.size() > 1) keyNum = key[1];
        if (vowelToKeyCode.find(keyChar) != vowelToKeyCode.end() && values.size() > 0) {
            UInt32 keyCode = vowelToKeyCode.at(keyChar);
            keyCode |= (keyNum == '1') ? MASK_ROOF : ((keyNum == '2') ? MASK_HOOK: 0);
//            LOG_DEBUG("key %c, %c - keyCode %u", keyChar, keyNum, keyCode);
            
            codeTable.insert(std::pair<UInt32, vector<UInt16>>(keyCode, values));
        }
    }
    
    if (codeTable.size() > 0) {
        codeTable.insert(std::pair<UInt32, vector<UInt16>>(CODE_TABLE_CHAR_TYPE, { charType }));
        codeTableList.push_back(codeTable);
        LOG_INFO("Code Table added, character encoding count: %lu", codeTableList.size());
    }
}
