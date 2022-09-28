//
//  ContentView.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import SwiftUI

struct PreferenceView: View {
    @AppStorage("SimpleVNKey.VNEnabled") private var isVNEnabled = false
    @AppStorage("SimpleVNKey.InputMethod") private var inputMethod = 0
    
    var body: some View {
        VStack(alignment: .leading) {
            Text("Preferences: ")
            Toggle(isOn: $isVNEnabled) {
                Text("Enable Vietnamese")
            }
            Picker("", selection: $isVNEnabled) {
                    Text("Vietnamese").tag(true)
                    Text("English").tag(false)
            }.labelsHidden()
                .pickerStyle(RadioGroupPickerStyle())
                .onSubmit {
                    print("submit")
                }
            
            Picker(selection: $inputMethod,
                label: Text("Input Method")) {
                    Text("VNI").tag(0)
                    Text("Simple Telex").tag(1)
//                    Text("Telex").tag(2)
            }.pickerStyle(RadioGroupPickerStyle())
            
    
        }.padding()
    }
}

struct PreferenceView_Previews: PreviewProvider {
    static var previews: some View {
        Group {
            PreferenceView()
                .environmentObject(SettingViewModel())
        }
    }
}
