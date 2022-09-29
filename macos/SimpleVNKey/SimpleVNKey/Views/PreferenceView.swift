//
//  ContentView.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import SwiftUI

//
// General
//
// About
//
struct PreferenceView: View {
//    @AppStorage("SimpleVNKey.VNEnabled") private var isVNEnabled = false
//    @AppStorage("SimpleVNKey.InputMethod") private var inputMethod = 0
//    @AppStorage("SimpleVNKey.HotKey") private var hotKey = 0;
    
    var body: some View {
            TabView {
                GeneralSettingView()
                    .environmentObject(AppDelegate.instance.appPreference)
                    .tabItem {
                        Label("General", systemImage: "gear.circle.fill")
                    }
                AboutView()
                    .tabItem {
                        Label("About", systemImage: "info.circle.fill")
                    }
            }
            .padding()
    }
}

struct PreferenceView_Previews: PreviewProvider {
    @AppStorage("SimpleVNKey.VNEnabled") var isVNEnabled = false
    @State static var inputMethod = 0
    @AppStorage("SimpleVNKey.HotKey") var hotKey = 0
    
    static var previews: some View {
        Group {
            PreferenceView()
        }
    }
}
