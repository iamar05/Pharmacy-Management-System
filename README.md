# 💊 Medicine Stock Management System

A console-based **Medicine Stock Management System** developed in **C** as a Data Structures project. The system is designed to efficiently manage medicine inventory using three fundamental data structures: **Hash Table, Min-Heap, and Queue**.

The project provides features for adding, searching, updating, deleting, and viewing medicines, along with expiry management and a queue-based restocking system.

---

## 🚀 Features

- ➕ **Add New Medicines** — Add medicine records with ID, name, quantity, price, and expiry date.
- 🔍 **Search Medicines by Name** — Quickly search for medicines using their names.
- 📋 **View All Medicines** — Display all medicines currently available in the inventory.
- 📅 **Expiry Management** — View medicines according to their expiry dates.
- ⚠️ **Expiring Soon Alert** — Check the top 5 medicines that will expire soonest.
- 🔄 **Update Medicine Quantity** — Modify the current stock quantity of a medicine.
- 🗑️ **Delete Medicines** — Remove medicines from the inventory.
- 📦 **Request Restock** — Create restock requests for medicines.
- 🔁 **Process Restock Queue** — Process pending restock requests using a queue.
- 📑 **View Pending Restock Requests** — Display all currently pending restock requests.
- 🎨 **Colorful Terminal Interface** — User-friendly command-line interface with ANSI colors and structured menus.
- 🖥️ **Windows Console Support** — Includes UTF-8 and ANSI escape-code support for Windows terminals.

---

## 🧠 Data Structures Used

### 🔹 Hash Table

Used for efficient **medicine searching, insertion, and deletion** based on medicine names.

### 🔹 Min-Heap

Used to organize medicines according to their **expiry dates**, allowing the system to identify medicines that will expire soonest.

### 🔹 Queue

Used to manage **restock requests** following the **FIFO (First In, First Out)** principle.

---

## 🛠️ Technologies

- **Language:** C
- **Data Structures:** Hash Table, Min-Heap, Queue
- **Memory Management:** Dynamic Memory Allocation
- **Interface:** Console / Terminal
- **Compiler:** GCC / MinGW
- **Platform:** Windows / Terminal

---

## 📌 Main Operations

```text
[1]  Add New Medicine
[2]  Search Medicine by Name
[3]  View All Medicines
[4]  View Medicines by Expiry Order
[5]  Check Expiring Soon (Top 5)
[6]  Update Medicine Quantity
[7]  Delete Medicine
[8]  Request Restock
[9]  Process Restock Queue
[10] View Restock Queue
[0]  Exit
