//
//  AppDelegate.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//
import SwiftUI

struct Global {
    static var myEventSource: CGEventSource?;
}

class AppDelegate: NSObject, NSApplicationDelegate {
    static private(set) var instance: AppDelegate!
    
    private var kbEngine = KBEngineWrapper();
    
    private var eventTap: CFMachPort?;
    
    private var isVNEnabled = false;

    private var statusBarItem: NSStatusItem!
    private var popover: NSPopover!
    

    // Create an instance of our custom main menu we are building
    let menu = MainMenu()
    
    func getEngine() -> KBEngineWrapper {
        return kbEngine;
    }

    func getInputMethod() -> UInt8 {
        return kbEngine.getInputMethod()
    }
    
    func setInputMethod(inputMethod: UInt8) {
        kbEngine.setInputMethod(inputMethod)
        
        UserDefaults.standard.set(inputMethod, forKey: "Vietnamese.InputMethod")
    }
    
    func getVNEnabled() -> Bool{
        return self.isVNEnabled
    }
    
    func enableVN() {
        self.isVNEnabled = true;
        self.kbEngine.resetBuffer()
        
        UserDefaults.standard.set(true, forKey: "Vietnamese.Enabled")
        menu.setEnableVNMenuItem(enabled: true)
    }
    
    func disableVN() {
        self.isVNEnabled = false;
        self.kbEngine.resetBuffer()
        
        UserDefaults.standard.set(false, forKey: "Vietnamese.Enabled")
        menu.setEnableVNMenuItem(enabled: false)
    }
    
    func loadPreference() {
        isVNEnabled = UserDefaults.standard.bool(forKey: "Vietnamese.Enabled")
        let inputMethod = UserDefaults.standard.integer(forKey: "Vietnamese.InputMethod")
        print("Vietnamese.Enabled = ", isVNEnabled)
        print("Vietnamese.InputMethod = ", inputMethod)
        
        isVNEnabled ? enableVN() : disableVN()
        setInputMethod(inputMethod: UInt8(inputMethod))
    }
    
    func setUpMenu() {
        statusBarItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        
//
//        popover = NSPopover()
//        popover.contentSize = NSSize(width: 300, height: 300)
//        popover.behavior = .transient
//        popover.contentViewController = NSHostingController(rootView: ContentView())
//
//
//        // Assign our custom menu to the status bar
        statusBarItem.menu = menu.build(statusBarItem: statusBarItem)
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        print("In AppDelegate")
        AppDelegate.instance = self
    
        
        initCGEvent()
        
        loadPreference()
        setUpMenu()
        
        startCFRunLoop()
    }
    
    @objc func togglePopover() {
//        if let button = statusBarItem.button {
//            if popover.isShown {
//                self.popover.performClose(nil)
//            } else {
//                self.popover.show(relativeTo: button.bounds, of: button, preferredEdge: NSRectEdge.minY)
//            }
//        }

    }
    
    
    func initCGEvent()  {
        Global.myEventSource = CGEventSource.init(stateID: CGEventSourceStateID.privateState);
        
        // Create an event tap. We are interested in key presses.
        var eventMask = ((1 << CGEventType.keyDown.rawValue) |
                         (1 << CGEventType.keyUp.rawValue) |
                         (1 << CGEventType.flagsChanged.rawValue));
        
        eventMask |= (
            (1 << CGEventType.leftMouseDown.rawValue) |
            (1 << CGEventType.rightMouseDown.rawValue) |
            (1 << CGEventType.leftMouseDragged.rawValue) |
            (1 << CGEventType.rightMouseDragged.rawValue));

        let userInfo = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())

        self.eventTap = CGEvent.tapCreate(tap: CGEventTapLocation.cgSessionEventTap,
                                               place: CGEventTapPlacement.headInsertEventTap,
                                               options: CGEventTapOptions.defaultTap,
                                               eventsOfInterest: CGEventMask(eventMask),
                                               callback: eventTapCallback,
                                               userInfo: userInfo)
        if ((self.eventTap) == nil) {
            print("failed to create event tap")
            exit(1)
        }
        
        print("CGEventTap Created")
        
        let runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0)
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        // will enabled by load preference
        CGEvent.tapEnable(tap: self.eventTap!, enable: true)
    }
    
    func startCFRunLoop() {
        CFRunLoopRun()
    }
}


/**
 * CGEvent call back function
 */
