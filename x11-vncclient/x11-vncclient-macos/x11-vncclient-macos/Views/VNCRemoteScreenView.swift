//
//  VNCRemoteScreenView.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/27.
//

import Foundation
import SwiftUI


struct VNCRemoteScreenView: View {
    @Binding var vncHostList: [VNCRemoteHostInfo]
    @Binding var selectedHost: VNCRemoteHostInfo?
    @Binding var selectedIndex: Int?

    var body: some View {
        VStack {
            Text("VNC Remote Host: \(selectedHost?.hostAddr ?? "nil")")
            Button(action: {
                if selectedHost?.connStatus.vncSocketStatus ==
                    VNCSocketConnectionStatus.VNCSOCKET_CONNECTING {
                    selectedHost?.connStatus.vncSocketStatus =
                        VNCSocketConnectionStatus.VNCSOCKET_NOT_CONNECT
                } else {
                    selectedHost?.connStatus.vncSocketStatus = VNCSocketConnectionStatus.VNCSOCKET_CONNECTING
                }
                if let selectedIndex = selectedIndex, let selectedHost = selectedHost {
                    vncHostList[selectedIndex] = selectedHost
                }
            }) {
                if selectedHost?.connStatus.vncSocketStatus ==
                    VNCSocketConnectionStatus.VNCSOCKET_CONNECTING {
                    Text("Cancel")
                } else {
                    Text("Connect")
                }
            }
            if selectedHost?.connStatus.vncSocketStatus ==
                VNCSocketConnectionStatus.VNCSOCKET_CONNECTING {
                Text("Connecting to \(selectedHost?.hostAddr ?? "nil")")
            }
        }
    }
}

#Preview {
    VNCRemoteScreenView(vncHostList: .constant([]), selectedHost: .constant(VNCRemoteHostInfo(hostAddr: "test", hostPort: 1234, ipAddr: IPAddress.ipv4(IPV4Address(from: "123.123.123.123")), connStatus: VNCHostConnStatus(vncSocketStatus: VNCSocketConnectionStatus.VNCSOCKET_NOT_CONNECT))), selectedIndex: .constant(0))
}
