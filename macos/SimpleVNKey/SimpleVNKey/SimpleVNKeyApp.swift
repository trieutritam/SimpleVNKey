//
//  SimpleVNKeyApp.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import SwiftUI


@main
struct SimpleVNKeyApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    
    var body: some Scene {
        Settings {
            PreferenceView()
        }
    }
    
    func askPermission() {
        let alert = NSAlert.init()
        let appName =  Bundle.main.infoDictionary!["CFBundleName"] as! String
        alert.messageText = appName + " cần bạn cấp quyền để có thể hoạt động!"
        alert.informativeText = "Vui lòng chạy lại ứng dụng sau khi cấp quyền."
        alert.addButton(withTitle: "Không")
        alert.addButton(withTitle: "Cấp quyền")
        let res = alert.runModal()
        if (res == NSApplication.ModalResponse.alertSecondButtonReturn) {
            // Open System Preference
            let prefpaneUrl = URL(string: "x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility")!
            NSWorkspace.shared.open(prefpaneUrl)
        }
    }

    init() {
        print("Init SimpleVNKeyApp!")
        
        // check if user granted Accessabilty permission
        let options: NSDictionary = [kAXTrustedCheckOptionPrompt.takeUnretainedValue() as NSString: false]
        let accessEnabled = AXIsProcessTrustedWithOptions(options)
        if !accessEnabled {
            print("No accessibility enabled")
            self.askPermission();
            exit(1)
        }
    }
}

