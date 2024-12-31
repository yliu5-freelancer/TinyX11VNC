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
    @State private var hostIPAddress = IPAddress.ipv4(IPV4Address(from: ""))
    @State private var hostAddrIsValid: Bool = false
    @State private var hostPortIsValid: Bool = false

    let hostAddrRegex = "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$|^[a-zA-Z0-9-]{1,63}(\\.[a-zA-Z0-9-]{1,63})*\\.[a-zA-Z]{2,}$"
    let hostPortRegex = "^([0-9]{1,5})$"

    var body: some View {
        VStack {
            VStack {
                Text("X11 Remote Desktop")
                    .font(.title)
                    .padding()
                ZStack {
                    HStack {
                        Text("Host: ")
                        TextField("IP or FQDN", text: $hostAddr)
                            .textFieldStyle(.roundedBorder)
                            .disableAutocorrection(true)
                            .onChange(of: hostAddr) { oldValue, newValue in
                                hostAddr = newValue.filter { $0.isNumber || $0.isLetter || $0 == "." || $0 == "-" }
                                hostAddrIsValid = NSPredicate(format: "SELF MATCHES %@", hostAddrRegex).evaluate(with: hostAddr)
                            }
                            .help("Please Input a IP address or FQDN.")
                    }
                    if !hostAddrIsValid && !hostAddr.isEmpty {
                        Text("Please Input a Vaild IP address or FQDN.")
                            .font(.caption)
                            .foregroundColor(.white)
                            .background(Color.red.opacity(0.5))
                            .cornerRadius(8)
                            .shadow(radius: 5)
                            .animation(.easeInOut, value: hostAddrIsValid)
                            .offset(x: -75 , y: -20)
                    }
                }
                ZStack {
                    HStack {
                        Text("Port: ")
                        TextField("Port", text: $hostPort)
                            .textFieldStyle(.roundedBorder)
                            .disableAutocorrection(true)
                            .onChange(of: hostPort) { oldValue, newValue in
                                hostPort = newValue.filter { $0.isNumber }
                                hostPortIsValid = NSPredicate(format: "SELF MATCHES %@", hostPortRegex).evaluate(with: hostPort)
                                if !hostPort.isEmpty {
                                    let hostPortUInt64 = UInt64(hostPort) ?? 0
                                    if hostPortUInt64 >= 65535 || hostPortUInt64 == 0 {
                                        hostPortIsValid = false;
                                    }
                                }
                            }
                            .help("Please Input a Port Number.")
                    }
                    if !hostPortIsValid && !hostPort.isEmpty {
                        Text("Please Input a Vaild Port Number (0 ~ 65535).")
                            .font(.caption)
                            .foregroundColor(.white)
                            .background(Color.red.opacity(0.5))
                            .cornerRadius(8)
                            .shadow(radius: 5)
                            .animation(.easeInOut, value: hostPortIsValid)
                            .offset(x: -75 , y: -20)
                    }
                }
            }
            .padding()
            HStack {
                Button("Close") {
                    showRemoteDialog.toggle()
                }
                Button("Ok") {
                    if !hostAddr.isEmpty && !hostPort.isEmpty {
                        if (hostAddrIsValid && hostPortIsValid) {
                            hostList.append(VNCHostInfo(hostAddr: hostAddr, hostPort: UInt16(hostPort) ?? 0, ipAddr: hostIPAddress, connStatus: VNCHostConnStatus(vncSocketStatus: VNCSocketConnectionStatus.VNCSOCKET_NOT_CONNECT)))
                            showRemoteDialog.toggle()
                        }
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
