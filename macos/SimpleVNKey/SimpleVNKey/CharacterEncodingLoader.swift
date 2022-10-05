//
//  CharacterEncodingLoader.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 05/10/2022.
//

import Foundation
import Yaml
import OSLog

class CharacterEncodingLoader {
    private static let log = Logger(
        subsystem: Bundle.main.bundleIdentifier!,
        category: String(describing: CharacterEncodingLoader.self)
    )
    
    var appDelegate: AppDelegate?
    
    init(_ appDelegate: AppDelegate) {
        self.appDelegate = appDelegate
    }
    func loadFromFiles() {
        let fm = FileManager.default
        do {
            var homeUrl = fm.homeDirectoryForCurrentUser
            homeUrl.appendPathComponent(".simplevnkey")
            
            let path = homeUrl.path
            print("Load file from: ", path)
            let items = try fm.contentsOfDirectory(atPath: path)

            
            for item in items {
                if (item.hasSuffix(".yaml") || item.hasSuffix(".yml")) {
                    print("Found File: \(item)")
                    var fileUrl = homeUrl.absoluteURL
                    fileUrl.appendPathComponent(item)
                    self.loadFile(fileUrl)
                }
            }
        } catch {
            print("Error while loading character encoding")
        }
    }
    
    private func loadFile(_ filePath: URL) {
        Self.log.info("Load Char Encoding file: \(filePath.path)")
        do {
            let data = try String(contentsOf: filePath, encoding: .utf8)
            
            let yaml = try Yaml.load(data)
            if (yaml["meta"] != nil) {
                guard let encodingName = yaml["meta"]["name"].string else { return }
                guard let encodingAuthor = yaml["meta"]["author"].string else { return }
                guard let characters = yaml["characters"].dictionary else { return }
                
                Self.log.info("Character Encoding: \(encodingName) - Author: \(encodingAuthor)")
                var charEnc = Dictionary<String, Array<UInt16>>()
                for key in characters.keys {
                    //print("Key: ", key)
                    let values = characters[key]?.array
                    var charList = Array<UInt16>()
                    for charCode in values! {
                        charList.append(UInt16(charCode.int!))
                    }
                    charEnc[key.string!] = charList //setValue(charList, forKey: key.string!)
                }
                if (charEnc.count > 0) {
                    appDelegate?.getEngine().addCharacterEncoding((charEnc as NSDictionary) as! [AnyHashable : Any])
                    
                    Self.log.info("Added Character Encoding \(encodingName) - \(encodingAuthor)")
                    // TODO: trigger Menu to create menu item
                }
            }
            else {
                Self.log.warning("Could not load Yaml Meta from file.")
            }
        }
        catch {
            Self.log.error("Load file error: \(filePath)")
        }
    }
}
