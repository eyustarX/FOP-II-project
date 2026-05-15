<div align="center">

# 📚 Library Management System

![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![MySQL](https://img.shields.io/badge/MySQL-8.0-4479A1?style=for-the-badge&logo=mysql&logoColor=white)
![Status](https://img.shields.io/badge/Status-Complete-2e7d32?style=for-the-badge)
![License](https://img.shields.io/badge/License-Academic-b8860b?style=for-the-badge)

*A console-based library platform built with C++ and MySQL —*
*featuring three-tier access control, real-time inventory tracking, and full audit logging.*

</div>

---

## § About the Project

The **Library Management System** is a console application developed to streamline library operations. Built with **C++** for application logic and **MySQL** for persistent data management, it covers the full lifecycle of library activity — from member registration through to book returns.

The project demonstrates real-world concepts including database connectivity, role-based access control, input validation, file-based backup, and activity auditing — all within a clean, structured C++ codebase.

| | |
|---|---|
| 🔐 **Three-tier access control** | Admin, Librarian, and Member roles with scoped permissions |
| 📊 **Complete audit logging** | Every action is recorded to the `activity_logs` table |
| 💾 **One-click backup** | Exports all four tables to formatted plain-text files |
| 📈 **Live inventory tracking** | Available and borrowed copy counts update in real time |
| 🔍 **Powerful search** | Search books and members by any field |

---

## ⊞ Role Portals

### 👑 Administrator — Full system access

| Feature | Description |
|--------|-------------|
| Book Management | Add, update, delete, and search the entire catalog |
| Member Management | View, search, and update member information |
| Librarian Management | Full CRUD operations for librarian accounts |
| Borrowing System | Issue books and process returns |
| Backup System | Export all database tables to text files |
| Activity Logs | Complete audit trail of all system actions |

### 📋 Librarian — Catalog & transactions

| Feature | Description |
|--------|-------------|
| Catalog Management | Full control over book inventory |
| Member Services | Assist members with account updates |
| Transaction Processing | Issue and return books efficiently |
| Availability Check | Real-time book availability status |

### 📖 Member — Self-service library

| Feature | Description |
|--------|-------------|
| Self-Registration | Create your own account and receive a member code |
| Book Discovery | Search and browse the entire catalog |
| Self-Borrowing | Borrow up to 3 books at a time (14-day period) |
| Account Management | Update profile info and change password |
| Borrowing History | Track active borrows with due dates |

---

## ⊟ Database Schema

users         (id, full_name, email, password, role, phone, member_code, created_at)
books         (id, title, author, isbn, category, publisher, published_year,
               total_copies, available_copies, borrowed_copies)
borrow_records(id, member_id, book_id, issued_by, borrow_date, due_date,
               return_date, status, renew_count)
activity_logs (id, user_id, action, target_table, target_id, created_at)

---

## ⊡ Project Structure
library-management-system/
│
├── 📄 library_management.cpp    # Main application source
├── 📄 library_db.sql            # Database schema + seed data
│
├── 📁 backups/                  # Generated on backup run
│   ├── users_backup.txt
│   ├── books_backup.txt
│   ├── borrows_backup.txt
│   └── logs_backup.txt
│
└── 📄 README.md                 # Documentation
---

## ✦ Contributors

| Avatar | Name | Role |
|--------|------|------|
| 🟫 | **Eyob Tamiru** | Lead Developer |
| 🟦 | **Ermiyas Alemayehu** | Developer |
| 🟩 | **Eyob Arega** | Developer |
| 🟧 | **Ermiyas Getachew** | System Designer |
| 🟪 | **Eyerusalem Hailu** | Documentation |

---

<div align="center">

*Library Management System · C++ & MySQL · Built with care*

</div>
