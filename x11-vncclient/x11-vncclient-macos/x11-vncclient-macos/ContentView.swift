//
//  ContentView.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/26.
//

import SwiftUI

struct ContentView: View {
    let items: [String] = []
    
    @State private var selectedHost: String? = nil

    var body: some View {
        NavigationSplitView {
            List(items, id: \.self, selection: $selectedHost) { item in
                Text(item)
            }
            .frame(minWidth: 150) // 设置最小宽度
        } detail: {
        }
    }
}

#Preview {
    ContentView()
}
