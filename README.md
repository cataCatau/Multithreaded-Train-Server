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
