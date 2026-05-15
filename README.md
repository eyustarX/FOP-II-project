# 📚 Library Management System

<div align="center">

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=for-the-badge&logo=c%2B%2B)
![MySQL](https://img.shields.io/badge/MySQL-8.0-orange.svg?style=for-the-badge&logo=mysql)

### A complete, production-ready Library Management System built with C++ and MySQL

</div>

---

## 📖 About The Project

The **Library Management System** is a comprehensive console-based application that revolutionizes how libraries operate. Built entirely in **C++** with **MySQL** backend, this system provides a seamless experience for administrators, librarians, and library members.

**Why this project stands out:**
- 🔐 **Three-tier role-based access control**
- 📊 **Complete audit logging** of all activities  
- 💾 **One-click backup system** to text files
- 🔍 **Powerful search capabilities** across books and users
- 📈 **Real-time inventory tracking** with copy management

---

## ✨ Core Features

### 👑 Administrator Portal
| Feature | Description |
|---------|-------------|
| **Book Management** | Add, update, delete, and search books in the catalog |
| **Member Management** | View, search, and update member information |
| **Librarian Management** | Full CRUD operations for librarian accounts |
| **Borrowing System** | Issue books, process returns, track borrowings |
| **Backup System** | Export all database tables to formatted text files |
| **Activity Logs** | Complete audit trail of all system actions |

### 👩‍💼 Librarian Portal
| Feature | Description |
|---------|-------------|
| **Catalog Management** | Full control over book inventory |
| **Member Services** | Assist members with account updates |
| **Transaction Processing** | Issue and return books efficiently |
| **Availability Check** | Real-time book availability status |

### 👤 Member Portal
| Feature | Description |
|---------|-------------|
| **Self-Registration** | Create your own library account |
| **Book Discovery** | Search and browse the entire catalog |
| **Self-Borrowing** | Borrow books (max 3 at a time, 14-day period) |
| **Account Management** | Update profile and change password |
| **Borrowing History** | Track currently borrowed books with due dates |

---


### Database Schema

users (id, full_name, email, password, role, phone, created_at)
books (id, title, author, isbn, category, publisher, published_year, total_copies, available_copies, borrowed_copies)
borrow_records (id, member_id, book_id, issued_by, borrow_date, due_date, return_date, status, renew_count)
activity_logs (id, user_id, action, target_table, target_id, created_at)

library-management-system/
│
├── 📄 library_management.cpp    # Main application 
├── 📄 library_db.sql            # Database schema + sample data
│
├── 📁 backups/                  # Generated backup files
│   ├── users_backup.txt
│   ├── books_backup.txt
│   ├── borrows_backup.txt
│   └── logs_backup.txt
│
└── 📄 README.md                 # Documentation

Contributors

Name                    Role
----------------------------------------
Eyob Tamiru             Lead Developer
Ermiyas Alemayehu       Developer
Eyob Arega              Developer
Ermiyas Getachew        System Designer
Eyerusalem Hailu        Documentation
