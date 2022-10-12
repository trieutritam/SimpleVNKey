//
//  AppDelegate.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 26/09/2022.
//
import SwiftUI
import Combine
import OSLog

struct Global {
    static var myEventSource: CGEventSource?;
    static var log = Logger();
}

class AppDelegate: NSObject, NSApplicationDelegate {
    private static var log = Logger()

    static private(set) var instance: AppDelegate!
    
    private var aboutBoxWindowController: NSWindowController?

    private var characterEncodingLoader: CharacterEncodingLoader?
    
    private var kbEngine = KBEngineWrapper();
    
    private var eventTap: CFMachPort?;
    private var runLoopSource: CFRunLoopSource?;

    private var statusBarItem: NSStatusItem!
    private var popover: NSPopover!

    // Create an instance of our custom main menu we are building
    private let menu = MainMenu()
    
    var appPreference = SettingViewModel()

    func getEngine() -> KBEngineWrapper {
        return kbEngine;
    }

    func getInputMethod() -> UInt8 {
        return kbEngine.getInputMethod()
    }
    
    func setInputMethod(inputMethod: UInt8) {
        kbEngine.setInputMethod(inputMethod)
    }
    
    func getVNEnabled() -> Bool{
        return appPreference.isVNEnabled
    }
    
    func enableVN() {
        self.kbEngine.resetBuffer()
        menu.setEnableVNMenuItem(enabled: true)
    }
    
    func disableVN() {
        self.kbEngine.resetBuffer()
        menu.setEnableVNMenuItem(enabled: false)
    }
    
    func setEnableVNState(_ enabled: Bool) {
        appPreference.isVNEnabled = enabled
    }
    
    func setInputMethodState(inputMethod: Int) {
        appPreference.inputMethod = inputMethod
    }
    
    func updateSwitchHotkeyIndicator() {
        menu.updateSwitchHotkeyIndicator(isCtrl: appPreference.hotKeyControl,
                                         isOpt: appPreference.hotKeyOption,
                                         isCmd: appPreference.hotKeyCommand,
                                         isShift: appPreference.hotKeyShift,
                                         hotKeyChar: appPreference.hotKeyCharacter)
    }
    
    func setUpMenu(charEncList: Array<CharacterEncodingInfo>) {
        statusBarItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        
        // Assign our custom menu to the status bar
        statusBarItem.menu = menu.build(statusBarItem: statusBarItem)
        
        statusBarItem.menu?.delegate = menu;
        
        for enc in charEncList {
            menu.addCharacterEncodingMenu(info: enc)
        }
        
        updateSwitchHotkeyIndicator()
    }
    
    

    func applicationDidFinishLaunching(_ notification: Notification) {
        print("In AppDelegate")
        AppDelegate.instance = self
        
        characterEncodingLoader = CharacterEncodingLoader(self)
        let charEncList = characterEncodingLoader!.loadFromFiles()
        
        initCGEvent()
        
        appPreference.loadSettings();
        appPreference.setCharacterEncodingList(charEncList)
        setUpMenu(charEncList: charEncList)
        
        startCFRunLoop()
        
        
    }
    
    func addCharacterEncodingToEngine(charEnc: Dictionary<String, Array<UInt16>>, charType: UInt8) {
        kbEngine.addCharacterEncoding(
            (charEnc as NSDictionary) as! [AnyHashable : Any],
            charType: charType)
    }
    
    func setActiveCharacterEncoding(_ characterEnc: Int) {
        if ( characterEnc >= kbEngine.getTotalCodeTable()) {
            Self.log.warning("Seleted Character Encoding not found. Reset to Default")
            //Update the ViewModel so that the app reset the setting
            appPreference.characterEncoding = CharacterEncodingType.Unicode.rawValue
        }
        else {
            kbEngine.setActiveCodeTable(UInt8(characterEnc));
        }
    }
    
    func setUseModernToneMethod(_ isUse: Bool) {
        kbEngine.setUseModernToneMethod(isUse)
    }
    
    func showAboutDialog() {
        if (aboutBoxWindowController == nil) {
            let styleMask: NSWindow.StyleMask = [.closable, .miniaturizable,/* .resizable,*/ .titled]
            let window = NSWindow()
            window.styleMask = styleMask
            window.title = String(localized: "About") + " " + (Bundle.main.appName)
            window.contentView = NSHostingView(rootView: AboutView())
            window.center()
            aboutBoxWindowController = NSWindowController(window: window)
        }
        
        aboutBoxWindowController?.showWindow(aboutBoxWindowController?.window)
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
            Self.log.error("failed to create event tap")
            exit(1)
        }
        
        Self.log.debug("CGEventTap Created")
        
