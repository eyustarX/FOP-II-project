#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <mysql.h>
#include <limits>
#include <sstream>
using namespace std;

const string DB_HOST = "localhost";
const string DB_USER = "root";
const string DB_PASS = "password";
const string DB_NAME = "library_db";
const int DB_PORT = 3306;

MYSQL *conn = nullptr;

struct Session {
    int    userId = -1;
    string role   = "";
    string name   = "";
} session;

string formatMemberCode(int userId)
{
    stringstream ss;
    ss << "MBR-" << setw(4) << setfill('0') << userId;
    return ss.str();
}

bool getValidatedIntInput(const string& prompt, int& value, int minValue = 1, int maxValue = INT_MAX);
bool getValidatedStringInput(const string& prompt, string& value, bool allowEmpty = false);
bool getValidatedYearInput(const string& prompt, int& year);
bool getValidatedEmailInput(const string& prompt, string& email);
bool getValidatedPhoneInput(const string& prompt, string& phone);
void changePassword();

string today()
{
    time_t t = time(nullptr);
    tm *now = localtime(&t);
    stringstream ss;
    ss << put_time(now, "%Y-%m-%d");
    return ss.str();
}

string addDays(const string &date, int days)
{
    int year  = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    int day   = stoi(date.substr(8, 2));
    day += days;
    int dim[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    while (day > dim[month])
    {
        day -= dim[month];
        month++;
        if (month > 12)
        {
            month = 1;
            year++;
            // Check for leap year
            bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            dim[2] = isLeap ? 29 : 28;
        }
    }
    string result = to_string(year) + "-";
    if (month < 10) result += "0";
    result += to_string(month) + "-";
    if (day < 10) result += "0";
    result += to_string(day);
    return result;
}

void printLine(char c = '-', int w = 65)
{
    for (int i = 0; i < w; i++)
        cout << c;
    cout << "\n";
}

void printHeader(const string &title)
{
    printLine('=');
    cout << "\t\t\t" << title << "\n";
    printLine('=');
}

string escapeStr(const string &s)
{
    if (!conn)
        return s;
    char *buf = new char[s.size() * 2 + 1];
    mysql_real_escape_string(conn, buf, s.c_str(), s.size());
    string result(buf);
    delete[] buf;
    return result;
}

bool execSQL(const string &sql)
{
    if (mysql_query(conn, sql.c_str()))
    {
        cerr << "  [SQL Error] " << mysql_error(conn) << "\n";
        return false;
    }
    return true;
}

MYSQL_RES *querySQL(const string &sql)
{
    if (mysql_query(conn, sql.c_str()))
    {
        cerr << "  [SQL Error] " << mysql_error(conn) << "\n";
        return nullptr;
    }
    return mysql_store_result(conn);
}

string field(MYSQL_ROW row, int i)
{
    return row[i] ? string(row[i]) : "";
}

bool getValidatedIntInput(const string& prompt, int& value, int minValue, int maxValue)
{
    cout << prompt;
    cin >> value;
    if (cin.fail())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "   Invalid input! Please enter a valid number.\n";
        return false;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (value < minValue || value > maxValue)
    {
        cout << "   Value must be between " << minValue << " and " << maxValue << ".\n";
        return false;
    }
    return true;
}

bool getValidatedStringInput(const string& prompt, string& value, bool allowEmpty)
{
    cout << prompt;
    getline(cin, value);

    if (!allowEmpty && value.empty())
    {
        cout << "   This field cannot be empty.\n";
        return false;
    }
    return true;
}

bool getValidatedYearInput(const string& prompt, int& year)
{
    cout << prompt;
    cin >> year;
    if (cin.fail())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "   Invalid input! Please enter a valid year.\n";
        return false;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    time_t t = time(nullptr);
    int currentYear = localtime(&t)->tm_year + 1900;

    if (year < 1400 || year > currentYear)
    {
        cout << "   Invalid year! Please enter a year between 1400 and " << currentYear << ".\n";
        return false;
    }
    return true;
}

bool getValidatedEmailInput(const string& prompt, string& email)
{
    cout << prompt;
    getline(cin, email);
    if (email.empty())
    {
        cout << "   Email cannot be empty.\n";
        return false;
    }
    size_t at = email.find('@');
    if (at == string::npos || at == 0 || email.find('.', at) == string::npos)
    {
        cout << "   Invalid email format! Example: user@domain.com\n";
        return false;
    }
    return true;
}

bool getValidatedPhoneInput(const string& prompt, string& phone)
{
    cout << prompt;
    getline(cin, phone);
    if (phone.empty())
    {
        cout << "   Phone number cannot be empty.\n";
        return false;
    }
    if (phone.length() < 10)
    {
        cout << "   Invalid phone number! Please enter at least 10 digits.\n";
        return false;
    }
    for (char c : phone)
    {
        if (!isdigit(c) && c != '-' && c != '+')
        {
            cout << "   Invalid phone number! Use digits, -, or + only.\n";
            return false;
        }
    }
    return true;
}

void addLog(int userId, const string &action,
            const string &table = "", int targetId = 0)
{
    string sql = "INSERT INTO activity_logs (user_id, action, target_table, target_id) VALUES (" +
                 to_string(userId) + ", '" + escapeStr(action) + "', '" + escapeStr(table) + "', " + to_string(targetId) + ")";
    execSQL(sql);
}

void changePassword()
{
    printHeader("CHANGE PASSWORD");

    string oldPass, newPass, confirmPass;

    cout << "  Enter current password: ";
    getline(cin, oldPass);

    MYSQL_RES *res = querySQL("SELECT id FROM users WHERE id=" + to_string(session.userId) +
        " AND password='" + escapeStr(oldPass) + "'");

    if (!res || mysql_num_rows(res) == 0)
    {
        if (res) mysql_free_result(res);
        cout << "\n   Current password is incorrect!\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    mysql_free_result(res);

    cout << "  Enter new password: ";
    getline(cin, newPass);

    if (newPass.length() < 4)
    {
        cout << "   Password must be at least 4 characters long.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    cout << "  Confirm new password: ";
    getline(cin, confirmPass);
    if (newPass != confirmPass)
    {
        cout << "   Passwords do not match!\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    if (execSQL("UPDATE users SET password='" + escapeStr(newPass) + "' WHERE id=" + to_string(session.userId)))
    {
        addLog(session.userId, "CHANGE_PASSWORD", "users", session.userId);
        cout << "\n   Password changed successfully!\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

bool connectDB()
{
    conn = mysql_init(nullptr);
    if (!conn)
    {
        cerr << "  mysql_init failed\n";
        return false;
    }

    if (!mysql_real_connect(conn, DB_HOST.c_str(), DB_USER.c_str(), DB_PASS.c_str(),
                            DB_NAME.c_str(), DB_PORT, nullptr, 0))
    {
        cerr << "\n  [DB Connection Failed]\n"
             << "  " << mysql_error(conn) << "\n"
             << "\n  Make sure:\n"
             << "    1. MySQL is running\n"
             << "    2. You ran library_db.sql\n"
             << "    3. DB_USER / DB_PASS match your MySQL setup\n\n";
        return false;
    }
    return true;
}

void closeDB()
{
    if (conn)
    {
        mysql_close(conn);
        conn = nullptr;
    }
}

void seedAdmin()
{
    MYSQL_RES *res = querySQL(
        "SELECT id FROM users WHERE role='admin' LIMIT 1");
    if (!res)
        return;
    bool exists = (mysql_num_rows(res) > 0);
    mysql_free_result(res);
    if (exists)
        return;

    execSQL("INSERT INTO users (full_name, email, password, role, phone) "
            "VALUES ('System Admin','admin@library.com','admin123','admin','000-000-0000')");
    cout << "  Default admin created: admin@library.com / admin123\n";
}

void backupSystem()
{
    printHeader("BACKUP SYSTEM");
    try {
        {
            ofstream f("users_backup.txt");
            if (!f.is_open())
            {
                cout << "   Failed to create users_backup.txt\n";
                return;
            }
            MYSQL_RES *res = querySQL("SELECT * FROM users");
            if (res)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)))
                    f << field(row, 0) << "|" << field(row, 1) << "|"
                      << field(row, 2) << "|" << field(row, 3) << "|"
                      << field(row, 4) << "|" << field(row, 5) << "|"
                      << field(row, 6) << "\n";
                mysql_free_result(res);
            }
            f.close();
        }
        {
            ofstream f("books_backup.txt");
            if (!f.is_open())
            {
                cout << "   Failed to create books_backup.txt\n";
                return;
            }
            MYSQL_RES *res = querySQL("SELECT * FROM books");
            if (res)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)))
                    f << field(row, 0) << "|" << field(row, 1) << "|"
                      << field(row, 2) << "|" << field(row, 3) << "|"
                      << field(row, 4) << "|" << field(row, 5) << "|"
                      << field(row, 6) << "|" << field(row, 7) << "|"
                      << field(row, 8) << "|" << field(row, 9) << "\n";
                mysql_free_result(res);
            }
            f.close();
        }
        {
            ofstream f("borrows_backup.txt");
            if (!f.is_open())
            {
                cout << "   Failed to create borrows_backup.txt\n";
                return;
            }
            MYSQL_RES *res = querySQL("SELECT * FROM borrow_records");
            if (res)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)))
                    f << field(row, 0) << "|" << field(row, 1) << "|"
                      << field(row, 2) << "|" << field(row, 3) << "|"
                      << field(row, 4) << "|" << field(row, 5) << "|"
                      << field(row, 6) << "|" << field(row, 7) << "|"
                      << field(row, 8) << "\n";
                mysql_free_result(res);
            }
            f.close();
        }
        {
            ofstream f("logs_backup.txt");
            if (!f.is_open())
            {
                cout << "   Failed to create logs_backup.txt\n";
                return;
            }
            MYSQL_RES *res = querySQL("SELECT * FROM activity_logs");
            if (res)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)))
                    f << field(row, 0) << "|" << field(row, 1) << "|"
                      << field(row, 2) << "|" << field(row, 3) << "|"
                      << field(row, 4) << "|" << field(row, 5) << "\n";
                mysql_free_result(res);
            }
            f.close();
        }
        addLog(session.userId, "BACKUP");
        cout << "\n   Backup saved successfully to:\n"
             << "    users_backup.txt\n"
             << "    books_backup.txt\n"
             << "    borrows_backup.txt\n"
             << "    logs_backup.txt\n";
    }
    catch (const exception& e)
    {
        cout << "   Backup failed: " << e.what() << "\n";
    }

    cout << "\n  Press Enter to continue...";
    cin.get();
}

