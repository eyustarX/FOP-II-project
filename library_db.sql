CREATE DATABASE IF NOT EXISTS library_db;
USE library_db;

-- 1. USERS TABLE (member_code REMOVED)
CREATE TABLE users(
    id INT AUTO_INCREMENT PRIMARY KEY,
    full_name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    role ENUM('admin', 'librarian', 'member') NOT NULL,
    phone VARCHAR(20),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    INDEX idx_email (email),
    INDEX idx_role (role)
);

-- 2. BOOKS TABLE (same)
CREATE TABLE books(
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    author VARCHAR(150) NOT NULL,
    isbn VARCHAR(20) NOT NULL UNIQUE,
    category VARCHAR(80),
    publisher VARCHAR(150),
    published_year YEAR,
    total_copies INT NOT NULL DEFAULT 1,
    available_copies INT NOT NULL DEFAULT 1,
    borrowed_copies INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_title (title),
    INDEX idx_isbn (isbn)
);

-- 3. BORROW RECORDS TABLE (same)
CREATE TABLE borrow_records(
    id INT AUTO_INCREMENT PRIMARY KEY,
    member_id INT NOT NULL,
    book_id INT NOT NULL,
    issued_by INT NULL,
    borrow_date DATE NOT NULL,
    due_date DATE NOT NULL,
    return_date DATE,
    status ENUM('borrowed', 'returned') NOT NULL DEFAULT 'borrowed',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (member_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    FOREIGN KEY (issued_by) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_member_id (member_id),
    INDEX idx_book_id (book_id),
    INDEX idx_status (status)
);

-- 4. ACTIVITY LOGS TABLE (same)
CREATE TABLE activity_logs(
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    action VARCHAR(100) NOT NULL,
    target_table VARCHAR(50),
    target_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_user_id (user_id),
    INDEX idx_action (action)
);

INSERT INTO users (full_name, email, password, role, phone) VALUES
('System Admin', 'admin@library.com', 'admin123', 'admin', '0911000000'),
('Ermiyas Alemayehu', 'ermiyas.alemayehu@library.com', 'librarian123', 'librarian', '0911222333'),
('Eyob Arega', 'eyob.arega@library.com', 'librarian123', 'librarian', '0911444555'),
('Eyob Tamiru', 'eyob.tamiru@email.com', 'member123', 'member', '0911666777'),
('Ermiyas Getachew', 'ermiyas.getachew@email.com', 'member123', 'member', '0911888999'),
('Eyerusalem Hailu', 'eyerusalem.hailu@email.com', 'member123', 'member', '0911000111');

INSERT INTO books (title, author, isbn, category, publisher, published_year, total_copies, available_copies, borrowed_copies) VALUES

('Engineering Mechanics: Statics and Dynamics', 'R.C. Hibbeler', '978-0-13-391542-6', 'Mechanical Engineering', 'Pearson', 2016, 15, 15, 0),
('Fundamentals of Electric Circuits', 'Charles K. Alexander', '978-0-07-802822-9', 'Electrical Engineering', 'McGraw-Hill', 2016, 12, 12, 0),
('Structural Analysis', 'R.C. Hibbeler', '978-0-13-461067-2', 'Civil Engineering', 'Pearson', 2017, 12, 12, 0),
('Materials Science and Engineering: An Introduction', 'William D. Callister Jr.', '978-1-119-40549-8', 'Materials Engineering', 'Wiley', 2018, 15, 15, 0),
('Advanced Engineering Mathematics', 'Erwin Kreyszig', '978-0-470-45836-5', 'Mathematics', 'Wiley', 2011, 20, 20, 0),
('Transport Processes and Separation Processes', 'Christie J. Geankoplis', '978-0-13-101367-4', 'Chemical Engineering', 'Prentice Hall', 2003, 10, 10, 0),
('Thermodynamics: An Engineering Approach', 'Yunus A. Cengel', '978-0-07-339817-4', 'Mechanical Engineering', 'McGraw-Hill', 2014, 15, 15, 0),
('Software Engineering: A Practitioner''s Approach', 'Roger S. Pressman', '978-0-07-802212-8', 'Software Engineering', 'McGraw-Hill', 2014, 15, 15, 0),
('Clean Code: A Handbook of Agile Software Craftsmanship', 'Robert C. Martin', '978-0-13-235088-4', 'Software Engineering', 'Prentice Hall', 2008, 15, 15, 0),
('Design Patterns: Elements of Reusable Object-Oriented Software', 'Erich Gamma', '978-0-20-163361-0', 'Software Design', 'Addison-Wesley', 1994, 12, 12, 0),
('Software Project Management', 'Bob Hughes', '978-0-07-712279-9', 'Project Management', 'McGraw-Hill', 2012, 12, 12, 0);