func eventTapCallback(proxy: CGEventTapProxy, type: CGEventType, event: CGEvent, refcon: UnsafeMutableRawPointer?) -> Unmanaged<CGEvent>? {
    // Don't process event fired by our self
    if (event.getIntegerValueField(CGEventField.eventSourceStateID) == Global.myEventSource!.sourceStateID.rawValue) {
        return Unmanaged.passRetained(event);
    }
    
    let appDelegate = Unmanaged<AppDelegate>.fromOpaque(refcon!).takeUnretainedValue()
    let engine = appDelegate.getEngine();
    let flag = event.flags
    
    var statusShiftCap: UInt8 = 0;
    statusShiftCap += flag.contains(CGEventFlags.maskShift) ? 1 : 0
    statusShiftCap += flag.contains(CGEventFlags.maskAlphaShift) ? 2 : 0
    
    if [.leftMouseDown, .rightMouseDown, .leftMouseDragged, .rightMouseDragged].contains(type) {
        engine.resetBuffer();
    }
    else if [.keyDown].contains(type) {
        let otherControl = flag.contains(CGEventFlags.maskCommand)
                            || flag.contains(CGEventFlags.maskControl)
                            || flag.contains(CGEventFlags.maskAlternate)
                            || flag.contains(CGEventFlags.maskSecondaryFn)
        
        let keyCode = event.getIntegerValueField(.keyboardEventKeycode)
        // HotKey: Check Control + Shift + Space
        if (flag.contains(CGEventFlags.maskControl)
            && flag.contains(CGEventFlags.maskShift)
            && keyCode == VKKeyCode.KEY_SPACE.rawValue) {
            
            if (appDelegate.getVNEnabled()) {
                appDelegate.disableVN()
            }
            else {
                appDelegate.enableVN()
            }
        }
        
        if (appDelegate.getVNEnabled()) {
            var charCode = UniChar()
            var length = 0
            event.keyboardGetUnicodeString(maxStringLength: 1, actualStringLength: &length, unicodeString: &charCode)
            
            let keyCode = event.getIntegerValueField(.keyboardEventKeycode)
            
            let keystrokes = engine.process(charCode,
                                        keycode: UInt8(keyCode),
                                        shiftOrCapPressed: statusShiftCap,
                                        otherControlPressed: otherControl)
        
            if (!keystrokes.isEmpty) {
                // First send empty character to prevent error due to autocomplete of browser
                sendDummyCharacter(proxy: proxy);
                
                for keystroke in keystrokes {
                    let keyData = keystroke as? UInt32
                    //print(keyData ?? "null")

                    var eUp: CGEvent?
                    var eDown: CGEvent?
                    // Check unicode mask
                    if ((keyData! & 0x00010000) == 0x00010000) {
                        eUp = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: 0x0032, keyDown: false)
                        eDown = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: 0x0032, keyDown: true)
                        //print("keyData = ", UniChar(keyData! ^ 0x00010000))
                        let tempChar = [UniChar(keyData! ^ 0x00010000)]
                        eUp?.keyboardSetUnicodeString(stringLength: tempChar.count, unicodeString: tempChar)
                        eDown?.keyboardSetUnicodeString(stringLength: tempChar.count, unicodeString: tempChar)
                    }
                    else {
                        eUp = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: CGKeyCode(keyData!), keyDown: false)
                        eDown = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: CGKeyCode(keyData!), keyDown: true)
                    }
                    eDown?.tapPostEvent(proxy)
                    eUp?.tapPostEvent(proxy)
                    
                }
                
                return nil;
            }
        }
    }
    
    return Unmanaged.passRetained(event)
}

/**
 * Fix browser autocomplete.
 * - Send empty character
 * - Send delete
 */
func sendDummyCharacter(proxy: CGEventTapProxy) {
    // send an empty character
    var eUp = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: VKKeyCode.KEY_SPACE.rawValue, keyDown: false)
    var eDown = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: VKKeyCode.KEY_SPACE.rawValue, keyDown: true)
    let tempChar = [UniChar(0x202F)]
    eUp?.keyboardSetUnicodeString(stringLength: 1, unicodeString: tempChar)
    eDown?.keyboardSetUnicodeString(stringLength: 1, unicodeString: tempChar)
    eDown?.tapPostEvent(proxy)
    eUp?.tapPostEvent(proxy)
    
    //send delete back
    eUp = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: VKKeyCode.KEY_DELETE.rawValue, keyDown: false)
    eDown = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: VKKeyCode.KEY_DELETE.rawValue, keyDown: true)
    eDown?.tapPostEvent(proxy)
    eUp?.tapPostEvent(proxy)
}
