# 🚂 Multithreaded Train Schedule Server

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)
![Concurrency](https://img.shields.io/badge/Concurrency-Pthreads-green.svg)
![Database](https://img.shields.io/badge/Database-XML-lightgrey.svg)
![Build](https://img.shields.io/badge/Build-Make-brightgreen.svg)

## 📖 Overview

**Multithreaded Train Server** is a high-performance, concurrent Client-Server application designed to simulate a real-time railway management system. Developed in **C** on Linux, it serves as a central hub (Single Source of Truth) for managing train schedules, tracking real-time delays, and handling ticket bookings for multiple connected clients simultaneously.

The system is built upon a **Thread Pool architecture** (Pre-threading model), ensuring efficient resource utilization and low latency even under heavy network load.

## 🚀 Key Features

* **Concurrency & Scalability:** Utilizes a fixed **Thread Pool** of 50 worker threads to handle concurrent client connections, eliminating the overhead of dynamic thread creation.
* **Data Integrity:** Implements robust synchronization using **Mutex locks** (`pthread_mutex`) to protect shared resources (RAM data structures and file I/O) against race conditions.
* **Real-Time Updates:** Allows administrators to report train delays or early arrivals, which are instantly propagated to the system and visible to all clients.
* **Data Persistence:** Integrates **Libxml2** to parse and persist schedule data and ticket availability to an XML database (`database.xml`).
* **Role-Based Access Control:** Features a secure login system with distinct roles for **Admins** (can modify schedules) and **Regular Users** (can view/book).
* **Custom Protocol:** Operates over a text-based TCP request-response protocol designed for reliability and ease of debugging.

## 🛠️ Tech Stack

* **Language:** C (C11 Standard)
* **OS:** Linux (POSIX Compliant)
* **Networking:** TCP/IP Sockets (`<sys/socket.h>`, `<netinet/in.h>`)
* **Concurrency:** POSIX Threads (`<pthread.h>`)
* **XML Parsing:** Libxml2 (`<libxml/parser.h>`)
* **Build System:** GNU Make

## 🏗️ Architecture

The application follows a **Pre-threaded Server** model:

1.  **Initialization:** The `Main Thread` sets up the TCP socket, loads the `database.xml` into memory, and spawns a pool of **Worker Threads**.
2.  **Connection Handling:** Worker threads compete to `accept()` incoming connections from a shared queue, protected by synchronization primitives.
3.  **Session Management:** Once connected, a dedicated thread handles the client's session until disconnection, processing commands like `login`, `buy_ticket`, or `get_schedule`.
4.  **Shared Memory:** Critical data (train delays, seats) resides in RAM for fast access, with periodic or event-driven writes to the XML file for persistence.

## ⚙️ Installation & Build

### Prerequisites
Ensure you have the GCC compiler and Libxml2 development libraries installed.

**On Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libxml2-dev
```
### Compilation
Clone the repository and compile the source code using make:
```Bash
git clone [https://github.com/cataCatau/Multithreaded-Train-Server.git](https://github.com/cataCatau/Multithreaded-Train-Server.git)
cd Multithreaded-Train-Server
make
```
This will generate two executables: server and client.
## 💻 Usage
### 1. Start the Server
Run the server first. It listens on port 3010 by default.
```Bash
./server
```
### 2. Start the Client
Open a new terminal window to simulate a user:
```Bash
./client
```
Note: To connect from a different machine, update the IP address in client.c before compiling.
## 🔐 Authentication
The system comes with pre-configured users in users.txt:

| Role | Username | Password | Permissions |
| :--- | :--- | :--- | :--- |
| **Admin** | `admin` | `admin` | Report delays, View schedules |
| **User** | `catau` | `Alorap9*` | Buy tickets, View schedules |

## 📡 Command Reference
Once connected via the client, use the following commands:

### General Commands (No Login Required)
**help** - Display the list of available commands.

**login [username] [password]** - Authenticate into the system.

**register [username] [password]** - Create a new user account.

**exit** - Disconnect from the server.

### Information Commands
**get_schedule** - Display the complete train schedule.

**get_schedule [station]** - List trains departing from a specific station.

**get_schedule [station] [station]** - List trains departing from a specific station and heading to a specific station.

**get_train_info [ID]** - Show detailed route and status for a train ID.

**get_departures [station]** - List trains leaving a station in the next hour.

**get_arrivals [station]** - List trains arriving at a station in the next hour.

**get_delays** - Show all trains currently delayed.

**get_earlys** - Show all trains running ahead of schedule.

### User Actions (Login Required)

**logout** - Sign out of the current session.

**buy_ticket [TrainID]** - Purchase a ticket (decrements available seat count).

### Admin Actions (Admin Only)

**report_delay [TrainID] [minutes]** - Report a delay (updates system & XML).

**report_early [TrainID] [minutes]** - Report an early arrival.

## 📂 Project Structure
```text
├── server.c         # Entry point: Server initialization & Thread Pool
├── client.c         # Entry point: Client application
├── commands.c       # Business logic: Command processing & User actions
├── xml_handler.c    # Data Layer: XML parsing (Libxml2) & Persistence
├── shared.h         # Header: Shared data structures (struct tren, constants)
├── database.xml     # Database: Train schedules, routes, and seat counts
├── users.txt        # Database: User credentials
└── Makefile         # Build script for automation
```
## 👤 Author
**Catalin Tarca**

**University: Alexandru Ioan Cuza University, Faculty of Computer Science**

**GitHub: cataCatau**
