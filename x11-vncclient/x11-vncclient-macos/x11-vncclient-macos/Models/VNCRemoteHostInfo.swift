//
//  VNCRemoteHostInfo.swift
//  x11-vncclient-macos
//
//  Created by Yu Liu on 2024/12/30.
//

import Foundation

enum IPAddress: Hashable, Equatable {
    case ipv4(IPV4Address)
    case ipv6(IPV6Address)

    static func == (lhs: IPAddress, rhs: IPAddress) -> Bool {
        switch (lhs, rhs) {
        case (.ipv4(_), .ipv4(_)):
            return lhs.toString() == rhs.toString()
        case (.ipv6(_), .ipv6(_)):
            return lhs.toString() == rhs.toString()
        default:
            return false
        }
    }
    func toString() -> String {
        switch self {
        case .ipv4(let address):
            return address.toString()
        case .ipv6(let address):
            return address.toString()
        }
    }
}

struct IPV4Address: Hashable, Equatable {
    var fullAddress: UInt32 = 0
    var sepAddress: [UInt8] = []
    
    init(from ipv4String: String) {
    }
    func toString() -> String {
        var fullStringAddr = ""
        return fullStringAddr
    }
}

struct IPV6Address: Hashable, Equatable {
    var fullAddress: UInt128 = 0
    var sepAddress: [UInt16] = []

    init(from ipv6String: String) {
        
    }
    func toString() -> String {
        var fullStringAddr = ""
        return fullStringAddr
    }
}

enum VNCSocketConnectionStatus {
    case VNCSOCKET_NOT_CONNECT
    case VNCSOCKET_CONNECTING
    case VNCSOCKET_CONNECTED
}

struct VNCHostConnStatus: Hashable, Equatable {
    var vncSocketStatus: VNCSocketConnectionStatus
}

struct VNCRemoteHostInfo: Hashable, Equatable {
    var id: UUID = UUID()
    var hostAddr: String
    var hostPort: UInt16
    var ipAddr: IPAddress
    var connStatus: VNCHostConnStatus
}
