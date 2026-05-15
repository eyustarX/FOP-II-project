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

### `users`
| Column | Type | Description |
|--------|------|-------------|
| `id` | INT (PK) | Auto-increment primary key |
| `full_name` | VARCHAR | Member or staff full name |
| `email` | VARCHAR | Unique login email |
| `password` | VARCHAR | Account password |
| `role` | ENUM | `admin` · `librarian` · `member` |
| `phone` | VARCHAR | Contact phone number |
| `member_code` | VARCHAR | Auto-generated code e.g. `MBR-0001` |
| `created_at` | TIMESTAMP | Account creation date |

### `books`
| Column | Type | Description |
|--------|------|-------------|
| `id` | INT (PK) | Auto-increment primary key |
| `title` | VARCHAR | Book title |
| `author` | VARCHAR | Author name |
| `isbn` | VARCHAR | Unique ISBN identifier |
| `category` | VARCHAR | Genre or subject category |
| `publisher` | VARCHAR | Publishing house |
| `published_year` | YEAR | Year of publication |
| `total_copies` | INT | Total copies owned |
| `available_copies` | INT | Copies currently on shelf |
| `borrowed_copies` | INT | Copies currently issued out |

### `borrow_records`
| Column | Type | Description |
|--------|------|-------------|
| `id` | INT (PK) | Auto-increment primary key |
| `member_id` | INT (FK) | References `users.id` |
| `book_id` | INT (FK) | References `books.id` |
| `issued_by` | INT (FK) | Staff `users.id` who issued the book |
| `borrow_date` | DATE | Date the book was borrowed |
| `due_date` | DATE | Expected return date (borrow + 14 days) |
| `return_date` | DATE | Actual return date |
| `status` | ENUM | `borrowed` · `returned` |
| `renew_count` | INT | Number of times renewed |

### `activity_logs`
| Column | Type | Description |
|--------|------|-------------|
| `id` | INT (PK) | Auto-increment primary key |
| `user_id` | INT (FK) | References `users.id` |
| `action` | VARCHAR | e.g. `LOGIN` · `ADD_BOOK` · `RETURN_BOOK` |
| `target_table` | VARCHAR | Table the action affected |
| `target_id` | INT | Row ID that was affected |
| `created_at` | TIMESTAMP | When the action occurred |

---
  ## library-management-system/
    │
    ├── library_management.cpp    # Main application source
    ├── library_db.sql            # Database schema + sample data
    │
    │   ├── users_backup.txt      # All user records
    │   ├── books_backup.txt      # Full book catalog
    │   ├── borrows_backup.txt    # Borrow history
    │   └── logs_backup.txt       # Activity audit trail
    │
    └── README.md                 # Project documentation
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
