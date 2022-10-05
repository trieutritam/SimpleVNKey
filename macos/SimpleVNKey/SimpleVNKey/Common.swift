//
//  Common.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 27/09/2022.
//

import Foundation

import Yaml

enum InputMethod : Int {
    case VNI = 0, SIMPLE_TELEX = 1, TELEX = 2
}

enum CharacterEncoding : Int {
    case Unicode = 0
}

enum HotKeyMask {
    static let Control  = 1   //00000001
    static let Option   = 2   //00000010
    static let Command  = 4  //00000100
    static let Shift    = 8  //00001000
}
