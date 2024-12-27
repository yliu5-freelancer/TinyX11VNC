//
//  VNCRemoteScreenView.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/27.
//

import Foundation
import SwiftUI


struct VNCRemoteScreenView: View {
    @Binding var selectedHost: VNCHostInfo?
    var body: some View {
        Text("VNC Remote Screen: \(selectedHost!.hostAddr)")
    }
}

#Preview {
    VNCRemoteScreenView(selectedHost: .constant(VNCHostInfo(hostAddr: "test", hostPort: 1234)))
}
