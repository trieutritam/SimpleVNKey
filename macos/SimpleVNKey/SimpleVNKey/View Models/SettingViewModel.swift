//
//  Settings.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import Foundation
import Combine

class SettingViewModel: ObservableObject {
    
    @Published var inputMethod = 0;
    @Published var isVNEnabled = false;
    
    @Published var hotKeyControl = true
    @Published var hotKeyOption = false
    @Published var hotKeyCommand = false
    @Published var hotKeyShift = true
    @Published var hotKeyCharacter = UInt16(VKKeyCode.KEY_0.rawValue)
    
    private var cancellables = Set<AnyCancellable>()
    
    
    func loadSettings() {
        print("Load Settings")
        isVNEnabled = (UserDefaults.standard.value(forKey: "SimpleVNKey.Enabled") ?? false) as! Bool
        inputMethod = (UserDefaults.standard.value(forKey: "SimpleVNKey.InputMethod") ?? 0) as! Int
        
        // Default Hot Key Ctrl + Shift + Space
        hotKeyControl = (UserDefaults.standard.value(forKey: "SimpleVNKey.HotKeyControl")    ?? true) as! Bool
        hotKeyOption = (UserDefaults.standard.value(forKey: "SimpleVNKey.HotKeyOption")      ?? false) as! Bool
        hotKeyCommand = (UserDefaults.standard.value(forKey: "SimpleVNKey.HotKeyCommand")    ?? false) as! Bool
        hotKeyShift = (UserDefaults.standard.value(forKey: "SimpleVNKey.HotKeyShift")        ?? true) as! Bool
        hotKeyCharacter = (UserDefaults.standard.value(forKey: "SimpleVNKey.HotKeyCharacter") ?? VKKeyCode.KEY_SPACE.rawValue) as! UInt16
        
        print("Load SimpleVNKey.isVNEnabled: ", isVNEnabled)
        print("Load SimpleVNKey.InputMethod: ", inputMethod)
        print("Load SimpleVNKey.HotKey: ", hotKeyControl, hotKeyOption, hotKeyCommand, hotKeyShift, hotKeyCharacter)
        
        guard let appDelegate = AppDelegate.instance else { return }
        isVNEnabled ? appDelegate.enableVN() : appDelegate.disableVN()
        appDelegate.setInputMethod(inputMethod: UInt8(inputMethod))
    }
    
    
    init() {
        self.loadSettings()
        
        // handle isVNChanged
        $isVNEnabled.sink { isVNEnabled in
            UserDefaults.standard.set(isVNEnabled, forKey: "SimpleVNKey.Enabled")
            
            guard let appDelegate = AppDelegate.instance else { return }
            isVNEnabled ? appDelegate.enableVN() : appDelegate.disableVN()
        }.store(in: &cancellables)
        
        $inputMethod.sink { selInputMethod in
            UserDefaults.standard.set(selInputMethod, forKey: "SimpleVNKey.InputMethod")
            guard let appDelegate = AppDelegate.instance else { return }
            appDelegate.setInputMethod(inputMethod: UInt8(selInputMethod))
        }.store(in: &cancellables)
        
        // Hot keys
        $hotKeyControl.sink { val in
            self.processHotkey(appStoreKey: "SimpleVNKey.HotKeyControl", val: val)
        }.store(in: &cancellables)
        
        $hotKeyOption.sink { val in
            self.processHotkey(appStoreKey: "SimpleVNKey.HotKeyOption", val: val)
        }.store(in: &cancellables)
        
        $hotKeyCommand.sink { val in
            self.processHotkey(appStoreKey: "SimpleVNKey.HotKeyCommand", val: val)
        }.store(in: &cancellables)
        
        $hotKeyShift.sink { val in
            self.processHotkey(appStoreKey: "SimpleVNKey.HotKeyShift", val: val)
        }.store(in: &cancellables)
        
        $hotKeyCharacter.sink { val in
            self.processHotkey(appStoreKey: "SimpleVNKey.HotKeyCharacter", val: val)
        }.store(in: &cancellables)
    }
    
    func processHotkey(appStoreKey: String, val: Published<Any>.Publisher.Output) -> Void {
        print(appStoreKey, val)
        UserDefaults.standard.set(val, forKey: appStoreKey)
    }
}
