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

#define IS_BREAKCODE(x) (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), x) != _wordBreakCode.end())
#define IS_IM_CODE(x) (InputMethodMapping[this->_currentInputMethod].find(x) != InputMethodMapping[_currentInputMethod].end())
#define IS_DOUBLE_CHAR_TYPE(x) (codeTableList[x][CODE_TABLE_CHAR_TYPE][0] > 1)

#define IS_SHIFT_PRESSED(x) ((x & 1) == 1)

enum InputMethodType {
    VNI,
    SimpleTelex,
    Telex,
    
    InputMethodTypeCount
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
        {KEY_8, Breve},
        {KEY_9, Dd},
        {KEY_ESC, EscChar},
        {KEY_EMPTY, Normal}
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
        {KEY_EMPTY, Normal}
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
        {KEY_EMPTY, Normal}
    }
};

// map tone to key mapping, will be init when we change input method
static map<int, UInt16> ToneToKeyCodeMapping = {};

bool checkWord(const vector<BufferEntry*> &word)
{
    // find the fist vowel
    int startVowel = -1;
    
    vector<BufferEntry*> wordTrimmed;
    
    for(int i=0; i < word.size(); i++) {
        if (!IS_BREAKCODE(word[i]->keyCode)) {
            wordTrimmed.push_back(word[i]);
        }
    }
    
    if (wordTrimmed.size() == 0)
        return false;
    
    // check consonant at first of word
    bool isFoundConsonant = false;
    vector<UInt16> foundConsonant;
    for (int i=0; i < consonantList.size(); i++) {
        vector<UInt16> consonant = consonantList[i];
        
        if (consonant.size() > foundConsonant.size()) {
            isFoundConsonant = true;
            for (int j = 0; j < consonant.size(); j++) {
                if (consonant[j] != wordTrimmed[j]->keyCode) {
                    isFoundConsonant = false;
                    break;
                }
            }
            if (isFoundConsonant) {
                foundConsonant = consonant;
            }
        }
    }
    
    LOG_DEBUG("Found consonant, size: %lu", foundConsonant.size());
    PRINT_VECTOR(foundConsonant);
    
    if (wordTrimmed.size() == foundConsonant.size()) {
        return true; // return true since this word only have consonant
    }
    
    if (wordTrimmed.size() > foundConsonant.size()) {
        if (IS_VOWEL(wordTrimmed[foundConsonant.size()]->keyCode)) {
            startVowel = (int)foundConsonant.size();
        }
    }
    
    LOG_DEBUG("Start vowel: %d", startVowel);

    if (startVowel == -1)
        return false;
    
    BufferEntry *firstEntry = wordTrimmed[startVowel];
    int expectedSize = (int) wordTrimmed.size() - startVowel;
    
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
                    BufferEntry *curChar = wordTrimmed[(i-1) + startVowel];
                    LOG_DEBUG("expected keycode %2X, syllabel: %2X", curChar->keyCode, syllable[i]);
                    if (curChar->keyCode != syllable[i]) {
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

vector<BufferEntry*> kbengine::getCurrentWord()
{
    vector<BufferEntry*> word;
    
    int endIdx = this->_bufferSize;
    for(int i = this->_bufferStartWordIdx; i < endIdx ; i++)
    {
        if (this->_buffer[i].processed) continue;
        word.push_back(&(this->_buffer[i]));
    }
    
    LOG_DEBUG("Get Current Word, size: %lu", word.size());
    
    return word;
}

/*
 * Copy of the current word
 */
vector<BufferEntry> kbengine::extractCurrentWord()
{
    vector<BufferEntry> word;

    int endIdx = this->_bufferSize;
    for(int i = this->_bufferStartWordIdx; i < endIdx ; i++)
    {
        if (this->_buffer[i].processed) continue;
        word.push_back(this->_buffer[i]);
    }

    LOG_DEBUG("Extract Current Word, size: %lu", word.size());

    return word;
}

int kbengine::_findSyllableV2(const vector<BufferEntry*> &word,
                              vector<UInt16> &syllableCombine,
                              const UInt16 &expectedType, const UInt16 &expectedKey)
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

ProcessResult kbengine::_processDV2(const vector<BufferEntry*> &word, const UInt8 &keycode) {
    int foundIdx = -1;
    vector<UInt16> syllableCombine;
    
    ProcessResult result;
    
    // check the previous keycode is d or not
    for(int i = (int)word.size() - 1; i >= 0; i --) {
        BufferEntry *entry = word[i];
        
        if (entry->keyCode == KEY_D) {
            foundIdx = i;
            break;
        }
    }
    
    if (foundIdx >= 0) {
        if (word[foundIdx]->roofType == ROOF) {
            word[foundIdx]->roofType = ORIGIN;
            result.processed = false;
        }
        else {
            word[foundIdx]->roofType = ROOF;
            result.processed = true;
        }

        result.startPosition = foundIdx;
    }
    
    return result;
}

int kbengine::_placeToneTraditionalRuleV2(const vector<BufferEntry*> &word, int foundIdx, vector<UInt16> syllableCombine)
{
    int tonePosition = -1;
    
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
        if ((vowelCount == 2 && lastIsVowel) && foundIdx > 0) {
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

int kbengine::_placeToneModernRuleV2(const vector<BufferEntry*> &word, int foundIdx, vector<UInt16> syllableCombine)
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
    
    for(int i = foundIdx; i < word.size(); i++) {
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
    else {
        //Rule ALL:  if word has any ROOF, we place tone at that position
        for(int i = (int)word.size() - 1; i >= 0; i--) {
            BufferEntry *pEntry = word[i];
            if (pEntry->roofType == RoofType::ROOF || pEntry->roofType == RoofType::HOOK) {
                return i;
            }
        }
    }
    
    LOG_DEBUG("Tone position: %d", tonePosition);
    
    
    return tonePosition;
}


// This handle hook O/U, specialize for Telex
ProcessResult kbengine::_processHookOUV2(const vector<BufferEntry*> word,
                                         const UInt8 &keycode, const UInt8 &shiftCap,
                                         const UInt16 &expectedKey)
{
    LOG_DEBUG("_processHookOU, keycode: %d, buffSize: %d", keycode, this->_bufferSize);
    
    ProcessResult result;
    
    // find previous char have keycode or not
    if (word.size() > 0) {
        int curIndex = (int)word.size() - 1;
        BufferEntry *prevEntry = word[curIndex];
        
        // if previous already have u+/o+, we need to remove it and replace with  ']' or '['
        if (prevEntry->keyCode == expectedKey && prevEntry->roofType == RoofType::HOOK) {
            // delete previous u+ or o+
            while(_bufferSize > 0) {
                int idx = _bufferSize-1;
                _bufferSize--;

                LOG_DEBUG(">>> Delete char");
                if (!_buffer[idx].processed)
                    break;
            }

            result.startPosition = curIndex;
            result.processed = false;
        }
    }
    
    // add u+/o+
    if (result.startPosition < 0) {
        auto imMap = InputMethodMapping[_currentInputMethod];
        UInt16 hookAllKey = KEY_EMPTY;
        for (const auto& [key, value] : imMap) {
            if (value == HookAll) {
                hookAllKey = key;
                break;
            }
        }
        // add key o / u
        BufferEntry e = { expectedKey };
        e.cap = shiftCap > 0 ? true : false;
        e.roofType = RoofType::HOOK;
        _buffer[_bufferSize] = e;
        _bufferSize++;
        
        // add hook o / u
        BufferEntry hookEntry = { hookAllKey };
        hookEntry.cap = false;
        hookEntry.processed = true;
        hookEntry.isIMCode = true;
        _buffer[_bufferSize] = hookEntry;
        _bufferSize++;
        
        result.startPosition = _bufferStartWordIdx;
        result.processed = true;
        result.ignoreKeyCode = true;
    }
    
    
    return result;
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
            if (word[i]->cap) {
                // if keyCode not in codeTable, it is original keyCode,
                // we need to check Shift state and add mask for further process by caller
                this->_keyCodeOutput.push_back(word[i]->keyCode | SHIFT_MASK);
            }
            else {
                this->_keyCodeOutput.push_back(word[i]->keyCode);
            }
        }
    }
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

vector<UInt32> kbengine::getOutputBuffer()
{
    return this->_keyCodeOutput;
}

void kbengine::setUseModernTone(bool isUse)
{
    _useModernTone = isUse;
    
    this->resetBuffer();
}

bool kbengine::isUseModernTone()
{
    return _useModernTone;
}

void kbengine::setAutoRestoreWord(bool isEnable)
{
    isEnableAutoRestoreWord = isEnable;
    this->resetBuffer();
}

void kbengine::_addKeyCodeV2(const UInt16 &keycode, const UInt8 &shiftCap,
                             const bool &processed, const bool &isBreakCode)
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
    e.isIMCode = IS_IM_CODE(keycode);
    e.isBreakCode = IS_BREAKCODE(keycode);
    this->_buffer[this->_bufferSize] = e;
    this->_bufferSize++;
}

//void kbengine::_addKeyCode(const UInt8 &keycode, const UInt8 &shiftCap, bool processed) {
//    if (this->_bufferSize == MAX_BUFF) {
//        // need to remove all word except the last one
//        UInt16 wordSize = this->_bufferSize - this->_bufferStartWordIdx;
//        
//        if (wordSize < MAX_BUFF) {
//            for(int i = this->_bufferStartWordIdx; i < this->_bufferSize; i ++) {
//                int buffIndex = i - this->_bufferStartWordIdx;
//                this->_buffer[buffIndex] = this->_buffer[i];
//            }
//        }
//        else {
//            wordSize = 0;
//        }
//        this->_bufferStartWordIdx = 0;
//        this->_bufferSize = wordSize;
//        
//        LOG_DEBUG("Moved current: %d, size: %d", this->_bufferStartWordIdx, wordSize);
//    }
//    
//    BufferEntry e = { keycode };
//    e.cap = shiftCap > 0 ? true : false;
//    e.processed = processed;
//    this->_buffer[this->_bufferSize] = e;
//    this->_bufferSize++;
//    
//    // Check if we start new word
//    if (!processed && std::find (_wordBreakCode.begin(), _wordBreakCode.end(), keycode) != _wordBreakCode.end())
//    {
//        this->_startNewWord();
//    }
//}

// Only correct for the case uo
ProcessResult kbengine::_correctUO(const vector<BufferEntry*> &word, const UInt8 &keycode)
{
    bool foundPreviousHook = false;

    for (int i = 0; i < word.size(); i++) {
        BufferEntry *entry = word[i];

        if (entry->roofType == RoofType::HOOK) {
            foundPreviousHook = true;
            break;
        }
    }

    LOG_DEBUG("Correct uo+ -> u+o+, foundPreviousHook: %d", foundPreviousHook);

    int foundIdx = -1;
    ProcessResult result;

    if (foundPreviousHook) {
        vector<UInt16> matchCombine;
        foundIdx = this->_findSyllableV2(word, matchCombine, MASK_HOOK, KEY_U);

        // only process if match uo and any <char>
        if (matchCombine.size() >= 4) {
            result.startPosition = foundIdx;

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
            LOG_DEBUG("CorrectUO - Can set hook: %d, foundIdx: %d", canSetHook, foundIdx);

            if (canSetHook) {
                result.processed = true;
            }

        }
    }

    return result;
}



void kbengine::_restoreWordIfAny() {
    bool isValidWord = checkWord(getCurrentWord());
    LOG_DEBUG(">>>> Restore word if invalid. Is valid word: %d", isValidWord);
    
    if(isValidWord) return;
    
    // check if we need to restore, if word already in original form
    bool needRestore = false;
    for (int i = _bufferStartWordIdx; i < _bufferSize; i++) {
        BufferEntry *entry = &(_buffer[i]);
        if (entry->roofType != RoofType::ORIGIN || entry->tone != Tone0 || entry->processed) {
            needRestore = true;
            break;
        }
    }
    
    if (needRestore) {
        LOG_DEBUG(">>>> Restore word <<<<");
        
        ProcessResult result;
        result.startPosition = 0;
        
        vector<BufferEntry> origWord = extractCurrentWord();
        
        // This is the copy of word before it's processed by keystroke
        vector<BufferEntry*> pOrigWord;
        for (int i = 0; i < origWord.size(); i++) {
            if (!IS_BREAKCODE(origWord[i].keyCode))
                pOrigWord.push_back(&(origWord[i]));
        }
        
        for (int i = _bufferStartWordIdx; i < _bufferSize; i++) {
            BufferEntry *entry = &(_buffer[i]);
            entry->roofType = RoofType::ORIGIN;
            entry->tone = Tone0;
            entry->processed = false;
        }
        
        _processResult(pOrigWord, getCurrentWord(), result, 0);
    }
}

int kbengine::process(const UInt16 &charCode, const UInt16 &keycode, const UInt8 &shiftCap, const bool &otherControl)
{
    this->_keyCodeOutput.clear();
    
    bool printable = charCode >= 32 && charCode <= 127;
    
    LOG_DEBUG("KeyCode: %d - charCode: %c, shiftCap: %d, otherControl: %d ", keycode, UInt8(charCode), shiftCap, otherControl);
    
    vector<BufferEntry*> pCurrentWord = getCurrentWord();
    vector<BufferEntry> origWord = extractCurrentWord();
    
    // This is the copy of word before it's processed by keystroke
    vector<BufferEntry*> pOrigWord;
    for (int i = 0; i < origWord.size(); i++) {
        pOrigWord.push_back(&(origWord[i]));
    }
    
    if (!otherControl && printable) {
        auto result = _processWord(pCurrentWord, keycode, shiftCap, otherControl);
        LOG_DEBUG("Process Result: %d, %d", result.startPosition, result.processed);
        
        if (!result.ignoreKeyCode) {
            _addKeyCodeV2(keycode, shiftCap, result.processed);
            
            ProcessResult ret2;
            
            // correct tone
            if (!IS_IM_CODE(keycode)) {
                pCurrentWord = getCurrentWord();
                ret2 = _correctToneV2(pCurrentWord, keycode);
                if (ret2.processed && ret2.startPosition >= 0) {
                    result = ret2;
                }
            }

            // correct uo+ -> u+o+
            ret2 = _correctUO(pCurrentWord, keycode);
            if (ret2.processed && ret2.startPosition >= 0) {
                result = ret2;
            }
        }
        
        if (result.startPosition >= 0) {
            _processResult(pOrigWord, pCurrentWord, result, 0);
        }
        
        // Check if we start new word
        //if (!result.ignoreKeyCode && !result.processed && IS_BREAKCODE(keycode)) {
        if (!result.processed && IS_BREAKCODE(keycode)) {
            
            if (isEnableAutoRestoreWord) {
                _restoreWordIfAny();
            }
            
            _startNewWord();
        }
    }
    else {  // keycode is not printable
        if (keycode == KEY_DELETE) {
            if (shiftCap > 0 || otherControl) {
                this->resetBuffer();
            }
            else {
                auto lastCharDel = processDelete1(pOrigWord);
                
                if (lastCharDel != NULL && _getCurrentCodeTableCharType() > 1) {
                    auto charCode = _getCharacterCode(*(lastCharDel));
                    if (charCode > 0) {
                        charCode = charCode ^ UNICODE_MASK;  //remove unicode mask

                        UInt8 highByte = BYTE_HIGH(charCode);

                        if ( highByte > 0) {
                            // delete 2 bytes char, we need to send 2 DELETE KEY
                            ProcessResult result;
                            result.startPosition = (int)pOrigWord.size() - 1;
                            _processResult(pOrigWord, pCurrentWord, result, 0);
                        }
                    }
                }
            }
        }
        else {
            // Check if we start new word
            if (IS_BREAKCODE(keycode)) {
                _addKeyCodeV2(keycode, shiftCap, true, true);
                
                if (isEnableAutoRestoreWord) {
                    _restoreWordIfAny();
                }
                
                _startNewWord();
            }
            else {
                resetBuffer();
            }
        }
    }
    
    LOG_DEBUG("Current Buffer: beginIdx: %d | Size: %d,", _bufferStartWordIdx, _bufferSize);
    
    return 0;
}

BufferEntry* kbengine::processDelete1(const vector<BufferEntry*> &pOrigWord) {
    if (this->_bufferSize == 0)
        return NULL;
    
    BufferEntry* deleted = &(_buffer[this->_bufferSize - 1]);
    
    while (deleted->processed && !deleted->isBreakCode) {
        this->_bufferSize--;
        
        deleted = &(_buffer[this->_bufferSize - 1]);
    }
    
    // last char deleted
    deleted = &(_buffer[this->_bufferSize - 1]);
    
    this->_bufferSize--;
    
    // Reupdate start position of word
    if (this->_bufferSize <= this->_bufferStartWordIdx) {
        this->_bufferStartWordIdx = 0;

        // Find the start position of the word
        for (int i = this->_bufferSize - 1; i > 0; i--) {
            if (this->_buffer[i].processed)
                continue;
            
            if (IS_BREAKCODE(this->_buffer[i].keyCode)) {
                this->_bufferStartWordIdx = i + 1;
                break;
            }
        }
        
        deleted = NULL;
    }
    
    return deleted;
}

void kbengine::processDelete2(const vector<BufferEntry*> &pOrigWord) {
    BufferEntry *processEntry = this->_processBackSpacePressedV2();
    
    int addDelete = 0;
    if (processEntry != NULL) {
        if (processEntry->isIMCode && !processEntry->processed) {
            // Find previous processed keycode to process
            for (int i = this->_bufferSize - 1; i > _bufferStartWordIdx; i--) {
                if (this->_buffer[i].processed && this->_buffer[i].keyCode == processEntry->keyCode) {
                    processEntry = &(_buffer[i]);
                    addDelete = 1;
                }
            }
        }
        
        if (processEntry->processed && processEntry->isIMCode && _bufferSize > 0) {
            vector<BufferEntry*> pCurrentWord = getCurrentWord();
            
            auto result = _processWord(pCurrentWord, processEntry->keyCode, processEntry->cap, false);
            
            if (result.startPosition >= 0) {
                _processResult(pOrigWord, pCurrentWord, result, 0); //result.adjustDelete + addDelete);
            }
        }
    }
}

void kbengine::_processResult(vector<BufferEntry*> pOldWord,
                              vector<BufferEntry*> word,
                              ProcessResult result, int adjustDelete)
{
    int numBackSpaces = _calculateNumberOfBackSpaceV2(pOldWord, result.startPosition);
    LOG_DEBUG("Process Output, numBackSpaces: %d, from index: %d", numBackSpaces, result.startPosition);
    vector<BufferEntry*> newWord = getCurrentWord();
    this->_processKeyCodeOutputV2(newWord, numBackSpaces + adjustDelete, result.startPosition);
}

ProcessResult kbengine::_processWord(vector<BufferEntry*> word, const UInt16 &keycode, const UInt8 &shiftCap, const bool &otherControl)
{
    ProcessResult processResult;
    
    if (word.size() >= 0) {
        auto action = InputMethodMapping[this->_currentInputMethod].find(keycode);
        LOG_DEBUG("keycode: %d, action found: %d", keycode, action != InputMethodMapping[_currentInputMethod].end());
        
        KeyEvent result = Normal;
        if (!IS_SHIFT_PRESSED(shiftCap) && !otherControl && action != InputMethodMapping[_currentInputMethod].end())
        {
            result = (KeyEvent) action->second;
            LOG_DEBUG("Action: %d", result);
            
            switch (result) {
                case RoofAll:
                    processResult = this->_processMarkV2(word, keycode, RoofType::ROOF);
                    break;
                case RoofA:
                    processResult = this->_processMarkV2(word, keycode, RoofType::ROOF_A);
                    break;
                case RoofO:
                    processResult = this->_processMarkV2(word, keycode, RoofType::ROOF_O);
                    break;
                case RoofE:
                    processResult = this->_processMarkV2(word, keycode, RoofType::ROOF_E);
                    break;
                case HookAll:
                    processResult = this->_processMarkV2(word, keycode, RoofType::HOOK);
                    break;
                case Breve:
                    processResult = this->_processMarkV2(word, keycode, RoofType::BREVE);
                    break;
                case Dd:
                    processResult = this->_processDV2(word, keycode);
                    break;
                case HookO:
                    processResult = this->_processHookOUV2(word, keycode, shiftCap, KEY_O);
                    break;
                case HookU:
                    processResult = this->_processHookOUV2(word, keycode, shiftCap, KEY_U);
                    break;
                case Tone0:
                case Tone1:
                case Tone2:
                case Tone3:
                case Tone4:
                case Tone5:
                    processResult = this->_processToneV2(word, keycode, result);
                    break;
                case EscChar:
                    this->resetBuffer();
                    break;
                default:
                    break;
            }
        }
    }
    
    return processResult;
}

ProcessResult kbengine::_processMarkV2(const vector<BufferEntry*> &word,
                             const UInt16 &keycode,
                             const RoofType &roofType, const bool &fromCorrectFunc)
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
    
    ProcessResult result;
    int foundIdx = this->_findSyllableV2(word, matchCombine, maskType, expectedKey);
    
    LOG_DEBUG("FoundIdx: %d, matchCombine.size: %lu", foundIdx, matchCombine.size());
    
    if (matchCombine.size() >= 0 && foundIdx >= 0) {
        PRINT_VECTOR(matchCombine);

        bool canSetHook = false;
        
        // ignore the first element since it is category of combine
        for (int i = 1; i < matchCombine.size(); i++) {
            if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                int buffIndex = (i-1) + foundIdx;   // (i-1) since we ignore the first value of combine
                BufferEntry *entry = word[buffIndex];
                
                if (entry->roofType != targetRoofType) {
                    entry->roofType = targetRoofType;
                    canSetHook = true;
                }
            }
        }
        
        bool isReverse = false;
        // not set any hook, we reverse hook if this process is not from correct function
        // in correct function we don't reverse hook
        if (!canSetHook && !fromCorrectFunc) {
            for (int i = 1; i < matchCombine.size(); i++) {
                if (i == 1 || (matchCombine[i] & MASK_EXTRA_MARK) == MASK_EXTRA_MARK) {
                    int buffIndex = (i-1) + foundIdx;
                    BufferEntry *entry = word[buffIndex];
                    // update roofType
                    entry->roofType = ORIGIN;
                    isReverse = true;
                }
            }
        }
        
        // if not set hook and not reverse, mean this keycode cannot set mark
        // we set foundIdx to -1 to indicate this function ignore keycode
        if (!(canSetHook || isReverse)) {
            foundIdx = -1;
        }
        
        result.startPosition = foundIdx;
        
        if (canSetHook) {
            result.processed = true;
        }
        else if (isReverse) {
            result.processed = false;
        }
    }
    
    return result;
}


