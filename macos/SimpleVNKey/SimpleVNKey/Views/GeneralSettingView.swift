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
    
    var body: some View {
        VStack(alignment: .leading) {
            Section() {
                Form {
                    Picker("Input Mode", selection: $appSettingVM.isVNEnabled) {
                        Text("Vietnamese").tag(true)
                        Text("English").tag(false)
                    }.frame(width: 200)
                    
                    Picker(selection: $appSettingVM.inputMethod,
                           label: Text("Input Method")) {
                        Text("VNI").tag(0)
                        Text("Simple Telex").tag(1)
                    }.pickerStyle(RadioGroupPickerStyle())
                        .horizontalRadioGroupLayout()
                }
            }
            Divider()
            Section("Switch Input Method Hot Key") {
                Form {
                    HStack(spacing: 20) {
                        Toggle("Ctrl", isOn: $appSettingVM.hotKeyControl)
                        Toggle("⌥ Alt", isOn: $appSettingVM.hotKeyOption)
                        Toggle("⌘ Cmd", isOn: $appSettingVM.hotKeyCommand)
                        Toggle("⇧ Shift", isOn: $appSettingVM.hotKeyShift)
                        KeycodeCaptureView(keycode: $appSettingVM.hotKeyCharacter, onKeyDownAction: { event in
                            print ("Pressed: " , event.keyCode)
                            appSettingVM.hotKeyCharacter =  event.keyCode
                        })
                        .frame(width: 50, height: 22)
//                        ZStack(alignment: .leading) {
//                            Text(hotKeyCharDisplay)
//                                .frame(width: 100, alignment: .leading)
//                                .padding(EdgeInsets.init(top: 3, leading: 35, bottom: 3, trailing: 0))
//                            TextField("", text: $hotKeyChar)
//                                .disableAutocorrection(true)
//                                .onChange(of: hotKeyChar, perform: { newValue in
//                                    if (newValue.count == 0) {
//                                        return
//                                    }
////
//                                    if (newValue == " ") {
//                                        hotKeyCharDisplay = String(localized: "Space")
//                                    }
//                                    else if (newValue.count > 0) {
//                                        hotKeyCharDisplay = hotKeyChar.uppercased()
//                                    }
//
//
//                                    let chars = Array(hotKeyChar)
//                                    // set to VM state
//                                    appSettingVM.hotKeyCharacter =  (chars.count > 0) ? UInt16(chars[0].asciiValue ?? 0) : 0
//
//                                    hotKeyChar = ""
//                                }).frame(width: 30).labelsHidden()
//                        }
                    }
                }
            }.task {
//                let hotkeyChar = Character.init(Unicode.Scalar.init(UInt8(appSettingVM.hotKeyCharacter)))
//                let n = String(hotkeyChar).unicodeScalars.compactMap(\.properties.name).first
//                hotKeyCharDisplay = n!.localizedUppercase
                
                let hotkeyChar = Character.init(Unicode.Scalar.init(UInt8(appSettingVM.hotKeyCharacter)))
                hotKeyCharDisplay = (appSettingVM.hotKeyCharacter == 32) ? String(localized: "Space") : String(hotkeyChar).uppercased()
            }
        }.frame(width: 400)
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
