//
//  ContentView.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/26.
//

import SwiftUI


struct X11VNCClientView: View {
    @State private var vncHostList: [VNCHostInfo] = []
    @State private var selectedHost: VNCHostInfo? = nil
    @State private var showRemoteDialog = false
    @State private var showSettingsDialog = false

    var body: some View {
        NavigationSplitView {
            List(vncHostList, id: \.self, selection: $selectedHost) { host in
                HStack {
                    Image(systemName: "desktopcomputer.and.macbook")
                    Text(host.hostAddr)
                        .contextMenu {
                            Button(action: {
                                print("Connect to \(host.hostAddr)")
                            }) {
                                HStack {
                                    Text("Connect")
                                    Image(systemName: "link.icloud")
                                }
                            }
                            Button(action: {
                                showSettingsDialog.toggle()
                            }) {
                                HStack {
                                    Text("Settings")
                                    Image(systemName: "gearshape.circle")
                                }
                            }
                        }
                }
                .sheet(isPresented: $showSettingsDialog) {
                    VNCRemoteServerSettingsView(selectedHost: $selectedHost)
                }
                
            }
            .frame(minWidth: 150)
        } detail: {
            if selectedHost != nil {
                let vncHostIndex = vncHostList.firstIndex(of: selectedHost!)
                VNCRemoteScreenView(vncHostList: $vncHostList, selectedHost: $selectedHost, selectedIndex: .constant(vncHostIndex))
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
    X11VNCClientView()
}