ProcessResult kbengine::_processToneV2(const vector<BufferEntry*> &word, const UInt8 &keycode,
                                       const KeyEvent &tone, const bool &fromCorrectFunc)
{
    ProcessResult result;
    vector<UInt16> syllableCombine;
    
    vector<BufferEntry*> wordTrimmed;
    //Remove breakcode in the word (keep only A-z)
    for (int i=0; i < word.size(); i++) {
        if (IS_BREAKCODE(word[i]->keyCode)) continue;
        wordTrimmed.push_back(word[i]);
    }
    
    int foundIdx = this->_findSyllableV2(wordTrimmed, syllableCombine, MASK_ORIGIN | MASK_ROOF | MASK_HOOK);
    
    if (syllableCombine.size() >= 0) {
        int tonePosition = _useModernTone ? _placeToneModernRuleV2(wordTrimmed, foundIdx, syllableCombine)
                                        : _placeToneTraditionalRuleV2(wordTrimmed, foundIdx, syllableCombine);

        if (tonePosition >= 0) {
            LOG_DEBUG("Target tone posision: %d", tonePosition);
            
            KeyEvent targetTone = tone;
            bool isCaseReset = false;
            
            // check target tonePosision have tone or not
            //if (this->_buffer[tonePosition].tone == tone && !fromCorrectFunc) {
            if (wordTrimmed[tonePosition]->tone == tone && !fromCorrectFunc) {
                LOG_DEBUG("Found previous tone at target posision: %d", tonePosition);
                isCaseReset = true;
            }
            
            int previousTonePos = -1;
            // check whole word to detect previous tone -> this case is move tone
            for (auto i = 0; i < wordTrimmed.size(); i++) {
                LOG_DEBUG("Buffer index: %d, tone: %d", i, wordTrimmed[i]->tone);
                if (wordTrimmed[i]->tone != KeyEvent::Tone0) {
                    previousTonePos = i;
                    break;
                }
            }
            
            if (isCaseReset) {
                wordTrimmed[tonePosition]->tone = KeyEvent::Tone0;
                result.processed = false;
            }
            else {
                wordTrimmed[tonePosition]->tone = targetTone;
                result.processed = true;
            }
            
            if (previousTonePos >= 0 && previousTonePos != tonePosition) {
                wordTrimmed[previousTonePos]->tone = KeyEvent::Tone0;
                tonePosition = previousTonePos;
            }

            result.startPosition = tonePosition;
        }
    }
    
    return result;
}

