//
//  VNCRemoteServerSettingsView.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/28.
//

import SwiftUI

struct VNCRemoteServerSettingsView: View {
    @Binding var selectedHost: VNCRemoteHostInfo?
    var body: some View {
        VStack {
            Text("VNC Host Information")
                .font(.title)
            VStack(alignment: .leading){
                HStack {
                    Text("Host Name: ")
                    Text(selectedHost?.hostAddr ?? "")
                }
                HStack {
                    Text("Host Port: ")
                    Text(String(selectedHost?.hostPort ?? 0))
                }
                HStack {
                    Text("IP Address: ")
//                    Text(selectedHost)
                }
            }
        }
    }
}

#Preview {
    VNCRemoteServerSettingsView(selectedHost: .constant(VNCRemoteHostInfo(hostAddr: "test", hostPort: 1234, ipAddr: IPAddress.ipv4(IPV4Address(from: "123.123.123.123")), connStatus: VNCHostConnStatus(vncSocketStatus: VNCSocketConnectionStatus.VNCSOCKET_NOT_CONNECT))))
}
