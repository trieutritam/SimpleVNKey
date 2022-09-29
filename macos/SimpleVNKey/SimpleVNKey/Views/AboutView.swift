//
//  AboutView.swift
//  SimpleVNKey
//
//  Created by tamtt5 on 28/09/2022.
//

import SwiftUI

extension Bundle {
    public var appIconName: String? {
        if let icons = infoDictionary?["CFBundleIcons"] as? [String: Any],
            let primaryIcon = icons["CFBundlePrimaryIcon"] as? [String: Any],
            let iconFiles = primaryIcon["CFBundleIconFiles"] as? [String],
            let lastIcon = iconFiles.last {
            return lastIcon
        }
        return nil
    }
}

struct AboutView: View {
    var body: some View {
        HStack(alignment: .top) {
            Image("AppLogo")
                .padding()
            VStack(alignment: .leading) {
                Text(Bundle.main.infoDictionary!["CFBundleName"] as! String)
                    .bold()
                Text(String(localized: "Version: ") + (Bundle.main.infoDictionary!["CFBundleShortVersionString"] as! String))
                let author = Bundle.main.object(forInfoDictionaryKey: "Author") as! String
                Text(String(localized: "Author: ") + author)
            }.padding()
        }
        .frame(minWidth: 300, minHeight: 100, alignment: .topLeading)
        .padding()
    }
}

struct AboutView_Previews: PreviewProvider {
    static var previews: some View {
        Group {
            AboutView()
        }
    }
}
