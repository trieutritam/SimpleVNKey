//
//  Menu.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//

import Cocoa
import SwiftUI


class MainMenu: NSObject, NSMenuDelegate {
    
    // A new menu instance ready to add items to
    let menu = NSMenu(title: "Quick")
//    let menuItems = Bundle.main.object(forInfoDictionaryKey: "SampleLinks") as! [String: String]
    private var inputMethodMenu = NSMenu()
    private var statusBarItem: NSStatusItem?
    
    private var enableVNItem: NSMenuItem?
    
    func menuWillOpen(_ menu: NSMenu) {
        
        menu.item(at: 0)?.state = AppDelegate.instance.getVNEnabled() ? NSControl.StateValue.on : NSControl.StateValue.off
        
        let subMenu = menu.item(withTag: 10)?.submenu;
        subMenu?.item(at: 0)?.state = AppDelegate.instance.getInputMethod() == 0 ? NSControl.StateValue.on : NSControl.StateValue.off
        subMenu?.item(at: 1)?.state = AppDelegate.instance.getInputMethod() == 1 ? NSControl.StateValue.on : NSControl.StateValue.off
        
        AppDelegate.instance.updateSwitchHotkeyIndicator()
    }
    
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

        if let statusBarButton = statusBarItem.button {
            statusBarButton.image = NSImage(named: AppDelegate.instance.getVNEnabled() ? "MenuIconV" : "MenuIconE")
            // statusBarButton.action = #selector(togglePopover)
            statusBarButton.imagePosition = .imageLeading
            
        }
        
        enableVNItem = NSMenuItem(
                title: String(localized: "Enable Vietnamese"),
                action: #selector(toggleVNMode),
                keyEquivalent: "")
        
        enableVNItem!.target = self
        menu.addItem(enableVNItem!)
        
        self.addInputMethodMenu(menu: menu)
        
        // Adding a seperator
        menu.addItem(NSMenuItem.separator())
        

        let openPreference = NSMenuItem(
          title: String(localized: "Preferences..."),
          action: #selector(openPreference),
          keyEquivalent: ""
        )
        openPreference.target = self
        menu.addItem(openPreference)
        
        // This is where we actually add the updates menu item we pass in
        let checkForUpdatesMenuItem = NSMenuItem(
          title: String(localized: "Check For Updates..."),
          action: nil,
          keyEquivalent: ""
        )
        checkForUpdatesMenuItem.target = self
        menu.addItem(checkForUpdatesMenuItem)
        
        // We add an About pane.
        let aboutMenuItem = NSMenuItem(
            title: String(localized: "About SimpleVNKey"),
            action: #selector(about),
            keyEquivalent: ""
        )
        aboutMenuItem.target = self
        // This is where we actually add our about item to the menu
        menu.addItem(aboutMenuItem)
        
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
            title: String(localized: "Quit SimpleVNKey"),
            action: #selector(quit),
            keyEquivalent: "" //q"
        )
        quitMenuItem.target = self
        menu.addItem(quitMenuItem)
        
        return menu
    }
    
    func updateSwitchHotkeyIndicator(isCtrl: Bool, isOpt: Bool, isCmd: Bool, isShift: Bool, hotKeyChar: UInt16) {
        guard let char = Unicode.Scalar.init(hotKeyChar) else { return }
        
        enableVNItem!.keyEquivalent = String(Character.init(char))
        
        if isCtrl {
            enableVNItem!.keyEquivalentModifierMask.insert(NSEvent.ModifierFlags.control)
        }
        else {
            enableVNItem!.keyEquivalentModifierMask.remove(NSEvent.ModifierFlags.control)
        }
        
        if isOpt {
            enableVNItem!.keyEquivalentModifierMask.insert(NSEvent.ModifierFlags.option)
        }
        else {
            enableVNItem!.keyEquivalentModifierMask.remove(NSEvent.ModifierFlags.option)
        }
        
        if isCmd {
            enableVNItem!.keyEquivalentModifierMask.insert(NSEvent.ModifierFlags.command)
        }
        else {
            enableVNItem!.keyEquivalentModifierMask.remove(NSEvent.ModifierFlags.command)
        }
        
        if isShift {
            enableVNItem!.keyEquivalentModifierMask.insert(NSEvent.ModifierFlags.shift)
        }
        else {
            enableVNItem!.keyEquivalentModifierMask.remove(NSEvent.ModifierFlags.shift)
        }
    }
    
    func addInputMethodMenu(menu: NSMenu) {
        // VNI Menu
        let inputVNI = NSMenuItem(title: String(localized: "VNI"), action: #selector(selectInputSource), keyEquivalent: "")
        inputVNI.representedObject = InputMethod.VNI.rawValue
        inputVNI.target = self
//        inputVNI.state = curInput == InputMethod.VNI.rawValue ? NSControl.StateValue.on : NSControl.StateValue.off
        
        // Simple Telex
        let inputTelex = NSMenuItem(title: String(localized: "Simple Telex"), action: #selector(selectInputSource), keyEquivalent: "")
        inputTelex.representedObject = InputMethod.SIMPLE_TELEX.rawValue
        inputTelex.target = self
//        inputTelex.state = curInput == InputMethod.SIMPLE_TELEX.rawValue ? NSControl.StateValue.on : NSControl.StateValue.off
        
        inputMethodMenu.addItem(inputVNI)
        inputMethodMenu.addItem(inputTelex)
        
        let inputMethodItem = NSMenuItem(title: String(localized: "Input Methods"), action: nil, keyEquivalent: "")
        inputMethodItem.submenu = inputMethodMenu
        inputMethodItem.target = self
        inputMethodItem.tag = 10
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
    
    @objc func openPreference(sender: NSMenuItem) {
        NSApp.sendAction(Selector(("showPreferencesWindow:")), to: nil, from: nil)
        //NSApp.mainWindow?.display()
        //NSApp.sendAction(Selector(("showMainWindows:")), to: nil, from: nil)
        NSApp.activate(ignoringOtherApps: true)
    }
    
    @objc func about(sender: NSMenuItem) {
        NSApp.activate(ignoringOtherApps: true)
        NSApp.orderFrontStandardAboutPanel()
    }
    
    @objc func toggleVNMode(sender: NSMenuItem) {
        sender.state = sender.state == NSControl.StateValue.on ?
                            NSControl.StateValue.off : NSControl.StateValue.on
        
        if (sender.state == NSControl.StateValue.on) {
            AppDelegate.instance.setEnableVNState(true)
        }
        else {
            AppDelegate.instance.setEnableVNState(false)
        }
    }
    
    @objc func selectInputSource(sender: NSMenuItem) {
        let selItem = sender.representedObject as! Int
        print("select: ", selItem)
        
        AppDelegate.instance.setInputMethodState(inputMethod: selItem)
        
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
