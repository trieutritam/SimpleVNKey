//
//  Settings.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import Foundation

class SettingViewModel: ObservableObject {
    
    @Published var inputMethod = 0;
    
    func loadSettings() {
        inputMethod = Int(AppDelegate.instance.getInputMethod())
        print("load input method: ", inputMethod)
    }
    
    func load() {
        self.loadSettings()
    }
}
