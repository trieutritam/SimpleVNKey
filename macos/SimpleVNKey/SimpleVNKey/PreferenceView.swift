//
//  ContentView.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import SwiftUI

struct PreferenceView: View {
    @ObservedObject private var viewModel = SettingViewModel()
    
    var body: some View {
        VStack(alignment: .leading) {
            Text("Preference")
            
            Picker(selection: $viewModel.inputMethod,
                label: Text("Input Method")) {
                    Text("VNI").tag(0)
                    Text("Simple Telex").tag(1)
                    Text("Telex").tag(2)
            }.pickerStyle(RadioGroupPickerStyle())
                .task {
                    viewModel.loadSettings()
                }
    
        }.padding()
    }
}

struct PreferenceView_Previews: PreviewProvider {
    static var previews: some View {
        Group {
            PreferenceView()
        }
    }
}
