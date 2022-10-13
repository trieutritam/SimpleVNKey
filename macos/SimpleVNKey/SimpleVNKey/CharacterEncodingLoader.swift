//
//  CharacterEncodingLoader.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 05/10/2022.
//

import Foundation
import Yaml
import OSLog

struct CharacterEncodingInfo {
    var id: Int
    var name: String
    var author: String
    var charType: UInt8
}

let ENCODING_DIR = ".simplevnkey/encoding"

class CharacterEncodingLoader {
    private static let log = Logger(
        subsystem: Bundle.main.bundleIdentifier!,
        category: String(describing: CharacterEncodingLoader.self)
    )
    
    var appDelegate: AppDelegate?
    
    init(_ appDelegate: AppDelegate) {
        self.appDelegate = appDelegate
    }
    
    // Return Id & name of character encoding
    func loadFromFiles() -> Array<CharacterEncodingInfo> {
        let fm = FileManager.default
        var charEncList = Array<CharacterEncodingInfo>()
        do {
            var homeUrl = fm.homeDirectoryForCurrentUser
            homeUrl.appendPathComponent(ENCODING_DIR)
            
            let path = homeUrl.path
            print("Load file from: ", path)
            let items = try fm.contentsOfDirectory(atPath: path)

            var itemIndex = 0;
            for item in items.sorted() {
                if (item.hasSuffix(".yaml") || item.hasSuffix(".yml")) {
                    print("Found File: \(item)")
                    var fileUrl = homeUrl.absoluteURL
                    fileUrl.appendPathComponent(item)
                    var charEncInfo = self.loadFile(fileUrl)
                    if (charEncInfo != nil) {
                        itemIndex = itemIndex + 1
                        charEncInfo!.id = itemIndex
                        charEncList.append(charEncInfo!)
                    }
                }
            }
        } catch {
            print("Error while loading character encoding")
        }
        return charEncList
    }
    
    private func loadFile(_ filePath: URL) -> CharacterEncodingInfo? {
        Self.log.info("Load Char Encoding file: \(filePath.path)")
        var charEncInfo: CharacterEncodingInfo?
        do {
            let data = try String(contentsOf: filePath, encoding: .utf8)
            
            let yaml = try Yaml.load(data)
            if (yaml["meta"] != nil
                && yaml["meta"]["name"].string != nil
                && yaml["meta"]["author"].string != nil
                && yaml["characters"].dictionary != nil
            ) {
                let encodingName = yaml["meta"]["name"].string!
                let encodingAuthor = yaml["meta"]["author"].string!
                let charType = yaml["meta"]["char-type"].int!
                let characters = yaml["characters"].dictionary!
                Self.log.info("Character Encoding: \(encodingName) - Author: \(encodingAuthor) - CharType: \(charType)")
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
                    Self.log.info("Added Character Encoding \(encodingName) - \(encodingAuthor) - charType: \(charType)")
                    
                    appDelegate?.addCharacterEncodingToEngine(charEnc: charEnc,
                                                              charType: UInt8(charType))
                    
                    charEncInfo = CharacterEncodingInfo(id: 0, name: encodingName, author: encodingAuthor, charType: UInt8(charType))
                }
            }
            else {
                Self.log.warning("Could not load Yaml Meta from file.")
            }
        }
        catch {
            Self.log.error("Load file error: \(filePath)")
        }
        
        return charEncInfo
    }
}
