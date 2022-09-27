//
//  Menu.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import Cocoa
import SwiftUI


class MainMenu: NSObject {
    
    // A new menu instance ready to add items to
    let menu = NSMenu()
//    let menuItems = Bundle.main.object(forInfoDictionaryKey: "SampleLinks") as! [String: String]
    private var inputMethodMenu = NSMenu()
    private var statusBarItem: NSStatusItem?
    
    private var enableVNItem: NSMenuItem?
    
    func build(statusBarItem: NSStatusItem) -> NSMenu {
        //    let nowPlayingView = ContentView()
        //    let contentView = NSHostingController(rootView: nowPlayingView)
        //    contentView.view.frame.size = CGSize(width: 200, height: 80)
        //
        //    // This is where we actually add our now playing view to the menu
        //    let customMenuItem = NSMenuItem()
        //    customMenuItem.view = contentView.view
        //    menu.addItem(customMenuItem)
        
        self.statusBarItem = statusBarItem
//
        if let statusBarButton = statusBarItem.button {
            statusBarButton.image = NSImage(named: AppDelegate.instance.getVNEnabled() ? "MenuIconV" : "MenuIconE")
//            statusBarButton.action = #selector(togglePopover)
            statusBarButton.imagePosition = .imageLeading

        }
        
        let checkForUpdatesMenuItem = NSMenuItem(
          title: "Check For Updates...",
          action: nil,
          keyEquivalent: ""
        )
        checkForUpdatesMenuItem.target = self
        
        
        enableVNItem = NSMenuItem(
                title: "Enable Vietnamese",
                action: #selector(toggleVNMode),
                keyEquivalent: "")
        
        enableVNItem!.target = self
        enableVNItem!.state = AppDelegate.instance.getVNEnabled() ? NSControl.StateValue.on : NSControl.StateValue.off
        menu.addItem(enableVNItem!)
        
        self.addInputMethodMenu(menu: menu)
        
        // Adding a seperator
        menu.addItem(NSMenuItem.separator())
        
        // We add an About pane.
        let aboutMenuItem = NSMenuItem(
            title: "About SimpleVNKey",
            action: #selector(about),
            keyEquivalent: ""
        )
        
        aboutMenuItem.target = self
        
        // This is where we actually add our about item to the menu
        menu.addItem(aboutMenuItem)
        // This is where we actually add the updates menu item we pass in
        menu.addItem(checkForUpdatesMenuItem)
        
        // Adding a seperator
        menu.addItem(NSMenuItem.separator())
        
//        // Loop though our sorted link list and create a new menu item for
//        // each, and then add it to the menu
//        for (title, link) in menuItems.sorted( by: { $0.0 < $1.0 }) {
//            let menuItem = NSMenuItem(
//                title: title,
//                action: #selector(linkSelector),
//                keyEquivalent: ""
//            )
//            menuItem.target = self
//            menuItem.representedObject = link
//
//            menu.addItem(menuItem)
//        }
//
//        // Adding a seperator
//        menu.addItem(NSMenuItem.separator())
        
        // Adding a quit menu item
        let quitMenuItem = NSMenuItem(
            title: "Quit SimpleVNKey",
            action: #selector(quit),
            keyEquivalent: "" //q"
        )
        quitMenuItem.target = self
        menu.addItem(quitMenuItem)
        
        return menu
    }
    
    func addInputMethodMenu(menu: NSMenu) {
        let curInput = AppDelegate.instance.getInputMethod();
        
        // VNI Menu
        let inputVNI = NSMenuItem(title: "VNI", action: #selector(selectInputSource), keyEquivalent: "")
        inputVNI.representedObject = InputMethod.VNI.rawValue
        inputVNI.target = self
        inputVNI.state = curInput == InputMethod.VNI.rawValue ? NSControl.StateValue.on : NSControl.StateValue.off
        
        // Simple Telex
        let inputTelex = NSMenuItem(title: "Simple Telex", action: #selector(selectInputSource), keyEquivalent: "")
        inputTelex.representedObject = InputMethod.SIMPLE_TELEX.rawValue
        inputTelex.target = self
        inputTelex.state = curInput == InputMethod.SIMPLE_TELEX.rawValue ? NSControl.StateValue.on : NSControl.StateValue.off
        
        inputMethodMenu.addItem(inputVNI)
        inputMethodMenu.addItem(inputTelex)
        
        let inputMethodItem = NSMenuItem(title: "Input Methods", action: nil, keyEquivalent: "")
        inputMethodItem.submenu = inputMethodMenu
        inputMethodItem.target = self
        menu.addItem(inputMethodItem)
    }
    
    func uncheckAllInputMethodMenuItem() {
        inputMethodMenu.items.forEach({ item in
            item.state = NSControl.StateValue.off
        })
    }
    
    func setEnableVNMenuItem(enabled: Bool) {
        if let statusBarButton = self.statusBarItem?.button {
            statusBarButton.image = NSImage(named: enabled ? "MenuIconV" : "MenuIconE")
        }
        enableVNItem?.state = AppDelegate.instance.getVNEnabled() ? NSControl.StateValue.on : NSControl.StateValue.off
    }
    
    
    @objc func linkSelector(sender: NSMenuItem) {
        let link = sender.representedObject as! String
        guard let url = URL(string: link) else { return }
        NSWorkspace.shared.open(url)
    }
    
    @objc func about(sender: NSMenuItem) {
        NSApp.activate(ignoringOtherApps: true)
        NSApp.orderFrontStandardAboutPanel()
    }
    
    @objc func toggleVNMode(sender: NSMenuItem) {
        sender.state = sender.state == NSControl.StateValue.on ?
                            NSControl.StateValue.off : NSControl.StateValue.on
        
        if (sender.state == NSControl.StateValue.on) {
            AppDelegate.instance.enableVN()
        }
        else {
            AppDelegate.instance.disableVN()
        }
    }
    
    @objc func selectInputSource(sender: NSMenuItem) {
        let selItem = sender.representedObject as! Int
        print("select: ", selItem)
        
        AppDelegate.instance.setInputMethod(inputMethod: UInt8(selItem))
        
        self.uncheckAllInputMethodMenuItem();
        sender.state = NSControl.StateValue.on
        
        //NSApp.sendAction(Selector(("showPreferencesWindow:")), to: nil, from: nil)
//        NSApp.mainWindow?.display()
//        NSApp.sendAction(Selector(("showMainWindows:")), to: nil, from: nil)
        //NSApp.activate(ignoringOtherApps: true)
    }
    
    // The selector that quits the app
    @objc func quit(sender: NSMenuItem) {
        NSApp.terminate(self)
    }
}
