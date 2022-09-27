#include "vnlexi.h"

map<UInt8, vector<vector<UInt16>>> syllableTable = {
    {
        KEY_A, {
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A },
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_C },
            {MASK_ORIGIN,                               KEY_A, KEY_C, KEY_H},
            {MASK_ORIGIN,                               KEY_A, KEY_I},
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_M},
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_N},
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_N, KEY_G},
            {MASK_ORIGIN,                               KEY_A, KEY_N, KEY_H},
            {MASK_ORIGIN,                               KEY_A, KEY_O},
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_P},
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_T},
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_U},
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_A, KEY_Y},
        },
    },
    {
        KEY_E, {
            {MASK_ORIGIN | MASK_ROOF, KEY_E },
            {MASK_ORIGIN            , KEY_E, KEY_C },
            {MASK_ORIGIN | MASK_ROOF, KEY_E, KEY_M },
            {MASK_ORIGIN | MASK_ROOF, KEY_E, KEY_N },
            {MASK_ORIGIN            , KEY_E, KEY_N, KEY_G },
            {MASK_ORIGIN            , KEY_E, KEY_O },
            {MASK_ORIGIN | MASK_ROOF, KEY_E, KEY_P },
            {MASK_ORIGIN | MASK_ROOF, KEY_E, KEY_T },
            // special for ê: êch, ênh, êu
            {MASK_ORIGIN | MASK_ROOF, KEY_E, KEY_C, KEY_H },
            {MASK_ORIGIN | MASK_ROOF, KEY_E, KEY_N, KEY_H },
            {MASK_ORIGIN | MASK_ROOF, KEY_E, KEY_U }
        },
    },
    {
        KEY_I, {
            {MASK_ORIGIN, KEY_I },
            {MASK_ORIGIN, KEY_I, KEY_C, KEY_H },
            {MASK_ORIGIN, KEY_I, KEY_M },
            {MASK_ORIGIN, KEY_I, KEY_N },
            {MASK_ORIGIN, KEY_I, KEY_N, KEY_H },
            {MASK_ORIGIN, KEY_I, KEY_P },
            {MASK_ORIGIN, KEY_I, KEY_T },
            {MASK_ORIGIN, KEY_I, KEY_U },
            
            // IA
            {MASK_ORIGIN, KEY_I, KEY_A },
            
            // iê - also included in map KEY_E
            {MASK_ORIGIN | MASK_ROOF, KEY_I, KEY_E | MASK_EXTRA_MARK, KEY_C },
            {MASK_ORIGIN | MASK_ROOF, KEY_I, KEY_E | MASK_EXTRA_MARK, KEY_M },
            {MASK_ORIGIN | MASK_ROOF, KEY_I, KEY_E | MASK_EXTRA_MARK, KEY_N },
            {MASK_ORIGIN | MASK_ROOF, KEY_I, KEY_E | MASK_EXTRA_MARK, KEY_N, KEY_G },
            {MASK_ORIGIN | MASK_ROOF, KEY_I, KEY_E | MASK_EXTRA_MARK, KEY_P },
            {MASK_ORIGIN | MASK_ROOF, KEY_I, KEY_E | MASK_EXTRA_MARK, KEY_T },
            {MASK_ORIGIN | MASK_ROOF, KEY_I, KEY_E | MASK_EXTRA_MARK, KEY_U },
        }
    },
    {
        KEY_Y, {
            { MASK_ORIGIN, KEY_Y },
            { MASK_ORIGIN | MASK_ROOF, KEY_Y, KEY_E | MASK_EXTRA_MARK, KEY_M },
            { MASK_ORIGIN | MASK_ROOF, KEY_Y, KEY_E | MASK_EXTRA_MARK, KEY_N },
            { MASK_ORIGIN | MASK_ROOF, KEY_Y, KEY_E | MASK_EXTRA_MARK, KEY_N, KEY_G },
            { MASK_ORIGIN | MASK_ROOF, KEY_Y, KEY_E | MASK_EXTRA_MARK, KEY_T },
            { MASK_ORIGIN | MASK_ROOF, KEY_Y, KEY_E | MASK_EXTRA_MARK, KEY_U },
        }
    },
    {
        KEY_O, {
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_O },
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_O, KEY_C },
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_O, KEY_I },
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_O, KEY_M },
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_O, KEY_N },
            {MASK_ORIGIN | MASK_ROOF                ,   KEY_O, KEY_N, KEY_G },
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_O, KEY_P },
            {MASK_ORIGIN | MASK_ROOF | MASK_BOWLHOOK,   KEY_O, KEY_T },
            // oo
            {MASK_ORIGIN                            ,   KEY_O, KEY_O, KEY_C },
            {MASK_ORIGIN                            ,   KEY_O, KEY_O, KEY_N, KEY_G },
            // oa
            {MASK_ORIGIN                            ,   KEY_O, KEY_A },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_C },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_C, KEY_H },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_I },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_M },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_N },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_N, KEY_G },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_N, KEY_H },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_O },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_P },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_T },
            {MASK_ORIGIN                            ,   KEY_O, KEY_A, KEY_Y },
        }
    },
    {   KEY_U, {
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_C },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_I },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_M },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_N },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_N, KEY_G },
            {MASK_ORIGIN                , KEY_U, KEY_P },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_T },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_U },
        
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_Y },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_Y, KEY_C, KEY_H },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_Y, KEY_N },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_Y, KEY_N, KEY_H },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_Y, KEY_P },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_Y, KEY_T },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_Y, KEY_U },
            {MASK_ORIGIN                , KEY_U, KEY_Y, KEY_A },
        
            // ua
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_A },
        
            {MASK_ORIGIN                , KEY_U, KEY_Y, KEY_E, KEY_N },
            {MASK_ORIGIN                , KEY_U, KEY_Y, KEY_E, KEY_T },
        
            // uo
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_C },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_I },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_M },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_N },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_N, KEY_G },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_P },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_T },
            {MASK_ORIGIN | MASK_BOWLHOOK, KEY_U, KEY_O | MASK_EXTRA_MARK, KEY_U },
        }
    },
};

