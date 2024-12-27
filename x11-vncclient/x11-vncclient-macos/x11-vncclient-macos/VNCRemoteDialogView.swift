//
//  VNCRemoteDialogView.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/27.
//

import Foundation
import SwiftUI

struct VNCRemoteDialogView: View {
    @Binding var showRemoteDialog: Bool
    @Binding var hostList: [VNCHostInfo]
    @State private var hostAddr: String = ""
    @State private var hostPort: String = ""

    var body: some View {
        VStack {
            VStack {
                Text("X11 Remote Desktop")
                    .font(.title)
                    .padding()
                HStack {
                    Text("Host: ")
                    TextField("IP or FQDN", text: $hostAddr)
                        .textFieldStyle(.roundedBorder)
                }
                HStack {
                    Text("Port: ")
                    TextField("Port", text: $hostPort)
                        .textFieldStyle(.roundedBorder)
                }
            }
            .padding()
            HStack {
                Button("Close") {
                    showRemoteDialog.toggle()
                }
                Button("Ok") {
                    if !hostAddr.isEmpty && !hostPort.isEmpty {
                        hostList.append(VNCHostInfo(hostAddr: hostAddr, hostPort: UInt16(hostPort) ?? 0))
                        showRemoteDialog.toggle()
                    }
                }
            }
            .padding()
        }
    }
}

#Preview {
    VNCRemoteDialogView(showRemoteDialog: .constant(false), hostList:.constant([]))
}
