# KeyValueDatabase - Local Key-Value Database for ESP32

A fast, lightweight and persistent key-value database for ESP32 with **O(log n)** access time.  
Keys are stored in RAM/PSRAM for fast lookup, while values are stored on the flash LittleFS filesystem .

This library is designed for applications that need structured, persistent storage.

---

## ✨ Features

- **O(log n)** lookup, insert and update operations  
- **Keys stored in RAM/PSRAM**, values stored on flash  
- **Thread‑safe** (internal locking)  
- Works with **LittleFS**
- Suitable for configuration storage, sensor logs, device state, structured data, etc.

---

## 📁 How it works

The database stores:

- **Keys** in memory (balanced tree structure)
- **Values** on disk (flash filesystem)

This allows:

- Fast key lookup  
- Efficient value updates  
- Predictable performance even with large datasets