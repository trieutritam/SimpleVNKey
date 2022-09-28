//
//  Settings.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import Foundation

class SettingViewModel: ObservableObject {
    
    @Published var inputMethod = 0;
    @Published var isVNEnabled = false;
    
    func loadSettings() {
        isVNEnabled = UserDefaults.standard.bool(forKey: "SimpleVNKey.Enabled")
        inputMethod = UserDefaults.standard.integer(forKey: "SimpleVNKey.InputMethod")
    }
    
    func saveSettings() {
        UserDefaults.standard.set(isVNEnabled, forKey: "SimpleVNKey.Enabled")
        UserDefaults.standard.set(inputMethod, forKey: "SimpleVNKey.InputMethod")
    }
}