        self.runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0)
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        // will enabled by load preference
        CGEvent.tapEnable(tap: self.eventTap!, enable: true)
    }
    
    func startCFRunLoop() {
        CFRunLoopRun()
    }
    
    func applicationWillTerminate(_ notification: Notification) {
        CGEvent.tapEnable(tap: self.eventTap!, enable: false)
        
        CFRunLoopStop(CFRunLoopGetCurrent())
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), self.runLoopSource, .commonModes)
        
        CFMachPortInvalidate(self.eventTap)
    }
}

func checkHotKeyPressed(flag: CGEventFlags, actualKeyCode: Int64, appSetting: SettingViewModel) -> Bool {
    if (flag.contains(CGEventFlags.maskControl) != appSetting.hotKeyControl) {
        return false
    }
    if (flag.contains(CGEventFlags.maskAlternate) != appSetting.hotKeyOption) {
        return false
    }
    if (flag.contains(CGEventFlags.maskCommand) != appSetting.hotKeyCommand) {
        return false
    }
    if (flag.contains(CGEventFlags.maskShift) != appSetting.hotKeyShift) {
        return false
    }
    
    let expectedKeyCode = appSetting.hotKeyCharacter
    Global.log.debug("expectedCode: \(String(format: "%02X", expectedKeyCode))")
    if (actualKeyCode != expectedKeyCode) {
        return false
    }
    
    return true;
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
        #if !DEBUG
            engine.resetBuffer();
        #endif
    }
    else if [.keyDown].contains(type) {
        let otherControl = flag.contains(CGEventFlags.maskCommand)
                            || flag.contains(CGEventFlags.maskControl)
                            || flag.contains(CGEventFlags.maskAlternate)
                            || flag.contains(CGEventFlags.maskSecondaryFn)
        
        let keyCode = event.getIntegerValueField(.keyboardEventKeycode)
        
        if (checkHotKeyPressed(flag: flag, actualKeyCode: keyCode, appSetting: appDelegate.appPreference))
        {
            if (appDelegate.getVNEnabled()) {
                appDelegate.setEnableVNState(false)
            }
            else {
                appDelegate.setEnableVNState(true)
            }
        }
        else {
            if (appDelegate.getVNEnabled()) {
                var charCode = UniChar()
                var length = 0
                event.keyboardGetUnicodeString(maxStringLength: 1, actualStringLength: &length, unicodeString: &charCode)
                
                let keyCode = event.getIntegerValueField(.keyboardEventKeycode)
                
                let keystrokes = engine.process(charCode,
                                            keycode: UInt16(keyCode),
                                            shiftOrCapPressed: statusShiftCap,
                                            otherControlPressed: otherControl)
            
                if (!keystrokes.isEmpty) {
                    // First send empty character to prevent error due to autocomplete of browser
                    sendDummyCharacter(proxy: proxy);
                    
                    for keystroke in keystrokes {
                        let keyData = keystroke as? UInt32

                        // Check unicode mask
                        if ((keyData! & 0x00010000) == 0x00010000) {
                            let actualCharCode = keyData! ^ 0x00010000
                            
                            Global.log.debug("send keycode: \(String(format:"%02X", actualCharCode))")
                                  
                            let tempChar = [UniChar(actualCharCode)]
                            sendKeyStroke(proxy: proxy, keyData: UInt32(VKKeyCode.KEY_SPACE.rawValue), unicodeString: tempChar)
                        }
                        else {
                            Global.log.debug("send keycode: \(keyData!)")
                            sendKeyStroke(proxy: proxy, keyData: keyData!)
                        }
                    }
                    
                    return nil;
                }
            }
        }
    }
    
    return Unmanaged.passUnretained(event);//Unmanaged.passRetained(event)
}

func sendKeyStroke(proxy: CGEventTapProxy, keyData: UInt32, unicodeString: [UniChar]? = nil) {
    autoreleasepool{
        let eUp = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: CGKeyCode(keyData), keyDown: false)
        let eDown = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: CGKeyCode(keyData), keyDown: true)
        
        if (unicodeString != nil) {
            eUp?.keyboardSetUnicodeString(stringLength: unicodeString!.count, unicodeString: unicodeString!)
            eDown?.keyboardSetUnicodeString(stringLength: unicodeString!.count, unicodeString: unicodeString!)
        }
        
        eDown?.tapPostEvent(proxy)
        eUp?.tapPostEvent(proxy)
    }
}

/**
 * Fix browser autocomplete.
 * - Send empty character to cancel auto suggestion
 * - Send delete empty character
 */
func sendDummyCharacter(proxy: CGEventTapProxy) {
    // send an empty character
    var eUp = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: VKKeyCode.KEY_SPACE.rawValue, keyDown: false)
    var eDown = CGEvent.init(keyboardEventSource: Global.myEventSource, virtualKey: VKKeyCode.KEY_SPACE.rawValue, keyDown: true)
    let tempChar = [UniChar(0x00A0)] //in MSWord, cannot use the [UniChar(0x202F)] for dummy
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
