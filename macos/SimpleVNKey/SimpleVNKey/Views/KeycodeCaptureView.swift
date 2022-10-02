//
//  AppKitView.swift
//  Test
//
//  Created by tamtt5 on 01/10/2022.
//

import SwiftUI
import Cocoa

protocol KeycodeFieldDelegate {
    func handleKeyDown(event: NSEvent)
}

class KeycodeField: NSTextView {
    var keycodeDelegate: KeycodeFieldDelegate?
    var selectedKeyCode:UInt16?
    
    let KeyCodeChar = [
        53                              : "⎋" ,
        51                              : "⌫" ,
        48                              : "⇥" ,
        76                              : "↩" ,
        36                              : "↩" ,
        49                              : "⎵" ,
        123                              : "←" ,
        124                              : "→" ,
        125                              : "↓" ,
        126                              : "↑" ,
        
        122 : "F1",
        120 : "F2",
        99  : "F3",
        118 : "F4",
        96  : "F5",
        97  : "F6",
        98  : "F7",
        100 : "F8",
        101 : "F9",
        109 : "F10",
        111 : "F12",
    ]
    
    func setKeyCode(keycode: UInt16) {
        selectedKeyCode = keycode
        
        if (KeyCodeChar[Int(keycode)] != nil) {
            self.string = String(KeyCodeChar[Int(keycode)]!)
        }
        else {
            guard let cgEvent = CGEvent.init(keyboardEventSource: nil, virtualKey: CGKeyCode(keycode), keyDown: true) else { return }

            guard let event = NSEvent.init(cgEvent: cgEvent) else { return }
            guard let chars = event.characters(byApplyingModifiers: .numericPad) else { return }
            guard let firstChar = chars.first else { return }
            
            let string = firstChar.unicodeScalars
            self.string = String(string).uppercased()
        }
    }
    
    override func keyDown(with event: NSEvent) {
        guard let delegate = keycodeDelegate else { return }

        selectedKeyCode = nil
        if (KeyCodeChar[Int(event.keyCode)] != nil) {
            self.string = String(KeyCodeChar[Int(event.keyCode)]!)
            selectedKeyCode = event.keyCode
            
            delegate.handleKeyDown(event: event)
        }
        else {
            guard let chars = event.characters(byApplyingModifiers: .numericPad) else { return }
            guard let firstChar = chars.first else { return }
            
            let string = firstChar.unicodeScalars
            self.string = String(string).uppercased()
            selectedKeyCode = event.keyCode
            
            delegate.handleKeyDown(event: event)
        }
    }

    override var focusRingMaskBounds: NSRect {
        return bounds
    }
    
    override func drawFocusRingMask() {
        let rect = bounds;
        rect.fill()
    }
}

struct KeycodeCaptureView: NSViewRepresentable {
    @Binding var keycode: UInt16
    var onKeyDownAction: ((NSEvent) -> Void)?
    
    func makeNSView(context: Context) -> KeycodeField {
        let textField = KeycodeField()
        
        textField.minSize = NSSize(width: 30, height: 22)
        textField.textContainerInset = NSSize(width: 2, height: 4)
        textField.keycodeDelegate = self
        textField.font = textField.font?.withSize(16)
        textField.alignment = NSTextAlignment.center
        textField.isEditable = false
        
        return textField
    }

    func updateNSView(_ view: KeycodeField, context: Context) {
        view.setKeyCode(keycode: keycode)
    }
}

extension KeycodeCaptureView: KeycodeFieldDelegate {
    
    func handleKeyDown(event: NSEvent) {
        if (onKeyDownAction != nil) {
            onKeyDownAction!(event)
        }
    }
}

struct KeycodeCaptureView_Previews: PreviewProvider {
    @State static var keyCode = UInt16(0)
    static var previews: some View {
        KeycodeCaptureView(keycode: $keyCode)
            .frame(width: 100, height: 50)
    }
}
