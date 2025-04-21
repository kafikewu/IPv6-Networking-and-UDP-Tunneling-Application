# IPv6 Networking and UDP Tunneling Application

This project explores IPv6 socket programming, particularly focusing on link-local addressing, and implements a single-hop UDP tunneling service with distinct control and data planes.

## Overview

The project encompasses two main parts:
1.  An adaptation of a simple UDP ping application to utilize IPv6 link-local addresses for communication.
2.  A tunneling service consisting of a server (`tunnels`) and a client (`tunnelc`) that forwards UDP traffic for other applications, effectively masking the original source and destination from direct observation.

## Core Components & Features

### 1. IPv6 UDP Ping (Based on Problem 1)

* **IPv6 Communication:** Implements a client/server UDP ping application using IPv6.
* **Link-Local Addressing:** Specifically designed to work with non-routable IPv6 link-local addresses (e.g., `fe80::...`).
* **Scope ID Handling:** Correctly configures `struct sockaddr_in6`, including `sin6_addr` and `sin6_scope_id`, to bind sockets to a specific network interface (e.g., `eth0`) using scope identifiers.
* **Address Parsing:** Handles IPv6 address strings including the interface specifier (e.g., `fe80::a6bb:6dff:fe44:fc43%eth0`).

### 2. UDP Tunneling Service (Based on Problem 2)

* **Tunneling Server (`tunnels`):**
    * **Concurrent Design:** Uses a parent process to handle control connections and forks child processes for active tunnel sessions.
    * **TCP Control Plane:** Accepts TCP connections for tunnel setup and teardown requests. Handles basic secret key authentication for session initiation.
    * **UDP Data Plane:** Child processes manage UDP sockets to receive traffic from the original UDP client and forward it to the final destination, and vice-versa.
    * **Dynamic Port Allocation:** Assigns UDP ports dynamically for the data plane communication.
    * **Session Management:** Maintains a table (`forwardtab`) of active tunneling sessions (limited capacity).
    * **I/O Multiplexing:** Uses `select()` to efficiently monitor activity on TCP (control) and UDP (data) sockets within child processes.
* **Tunneling Client (`tunnelc`):**
    * **Session Setup:** Connects to the `tunnels` server via TCP to request a new tunnel, providing the secret key, its own source IP, and the final destination IP/port.
    * **Receives Data Port:** Obtains the specific UDP port allocated by the `tunnels` child process for the data plane.
    * **Session Teardown:** Can signal the server via the TCP control connection to terminate the tunnel.
* **Application Integration:** Allows a standard UDP application (like the ping client `pingc`) to send its traffic to the `tunnels` server's data plane port, which then forwards it to the intended final destination server (e.g., `pings`). Responses are routed back through the tunnel.

### 3. (Optional) Bonus Features

* **(Bonus A) Basic Cryptographic Authentication:** Replaces the simple plain-text secret key in the tunneling control plane with a basic cryptographic authentication scheme using XOR-based "encryption" (E) and "decryption" (D) functions with hardcoded public/private keys. This focuses only on the control plane authentication between `tunnelc` and `tunnels`.
* **(Bonus B) IPv6 Remote Command:** Adapts a previous remote command client/server application (from Lab 3) to use IPv6 link-local addresses, similar to the IPv6 Ping component.

## Build Instructions

1.  **Prerequisites:** Ensure you have a C compiler (like `gcc`) and standard C libraries installed. For IPv6 functionality, a modern Linux/Unix-like system is expected.
    ```bash
    # Example for Debian/Ubuntu (if needed, typically pre-installed)
    sudo apt-get update
    sudo apt-get install build-essential
    ```
2.  **Compile:** Navigate to the project directory containing the source code (`pingc_v6.c`, `pings_v6.c`, `tunnels.c`, `tunnelc.c`, etc.) and the `Makefile`. Run `make`:
    ```bash
    make
    ```
    This will compile the necessary executables. Ensure the `Makefile` is correctly configured for all components (v1, v2, v3, v4 if bonus parts are included).

## Usage

*(Note: Replace example IPs, ports, and keys with your actual values. Assumes applications are run on different machines as described in the original problem context.)*

### IPv6 Ping (v1)

**Server (e.g., on amber06):**
```bash
# Get IPv6 link-local address for eth0 (e.g., fe80::a6bb:6dff:fe44:ddb8)
ip addr show eth0
./pings_v6 fe80::a6bb:6dff:fe44:ddb8%eth0 <port>

Client (e.g., on amber05):

# Get IPv6 link-local address for eth0 (e.g., fe80::a6bb:6dff:fe44:fc43)
ip addr show eth0
./pingc_v6 fe80::a6bb:6dff:fe44:ddb8%eth0 <port> fe80::a6bb:6dff:fe44:fc43%eth0

(Remember to hardcode %eth0 and scope ID 0x20 in the code as per instructions)

UDP Tunneling (v2)
1. Start Tunneling Server (e.g., on amber03):

./tunnels <tunnels_server_ip> <control_port> <secret_key>
# Example: ./tunnels 128.10.112.133 55550 abcdABCD

2. Setup Tunnel using Tunneling Client (e.g., on amber02):

./tunnelc <tunnels_server_ip> <control_port> <secret_key> <original_client_ip> <final_destination_ip> <final_destination_port>
# Example: ./tunnelc 128.10.112.133 55550 abcdABCD 128.10.112.134 128.10.112.135 56001
# --> This will print the allocated UDP data plane port (e.g., 60007) on stdout.

3. Start Final Destination Server (e.g., standard IPv4 ping server on amber05):

./pings <final_destination_ip> <final_destination_port>
# Example: ./pings 128.10.112.135 56001

4. Run Original Client via Tunnel (e.g., standard IPv4 ping client on amber04):

# Use the data plane port printed by tunnelc (e.g., 60007)
./pingc <tunnels_server_ip> <data_plane_port> <original_client_ip>
# Example: ./pingc 128.10.112.133 60007 128.10.112.134

(The ping traffic from pingc on amber04 goes to tunnels on amber03, which forwards it to pings on amber05. Responses follow the reverse path.)