void registerMember()
{
    printHeader("REGISTER NEW MEMBER");

    string fullName, email, password, phone;

    if (!getValidatedStringInput("  Full Name : ", fullName, false))
        return;
    if (!getValidatedEmailInput("  Email     : ", email))
        return;
    if (!getValidatedStringInput("  Password  : ", password, false))
        return;
    if (password.length() < 4)
    {
        cout << "   Password must be at least 4 characters long.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    if (!getValidatedPhoneInput("  Phone     : ", phone))
        return;

    MYSQL_RES *res = querySQL(
        "SELECT id FROM users WHERE email='" + escapeStr(email) + "'");
    if (res)
    {
        bool dup = mysql_num_rows(res) > 0;
        mysql_free_result(res);
        if (dup)
        {
            cout << "\n   Email already registered.\n";
            cout << "\n  Press Enter to continue...";
            cin.get();
            return;
        }
    }
    string sql = "INSERT INTO users (full_name, email, password, role, phone) "
        "VALUES ('" + escapeStr(fullName) + "','" + escapeStr(email) + "','" +
        escapeStr(password) + "','member','" + escapeStr(phone) + "')";

    if (execSQL(sql))
    {
        int newId = (int)mysql_insert_id(conn);
        addLog(newId, "REGISTER", "users", newId);
        cout << "\n   Account created successfully!\n";
        cout << "   Member ID: " << newId << "\n";
        cout << "   Member Code: " << formatMemberCode(newId) << "\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

bool login()
{
    printHeader("LOGIN");
    cout << "  Select Role:\n"
         << "    1. Admin\n"
         << "    2. Librarian\n"
         << "    3. Member\n"
         << "  Choice: ";
    int ch;
    cin >> ch;
    if (cin.fail())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "   Invalid choice.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return false;
    }
    cin.ignore();

    string roleFilter;
    if (ch == 1)
        roleFilter = "admin";
    else if (ch == 2)
        roleFilter = "librarian";
    else if (ch == 3)
        roleFilter = "member";
    else
    {
        cout << "   Invalid choice. Please select 1, 2, or 3.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return false;
    }

    string email, pass;
    if (!getValidatedEmailInput("  Email    : ", email))
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return false;
    }
    if (!getValidatedStringInput("  Password : ", pass, false))
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return false;
    }

    string sql = "SELECT id, full_name, role FROM users "
                 "WHERE email='" + escapeStr(email) +
                 "' AND password='" + escapeStr(pass) + "' AND role='" + roleFilter + "'";

    MYSQL_RES *res = querySQL(sql);
    if (!res || mysql_num_rows(res) == 0)
    {
        if (res)
            mysql_free_result(res);
        cout << "\n   Invalid credentials or wrong role.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    session.userId = stoi(field(row, 0));
    session.name   = field(row, 1);
    session.role   = field(row, 2);
    mysql_free_result(res);

    addLog(session.userId, "LOGIN", "users", session.userId);
    cout << "\n   Welcome, " << session.name
         << "!  Role: " << session.role << "\n";
    return true;
}

void logout()
{
    addLog(session.userId, "LOGOUT", "users", session.userId);
    cout << "\n   Session ended. Goodbye, " << session.name << "!\n";
    session.userId = -1;
    session.role   = "";
    session.name   = "";
}

void addBook()
{
    printHeader("ADD NEW BOOK");
    string title, author, isbn, category, publisher;
    int year, copies;

    if (!getValidatedStringInput("  Title          : ", title, false))
        return;
    if (!getValidatedStringInput("  Author         : ", author, false))
        return;
    if (!getValidatedStringInput("  ISBN           : ", isbn, false))
        return;

    MYSQL_RES *r = querySQL(
        "SELECT id FROM books WHERE isbn='" + escapeStr(isbn) + "'");
    if (r)
    {
        bool dup = mysql_num_rows(r) > 0;
        mysql_free_result(r);
        if (dup)
        {
            cout << "\n   ISBN already exists.\n";
            cout << "\n  Press Enter to continue...";
            cin.get();
            return;
        }
    }
    if (!getValidatedStringInput("  Category       : ", category, false))
        return;
    if (!getValidatedStringInput("  Publisher      : ", publisher, false))
        return;
    if (!getValidatedYearInput("  Published Year : ", year))
        return;
    if (!getValidatedIntInput("  How many copies to add : ", copies, 1, 1000))
        return;

    string sql =
       "INSERT INTO books (title,author,isbn,category,publisher,published_year,"
       "total_copies,available_copies,borrowed_copies) VALUES ('" + escapeStr(title) + "','" + escapeStr(author) +
       "','" + escapeStr(isbn) + "','" + escapeStr(category) + "','" + escapeStr(publisher) +
       "'," + to_string(year) + "," + to_string(copies) + "," + to_string(copies) + ",0)";

    if (execSQL(sql))
    {
        int newId = (int)mysql_insert_id(conn);
        addLog(session.userId, "ADD_BOOK", "books", newId);
        cout << "\n   Book added successfully! Book ID = " << newId << "\n";
        cout << "   Added " << copies << " copy(ies) to inventory.\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void viewAllBooks()
{
    printHeader("ALL BOOKS");
    cout << left << setw(5) << "ID" << setw(32) << "Title" << setw(22) << "Author"
         << setw(7) << "Avail" << setw(7) << "Total" << "\n";
    printLine();

    MYSQL_RES *res = querySQL(
        "SELECT id,title,author,available_copies,total_copies FROM books");
    if (!res)
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row;
    bool any = false;
    while ((row = mysql_fetch_row(res)))
    {
        any = true;
        string t = field(row, 1);
        if (t.size() > 30)
            t = t.substr(0, 29) + "..";
        string a = field(row, 2);
        if (a.size() > 20)
            a = a.substr(0, 19) + "..";
        cout << left << setw(5) << field(row, 0) << setw(32) << t << setw(22) << a
             << setw(7) << field(row, 3) << setw(7) << field(row, 4) << "\n";
    }
    mysql_free_result(res);
    if (!any)
        cout << "  No books found.\n";

    cout << "\n  Press Enter to continue...";
    cin.get();
}

void searchBook()
{
    printHeader("SEARCH BOOK");
    cout << "  Keyword (title / author / ISBN): ";
    string kw;
    getline(cin, kw);

    if (kw.empty())
    {
        cout << "   Please enter a search keyword.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }

    string sql = "SELECT id,title,author,isbn,available_copies,total_copies FROM books "
                 "WHERE title LIKE '%" + escapeStr(kw) + "%' "
                 "OR author LIKE '%" + escapeStr(kw) + "%' "
                 "OR isbn LIKE '%" + escapeStr(kw) + "%'";

    MYSQL_RES *res = querySQL(sql);
    if (!res)
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row;
    bool found = false;
    while ((row = mysql_fetch_row(res)))
    {
        found = true;
        cout << "\n  ID       : " << field(row, 0)
             << "\n  Title    : " << field(row, 1)
             << "\n  Author   : " << field(row, 2)
             << "\n  ISBN     : " << field(row, 3)
             << "\n  Avail    : " << field(row, 4) << "/" << field(row, 5)
             << "\n";
        printLine('-', 45);
    }
    mysql_free_result(res);
    if (!found)
        cout << "  No books matched.\n";
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void updateBook()
{
    printHeader("UPDATE BOOK");
    int id;
    if (!getValidatedIntInput("  Book ID: ", id, 1))
        return;

    MYSQL_RES *res = querySQL(
        "SELECT title, author, total_copies, available_copies, borrowed_copies FROM books WHERE id=" + to_string(id));
    if (!res || mysql_num_rows(res) == 0)
    {
        if (res)
            mysql_free_result(res);
        cout << "   Book not found.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string title        = field(row, 0);
    string author       = field(row, 1);
    int currentTotal    = stoi(field(row, 2));
    int currentAvailable= stoi(field(row, 3));
    int currentBorrowed = stoi(field(row, 4));
    mysql_free_result(res);

    cout << "\n  Current Information:\n";
    cout << "  Title    : " << title << "\n";
    cout << "  Author   : " << author << "\n";
    cout << "  Copies   : " << currentAvailable << "/" << currentTotal << " available\n\n";

    cout << "  New title [" << title << "]: ";
    string nt;
    getline(cin, nt);
    if (nt.empty()) nt = title;
    cout << "  New author [" << author << "]: ";
    string na;
    getline(cin, na);
    if (na.empty()) na = author;
    cout << "  New total copies [" << currentTotal << "]: ";
    string copiesInput;
    getline(cin, copiesInput);
    int newTotal = currentTotal;
    if (!copiesInput.empty())
    {
        newTotal = stoi(copiesInput);
        if (newTotal < currentBorrowed)
        {
            cout << "   Cannot reduce to " << newTotal << " copies because " << currentBorrowed << " books are borrowed.\n";
            cout << "  Total copies will remain " << currentTotal << "\n";
            newTotal = currentTotal;
        }
        else if (newTotal < 0)
        {
            cout << "   Invalid number! Total copies will remain " << currentTotal << "\n";
            newTotal = currentTotal;
        }
        else
        {
            int newAvailable = newTotal - currentBorrowed;
            execSQL("UPDATE books SET available_copies=" + to_string(newAvailable) + " WHERE id=" + to_string(id));
        }
    }
    string sql = "UPDATE books SET title='" + escapeStr(nt) +
                 "', author='" + escapeStr(na) +
                 "', total_copies=" + to_string(newTotal) +
                 " WHERE id=" + to_string(id);

    if (execSQL(sql))
    {
        addLog(session.userId, "UPDATE_BOOK", "books", id);
        cout << "\n   Book updated successfully.\n";
        if (newTotal != currentTotal)
            cout << "   Total copies changed from " << currentTotal << " to " << newTotal << "\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void deleteBook()
{
    printHeader("DELETE BOOK");
    int id;
    if (!getValidatedIntInput("  Book ID: ", id, 1))
        return;
    cout << "   WARNING: This action cannot be undone!\n";
    cout << "  Are you sure you want to delete this book? (yes/no): ";
    string confirm;
    getline(cin, confirm);
    if (confirm != "yes" && confirm != "YES" && confirm != "y")
    {
        cout << "  Delete cancelled.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    if (execSQL("DELETE FROM books WHERE id=" + to_string(id)))
    {
        addLog(session.userId, "DELETE_BOOK", "books", id);
        cout << "\n   Book deleted successfully.\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void checkAvailability()
{
    printHeader("CHECK AVAILABILITY");
    int id;
    if (!getValidatedIntInput("  Book ID: ", id, 1))
        return;

    MYSQL_RES *res = querySQL(
        "SELECT title,available_copies,total_copies FROM books WHERE id=" + to_string(id));
    if (!res || mysql_num_rows(res) == 0)
    {
        if (res)
            mysql_free_result(res);
        cout << "   Book not found.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    cout << "\n  Title    : " << field(row, 0)
         << "\n  Available: " << field(row, 1)
         << " / " << field(row, 2) << "\n";
    mysql_free_result(res);

    cout << "\n  Press Enter to continue...";
    cin.get();
}

void viewMembers()
{
    printHeader("ALL MEMBERS");
    cout << left
         << setw(5)  << "ID"
         << setw(12) << "Member Code"
         << setw(25) << "Name"
         << setw(30) << "Email" << "\n";
    printLine();

    MYSQL_RES *res = querySQL(
        "SELECT id, full_name, email FROM users WHERE role='member'");
    if (!res)
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row;
    bool any = false;
    while ((row = mysql_fetch_row(res)))
    {
        any = true;
        int userId = stoi(field(row, 0));
        cout << left
             << setw(5)  << userId
             << setw(12) << formatMemberCode(userId)
             << setw(25) << field(row, 1).substr(0, 23)
             << setw(30) << field(row, 2).substr(0, 28) << "\n";
    }
    mysql_free_result(res);
    if (!any)
        cout << "  No members found.\n";
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void searchMember()
{
    printHeader("SEARCH MEMBER");
    cout << "  Keyword (name / email): ";
    string kw;
    getline(cin, kw);
    if (kw.empty())
    {
        cout << "   Please enter a search keyword.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }

    string sql = "SELECT id, full_name, email, phone FROM users "
                 "WHERE role='member' AND (full_name LIKE '%" + escapeStr(kw) + "%' "
                 "OR email LIKE '%" + escapeStr(kw) + "%')";

    MYSQL_RES *res = querySQL(sql);
    if (!res)
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row;
    bool found = false;
    while ((row = mysql_fetch_row(res)))
    {
        found = true;
        int userId = stoi(field(row, 0));
        cout << "\n  ID         : " << userId
             << "\n  Member Code: " << formatMemberCode(userId)
             << "\n  Name       : " << field(row, 1)
             << "\n  Email      : " << field(row, 2)
             << "\n  Phone      : " << field(row, 3) << "\n";
        printLine('-', 45);
    }
    mysql_free_result(res);
    if (!found)
        cout << "  No members matched.\n";
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void updateMemberInfo()
{
    printHeader("UPDATE MEMBER INFO");
    int id;
    if (session.role == "member")
        id = session.userId;
    else
    {
        if (!getValidatedIntInput("  Member ID: ", id, 1))
            return;
    }

    MYSQL_RES *res = querySQL(
        "SELECT full_name,phone FROM users WHERE id=" + to_string(id) + " AND role='member'");
    if (!res || mysql_num_rows(res) == 0)
    {
        if (res)
            mysql_free_result(res);
        cout << "   Member not found.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string name = field(row, 0), phone = field(row, 1);
    mysql_free_result(res);

    cout << "  Current name  [" << name << "]: ";
    string nn;
    getline(cin, nn);
    if (nn.empty()) nn = name;

    if (!getValidatedPhoneInput("  New Phone   : ", phone))
        return;

    if (execSQL("UPDATE users SET full_name='" + escapeStr(nn) +
                "', phone='" + escapeStr(phone) +
                "' WHERE id=" + to_string(id)))
    {
        addLog(session.userId, "UPDATE_MEMBER", "users", id);
        cout << "\n   Member information updated successfully.\n";
        if (session.userId == id)
            session.name = nn;
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void issueBook()
{
    printHeader("ISSUE BOOK");
    int mid, bid;

    if (!getValidatedIntInput("  Member ID : ", mid, 1))
        return;
    if (!getValidatedIntInput("  Book ID   : ", bid, 1))
        return;

    MYSQL_RES *r = querySQL(
        "SELECT id FROM users WHERE id=" + to_string(mid) + " AND role='member'");
    if (!r || mysql_num_rows(r) == 0)
    {
        if (r) mysql_free_result(r);
        cout << "   Member not found.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    mysql_free_result(r);

    r = querySQL("SELECT available_copies FROM books WHERE id=" + to_string(bid));
    if (!r || mysql_num_rows(r) == 0)
    {
        if (r) mysql_free_result(r);
        cout << "   Book not found.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(r);
    int avail = stoi(field(row, 0));
    mysql_free_result(r);

    if (avail <= 0)
    {
        cout << "   No copies available for this book.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }

    string bd = today(), dd = addDays(bd, 14);
    string sql = "INSERT INTO borrow_records (member_id,book_id,issued_by,borrow_date,due_date,status,renew_count) "
        "VALUES (" + to_string(mid) + "," + to_string(bid) + "," + to_string(session.userId) +
        ",'" + bd + "','" + dd + "','borrowed',0)";

    if (execSQL(sql))
    {
        int newId = (int)mysql_insert_id(conn);
        execSQL("UPDATE books SET available_copies=available_copies-1, "
                "borrowed_copies=borrowed_copies+1 WHERE id=" + to_string(bid));
        addLog(session.userId, "ISSUE_BOOK", "borrow_records", newId);
        cout << "\n   Book issued successfully!"
             << "\n  Record ID : " << newId
             << "\n  Due Date  : " << dd << "\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void returnBook()
{
    printHeader("RETURN BOOK");
    int rid;
    if (!getValidatedIntInput("  Borrow Record ID: ", rid, 1))
        return;

    MYSQL_RES *res = querySQL(
        "SELECT book_id,status FROM borrow_records WHERE id=" + to_string(rid));
    if (!res || mysql_num_rows(res) == 0)
    {
        if (res) mysql_free_result(res);
        cout << "   Borrow record not found.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int bookId     = stoi(field(row, 0));
    string status  = field(row, 1);
    mysql_free_result(res);

    if (status == "returned")
    {
        cout << "   This book has already been returned.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }

    if (execSQL("UPDATE borrow_records SET return_date='" + today() +
                "', status='returned' WHERE id=" + to_string(rid)))
    {
        execSQL("UPDATE books SET available_copies=available_copies+1, "
                "borrowed_copies=borrowed_copies-1 WHERE id=" + to_string(bookId));
        addLog(session.userId, "RETURN_BOOK", "borrow_records", rid);
        cout << "\n   Book returned successfully on " << today() << "\n";
    }

    cout << "\n  Press Enter to continue...";
    cin.get();
}

void viewBorrowedBooks()
{
    printHeader("CURRENTLY BORROWED BOOKS");
    cout << left << setw(6)  << "RecID" << setw(6)  << "BkID" << setw(27) << "Book Title"
         << setw(7)  << "MbrID" << setw(12) << "Due Date" << setw(10) << "Status" << "\n";
    printLine();

    MYSQL_RES *res = querySQL(
        "SELECT br.id, br.book_id, b.title, br.member_id, br.due_date, br.status "
        "FROM borrow_records br JOIN books b ON br.book_id=b.id WHERE br.status != 'returned'");
    if (!res)
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row;
    bool any = false;
    while ((row = mysql_fetch_row(res)))
    {
        any = true;
        string t = field(row, 2);
        if (t.size() > 25)
            t = t.substr(0, 24) + "..";
        cout << left << setw(6)  << field(row, 0) << setw(6)  << field(row, 1)
             << setw(27) << t << setw(7)  << field(row, 3)
             << setw(12) << field(row, 4) << setw(10) << field(row, 5) << "\n";
    }
    mysql_free_result(res);
    if (!any)
        cout << "  No active borrows found.\n";
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void addLibrarian()
{
    printHeader("ADD LIBRARIAN");
    string name, email, pass, phone;

    if (!getValidatedStringInput("  Full Name : ", name, false))
        return;
    if (!getValidatedEmailInput("  Email     : ", email))
        return;
    if (!getValidatedStringInput("  Password  : ", pass, false))
        return;
    if (pass.length() < 4)
    {
        cout << "   Password must be at least 4 characters long.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    if (!getValidatedPhoneInput("  Phone     : ", phone))
        return;

    MYSQL_RES *r = querySQL(
        "SELECT id FROM users WHERE email='" + escapeStr(email) + "'");
    if (r)
    {
        bool dup = mysql_num_rows(r) > 0;
        mysql_free_result(r);
        if (dup)
        {
            cout << "\n   Email already in use.\n";
            cout << "\n  Press Enter to continue...";
            cin.get();
            return;
        }
    }

    string sql = "INSERT INTO users (full_name,email,password,role,phone) VALUES ('" +
        escapeStr(name) + "','" + escapeStr(email) + "','" +
        escapeStr(pass) + "','librarian','" + escapeStr(phone) + "')";

    if (execSQL(sql))
    {
        int newId = (int)mysql_insert_id(conn);
        addLog(session.userId, "ADD_LIBRARIAN", "users", newId);
        cout << "\n   Librarian added successfully. ID = " << newId << "\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void viewLibrarians()
{
    printHeader("ALL LIBRARIANS");
    cout << left << setw(5) << "ID" << setw(25) << "Name" << setw(30) << "Email" << "\n";
    printLine();

    MYSQL_RES *res = querySQL(
        "SELECT id,full_name,email FROM users WHERE role='librarian'");
    if (!res)
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row;
    bool any = false;
    while ((row = mysql_fetch_row(res)))
    {
        any = true;
        cout << left << setw(5)  << field(row, 0)
             << setw(25) << field(row, 1).substr(0, 23)
             << setw(30) << field(row, 2) << "\n";
    }
    mysql_free_result(res);
    if (!any)
        cout << "  No librarians found.\n";
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void updateLibrarian()
{
    printHeader("UPDATE LIBRARIAN");
    int id;
    if (!getValidatedIntInput("  Librarian ID: ", id, 1))
        return;

    MYSQL_RES *res = querySQL(
        "SELECT full_name,phone FROM users WHERE id=" + to_string(id) + " AND role='librarian'");
    if (!res || mysql_num_rows(res) == 0)
    {
        if (res) mysql_free_result(res);
        cout << "   Librarian not found.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string name = field(row, 0), phone = field(row, 1);
    mysql_free_result(res);

    cout << "  Current name  [" << name << "]: ";
    string nn;
    getline(cin, nn);
    if (nn.empty()) nn = name;

    if (!getValidatedPhoneInput("  New Phone   : ", phone))
        return;

    if (execSQL("UPDATE users SET full_name='" + escapeStr(nn) +
                "', phone='" + escapeStr(phone) + "' WHERE id=" + to_string(id)))
    {
        addLog(session.userId, "UPDATE_LIBRARIAN", "users", id);
        cout << "\n   Librarian updated successfully.\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void searchLibrarian()
{
    printHeader("SEARCH LIBRARIAN");
    cout << "  Keyword: ";
    string kw;
    getline(cin, kw);

    if (kw.empty())
    {
        cout << "   Please enter a search keyword.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }

    string sql = "SELECT id,full_name,email,phone FROM users WHERE role='librarian' "
                 "AND (full_name LIKE '%" + escapeStr(kw) + "%' "
                 "OR email LIKE '%" + escapeStr(kw) + "%')";

    MYSQL_RES *res = querySQL(sql);
    if (!res)
    {
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row;
    bool found = false;
    while ((row = mysql_fetch_row(res)))
    {
        found = true;
        cout << "\n  ID   : " << field(row, 0)
             << "\n  Name : " << field(row, 1)
             << "\n  Email: " << field(row, 2)
             << "\n  Phone: " << field(row, 3) << "\n";
        printLine('-', 45);
    }
    mysql_free_result(res);
    if (!found)
        cout << "  No librarians matched.\n";
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void removeLibrarian()
{
    printHeader("REMOVE LIBRARIAN");
    int id;
    if (!getValidatedIntInput("  Librarian ID: ", id, 1))
        return;
    cout << "   WARNING: This action cannot be undone!\n";
    cout << "  Are you sure you want to remove this librarian? (yes/no): ";
    string confirm;
    getline(cin, confirm);

    if (confirm != "yes" && confirm != "YES" && confirm != "y")
    {
        cout << "  Removal cancelled.\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    if (execSQL("DELETE FROM users WHERE id=" + to_string(id) + " AND role='librarian'"))
    {
        addLog(session.userId, "REMOVE_LIBRARIAN", "users", id);
        cout << "\n   Librarian removed successfully.\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void memberBorrowBook()
{
    printHeader("BORROW A BOOK");

    int bookId;
    if (!getValidatedIntInput("  Enter Book ID to borrow: ", bookId, 1))
        return;

    MYSQL_RES *res = querySQL(
        "SELECT title, available_copies FROM books WHERE id=" + to_string(bookId));

    if (!res || mysql_num_rows(res) == 0)
    {
        if (res) mysql_free_result(res);
        cout << "\n   Book not found!\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    string title  = field(row, 0);
    int available = stoi(field(row, 1));
    mysql_free_result(res);

    if (available <= 0)
    {
        cout << "\n   Sorry, this book is currently not available!\n";
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }

    res = querySQL(
        "SELECT COUNT(*) as count FROM borrow_records "
        "WHERE member_id=" + to_string(session.userId) + " AND status='borrowed'");

    if (res)
    {
        row = mysql_fetch_row(res);
        int borrowedCount = stoi(field(row, 0));
        mysql_free_result(res);

        if (borrowedCount >= 3)
        {
            cout << "\n   You have already borrowed " << borrowedCount << " books.\n";
            cout << "     Maximum borrow limit is 3 books.\n";
            cout << "     Please return some books first.\n";
            cout << "\n  Press Enter to continue...";
            cin.get();
            return;
        }
    }

    string bd = today();
    string dd = addDays(bd, 14);
    string sql = "INSERT INTO borrow_records "
                 "(member_id, book_id, issued_by, borrow_date, due_date, status, renew_count) "
                 "VALUES (" + to_string(session.userId) + "," + to_string(bookId) + "," +
                 to_string(session.userId) + ",'" + bd + "','" + dd + "','borrowed',0)";

    if (execSQL(sql))
    {
        int newId = (int)mysql_insert_id(conn);
        execSQL("UPDATE books SET available_copies=available_copies-1, "
                "borrowed_copies=borrowed_copies+1 WHERE id=" + to_string(bookId));
        addLog(session.userId, "BORROW_BOOK", "borrow_records", newId);

        cout << "\n   Book borrowed successfully!\n";
        cout << "     Book: " << title << "\n";
        cout << "     Due Date: " << dd << "\n";
    }
    cout << "\n  Press Enter to continue...";
    cin.get();
}

void adminMenu()
{
    int choice;
    do {
        printHeader("ADMIN DASHBOARD");
        cout << "   BOOK MANAGEMENT\n";
        cout << "     1. Add Book\n";
        cout << "     2. Update Book\n";
        cout << "     3. Delete Book\n";
        cout << "     4. View All Books\n";
        cout << "     5. Search Book\n";
        cout << "     6. Check Availability\n";
        cout << "   MEMBER MANAGEMENT\n";
        cout << "     7. View All Members\n";
        cout << "     8. Search Member\n";
        cout << "     9. Update Member Info\n";
        cout << "   LIBRARIAN MANAGEMENT\n";
        cout << "     10. Add Librarian\n";
        cout << "     11. View Librarians\n";
        cout << "     12. Update Librarian\n";
        cout << "     13. Search Librarian\n";
        cout << "     14. Remove Librarian\n";
        cout << "   BORROWING SYSTEM\n";
        cout << "     15. Issue Book\n";
        cout << "     16. Return Book\n";
        cout << "     17. View Borrowed Books\n";
        cout << "   ACCOUNT\n";
        cout << "     18. Change Password\n";
        cout << "   SYSTEM\n";
        cout << "     19. Backup System\n";
        cout << "     20. Logout\n";
        cout << "  Choice: ";

        if (!getValidatedIntInput("", choice, 1, 20))
            continue;

        switch(choice) {
            case 1:  addBook(); break;
            case 2:  updateBook(); break;
            case 3:  deleteBook(); break;
            case 4:  viewAllBooks(); break;
            case 5:  searchBook(); break;
            case 6:  checkAvailability(); break;
            case 7:  viewMembers(); break;
            case 8:  searchMember(); break;
            case 9:  updateMemberInfo(); break;
            case 10: addLibrarian(); break;
            case 11: viewLibrarians(); break;
            case 12: updateLibrarian(); break;
            case 13: searchLibrarian(); break;
            case 14: removeLibrarian(); break;
            case 15: issueBook(); break;
            case 16: returnBook(); break;
            case 17: viewBorrowedBooks(); break;
            case 18: changePassword();
                     cout << "\n  Press Enter to continue...";
                     cin.get();
                     break;
            case 19: backupSystem(); break;
            case 20: logout(); return;
        }
    } while(true);
}

void librarianMenu()
{
    int choice;
    do {
        printHeader("LIBRARIAN DASHBOARD");
        cout << "   BOOK MANAGEMENT\n";
        cout << "     1. Add Book\n";
        cout << "     2. Update Book\n";
        cout << "     3. Delete Book\n";
        cout << "     4. View All Books\n";
        cout << "     5. Search Book\n";
        cout << "     6. Check Availability\n";
        cout << "   MEMBER MANAGEMENT\n";
        cout << "     7. View All Members\n";
        cout << "     8. Search Member\n";
        cout << "     9. Update Member Info\n";
        cout << "   BORROWING SYSTEM\n";
        cout << "     10. Issue Book\n";
        cout << "     11. Return Book\n";
        cout << "     12. View Borrowed Books\n";
        cout << "   ACCOUNT\n";
        cout << "     13. Change Password\n";
        cout << "     14. Logout\n";
        cout << "  Choice: ";

        if (!getValidatedIntInput("", choice, 1, 14))
            continue;

        switch(choice) {
            case 1:  addBook(); break;
            case 2:  updateBook(); break;
            case 3:  deleteBook(); break;
            case 4:  viewAllBooks(); break;
            case 5:  searchBook(); break;
            case 6:  checkAvailability(); break;
            case 7:  viewMembers(); break;
            case 8:  searchMember(); break;
            case 9:  updateMemberInfo(); break;
            case 10: issueBook(); break;
            case 11: returnBook(); break;
            case 12: viewBorrowedBooks(); break;
            case 13: changePassword();
                     cout << "\n  Press Enter to continue...";
                     cin.get();
                     break;
            case 14: logout(); return;
        }
    } while(true);
}

void memberMenu()
{
    int choice;
    do {
        printHeader("MEMBER DASHBOARD");
        cout << "     1. Search Books\n";
        cout << "     2. View All Books\n";
        cout << "     3. Check Book Availability\n";
        cout << "     4. Borrow a Book\n";
        cout << "     5. View My Borrowed Books\n";
        cout << "     6. Update My Info\n";
        cout << "     7. Change Password\n";
        cout << "     8. Logout\n";
        cout << "  Choice: ";

        if (!getValidatedIntInput("", choice, 1, 8))
            continue;

        switch(choice) {
            case 1: searchBook(); break;
            case 2: viewAllBooks(); break;
            case 3: checkAvailability(); break;
            case 4: memberBorrowBook(); break;
            case 5: {
                MYSQL_RES *res = querySQL(
                    "SELECT br.id, b.title, br.borrow_date, br.due_date, br.status "
                    "FROM borrow_records br JOIN books b ON br.book_id=b.id "
                    "WHERE br.member_id=" + to_string(session.userId) + " AND br.status='borrowed'");
                if (res) {
                    printHeader("MY CURRENT BORROWED BOOKS");
                    MYSQL_ROW row;
                    bool hasRecords = false;
                    while ((row = mysql_fetch_row(res))) {
                        hasRecords = true;
                        cout << "  Record ID: " << field(row, 0)
                             << "\n  Title    : " << field(row, 1)
                             << "\n  Borrowed : " << field(row, 2)
                             << "\n  Due Date : " << field(row, 3)
                             << "\n  Status   : " << field(row, 4) << "\n\n";
                    }
                    if (!hasRecords) cout << "  You have no borrowed books.\n";
                    mysql_free_result(res);
                }
                cout << "\n  Press Enter to continue...";
                cin.get();
                break;
            }
            case 6: updateMemberInfo(); break;
            case 7: changePassword();
                    cout << "\n  Press Enter to continue...";
                    cin.get();
                    break;
            case 8: logout(); return;
        }
    } while(true);
}

int main()
{
    cout << "\t\tWELCOME TO LIBRARY MANAGEMENT SYSTEM\n";

    if (!connectDB())
    {
        cout << "  Cannot continue without database.\n";
        return 1;
    }

    seedAdmin();
    bool loggedIn = false;

    while (!loggedIn)
    {
        int opt;
        printHeader("MAIN MENU");
        cout << "     1. Login\n";
        cout << "     2. Register as Member\n";
        cout << "     3. Exit\n";
        cout << "  Choice: ";

        if (!getValidatedIntInput("", opt, 1, 3)) continue;

        switch(opt)
        {
            case 1:
                if (login())
                {
                    loggedIn = true;
                    if (session.role == "admin")
                        adminMenu();
                    else if (session.role == "librarian")
                        librarianMenu();
                    else if (session.role == "member")
                        memberMenu();
                    loggedIn = false;
                }
                break;
            case 2:
                registerMember();
                break;
            case 3:
                cout << "\n  Thank you for using Library Management System!\n";
                closeDB();
                return 0;
        }
    }

    closeDB();
    return 0;
}