/*
 * - get the tone of last word.
 * - call _processToneTraditional or _processToneNew to reprocess tone
 * Notes: only call this func if new char is Vowel
 */
ProcessResult kbengine::_correctToneV2(const vector<BufferEntry*> &word, const UInt8 &keycode)
{
    short currentTone = KeyEvent::Tone0;
    ProcessResult result;
    int startPos = -1;
    for (int i = 0; i < word.size(); i++) {
        BufferEntry *entry = word[i];

        if (entry->tone != KeyEvent::Tone0) {
            currentTone = entry->tone;
            startPos = i;
            break;
        }
    }
    
    if (currentTone != KeyEvent::Tone0) {
        UInt8 keyCode = ToneToKeyCodeMapping[currentTone];
        result = this->_processToneV2(word, keyCode, static_cast<KeyEvent>(currentTone), true);

        // check tone pos again
        int newTonePos = -1;
        for (int i = 0; i < word.size(); i++) {
            BufferEntry *entry = word[i];

            if (entry->tone != KeyEvent::Tone0) {
                currentTone = entry->tone;
                newTonePos = i;
                break;
            }
        }

        LOG_DEBUG("Correct Tone, keyCode = %d, curTonePos: %d, newTonePos: %d", keyCode, startPos, newTonePos);

        if (newTonePos < 0 || newTonePos == startPos) {
            result.processed = false;
            result.startPosition = -1;
        }
    }
    
    return result;
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

BufferEntry* kbengine::_processBackSpacePressedV2()
{
    if (this->_bufferSize == 0)
        return NULL;
    
    BufferEntry* deleted = &(_buffer[this->_bufferSize - 1]);
    
    this->_bufferSize--;
    
    // Reupdate start position of word
    if (this->_bufferSize <= this->_bufferStartWordIdx) {
        this->_bufferStartWordIdx = 0;

        // Find the start position of the word
        for (int i = this->_bufferSize - 1; i > 0; i--) {
            if (this->_buffer[i].processed)
                continue;
            
            if (IS_BREAKCODE(this->_buffer[i].keyCode)) {
                this->_bufferStartWordIdx = i + 1;
                break;
            }
        }
        
        deleted = NULL;
    }
    
    return deleted;
}
//
//void kbengine::_processBackSpacePressed()
//{
//    // remove all keycode marked processed (key tone, roof, hook,...)
//    while(this->_buffer[_bufferSize-1].processed && this->_bufferSize > 0) {
//        this->_bufferSize--;
//    }
//    
//    if (this->_bufferSize == 0) return;
//    
//    // create output to send delete in case 2 bytes
//    // case 1 byte we let system process
//    auto charCode = this->_getCharacterCode(this->_buffer[this->_bufferSize - 1]);
//    
//    int addDelete = 0;
//    if (_getCurrentCodeTableCharType() == 2 && (BYTE_HIGH(charCode) > 0)) {
//        addDelete = 2;
//    }
//    
//    this->_bufferSize--;
//    
//    // Reupdate start position of word
//    if (this->_bufferSize <= this->_bufferStartWordIdx) {
//        this->_bufferStartWordIdx = 0;
//
//        // Find the start position of the word
//        for (int i = this->_bufferSize - 1; i > 0; i--) {
//            if (this->_buffer[i].processed)
//                continue;
//            if (std::find (_wordBreakCode.begin(), _wordBreakCode.end(), this->_buffer[i].keyCode) != _wordBreakCode.end()) {
//                this->_bufferStartWordIdx = i + 1;
//                break;
//            }
//        }
//    }
//    
//    LOG_DEBUG("Number Delete: %d", addDelete);
//    if (addDelete > 0)
//        _processKeyCodeOutput(addDelete, _bufferSize); //we pass _bufferSize to process delete only
//}

// Recalculate startIndex of a word when user start new word
void kbengine::_startNewWord() {
    this->_bufferStartWordIdx = this->_bufferSize - 1;
    
    LOG_DEBUG("Start New Word, begin: %d", _bufferStartWordIdx);
}


BufferEntry* kbengine::getBuffer()
{
	return this->_buffer;
}

void kbengine::setInputMethod(const UInt8 &inputMethod)
{
    this->resetBuffer();
    
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