/**
 * 0: unicode table
 */
map<UInt32, vector<UInt16>> codeTable[] = {
    // Unicode
    {
        //KEY_A,    {A, a, Á, á, À, à, Ả, ả, Ã, ã, Ạ, ạ}
        {KEY_A,                 { 0x0041, 0x0061, 0x00C1, 0x00E1, 0x00C0, 0x00E0, 0x1EA2, 0x1EA3, 0x00C3, 0x00E3, 0x1EA0, 0x1EA1 }},
        {KEY_A | MASK_ROOF,     { 0x00C2, 0x00E2, 0x1EA4, 0x1EA5, 0x1EA6, 0x1EA7, 0x1EA8, 0x1EA9, 0x1EAA, 0x1EAB, 0x1EAC, 0x1EAD }},
        {KEY_A | MASK_BOWLHOOK, { 0x0102, 0x0103, 0x1EAE, 0x1EAF, 0x1EB0, 0x1EB1, 0x1EB2, 0x1EB3, 0x1EB4, 0x1EB5, 0x1EB6, 0x1EB7 }},
        
        {KEY_E,                 { 0x0045, 0x0065, 0x00C9, 0x00E9, 0x00C8, 0x00E8, 0x1EBA, 0x1EBB, 0x1EBC, 0x1EBD, 0x1EB8, 0x1EB9 }},
        {KEY_E | MASK_ROOF,     { 0x00CA, 0x00EA, 0x1EBE, 0x1EBF, 0x1EC0, 0x1EC1, 0x1EC2, 0x1EC3, 0x1EC4, 0x1EC5, 0x1EC6, 0x1EC7 }},
        
        {KEY_I,                { 0x0049, 0x0069, 0x00CD, 0x00ED, 0x00CC, 0x00EC, 0x1EC8, 0x1EC9, 0x128, 0x129, 0x1ECA, 0x1ECB }},
        {KEY_Y,                { 0x0059, 0x0079, 0x00DD, 0x00FD, 0x1EF2, 0x1EF3, 0x1EF6, 0x1EF7, 0x1EF8, 0x1EF9, 0x1EF4, 0x1EF5 }},
        
        {KEY_O,                 { 0x004F, 0x006F, 0x00D3, 0x00F3, 0x00D2, 0x00F2, 0x1ECE, 0x1ECF, 0x00D5, 0x00F5, 0x1ECC, 0x1ECD }},
        {KEY_O | MASK_ROOF,     { 0x00D4, 0x00F4, 0x1ED0, 0x1ED1, 0x1ED2, 0x1ED3, 0x1ED4, 0x1ED5, 0x1ED6, 0x1ED7, 0x1ED8, 0x1ED9 }},
        {KEY_O | MASK_BOWLHOOK, { 0x01A0, 0x01A1, 0x1EDA, 0x1EDB, 0x1EDC, 0x1EDD, 0x1EDE, 0x1EDF, 0x1EE0, 0x1EE1, 0x1EE2, 0x1EE3 }},

        {KEY_U,                 { 0x0055, 0x0075, 0x00DA, 0x00FA, 0x00D9, 0x00F9, 0x1EE6, 0x1EE7, 0x0168, 0x0169, 0x1EE4, 0x1EE5 }},
        {KEY_U | MASK_BOWLHOOK, { 0x01AF, 0x01B0, 0x1EE8, 0x1EE9, 0x1EEA, 0x1EEB, 0x1EEC, 0x1EED, 0x1EEE, 0x1EEF, 0x1EF0, 0x1EF1 }},
        
        {KEY_D,                 { 0x0044, 0x0064 }},
        {KEY_D | MASK_ROOF,     { 0x0110, 0x0111 }}
    }
};
