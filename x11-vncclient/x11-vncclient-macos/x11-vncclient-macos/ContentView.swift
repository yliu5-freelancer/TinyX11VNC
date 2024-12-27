//
//  ContentView.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/26.
//

import SwiftUI

struct VNCHostInfo: Identifiable, Hashable {
    var id: UUID = UUID()
    var hostAddr: String
    var hostPort: UInt16
}

struct ContentView: View {
    @State private var vncHostList: [VNCHostInfo] = []
    @State private var selectedHost: VNCHostInfo? = nil
    @State private var showRemoteDialog = false

    var body: some View {
        NavigationSplitView {
            List(vncHostList, id: \.self, selection: $selectedHost) { host in
                Text(host.hostAddr)
            }
            .frame(minWidth: 150) // 设置最小宽度
        } detail: {
            if selectedHost != nil {
                VNCRemoteScreenView(selectedHost: $selectedHost)
            } else {
                Text("Please select a remote desktop")
            }
        }
        .toolbar {
            ToolbarItem(placement: .navigation) {
                Button(action: {
                    showRemoteDialog.toggle()
                }) {
                    Image(systemName: "plus")
                }
            }
        }
        .sheet(isPresented: $showRemoteDialog) {
            VNCRemoteDialogView(showRemoteDialog: $showRemoteDialog, hostList: $vncHostList)
        }
    }
}

#Preview {
    ContentView()
}
