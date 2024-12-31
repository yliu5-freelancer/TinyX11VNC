//
//  VNCSocketConnection.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/31.
//

import Foundation
import Network

struct VNCRemoteSocketConnection {
    var address: IPAddress? = nil
    var port: UInt16? = nil
    var vncSocketConnection: NWConnection? = nil

    init(address: IPAddress, port: UInt16) {
        self.address = address
        self.port = port
        let endPointHost = NWEndpoint.Host(address.toString())
        let endPointHostPort = NWEndpoint.Port(rawValue: port)
        if let endPointHostPort = endPointHostPort {
            self.vncSocketConnection = NWConnection(host: endPointHost, port: endPointHostPort, using: .tcp)
        }
    }
    
    func start() {
        self.vncSocketConnection?.stateUpdateHandler = { state in
            switch state {
            case .ready:
                print("VNC Server Connected")
            case .failed(let connError):
                print("VNC Server Connection failed: \(connError)")
            default:
                break;
            }
        }
    }
}
