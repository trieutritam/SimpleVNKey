//
//  GeneralSettingViewNew.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 29/09/2022.
//

import SwiftUI

struct GeneralSettingView: View {
    @EnvironmentObject var appSettingVM: SettingViewModel;

    @State private var hotKeyChar: String = ""  // temporary key pressed
    @State private var hotKeyCharDisplay: String = ""
    @State private var hotkeyCode: UInt16 = UInt16.zero
    
    @State private var text = ""
    
    var body: some View {
        VStack(alignment: .leading) {
            Section() {
                Form {
                    Toggle("Enable Vietnamese",
                           isOn: $appSettingVM.isVNEnabled)
                    .toggleStyle(.switch)
                    
                    Picker(selection: $appSettingVM.inputMethod,
                           label: Text("Input Method")) {
                        Text("VNI").tag(0)
                        Text("Simple Telex").tag(1)
                        Text("Telex").tag(2)
                    }.horizontalRadioGroupLayout()
                    
                    Picker(selection: $appSettingVM.characterEncoding,
                           label: Text("Character Encoding")) {
                        ForEach(appSettingVM.listCharacterEncoding.indices, id: \.self) { (index: Int) in
                            let item = self.appSettingVM.listCharacterEncoding[index]
                            Text(item.name).tag(item.id)
                        }
                    }.horizontalRadioGroupLayout()
                    
                    Toggle("Use oà, uý (instead of òa úy)",
                           isOn: $appSettingVM.useModernToneMethod)
                    
                    Toggle("Auto restore keys for invalid word",
                           isOn: $appSettingVM.enableAutoRestoreWord)
                }
            }
            Divider()
            Section("Switch Input Method Hot Key") {
                GroupBox() {
                    HStack(spacing: 20) {
                        Toggle("Ctrl", isOn: $appSettingVM.hotKeyControl)
                        Toggle("⌥ Alt", isOn: $appSettingVM.hotKeyOption)
                        Toggle("⌘ Cmd", isOn: $appSettingVM.hotKeyCommand)
                        Toggle("⇧ Shift", isOn: $appSettingVM.hotKeyShift)
                        Text("+")
                    
                        KeycodeCaptureView(keycode: $appSettingVM.hotKeyCharacter, onKeyDownAction: { event in
                            print ("Pressed: " , event.keyCode)
                            appSettingVM.hotKeyCharacter =  event.keyCode
                        })
                        .frame(width: 90, height: 26)
                    }
                }
            }.task {
                if (appSettingVM.hotKeyCharacter != VKKeyCode.KEY_EMPTY.rawValue) {
                    let hotkeyChar = Character.init(Unicode.Scalar.init(UInt8(appSettingVM.hotKeyCharacter)))
                    hotKeyCharDisplay = (appSettingVM.hotKeyCharacter == 32) ? String(localized: "Space") : String(hotkeyChar).uppercased()
                }
            }
        }.frame(width: 450)
            .padding()
    }
}

struct GeneralSettingView_Previews: PreviewProvider {
    @State static var appSettingVM = SettingViewModel()
    
    static var previews: some View {
        GeneralSettingView()
            .environmentObject(appSettingVM)
    }
}
