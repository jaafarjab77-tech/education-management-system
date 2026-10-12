# 🎓 Education Management System

A command-line Education Management System developed in **C** for managing students, courses, grades, and user accounts. The system includes role-based access control and security features designed for educational purposes.

![Language](https://img.shields.io/badge/Language-C-blue)
![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey)
![Project](https://img.shields.io/badge/Type-Educational%20Project-green)

## ✨ Features

### 🔐 Security
- Login attempt limiting
- Account lockout after repeated failed login attempts
- Session timeout after inactivity
- Persistent login-attempt tracking
- User authentication and role-based access control

### 👥 User Roles

- **Admin:** User administration and system management.
- **Manager:** Course and grade management.
- **Staff:** Student registration and student record management.

### 📚 Student Management
- Register new students
- Edit student information
- View and manage student records

### 📖 Course Management
- Create and manage courses
- Update course information
- Delete courses when necessary

### 📊 Grade Management
- Enter and update student grades
- Calculate GPA, if supported by the implementation
- Generate grade reports

### 🗂️ Data Management
- Store records in text files
- Maintain separate files for different types of records
- Support data backup, if implemented

## 🛠️ Technologies

- **Programming Language:** C
- **Compiler:** GCC
- **Data Storage:** Text files
- **Development Environment:** Linux / Terminal

## 🚀 Getting Started

### Prerequisites

- GCC compiler
- Linux or another supported operating system
- Terminal access

### Installation

**1. Clone the repository**

Replace the example URL with your actual GitHub repository URL.

```bash
git clone https://github.com/YOUR_USERNAME/YOUR_REPOSITORY.git
cd YOUR_REPOSITORY
```

**2. Compile the program**

```bash
gcc -std=c99 -Wall -Wextra -o main main.c
```

**3. Run the application**

```bash
./main
```

Keep the required data files in the directory expected by the program.

## 🔒 Security Configuration

The system may include the following configurable security settings, depending on the implementation:

| Security feature | Purpose |
|---|---|
| Login attempt limit | Restricts repeated failed login attempts |
| Account lockout | Prevents login during a lockout period |
| Session timeout | Ends an inactive session |
| Persistent security state | Stores login-attempt information between runs |

Check `main.c` for the actual configuration values and behavior.

## 📁 Project Structure

```text
.
├── main.c
├── students.txt
├── courses.txt
├── grades.txt
├── managers.txt
├── staff.txt
├── .gitignore
└── README.md
```

The structure above is illustrative. The actual files may vary depending on your project. The compiled executable and security tracking files may be generated at runtime.

## 🧪 Testing

You can test the system manually by checking:

- Successful and unsuccessful login attempts
- Account lockout behavior
- Session timeout after inactivity
- Permissions for each user role
- Student and course record management
- Grade entry and report generation
- Data persistence after restarting the application

## ⚠️ Security Notes

This project is intended for educational use.

Before using it in a real-world environment, consider:

- Secure password hashing
- Stronger authentication and account recovery
- Database storage and input validation
- Access-control testing
- Secure handling of sensitive student records
- Reliable backups and recovery procedures

Do not publish real passwords or sensitive student information in the repository.

## 🔮 Future Improvements

- Database integration with SQLite or MySQL
- Strong password hashing
- Comprehensive audit logging
- Report export to PDF
- Improved data validation
- A graphical or web-based interface

## 🎯 Project Goals

- Apply C programming concepts to a practical application
- Implement role-based access control
- Practice file handling and data persistence
- Explore authentication and session management
- Develop a structured, maintainable command-line application

---

**Built with ❤️ using C**

*An educational project focused on student, course, and academic record management.*
