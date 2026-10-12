#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define MAX_LOGIN_ATTEMPTS 3
#define LOCKOUT_TIME 300 // 5 minutes in seconds
#define SESSION_TIMEOUT 1800 // 30 minutes in seconds

typedef struct {
    char username[50];
    int failed_attempts;
    time_t lockout_until;
    time_t last_activity;
} LoginAttempt;

int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower(*s1) != tolower(*s2)) {
            return tolower(*s1) - tolower(*s2);
        }
        s1++;
        s2++;
    }
    return tolower(*s1) - tolower(*s2);
}

char* strcasestr_custom(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    
    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        
        while (*h && *n && tolower(*h) == tolower(*n)) {
            h++;
            n++;
        }
        
        if (!*n) return (char*)haystack;
    }
    
    return NULL;
}

// Simple hash function for password obfuscation
unsigned long hash_password(const char* password) {
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Check if account is locked
int is_account_locked(LoginAttempt* attempt) {
    if (attempt->failed_attempts >= MAX_LOGIN_ATTEMPTS) {
        time_t current_time = time(NULL);
        if (current_time < attempt->lockout_until) {
            int remaining = (int)(attempt->lockout_until - current_time);
            printf("\n⚠️  ACCOUNT LOCKED ⚠️\n");
            printf("Too many failed login attempts.\n");
            printf("Please wait %d seconds before trying again.\n", remaining);
            return 1;
        } else {
            // Lockout expired, reset attempts
            attempt->failed_attempts = 0;
            attempt->lockout_until = 0;
        }
    }
    return 0;
}

// Record failed login attempt
void record_failed_attempt(LoginAttempt* attempt, const char* username) {
    strcpy(attempt->username, username);
    attempt->failed_attempts++;
    
    if (attempt->failed_attempts >= MAX_LOGIN_ATTEMPTS) {
        attempt->lockout_until = time(NULL) + LOCKOUT_TIME;
        printf("\n⚠️  ACCOUNT LOCKED FOR 5 MINUTES ⚠️\n");
        printf("Too many failed attempts. Please try again later.\n");
    } else {
        printf("Wrong username or password.\n");
        printf("Attempts remaining: %d\n", MAX_LOGIN_ATTEMPTS - attempt->failed_attempts);
    }
}

// Reset login attempts on successful login
void reset_login_attempts(LoginAttempt* attempt) {
    attempt->failed_attempts = 0;
    attempt->lockout_until = 0;
    attempt->last_activity = time(NULL);
}

// Check session timeout
int check_session_timeout(LoginAttempt* attempt) {
    time_t current_time = time(NULL);
    if (current_time - attempt->last_activity > SESSION_TIMEOUT) {
        printf("\n⚠️  SESSION EXPIRED ⚠️\n");
        printf("You have been logged out due to inactivity.\n");
        return 1;
    }
    attempt->last_activity = current_time;
    return 0;
}

// Save login attempts to file
void save_login_attempts(LoginAttempt* attempts, int count) {
    FILE* file = fopen(".login_attempts", "w");
    if (file != NULL) {
        for (int i = 0; i < count; i++) {
            fprintf(file, "%s|%d|%ld|%ld\n", 
                    attempts[i].username, 
                    attempts[i].failed_attempts,
                    (long)attempts[i].lockout_until,
                    (long)attempts[i].last_activity);
        }
        fclose(file);
    }
}

// Load login attempts from file
int load_login_attempts(LoginAttempt* attempts, int max_count) {
    FILE* file = fopen(".login_attempts", "r");
    int count = 0;
    if (file != NULL) {
        char line[200];
        while (fgets(line, sizeof(line), file) && count < max_count) {
            sscanf(line, "%[^|]|%d|%ld|%ld", 
                   attempts[count].username, 
                   &attempts[count].failed_attempts,
                   (long*)&attempts[count].lockout_until,
                   (long*)&attempts[count].last_activity);
            count++;
        }
        fclose(file);
    }
    return count;
}

// Find or create login attempt record
LoginAttempt* get_login_attempt(LoginAttempt* attempts, int* count, const char* username, int max_count) {
    for (int i = 0; i < *count; i++) {
        if (strcmp(attempts[i].username, username) == 0) {
            return &attempts[i];
        }
    }
    
    if (*count < max_count) {
        strcpy(attempts[*count].username, username);
        attempts[*count].failed_attempts = 0;
        attempts[*count].lockout_until = 0;
        attempts[*count].last_activity = time(NULL);
        (*count)++;
        return &attempts[*count - 1];
    }
    
    return NULL;
}

typedef struct Student {
    char student_id[20];
    char first_name[50];
    char last_name[50];
    char national_id[20];
    char date_of_birth[15];
    char place_of_birth[100];
    char field_of_study[100];
    char phone[20];
    char email[100];
    char enrollment_date[20];
    char status[20];
    struct Student* next;
} Student;

typedef struct Course {
    char course_code[20];
    char course_name[100];
    char instructor[50];
    int credits;
    char course_type[20];
    char status[20];
    struct Course* next;
} Course;

typedef struct Grade {
    char student_id[20];
    char course_code[20];
    int numeric_grade;
    char letter_grade[5];
    char date_time[30];
    char registered_by[50];
    struct Grade* next;
} Grade;

Student* load_students();
Course* load_courses();
Grade* load_grades();
void free_students(Student* head);
void free_courses(Course* head);
void free_grades(Grade* head);
Student* find_student_by_id(Student* head, char* student_id);
Course* find_course_by_code(Course* head, char* course_code);
float calculate_gpa(Grade* grades_head, char* student_id);
float calculate_course_average(Grade* grades_head, char* course_code);
void generate_reports_menu(char* manager_name);

void report_find_student(Student* students_head);
void report_all_students(Student* students_head);
void report_all_courses(Course* courses_head);
void report_deleted_courses(Course* courses_head);
void report_active_courses(Course* courses_head);
void report_student_grades(Student* students_head, Course* courses_head, Grade* grades_head);
void report_course_grades(Student* students_head, Course* courses_head, Grade* grades_head);
void report_course_grades_sorted(Student* students_head, Course* courses_head, Grade* grades_head);
void report_student_gpa(Student* students_head, Grade* grades_head);
void report_course_average(Course* courses_head, Grade* grades_head);
void report_students_by_gpa(Student* students_head, Grade* grades_head);
void report_students_passed_course(Student* students_head, Course* courses_head, Grade* grades_head);
void report_students_failed_course(Student* students_head, Course* courses_head, Grade* grades_head);
void report_probationary_students(Student* students_head, Grade* grades_head);
void report_probationary_students_course(Student* students_head, Course* courses_head, Grade* grades_head);

void staff_report_find_student(Student* students_head);
void staff_report_students_by_birth_date_range(Student* students_head);
void staff_report_students_by_field_of_study(Student* students_head);
void staff_report_students_by_birth_city(Student* students_head);
void staff_report_all_students(Student* students_head);
void staff_report_student_grades(Student* students_head, Course* courses_head, Grade* grades_head);
void staff_report_failing_students(Student* students_head, Grade* grades_head);
void staff_report_students_by_status(Student* students_head);

int main() {
    int choice;
    char username[50];
    char password[50];
    int admin_choice;
    
    char first_name[50];
    char last_name[50];
    char start_date[50];
    char department[50];
    char national_id[50];
    char phone[50];
    char email[50];
    char new_username[50];
    char new_password[50];
    
    char staff_first_name[50];
    char staff_last_name[50];
    char hire_date[50];
    char staff_rank[50];
    char staff_phone[50];
    char staff_email[50];
    char staff_username[50];
    char staff_password[50];
    
    // Security: Load login attempts
    LoginAttempt login_attempts[100];
    int attempt_count = load_login_attempts(login_attempts, 100);
    
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║   SECURE EDUCATION MANAGEMENT SYSTEM       ║\n");
    printf("║   🔒 Protected Login System                ║\n");
    printf("╚════════════════════════════════════════════╝\n");

    while(1) {
    
        printf("\n═══════════════════════════════════════\n");
        printf("           MAIN MENU\n");
        printf("═══════════════════════════════════════\n");
        printf("1. 🔑 Admin Login\n");
        printf("2. 👤 Group Manager Login\n");
        printf("3. 👥 Staff Login\n");
        printf("4. 🚪 Exit\n");
        printf("═══════════════════════════════════════\n");
        printf("Enter your choice: ");
        

        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                printf("\n🔐 ADMIN LOGIN\n");
                printf("Enter username: ");
                scanf("%s", username);
                
                // Security: Check if account is locked
                LoginAttempt* admin_attempt = get_login_attempt(login_attempts, &attempt_count, username, 100);
                if (admin_attempt != NULL && is_account_locked(admin_attempt)) {
                    save_login_attempts(login_attempts, attempt_count);
                    break;
                }
                
                printf("Enter password: ");
                scanf("%s", password);
                
                if(strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
                    // Security: Reset attempts on successful login
                    if (admin_attempt != NULL) {
                        reset_login_attempts(admin_attempt);
                        save_login_attempts(login_attempts, attempt_count);
                    }
                    
                    printf("\n✅ Admin login successful!\n");
                    printf("Welcome, Administrator\n");
                    
                    while(1) {
                        // Security: Check session timeout
                        if (admin_attempt != NULL && check_session_timeout(admin_attempt)) {
                            save_login_attempts(login_attempts, attempt_count);
                            break;
                        }
                        
                        printf("\n═══════════════════════════════════════\n");
                        printf("        ADMIN CONTROL PANEL\n");
                        printf("═══════════════════════════════════════\n");
                        printf("1. ➕ Add New Group Manager\n");
                        printf("2. ➕ Add New Education Staff\n");
                        printf("3. 📋 View Users List by Type\n");
                        printf("4. 🗑️  Delete User from System\n");
                        printf("5. 📊 Reports\n");
                        printf("6. 💾 Backup Files\n");
                        printf("7. 🚪 Logout\n");
                        printf("═══════════════════════════════════════\n");
                        printf("Enter your choice: ");
                        scanf("%d", &admin_choice);
                        
                        switch(admin_choice) {
                            case 1:
                                printf("Enter first name: ");
                                scanf("%s", first_name);
                                printf("Enter last name: ");
                                scanf("%s", last_name);
                                printf("Enter start date of group management: ");
                                scanf("%s", start_date);
                                printf("Enter department/group name: ");
                                scanf("%s", department);
                                printf("Enter national ID: ");
                                scanf("%s", national_id);
                                printf("Enter phone number: ");
                                scanf("%s", phone);
                                printf("Enter email: ");
                                scanf("%s", email);
                                printf("Enter username: ");
                                scanf("%s", new_username);
                                printf("Enter password: ");
                                scanf("%s", new_password);
                                
                                FILE *managers_file = fopen("managers.txt", "a");
                                if (managers_file != NULL) {
                                    fprintf(managers_file, "%s|%s|%s|%s|%s|%s|%s|%s|%s|ACTIVE|N/A\n", 
                                            first_name, last_name, start_date, department, national_id,
                                            phone, email, new_username, new_password);
                                    fflush(managers_file);
                                    fclose(managers_file);
                                    printf("Group Manager information collected successfully!\n");
                                    printf("Manager %s %s has been added to the system.\n", first_name, last_name);
                                    printf("Data saved to managers.txt file.\n");
                                } else {
                                    printf("Error: Could not save manager information to file.\n");
                                }
                                break;
                            case 2:
                                printf("Enter first name: ");
                                scanf("%s", staff_first_name);
                                printf("Enter last name: ");
                                scanf("%s", staff_last_name);
                                printf("Enter hire date: ");
                                scanf("%s", hire_date);
                                printf("Enter staff rank: ");
                                scanf("%s", staff_rank);
                                printf("Enter phone number: ");
                                scanf("%s", staff_phone);
                                printf("Enter email: ");
                                scanf("%s", staff_email);
                                printf("Enter username: ");
                                scanf("%s", staff_username);
                                printf("Enter password: ");
                                scanf("%s", staff_password);
                                
                                FILE *staff_file = fopen("staff.txt", "a");
                                if (staff_file != NULL) {
                                    fprintf(staff_file, "%s|%s|%s|%s|%s|%s|%s|%s|ACTIVE|2024-01-30\n", 
                                            staff_first_name, staff_last_name, hire_date, staff_rank, 
                                            staff_phone, staff_email, staff_username, staff_password);
                                    fflush(staff_file);
                                    fclose(staff_file);
                                    printf("Education Staff information collected successfully!\n");
                                    printf("Staff member %s %s has been added to the system.\n", staff_first_name, staff_last_name);
                                    printf("Data saved to staff.txt file.\n");
                                } else {
                                    printf("Error: Could not save staff information to file.\n");
                                }
                                break;
                            case 3:
                                printf("\n=== USER LIST BY TYPE ===\n");
                                
                                printf("\nGROUP MANAGERS:\n");
                                printf("===============\n");
                                
                                FILE *view_managers_file = fopen("managers.txt", "r");
                                if (view_managers_file != NULL) {
                                    char line[500];
                                    int manager_count = 0;
                                    int line_number = 0;
                                    while (fgets(line, sizeof(line), view_managers_file)) {
                                        line_number++;
                                        line[strcspn(line, "\n")] = 0;
                                        
                                        if (strlen(line) == 0) continue;
                                        
                                        char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                        int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                                        
                                        if (parsed == 11 && strcmp(status, "ACTIVE") == 0) {
                                            manager_count++;
                                            printf("\nManager #%d\n", manager_count);
                                            printf("Name: %s %s\n", fname, lname);
                                            printf("Department: %s\n", dept);
                                            printf("Start Date: %s\n", start);
                                            printf("Username: %s\n", user);
                                            printf("Status: %s\n", status);
                                            printf("-------------------\n");
                                        }
                                    }
                                    fclose(view_managers_file);
                                    if (manager_count == 0) {
                                        printf("No Group Managers found\n");
                                    }
                                } else {
                                    printf("No Group Managers found\n");
                                }
                                
                                printf("\nEDUCATION STAFF:\n");
                                printf("===============\n");
                                
                                FILE *view_staff_file = fopen("staff.txt", "r");
                                if (view_staff_file != NULL) {
                                    char line[500];
                                    int staff_count = 0;
                                    while (fgets(line, sizeof(line), view_staff_file)) {
                                        char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                        
                                        if (strcmp(status, "ACTIVE") == 0) {
                                            staff_count++;
                                            printf("\nStaff #%d\n", staff_count);
                                            printf("Name: %s %s\n", fname, lname);
                                            printf("Rank: %s\n", rank);
                                            printf("Hire Date: %s\n", hire);
                                            printf("Username: %s\n", user);
                                            printf("Status: %s\n", status);
                                            printf("-------------------\n");
                                        }
                                    }
                                    fclose(view_staff_file);
                                    if (staff_count == 0) {
                                        printf("No Active Education Staff found\n");
                                    }
                                } else {
                                    printf("No Education Staff found\n");
                                }
                                
                                printf("\nPress any key to return to Admin Menu...\n");
                                getchar(); 
                                getchar(); 
                                break;
                            case 4:

                            printf("Enter username to delete: ");
                                char delete_username[50];
                                scanf("%s", delete_username);
                                
                                printf("Is this a Manager or Staff member? (M/S): ");
                                char user_type;
                                scanf(" %c", &user_type);
                                
                                if (user_type == 'S' || user_type == 's') {
                                  
                                    FILE *staff_file = fopen("staff.txt", "r");
                                    if (staff_file != NULL) {
                                        char lines[100][500];
                                        int line_count = 0;
                                        int user_found = 0;
                                        int found_index = -1;
                                        
                                        while (fgets(lines[line_count], sizeof(lines[line_count]), staff_file) && line_count < 100) {
                                            line_count++;
                                        }
                                        fclose(staff_file);
                                        
                                        for (int i = 0; i < line_count; i++) {
                                            char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                            sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                            
                                            if (strcmp(user, delete_username) == 0) {
                                                user_found = 1;
                                                found_index = i;
                                                if (strcmp(status, "DELETED") == 0) {
                                                    printf("User '%s' is already deleted.\n", delete_username);
                                                    printf("Termination date: %s\n", term);
                                                } else {
                                                    sprintf(lines[i], "%s|%s|%s|%s|%s|%s|%s|%s|DELETED|2024-03-15\n", fname, lname, hire, rank, phone, email, user, pass);
                                                    printf("User '%s' has been successfully deleted from the system.\n", delete_username);
                                                    printf("Termination date: 2024-03-15\n");
                                                }
                                                break;
                                            }
                                        }
                                        
                                        if (!user_found) {
                                            printf("Error: User '%s' not found in staff records.\n", delete_username);
                                        } else if (user_found && found_index >= 0) {
                                            staff_file = fopen("staff.txt", "w");
                                            for (int i = 0; i < line_count; i++) {
                                                fputs(lines[i], staff_file);
                                            }
                                            fclose(staff_file);
                                        }
                                    } else {
                                        printf("Error: Could not access staff records.\n");
                                    }
                                } else {
                                    printf("Manager deletion not implemented yet.\n");
                                }
                                break;
                            case 5:
                                while(1) {
                                    printf("\n=== REPORTS MENU ===\n");
                                    printf("1. List of All Education Staff (with status)\n");
                                    printf("2. List of Group Managers\n");
                                    printf("3. List of Terminated/Deleted Staff\n");
                                    printf("4. Active Staff Summary\n");
                                    printf("5. Staff Statistics Report\n");
                                    printf("6. Return to Admin Menu\n");
                                    printf("Enter your choice: ");
                                    
                                    int report_choice;
                                    scanf("%d", &report_choice);
                                    
                                    switch(report_choice) {
                                        case 1:
                                            printf("\n=== ALL EDUCATION STAFF REPORT ===\n");
                                            printf("==========================================\n");
                                            
                                            FILE *all_staff_file = fopen("staff.txt", "r");
                                            if (all_staff_file != NULL) {
                                                char line[500];
                                                int staff_number = 0;
                                                while (fgets(line, sizeof(line), all_staff_file)) {
                                                    line[strcspn(line, "\n")] = 0;
                                                    if (strlen(line) == 0) continue;
                                                    
                                                    char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                    int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                                    
                                                    if (parsed >= 9) {
                                                        staff_number++;
                                                        printf("\nStaff #%d\n", staff_number);
                                                        printf("Name: %s %s\n", fname, lname);
                                                        printf("Position: %s\n", rank);
                                                        printf("Hire Date: %s\n", hire);
                                                        printf("Username: %s\n", user);
                                                        printf("Status: %s\n", status);
                                                        if (strcmp(status, "DELETED") == 0) {
                                                            printf("Termination Date: %s\n", term);
                                                        }
                                                        printf("Phone: %s\n", phone);
                                                        printf("Email: %s\n", email);
                                                        printf("----------------------------\n");
                                                    }
                                                }
                                                fclose(all_staff_file);
                                                if (staff_number == 0) {
                                                    printf("No staff records found.\n");
                                                }
                                            } else {
                                                printf("Error: Could not access staff records.\n");
                                            }
                                            printf("\nPress any key to return to Reports Menu...\n");
                                            getchar(); getchar();
                                            break;
                                            
                                        case 2:
                                            printf("\n=== GROUP MANAGERS REPORT ===\n");
                                            printf("==============================\n");
                                            
                                            FILE *managers_report_file = fopen("managers.txt", "r");
                                            if (managers_report_file != NULL) {
                                                char line[500];
                                                int manager_number = 0;
                                                while (fgets(line, sizeof(line), managers_report_file)) {
                                                    line[strcspn(line, "\n")] = 0;
                                                    if (strlen(line) == 0) continue;
                                                    
                                                    char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                    int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                                                    
                                                    if (parsed >= 10) {
                                                        manager_number++;
                                                        printf("\nManager #%d\n", manager_number);
                                                        printf("Name: %s %s\n", fname, lname);
                                                        printf("Department: %s\n", dept);
                                                        printf("Start Date: %s\n", start);
                                                        printf("Username: %s\n", user);
                                                        printf("Status: %s\n", status);
                                                        printf("National ID: %s\n", id);
                                                        printf("Phone: %s\n", phone);
                                                        printf("Email: %s\n", email);
                                                        printf("----------------------------\n");
                                                    }
                                                }
                                                fclose(managers_report_file);
                                                if (manager_number == 0) {
                                                    printf("No manager records found.\n");
                                                }
                                            } else {
                                                printf("Error: Could not access manager records.\n");
                                            }
                                            printf("\nPress any key to return to Reports Menu...\n");
                                            getchar(); getchar();
                                            break;
                                            
                                        case 3:
                                            printf("\n=== TERMINATED/DELETED STAFF REPORT ===\n");
                                            printf("========================================\n");
                                            
                                            FILE *deleted_staff_file = fopen("staff.txt", "r");
                                            if (deleted_staff_file != NULL) {
                                                char line[500];
                                                int deleted_count = 0;
                                                while (fgets(line, sizeof(line), deleted_staff_file)) {
                                                    line[strcspn(line, "\n")] = 0;
                                                    if (strlen(line) == 0) continue;
                                                    
                                                    char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                    int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                                    
                                                    if (parsed >= 9 && strcmp(status, "DELETED") == 0) {
                                                        deleted_count++;
                                                        printf("\nTerminated Staff #%d\n", deleted_count);
                                                        printf("Name: %s %s\n", fname, lname);
                                                        printf("Former Position: %s\n", rank);
                                                        printf("Hire Date: %s\n", hire);
                                                        printf("Termination Date: %s\n", term);
                                                        printf("Username: %s\n", user);
                                                        printf("Phone: %s\n", phone);
                                                        printf("Email: %s\n", email);
                                                        printf("----------------------------\n");
                                                    }
                                                }
                                                fclose(deleted_staff_file);
                                                if (deleted_count == 0) {
                                                    printf("No terminated staff records found.\n");
                                                } else {
                                                    printf("\nTotal Terminated Staff: %d\n", deleted_count);
                                                }
                                            } else {
                                                printf("Error: Could not access staff records.\n");
                                            }
                                            printf("\nPress any key to return to Reports Menu...\n");
                                            getchar(); getchar();
                                            break;
                                            
                                        case 4:
                                            printf("\n=== ACTIVE STAFF SUMMARY ===\n");
                                            printf("=============================\n");
                                            
                                            FILE *active_staff_file = fopen("staff.txt", "r");
                                            if (active_staff_file != NULL) {
                                                char line[500];
                                                int active_count = 0;
                                                while (fgets(line, sizeof(line), active_staff_file)) {
                                                    line[strcspn(line, "\n")] = 0;
                                                    if (strlen(line) == 0) continue;
                                                    
                                                    char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                    int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                                    
                                                    if (parsed >= 9 && strcmp(status, "ACTIVE") == 0) {
                                                        active_count++;
                                                        printf("\nActive Staff #%d\n", active_count);
                                                        printf("Name: %s %s\n", fname, lname);
                                                        printf("Position: %s\n", rank);
                                                        printf("Hire Date: %s\n", hire);
                                                        printf("Username: %s\n", user);
                                                        printf("----------------------------\n");
                                                    }
                                                }
                                                fclose(active_staff_file);
                                                if (active_count == 0) {
                                                    printf("No active staff found.\n");
                                                } else {
                                                    printf("\nTotal Active Staff: %d\n", active_count);
                                                }
                                            } else {
                                                printf("Error: Could not access staff records.\n");
                                            }
                                            printf("\nPress any key to return to Reports Menu...\n");
                                            getchar(); getchar();
                                            break;
                                            
                                        case 5:
                                            printf("\n=== STAFF STATISTICS REPORT ===\n");
                                            printf("================================\n");
                                            
                                            FILE *stats_file = fopen("staff.txt", "r");
                                            if (stats_file != NULL) {
                                                char line[500];
                                                int total_staff = 0, active_staff = 0, deleted_staff = 0;
                                                
                                                while (fgets(line, sizeof(line), stats_file)) {
                                                    line[strcspn(line, "\n")] = 0;
                                                    if (strlen(line) == 0) continue;
                                                    
                                                    char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                    int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                                    
                                                    if (parsed >= 9) {
                                                        total_staff++;
                                                        if (strcmp(status, "ACTIVE") == 0) {
                                                            active_staff++;
                                                        } else if (strcmp(status, "DELETED") == 0) {
                                                            deleted_staff++;
                                                        }
                                                    }
                                                }
                                                fclose(stats_file);
                                                
                                                printf("Total Staff Records: %d\n", total_staff);
                                                printf("Active Staff: %d\n", active_staff);
                                                printf("Terminated Staff: %d\n", deleted_staff);
                                                
                                                if (total_staff > 0) {
                                                    printf("Active Staff Percentage: %.1f%%\n", (float)active_staff / total_staff * 100);
                                                    printf("Terminated Staff Percentage: %.1f%%\n", (float)deleted_staff / total_staff * 100);
                                                }
                                            } else {
                                                printf("Error: Could not access staff records.\n");
                                            }
                                            
                                            FILE *mgr_stats_file = fopen("managers.txt", "r");
                                            if (mgr_stats_file != NULL) {
                                                char line[500];
                                                int total_managers = 0, active_managers = 0;
                                                
                                                while (fgets(line, sizeof(line), mgr_stats_file)) {
                                                    line[strcspn(line, "\n")] = 0;
                                                    if (strlen(line) == 0) continue;
                                                    
                                                    char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                    int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                                                    
                                                    if (parsed >= 10) {
                                                        total_managers++;
                                                        if (strcmp(status, "ACTIVE") == 0) {
                                                            active_managers++;
                                                        }
                                                    }
                                                }
                                                fclose(mgr_stats_file);
                                                
                                                printf("\nTotal Manager Records: %d\n", total_managers);
                                                printf("Active Managers: %d\n", active_managers);
                                            }
                                            
                                            printf("\nPress any key to return to Reports Menu...\n");
                                            getchar(); getchar();
                                            break;
                                            
                                        case 6:
                                            printf("Returning to Admin Menu...\n");
                                            break;
                                            
                                        default:
                                            printf("Invalid choice! Please try again.\n");
                                            break;
                                    }
                                    
                                    if(report_choice == 6) {
                                        break;
                                    }
                                }
                                break;
                            case 6:
                                printf("\n=== FILE BACKUP SYSTEM ===\n");
                                printf("This will create backup copies of all important system files.\n");
                                printf("Files to be backed up:\n");
                                printf("- managers.txt (Group Managers profiles)\n");
                                printf("- staff.txt (Education Staff profiles)\n");
                                printf("- courses.txt (Course information)\n");
                                printf("- grades.txt (Student grades)\n\n");
                                
                                char backup_path[200];
                                printf("Enter the destination path for backup files: ");
                                printf("\nExample: C:\\Backups\\ or D:\\MyBackups\\\n");
                                printf("Path: ");
                                getchar();
                                fgets(backup_path, sizeof(backup_path), stdin);
                                
                                backup_path[strcspn(backup_path, "\n")] = 0;
                                
                                time_t t = time(NULL);
                                struct tm tm = *localtime(&t);
                                char date_str[20];
                                sprintf(date_str, "%02d_%02d_%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
                                
                                printf("\nStarting backup process...\n");
                                printf("Backup Date: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
                                printf("Destination: %s\n\n", backup_path);
                                
                                int backup_count = 0;
                                char backup_files[10][100];
                                
                                char *files_to_backup[] = {"managers.txt", "staff.txt", "courses.txt", "grades.txt"};
                                char *file_descriptions[] = {"Group Managers profiles", "Education Staff profiles", "Course information", "Student grades"};
                                int num_files = 4;
                                
                                for (int i = 0; i < num_files; i++) {
                                    FILE *source_file = fopen(files_to_backup[i], "r");
                                    if (source_file != NULL) {
                                        char backup_filename[300];
                                        char *dot = strrchr(files_to_backup[i], '.');
                                        if (dot != NULL) {
                                            *dot = '\0';
                                            sprintf(backup_filename, "%s\\%s_%s.txt", backup_path, files_to_backup[i], date_str);
                                            *dot = '.';
                                        } else {
                                            sprintf(backup_filename, "%s\\%s_%s", backup_path, files_to_backup[i], date_str);
                                        }
                                        
                                        printf("Attempting to create: %s\n", backup_filename);
                                        
                                        FILE *backup_file = fopen(backup_filename, "w");
                                        if (backup_file != NULL) {
                                            char buffer[1024];
                                            size_t bytes;
                                            while ((bytes = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
                                                fwrite(buffer, 1, bytes, backup_file);
                                            }
                                            
                                            fclose(backup_file);
                                            fclose(source_file);
                                            
                                            printf("✓ Backed up %s (%s)\n", files_to_backup[i], file_descriptions[i]);
                                            printf("  → %s\n", backup_filename);
                                            strcpy(backup_files[backup_count], backup_filename);
                                            backup_count++;
                                        } else {
                                            fclose(source_file);
                                            printf("✗ Error: Could not create backup for %s\n", files_to_backup[i]);
                                            printf("  Attempted path: %s\n", backup_filename);
                                            printf("  Check if destination path exists and is writable.\n");
                                        }
                                    } else {
                                        printf("⚠ Warning: %s not found, skipping backup\n", files_to_backup[i]);
                                    }
                                }
                                
                                printf("\n=== BACKUP COMPLETED ===\n");
                                if (backup_count > 0) {
                                    printf("✓ Backup successful!\n");
                                    printf("Backup Date: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
                                    printf("Backup Location: %s\n", backup_path);
                                    printf("Files Backed Up: %d\n\n", backup_count);
                                    
                                    printf("Backup Files Created:\n");
                                    for (int i = 0; i < backup_count; i++) {
                                        char *filename = strrchr(backup_files[i], '\\');
                                        if (filename != NULL) {
                                            printf("- %s\n", filename + 1);
                                        } else {
                                            printf("- %s\n", backup_files[i]);
                                        }
                                    }
                                    
                                    printf("\n📁 All important system files have been safely backed up.\n");
                                    printf("💡 Keep these backup files in a safe location.\n");
                                    printf("💡 Regular backups help protect against data loss.\n");
                                } else {
                                    printf("✗ Backup failed - No files were backed up.\n");
                                    printf("Please check:\n");
                                    printf("- Destination path exists and is writable\n");
                                    printf("- Source files are available\n");
                                    printf("- You have sufficient disk space\n");
                                }
                                
                                printf("\nPress any key to return to Admin Menu...\n");
                                getchar(); getchar();
                                break;
                            case 7:
                                printf("\n✅ Logged out successfully\n");
                                printf("Session ended. Thank you!\n");
                                // Security: Save login state
                                save_login_attempts(login_attempts, attempt_count);
                                break;
                            default:
                                printf("Invalid choice! Please try again.\n");
                                break;
                        }
                        
                        if(admin_choice == 7) {
                            break;
                        }
                    }
                } else {
                    // Security: Record failed attempt
                    if (admin_attempt != NULL) {
                        record_failed_attempt(admin_attempt, username);
                        save_login_attempts(login_attempts, attempt_count);
                    } else {
                        printf("Wrong username or password\n");
                    }
                }
                break;
            case 2:
                printf("\n🔐 GROUP MANAGER LOGIN\n");
                printf("Enter username: ");
                scanf("%s", username);
                
                // Security: Check if account is locked
                LoginAttempt* manager_attempt = get_login_attempt(login_attempts, &attempt_count, username, 100);
                if (manager_attempt != NULL && is_account_locked(manager_attempt)) {
                    save_login_attempts(login_attempts, attempt_count);
                    break;
                }
                
                printf("Enter password: ");
                scanf("%s", password);
                
                FILE *manager_login_file = fopen("managers.txt", "r");
                int manager_found = 0;
                char manager_name[100];
                char manager_dept[50];
                
                if (manager_login_file != NULL) {
                    char line[500];
                    while (fgets(line, sizeof(line), manager_login_file)) {
                        char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                        int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                        
                        if (parsed >= 9 && strcmp(user, username) == 0 && strcmp(pass, password) == 0 && strcmp(status, "ACTIVE") == 0) {
                            manager_found = 1;
                            sprintf(manager_name, "%s %s", fname, lname);
                            strcpy(manager_dept, dept);
                            break;
                        }
                    }
                    fclose(manager_login_file);
                }
                
                if (manager_found) {
                    // Security: Reset attempts on successful login
                    if (manager_attempt != NULL) {
                        reset_login_attempts(manager_attempt);
                        save_login_attempts(login_attempts, attempt_count);
                    }
                    
                    printf("\n✅ Group Manager login successful!\n");
                    printf("Welcome, %s from %s\n", manager_name, manager_dept);
                    
                    int manager_choice;
                    while(1) {
                        // Security: Check session timeout
                        if (manager_attempt != NULL && check_session_timeout(manager_attempt)) {
                            save_login_attempts(login_attempts, attempt_count);
                            break;
                        }
                        
                        printf("\n═══════════════════════════════════════\n");
                        printf("      GROUP MANAGER MENU\n");
                        printf("═══════════════════════════════════════\n");
                        printf("1. ➕ Register a new course\n");
                        printf("2. ✍️  Register a student's grade\n");
                        printf("3. ✏️  Correct a student's grade\n");
                        printf("4. 📝 Correct course details\n");
                        printf("5. 🗑️  Delete details of an old course\n");
                        printf("6. 📊 Department head reports\n");
                        printf("7. ⚙️  Account settings\n");
                        printf("8. 🚪 Log out of the account\n");
                        printf("═══════════════════════════════════════\n");
                        printf("Enter your choice: ");
                        scanf("%d", &manager_choice);
                        
                        switch(manager_choice) {
                            case 1:
                                printf("\n=== REGISTER NEW COURSE ===\n");
                                char course_code[20], course_name[100], instructor[50], course_type[20];
                                int credits, type_choice;
                                int valid_input = 0;
                                
                                do {
                                    printf("Enter course name: ");
                                    getchar();
                                    fgets(course_name, sizeof(course_name), stdin);
                                    course_name[strcspn(course_name, "\n")] = 0;
                                    
                                    if (strlen(course_name) < 3) {
                                        printf("Error: Course name must be at least 3 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(course_name) > 99) {
                                        printf("Error: Course name is too long (max 99 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter instructor name: ");
                                    fgets(instructor, sizeof(instructor), stdin);
                                    instructor[strcspn(instructor, "\n")] = 0;
                                    
                                    if (strlen(instructor) < 2) {
                                        printf("Error: Instructor name must be at least 2 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(instructor) > 49) {
                                        printf("Error: Instructor name is too long (max 49 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter number of credits (1-6): ");
                                    if (scanf("%d", &credits) != 1) {
                                        printf("Error: Please enter a valid number.\n");
                                        while (getchar() != '\n');
                                        valid_input = 0;
                                        continue;
                                    }
                                    
                                    if (credits < 1 || credits > 6) {
                                        printf("Error: Credits must be between 1 and 6.\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("\n=== SELECT COURSE TYPE ===\n");
                                    printf("1. Theory\n");
                                    printf("2. Practical\n");
                                    printf("3. Laboratory\n");
                                    printf("4. Workshop\n");
                                    printf("Enter your choice (1-4): ");
                                    
                                    if (scanf("%d", &type_choice) != 1) {
                                        printf("Error: Please enter a valid number.\n");
                                        while (getchar() != '\n');
                                        valid_input = 0;
                                        continue;
                                    }
                                    
                                    switch (type_choice) {
                                        case 1:
                                            strcpy(course_type, "Theory");
                                            valid_input = 1;
                                            break;
                                        case 2:
                                            strcpy(course_type, "Practical");
                                            valid_input = 1;
                                            break;
                                        case 3:
                                            strcpy(course_type, "Laboratory");
                                            valid_input = 1;
                                            break;
                                        case 4:
                                            strcpy(course_type, "Workshop");
                                            valid_input = 1;
                                            break;
                                        default:
                                            printf("Error: Please select a valid option (1-4).\n");
                                            valid_input = 0;
                                            break;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter course code (e.g., BANK301): ");
                                    scanf("%s", course_code);
                                    
                                    int len = strlen(course_code);
                                    valid_input = 1;
                                    if (len < 4 || len > 10) {
                                        printf("Error: Course code must be 4-10 characters long.\n");
                                        valid_input = 0;
                                        continue;
                                    }
                                    
                                    for (int i = 0; i < len; i++) {
                                        course_code[i] = toupper(course_code[i]);
                                    }
                                    
                                    FILE *check_course_file = fopen("courses.txt", "r");
                                    int course_exists = 0;
                                    if (check_course_file != NULL) {
                                        char line[500];
                                        while (fgets(line, sizeof(line), check_course_file)) {
                                            char existing_code[20];
                                            sscanf(line, "%[^|]", existing_code);
                                            if (strcmp(existing_code, course_code) == 0) {
                                                course_exists = 1;
                                                break;
                                            }
                                        }
                                        fclose(check_course_file);
                                    }
                                    
                                    if (course_exists) {
                                        printf("Error: Course code %s already exists! Please choose a different code.\n", course_code);
                                        valid_input = 0;
                                    }
                                } while (!valid_input);
                                
                                FILE *courses_file = fopen("courses.txt", "a");
                                if (courses_file != NULL) {
                                    fprintf(courses_file, "%s|%s|%s|%d|%s|ACTIVE\n", 
                                            course_code, course_name, instructor, credits, course_type);
                                    fflush(courses_file);
                                    fclose(courses_file);
                                    
                                    printf("\n=== COURSE REGISTRATION SUCCESSFUL ===\n");
                                    printf("Course Code: %s\n", course_code);
                                    printf("Course Name: %s\n", course_name);
                                    printf("Instructor: %s\n", instructor);
                                    printf("Credits: %d\n", credits);
                                    printf("Course Type: %s\n", course_type);
                                    printf("Status: ACTIVE\n");
                                    printf("Course has been successfully added to the system!\n");
                                } else {
                                    printf("Error: Could not save course information to file.\n");
                                }
                                break;
                                
                            case 2:
                                printf("\n=== REGISTER STUDENT GRADES ===\n");
                                char student_id[20], grade_course[20], letter_grade[5];
                                int numeric_grade;
                                valid_input = 0;
                                
                                do {
                                    printf("Enter student ID: ");
                                    scanf("%s", student_id);
                                    
                                    if (strlen(student_id) < 3) {
                                        printf("Error: Student ID must be at least 3 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(student_id) > 19) {
                                        printf("Error: Student ID is too long (max 19 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        for (int i = 0; i < strlen(student_id); i++) {
                                            student_id[i] = toupper(student_id[i]);
                                        }
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter course code: ");
                                    scanf("%s", grade_course);
                                    
                                    if (strlen(grade_course) < 3) {
                                        printf("Error: Course code must be at least 3 characters long.\n");
                                        valid_input = 0;
                                        continue;
                                    }
                                    
                                    for (int i = 0; i < strlen(grade_course); i++) {
                                        grade_course[i] = toupper(grade_course[i]);
                                    }
                                    
                                    FILE *check_course_file = fopen("courses.txt", "r");
                                    int course_exists = 0;
                                    if (check_course_file != NULL) {
                                        char line[500];
                                        while (fgets(line, sizeof(line), check_course_file)) {
                                            char existing_code[20];
                                            sscanf(line, "%[^|]", existing_code);
                                            if (strcmp(existing_code, grade_course) == 0) {
                                                course_exists = 1;
                                                break;
                                            }
                                        }
                                        fclose(check_course_file);
                                    }
                                    
                                    if (!course_exists) {
                                        printf("Error: Course code %s does not exist in the system.\n", grade_course);
                                        printf("Please register the course first or check the course code.\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter numeric grade (0-100): ");
                                    if (scanf("%d", &numeric_grade) != 1) {
                                        printf("Error: Please enter a valid number.\n");
                                        while (getchar() != '\n');
                                        valid_input = 0;
                                        continue;
                                    }
                                    
                                    if (numeric_grade < 0 || numeric_grade > 100) {
                                        printf("Error: Grade must be between 0 and 100.\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                if (numeric_grade >= 97) strcpy(letter_grade, "A+");
                                else if (numeric_grade >= 93) strcpy(letter_grade, "A");
                                else if (numeric_grade >= 90) strcpy(letter_grade, "A-");
                                else if (numeric_grade >= 87) strcpy(letter_grade, "B+");
                                else if (numeric_grade >= 83) strcpy(letter_grade, "B");
                                else if (numeric_grade >= 80) strcpy(letter_grade, "B-");
                                else if (numeric_grade >= 77) strcpy(letter_grade, "C+");
                                else if (numeric_grade >= 73) strcpy(letter_grade, "C");
                                else if (numeric_grade >= 70) strcpy(letter_grade, "C-");
                                else if (numeric_grade >= 67) strcpy(letter_grade, "D+");
                                else if (numeric_grade >= 65) strcpy(letter_grade, "D");
                                else strcpy(letter_grade, "F");
                                
                                time_t t = time(NULL);
                                struct tm tm = *localtime(&t);
                                char datetime_str[30];
                                sprintf(datetime_str, "%04d-%02d-%02d %02d:%02d:%02d", 
                                        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                                        tm.tm_hour, tm.tm_min, tm.tm_sec);
                                
                                FILE *grades_file = fopen("grades.txt", "a");
                                if (grades_file != NULL) {
                                    fprintf(grades_file, "%s|%s|%d|%s|%s|%s\n", 
                                            student_id, grade_course, numeric_grade, letter_grade, 
                                            datetime_str, manager_name);
                                    fflush(grades_file);
                                    fclose(grades_file);
                                    
                                    printf("\n=== GRADE REGISTRATION SUCCESSFUL ===\n");
                                    printf("Student ID: %s\n", student_id);
                                    printf("Course Code: %s\n", grade_course);
                                    printf("Numeric Grade: %d\n", numeric_grade);
                                    printf("Letter Grade: %s\n", letter_grade);
                                    printf("Registration Date/Time: %s\n", datetime_str);
                                    printf("Registered by: %s\n", manager_name);
                                    printf("Grade has been successfully recorded!\n");
                                } else {
                                    printf("Error: Could not save grade information to file.\n");
                                }
                                break;
                                
                            case 3:
                                printf("\n=== EDIT STUDENT GRADES ===\n");
                                char edit_student_id[20], edit_course[20];
                                printf("Enter student ID: ");
                                scanf("%s", edit_student_id);
                                printf("Enter course code: ");
                                scanf("%s", edit_course);
                                
                                FILE *grades_edit_file = fopen("grades.txt", "r");
                                if (grades_edit_file != NULL) {
                                    char lines[200][500];
                                    int line_count = 0;
                                    int grade_found = 0;
                                    int found_index = -1;
                                    
                                    while (fgets(lines[line_count], sizeof(lines[line_count]), grades_edit_file) && line_count < 200) {
                                        line_count++;
                                    }
                                    fclose(grades_edit_file);
                                    
                                    for (int i = 0; i < line_count; i++) {
                                        char sid[20], sname[100], course[20], date[20], lgrade[5];
                                        int ngrade;
                                        sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%d|%[^|]|%s", sid, sname, course, &ngrade, lgrade, date);
                                        
                                        if (strcmp(sid, edit_student_id) == 0 && strcmp(course, edit_course) == 0) {
                                            grade_found = 1;
                                            found_index = i;
                                            printf("Current grade for %s in %s: %d (%s)\n", sname, course, ngrade, lgrade);
                                            printf("Enter new numeric grade (0-100): ");
                                            int new_grade;
                                            scanf("%d", &new_grade);
                                            
                                            char new_letter[5];
                                            if (new_grade >= 97) strcpy(new_letter, "A+");
                                            else if (new_grade >= 93) strcpy(new_letter, "A");
                                            else if (new_grade >= 90) strcpy(new_letter, "A-");
                                            else if (new_grade >= 87) strcpy(new_letter, "B+");
                                            else if (new_grade >= 83) strcpy(new_letter, "B");
                                            else if (new_grade >= 80) strcpy(new_letter, "B-");
                                            else if (new_grade >= 77) strcpy(new_letter, "C+");
                                            else if (new_grade >= 73) strcpy(new_letter, "C");
                                            else if (new_grade >= 70) strcpy(new_letter, "C-");
                                            else if (new_grade >= 67) strcpy(new_letter, "D+");
                                            else if (new_grade >= 65) strcpy(new_letter, "D");
                                            else strcpy(new_letter, "F");
                                            
                                            sprintf(lines[i], "%s|%s|%s|%d|%s|%s\n", sid, sname, course, new_grade, new_letter, date);
                                            printf("Grade updated successfully!\n");
                                            printf("New grade: %d (%s)\n", new_grade, new_letter);
                                            break;
                                        }
                                    }
                                    
                                    if (grade_found && found_index >= 0) {
                                        grades_edit_file = fopen("grades.txt", "w");
                                        for (int i = 0; i < line_count; i++) {
                                            fputs(lines[i], grades_edit_file);
                                        }
                                        fclose(grades_edit_file);
                                    } else {
                                        printf("Grade record not found for student %s in course %s\n", edit_student_id, edit_course);
                                    }
                                } else {
                                    printf("Error: Could not access grades file.\n");
                                }
                                break;
                                
                            case 4:
                                printf("\n=== EDIT COURSE DETAILS ===\n");
                                char edit_course_code[20];
                                printf("Enter course code to edit: ");
                                scanf("%s", edit_course_code);
                                
                                FILE *course_edit_file = fopen("courses.txt", "r");
                                if (course_edit_file != NULL) {
                                    char lines[100][500];
                                    int line_count = 0;
                                    int course_found = 0;
                                    int found_index = -1;
                                    
                                    while (fgets(lines[line_count], sizeof(lines[line_count]), course_edit_file) && line_count < 100) {
                                        line_count++;
                                    }
                                    fclose(course_edit_file);
                                    
                                    for (int i = 0; i < line_count; i++) {
                                        char code[20], name[100], instructor[50], course_type[20], status[20];
                                        int credits;
                                        sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%d|%[^|]|%s", code, name, instructor, &credits, course_type, status);
                                        
                                        if (strcmp(code, edit_course_code) == 0) {
                                            course_found = 1;
                                            found_index = i;
                                            printf("Current course details:\n");
                                            printf("Code: %s\n", code);
                                            printf("Name: %s\n", name);
                                            printf("Instructor: %s\n", instructor);
                                            printf("Credits: %d\n", credits);
                                            printf("Course Type: %s\n", course_type);
                                            printf("Status: %s\n", status);
                                            
                                            printf("\nEnter new course name: ");
                                            getchar();
                                            fgets(name, sizeof(name), stdin);
                                            name[strcspn(name, "\n")] = 0;
                                            printf("Enter new instructor name: ");
                                            fgets(instructor, sizeof(instructor), stdin);
                                            instructor[strcspn(instructor, "\n")] = 0;
                                            printf("Enter new number of credits: ");
                                            scanf("%d", &credits);
                                            
                                            int type_choice, valid_input;
                                            do {
                                                printf("\n=== SELECT NEW COURSE TYPE ===\n");
                                                printf("1. Theory\n");
                                                printf("2. Practical\n");
                                                printf("3. Laboratory\n");
                                                printf("4. Workshop\n");
                                                printf("Enter your choice (1-4): ");
                                                
                                                if (scanf("%d", &type_choice) != 1) {
                                                    printf("Error: Please enter a valid number.\n");
                                                    while (getchar() != '\n');
                                                    valid_input = 0;
                                                    continue;
                                                }
                                                
                                                switch (type_choice) {
                                                    case 1:
                                                        strcpy(course_type, "Theory");
                                                        valid_input = 1;
                                                        break;
                                                    case 2:
                                                        strcpy(course_type, "Practical");
                                                        valid_input = 1;
                                                        break;
                                                    case 3:
                                                        strcpy(course_type, "Laboratory");
                                                        valid_input = 1;
                                                        break;
                                                    case 4:
                                                        strcpy(course_type, "Workshop");
                                                        valid_input = 1;
                                                        break;
                                                    default:
                                                        printf("Error: Please select a valid option (1-4).\n");
                                                        valid_input = 0;
                                                        break;
                                                }
                                            } while (!valid_input);
                                            
                                            sprintf(lines[i], "%s|%s|%s|%d|%s|%s\n", code, name, instructor, credits, course_type, status);
                                            printf("Course details updated successfully!\n");
                                            break;
                                        }
                                    }
                                    
                                    if (course_found && found_index >= 0) {
                                        course_edit_file = fopen("courses.txt", "w");
                                        for (int i = 0; i < line_count; i++) {
                                            fputs(lines[i], course_edit_file);
                                        }
                                        fclose(course_edit_file);
                                    } else {
                                        printf("Course %s not found.\n", edit_course_code);
                                    }
                                } else {
                                    printf("Error: Could not access courses file.\n");
                                }
                                break;
                                
                            case 5:
                                printf("\n=== DELETE COURSE ===\n");
                                char delete_course_code[20];
                                printf("Enter course code to delete: ");
                                scanf("%s", delete_course_code);
                                
                                FILE *course_delete_file = fopen("courses.txt", "r");
                                if (course_delete_file != NULL) {
                                    char lines[100][500];
                                    int line_count = 0;
                                    int course_found = 0;
                                    int found_index = -1;
                                    
                                    while (fgets(lines[line_count], sizeof(lines[line_count]), course_delete_file) && line_count < 100) {
                                        line_count++;
                                    }
                                    fclose(course_delete_file);
                                    
                                    for (int i = 0; i < line_count; i++) {
                                        char code[20], name[100], instructor[50], course_type[20], status[20];
                                        int credits;
                                        sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%d|%[^|]|%s", code, name, instructor, &credits, course_type, status);
                                        
                                        if (strcmp(code, delete_course_code) == 0) {
                                            course_found = 1;
                                            found_index = i;
                                            printf("Course found: %s - %s\n", code, name);
                                            printf("Course Type: %s\n", course_type);
                                            printf("Are you sure you want to delete this course? (Y/N): ");
                                            char confirm;
                                            scanf(" %c", &confirm);
                                            
                                            if (confirm == 'Y' || confirm == 'y') {
                                                sprintf(lines[i], "%s|%s|%s|%d|%s|DELETED\n", code, name, instructor, credits, course_type);
                                                printf("Course %s has been deleted successfully.\n", code);
                                            } else {
                                                printf("Course deletion cancelled.\n");
                                                found_index = -1;
                                            }
                                            break;
                                        }
                                    }
                                    
                                    if (course_found && found_index >= 0) {
                                        course_delete_file = fopen("courses.txt", "w");
                                        for (int i = 0; i < line_count; i++) {
                                            fputs(lines[i], course_delete_file);
                                        }
                                        fclose(course_delete_file);
                                    } else if (!course_found) {
                                        printf("Course %s not found.\n", delete_course_code);
                                    }
                                } else {
                                    printf("Error: Could not access courses file.\n");
                                }
                                break;
                                
                            case 6:
                                generate_reports_menu(manager_name);
                                break;
                            case 7:
                                printf("\n=== ACCOUNT SETTINGS ===\n");
                                printf("Current account: %s\n", manager_name);
                                printf("Department: %s\n", manager_dept);
                                
                                FILE *info_file = fopen("managers.txt", "r");
                                if (info_file != NULL) {
                                    char line[500];
                                    while (fgets(line, sizeof(line), info_file)) {
                                        char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                                        
                                        if (strcmp(user, username) == 0) {
                                            printf("Current Email: %s\n", email);
                                            printf("Current Phone: %s\n", phone);
                                            printf("Start Date: %s (Admin-controlled)\n", start);
                                            printf("National ID: %s (Admin-controlled)\n", id);
                                            break;
                                        }
                                    }
                                    fclose(info_file);
                                }
                                
                                printf("\n=== WHAT YOU CAN MODIFY ===\n");
                                printf("✓ Password\n");
                                printf("✓ Email Address\n");
                                printf("✓ Phone Number\n");
                                printf("\n=== ADMIN-CONTROLLED (Cannot be changed) ===\n");
                                printf("✗ First Name, Last Name\n");
                                printf("✗ National ID Number\n");
                                printf("✗ Username\n");
                                printf("✗ Management Start Date\n");
                                printf("✗ Department/Group\n");
                                
                                printf("\n1. Change Password\n");
                                printf("2. Update Contact Information\n");
                                printf("3. View Complete Profile\n");
                                printf("4. Return to Main Menu\n");
                                printf("Enter your choice: ");
                                
                                int settings_choice;
                                scanf("%d", &settings_choice);
                                
                                switch(settings_choice) {
                                    case 1:
                                        printf("\n=== CHANGE PASSWORD ===\n");
                                        printf("Password Requirements:\n");
                                        printf("- Minimum 6 characters\n");
                                        printf("- Maximum 49 characters\n");
                                        printf("- No spaces allowed\n\n");
                                        
                                        printf("Enter current password: ");
                                        char current_pass[50], new_pass[50], confirm_pass[50];
                                        scanf("%s", current_pass);
                                        
                                        if (strcmp(current_pass, password) == 0) {
                                            int password_valid = 0;
                                            do {
                                                printf("Enter new password: ");
                                                scanf("%s", new_pass);
                                                
                                                if (strlen(new_pass) < 6) {
                                                    printf("Error: Password must be at least 6 characters long.\n");
                                                    continue;
                                                }
                                                if (strlen(new_pass) > 49) {
                                                    printf("Error: Password must be less than 50 characters.\n");
                                                    continue;
                                                }
                                                if (strchr(new_pass, ' ') != NULL) {
                                                    printf("Error: Password cannot contain spaces.\n");
                                                    continue;
                                                }
                                                if (strcmp(new_pass, current_pass) == 0) {
                                                    printf("Error: New password must be different from current password.\n");
                                                    continue;
                                                }
                                                
                                                password_valid = 1;
                                            } while (!password_valid);
                                            
                                            printf("Confirm new password: ");
                                            scanf("%s", confirm_pass);
                                            
                                            if (strcmp(new_pass, confirm_pass) == 0) {
                                                FILE *pass_file = fopen("managers.txt", "r");
                                                if (pass_file != NULL) {
                                                    char lines[100][500];
                                                    int line_count = 0;
                                                    int found_index = -1;
                                                    
                                                    while (fgets(lines[line_count], sizeof(lines[line_count]), pass_file) && line_count < 100) {
                                                        line_count++;
                                                    }
                                                    fclose(pass_file);
                                                    
                                                    for (int i = 0; i < line_count; i++) {
                                                        char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                        sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                                                        
                                                        if (strcmp(user, username) == 0) {
                                                            sprintf(lines[i], "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n", fname, lname, start, dept, id, phone, email, user, new_pass, status, term);
                                                            strcpy(password, new_pass);
                                                            found_index = i;
                                                            break;
                                                        }
                                                    }
                                                    
                                                    if (found_index >= 0) {
                                                        pass_file = fopen("managers.txt", "w");
                                                        for (int i = 0; i < line_count; i++) {
                                                            fputs(lines[i], pass_file);
                                                        }
                                                        fclose(pass_file);
                                                        printf("\n✓ Password updated successfully!\n");
                                                        printf("Your new password is now active.\n");
                                                    }
                                                } else {
                                                    printf("Error: Could not access manager records.\n");
                                                }
                                            } else {
                                                printf("Error: Passwords do not match!\n");
                                            }
                                        } else {
                                            printf("Error: Current password is incorrect!\n");
                                        }
                                        break;
                                        
                                    case 2:
                                        printf("\n=== UPDATE CONTACT INFORMATION ===\n");
                                        printf("You can only update the following information:\n");
                                        printf("- Email address\n");
                                        printf("- Phone number\n");
                                        printf("\nNote: Name, National ID, Username, Start Date, and Department can only be changed by Admin.\n\n");
                                        
                                        printf("1. Update Email Address\n");
                                        printf("2. Update Phone Number\n");
                                        printf("3. Update Both Email and Phone\n");
                                        printf("4. Return to Account Settings\n");
                                        printf("Enter your choice: ");
                                        
                                        int contact_choice;
                                        scanf("%d", &contact_choice);
                                        
                                        switch(contact_choice) {
                                            case 1:
                                            case 2:
                                            case 3:
                                                {
                                                    FILE *contact_file = fopen("managers.txt", "r");
                                                    if (contact_file != NULL) {
                                                        char lines[100][500];
                                                        int line_count = 0;
                                                        int manager_found = 0;
                                                        int found_index = -1;
                                                        char current_email[100], current_phone[20];
                                                        char new_email[100], new_phone[20];
                                                        char confirm = 'N';
                                                        
                                                        while (fgets(lines[line_count], sizeof(lines[line_count]), contact_file) && line_count < 100) {
                                                            line_count++;
                                                        }
                                                        fclose(contact_file);
                                                        
                                                        for (int i = 0; i < line_count; i++) {
                                                            char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                            sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                                                            
                                                            if (strcmp(user, username) == 0) {
                                                                manager_found = 1;
                                                                found_index = i;
                                                                strcpy(current_email, email);
                                                                strcpy(current_phone, phone);
                                                                
                                                                printf("\n=== CURRENT INFORMATION ===\n");
                                                                printf("Current Email: %s\n", current_email);
                                                                printf("Current Phone: %s\n", current_phone);
                                                                printf("\n");
                                                                
                                                                if (contact_choice == 1 || contact_choice == 3) {
                                                                    printf("Enter new email address: ");
                                                                    scanf("%s", new_email);
                                                                    
                                                                    if (strchr(new_email, '@') == NULL || strchr(new_email, '.') == NULL) {
                                                                        printf("Error: Invalid email format! Email must contain @ and .\n");
                                                                        break;
                                                                    }
                                                                } else {
                                                                    strcpy(new_email, current_email);
                                                                }
                                                                
                                                                if (contact_choice == 2 || contact_choice == 3) {
                                                                    printf("Enter new phone number: ");
                                                                    scanf("%s", new_phone);
                                                                    
                                                                    if (strlen(new_phone) < 7) {
                                                                        printf("Error: Phone number too short! Must be at least 7 digits.\n");
                                                                        break;
                                                                    }
                                                                } else {
                                                                    strcpy(new_phone, current_phone);
                                                                }
                                                                
                                                                printf("\n=== CONFIRM CHANGES ===\n");
                                                                if (contact_choice == 1 || contact_choice == 3) {
                                                                    printf("New Email: %s\n", new_email);
                                                                }
                                                                if (contact_choice == 2 || contact_choice == 3) {
                                                                    printf("New Phone: %s\n", new_phone);
                                                                }
                                                                printf("Confirm changes? (Y/N): ");
                                                                scanf(" %c", &confirm);
                                                                
                                                                if (confirm == 'Y' || confirm == 'y') {
                                                                    sprintf(lines[i], "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n", 
                                                                            fname, lname, start, dept, id, new_phone, new_email, user, pass, status, term);
                                                                    
                                                                    printf("Contact information updated successfully!\n");
                                                                    
                                                                    if (contact_choice == 1 || contact_choice == 3) {
                                                                        printf("✓ Email updated: %s → %s\n", current_email, new_email);
                                                                    }
                                                                    if (contact_choice == 2 || contact_choice == 3) {
                                                                        printf("✓ Phone updated: %s → %s\n", current_phone, new_phone);
                                                                    }
                                                                } else {
                                                                    printf("Changes cancelled.\n");
                                                                    found_index = -1;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        
                                                        if (manager_found && found_index >= 0 && (confirm == 'Y' || confirm == 'y')) {
                                                            contact_file = fopen("managers.txt", "w");
                                                            for (int i = 0; i < line_count; i++) {
                                                                fputs(lines[i], contact_file);
                                                            }
                                                            fclose(contact_file);
                                                        } else if (!manager_found) {
                                                            printf("Error: Manager account not found in system.\n");
                                                        }
                                                    } else {
                                                        printf("Error: Could not access manager records.\n");
                                                    }
                                                }
                                                break;
                                                
                                            case 4:
                                                printf("Returning to Account Settings...\n");
                                                break;
                                                
                                            default:
                                                printf("Invalid choice!\n");
                                                break;
                                        }
                                        break;
                                        
                                    case 3:
                                        printf("\n=== COMPLETE PROFILE ===\n");
                                        FILE *profile_file = fopen("managers.txt", "r");
                                        if (profile_file != NULL) {
                                            char line[500];
                                            while (fgets(line, sizeof(line), profile_file)) {
                                                char fname[50], lname[50], start[50], dept[50], id[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, start, dept, id, phone, email, user, pass, status, term);
                                                
                                                if (strcmp(user, username) == 0) {
                                                    printf("==========================================\n");
                                                    printf("PERSONAL INFORMATION (Admin-controlled):\n");
                                                    printf("==========================================\n");
                                                    printf("Full Name: %s %s\n", fname, lname);
                                                    printf("National ID: %s\n", id);
                                                    printf("Username: %s\n", user);
                                                    printf("Department: %s\n", dept);
                                                    printf("Management Start Date: %s\n", start);
                                                    printf("Account Status: %s\n", status);
                                                    
                                                    printf("\n==========================================\n");
                                                    printf("CONTACT INFORMATION (You can modify):\n");
                                                    printf("==========================================\n");
                                                    printf("Email Address: %s\n", email);
                                                    printf("Phone Number: %s\n", phone);
                                                    
                                                    printf("\n==========================================\n");
                                                    printf("SECURITY:\n");
                                                    printf("==========================================\n");
                                                    printf("Password: ******** (You can change this)\n");
                                                    break;
                                                }
                                            }
                                            fclose(profile_file);
                                        } else {
                                            printf("Error: Could not access profile information.\n");
                                        }
                                        printf("\nPress Enter to continue...");
                                        getchar(); getchar();
                                        break;
                                        
                                    case 4:
                                        printf("Returning to Group Manager Menu...\n");
                                        break;
                                        
                                    default:
                                        printf("Invalid choice!\n");
                                        break;
                                }
                                break;
                                
                            case 8:
                                printf("Logged out successfully.\n");
                                break;
                                
                            default:
                                printf("Invalid choice! Please try again.\n");
                                break;
                        }
                        
                        if(manager_choice == 8) {
                            printf("\n✅ Logged out successfully\n");
                            printf("Session ended. Thank you, %s!\n", manager_name);
                            // Security: Save login state
                            save_login_attempts(login_attempts, attempt_count);
                            break;
                        }
                    }
                } else {
                    // Security: Record failed attempt
                    if (manager_attempt != NULL) {
                        record_failed_attempt(manager_attempt, username);
                        save_login_attempts(login_attempts, attempt_count);
                    } else {
                        printf("Wrong username or password, or account is not active.\n");
                    }
                }
                break;
            case 3:
                printf("\n🔐 STAFF LOGIN\n");
                printf("Enter username: ");
                scanf("%s", username);
                
                // Security: Check if account is locked
                LoginAttempt* staff_attempt = get_login_attempt(login_attempts, &attempt_count, username, 100);
                if (staff_attempt != NULL && is_account_locked(staff_attempt)) {
                    save_login_attempts(login_attempts, attempt_count);
                    break;
                }
                
                printf("Enter password: ");
                scanf("%s", password);
                
                FILE *staff_login_file = fopen("staff.txt", "r");
                int staff_found = 0;
                char staff_name[100];
                char staff_rank[50];
                
                if (staff_login_file != NULL) {
                    char line[500];
                    while (fgets(line, sizeof(line), staff_login_file)) {
                        char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                        int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                        
                        if (parsed >= 8 && strcmp(user, username) == 0 && strcmp(pass, password) == 0 && strcmp(status, "ACTIVE") == 0) {
                            staff_found = 1;
                            sprintf(staff_name, "%s %s", fname, lname);
                            strcpy(staff_rank, rank);
                            break;
                        }
                    }
                    fclose(staff_login_file);
                }
                
                if (staff_found) {
                    // Security: Reset attempts on successful login
                    if (staff_attempt != NULL) {
                        reset_login_attempts(staff_attempt);
                        save_login_attempts(login_attempts, attempt_count);
                    }
                    
                    printf("\n✅ Staff login successful!\n");
                    printf("Welcome, %s (%s)\n", staff_name, staff_rank);
                    
                    int staff_choice;
                    while(1) {
                        // Security: Check session timeout
                        if (staff_attempt != NULL && check_session_timeout(staff_attempt)) {
                            save_login_attempts(login_attempts, attempt_count);
                            break;
                        }
                        
                        printf("\n═══════════════════════════════════════\n");
                        printf("    EMPLOYEE TRAINING MAIN MENU\n");
                        printf("Staff: %s | Position: %s\n", staff_name, staff_rank);
                        printf("═══════════════════════════════════════\n");
                        printf("1. ➕ Register a new student\n");
                        printf("2. ✏️  Edit student information\n");
                        printf("3. ✍️  Enter grades\n");
                        printf("4. 📊 Training employee reports\n");
                        printf("5. ⚙️  User account settings\n");
                        printf("6. 🚪 Log out of the user account\n");
                        printf("═══════════════════════════════════════\n");
                        printf("Enter your choice: ");
                        scanf("%d", &staff_choice);
                        
                        switch(staff_choice) {
                            case 1:
                                printf("\n=== REGISTER NEW STUDENT ===\n");
                                printf("Please provide the following student information:\n");
                                printf("==============================================\n");
                                
                                char student_id[20], student_fname[50], student_lname[50];
                                char national_id[20], date_of_birth[15], place_of_birth[100];
                                char field_of_study[100], student_email[100], student_phone[20];
                                int valid_input = 0;
                                
                                do {
                                    printf("Enter student ID (e.g., STU001): ");
                                    scanf("%s", student_id);
                                    
                                    if (strlen(student_id) < 3) {
                                        printf("Error: Student ID must be at least 3 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(student_id) > 19) {
                                        printf("Error: Student ID is too long (max 19 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        for (int i = 0; i < strlen(student_id); i++) {
                                            student_id[i] = toupper(student_id[i]);
                                        }
                                        
                                        FILE *check_student_file = fopen("students.txt", "r");
                                        int student_exists = 0;
                                        if (check_student_file != NULL) {
                                            char line[500];
                                            while (fgets(line, sizeof(line), check_student_file)) {
                                                char existing_id[20];
                                                sscanf(line, "%[^|]", existing_id);
                                                if (strcmp(existing_id, student_id) == 0) {
                                                    student_exists = 1;
                                                    break;
                                                }
                                            }
                                            fclose(check_student_file);
                                        }
                                        
                                        if (student_exists) {
                                            printf("Error: Student ID %s already exists! Please choose a different ID.\n", student_id);
                                            valid_input = 0;
                                        } else {
                                            valid_input = 1;
                                        }
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter first name: ");
                                    scanf("%s", student_fname);
                                    
                                    if (strlen(student_fname) < 2) {
                                        printf("Error: First name must be at least 2 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(student_fname) > 49) {
                                        printf("Error: First name is too long (max 49 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter last name: ");
                                    scanf("%s", student_lname);
                                    
                                    if (strlen(student_lname) < 2) {
                                        printf("Error: Last name must be at least 2 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(student_lname) > 49) {
                                        printf("Error: Last name is too long (max 49 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter national ID number: ");
                                    scanf("%s", national_id);
                                    
                                    if (strlen(national_id) < 5) {
                                        printf("Error: National ID must be at least 5 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(national_id) > 19) {
                                        printf("Error: National ID is too long (max 19 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        FILE *check_id_file = fopen("students.txt", "r");
                                        int id_exists = 0;
                                        if (check_id_file != NULL) {
                                            char line[500];
                                            while (fgets(line, sizeof(line), check_id_file)) {
                                                char existing_data[500];
                                                strcpy(existing_data, line);
                                                char *token = strtok(existing_data, "|");
                                                for (int i = 0; i < 3 && token != NULL; i++) {
                                                    token = strtok(NULL, "|");
                                                }
                                                if (token != NULL && strcmp(token, national_id) == 0) {
                                                    id_exists = 1;
                                                    break;
                                                }
                                            }
                                            fclose(check_id_file);
                                        }
                                        
                                        if (id_exists) {
                                            printf("Error: National ID %s already exists! Each student must have a unique national ID.\n", national_id);
                                            valid_input = 0;
                                        } else {
                                            valid_input = 1;
                                        }
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter date of birth (YYYY-MM-DD): ");
                                    scanf("%s", date_of_birth);
                                    
                                    if (strlen(date_of_birth) != 10 || date_of_birth[4] != '-' || date_of_birth[7] != '-') {
                                        printf("Error: Date must be in YYYY-MM-DD format (e.g., 2000-01-15).\n");
                                        valid_input = 0;
                                    } else {
                                        int year;
                                        sscanf(date_of_birth, "%d", &year);
                                        time_t t = time(NULL);
                                        struct tm tm = *localtime(&t);
                                        int current_year = tm.tm_year + 1900;
                                        
                                        if (year < 1950 || year > current_year) {
                                            printf("Error: Birth year must be between 1950 and %d.\n", current_year);
                                            valid_input = 0;
                                        } else {
                                            valid_input = 1;
                                        }
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter place of birth: ");
                                    getchar();
                                    fgets(place_of_birth, sizeof(place_of_birth), stdin);
                                    place_of_birth[strcspn(place_of_birth, "\n")] = 0;
                                    
                                    if (strlen(place_of_birth) < 2) {
                                        printf("Error: Place of birth must be at least 2 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(place_of_birth) > 99) {
                                        printf("Error: Place of birth is too long (max 99 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter field of study: ");
                                    fgets(field_of_study, sizeof(field_of_study), stdin);
                                    field_of_study[strcspn(field_of_study, "\n")] = 0;
                                    
                                    if (strlen(field_of_study) < 2) {
                                        printf("Error: Field of study must be at least 2 characters long.\n");
                                        valid_input = 0;
                                    } else if (strlen(field_of_study) > 99) {
                                        printf("Error: Field of study is too long (max 99 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter mobile phone number: ");
                                    scanf("%s", student_phone);
                                    
                                    if (strlen(student_phone) < 7) {
                                        printf("Error: Phone number too short! Must be at least 7 digits.\n");
                                        valid_input = 0;
                                    } else if (strlen(student_phone) > 19) {
                                        printf("Error: Phone number is too long (max 19 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        valid_input = 1;
                                    }
                                } while (!valid_input);
                                
                                do {
                                    printf("Enter email address: ");
                                    scanf("%s", student_email);
                                    
                                    if (strchr(student_email, '@') == NULL || strchr(student_email, '.') == NULL) {
                                        printf("Error: Invalid email format! Email must contain @ and .\n");
                                        valid_input = 0;
                                    } else if (strlen(student_email) > 99) {
                                        printf("Error: Email is too long (max 99 characters).\n");
                                        valid_input = 0;
                                    } else {
                                        FILE *check_email_file = fopen("students.txt", "r");
                                        int email_exists = 0;
                                        if (check_email_file != NULL) {
                                            char line[500];
                                            while (fgets(line, sizeof(line), check_email_file)) {
                                                char existing_data[500];
                                                strcpy(existing_data, line);
                                                char *token = strtok(existing_data, "|");
                                                for (int i = 0; i < 8 && token != NULL; i++) {
                                                    token = strtok(NULL, "|");
                                                }
                                                if (token != NULL && strcmp(token, student_email) == 0) {
                                                    email_exists = 1;
                                                    break;
                                                }
                                            }
                                            fclose(check_email_file);
                                        }
                                        
                                        if (email_exists) {
                                            printf("Error: Email %s already exists! Each student must have a unique email.\n", student_email);
                                            valid_input = 0;
                                        } else {
                                            valid_input = 1;
                                        }
                                    }
                                } while (!valid_input);
                                
                                time_t t = time(NULL);
                                struct tm tm = *localtime(&t);
                                char enrollment_date[20];
                                sprintf(enrollment_date, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                                
                                printf("\n=== STUDENT REGISTRATION SUMMARY ===\n");
                                printf("Student ID: %s\n", student_id);
                                printf("Name: %s %s\n", student_fname, student_lname);
                                printf("National ID: %s\n", national_id);
                                printf("Date of Birth: %s\n", date_of_birth);
                                printf("Place of Birth: %s\n", place_of_birth);
                                printf("Field of Study: %s\n", field_of_study);
                                printf("Mobile Phone: %s\n", student_phone);
                                printf("Email: %s\n", student_email);
                                printf("Enrollment Date: %s\n", enrollment_date);
                                printf("Status: ACTIVE\n");
                                printf("Registered by: %s\n", staff_name);
                                
                                printf("\nConfirm registration? (Y/N): ");
                                char confirm;
                                scanf(" %c", &confirm);
                                
                                if (confirm == 'Y' || confirm == 'y') {
                                    FILE *students_file = fopen("students.txt", "a");
                                    if (students_file != NULL) {
                                        fprintf(students_file, "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|ACTIVE\n", 
                                                student_id, student_fname, student_lname, national_id, date_of_birth,
                                                place_of_birth, field_of_study, student_phone, student_email, enrollment_date);
                                        fflush(students_file);
                                        fclose(students_file);
                                        
                                        printf("\n STUDENT REGISTRATION SUCCESSFUL!\n");
                                        printf("Student %s %s (ID: %s) has been successfully registered!\n", 
                                               student_fname, student_lname, student_id);
                                        printf("Registration completed by: %s\n", staff_name);
                                    } else {
                                        printf("Error: Could not save student information to file.\n");
                                    }
                                } else {
                                    printf("Registration cancelled.\n");
                                }
                                break;
                                
                            case 2:
                                printf("\n=== EDIT STUDENT INFORMATION ===\n");
                                char edit_student_id[20];
                                printf("Enter Student ID to edit: ");
                                scanf("%s", edit_student_id);
                                
                                for (int i = 0; i < strlen(edit_student_id); i++) {
                                    edit_student_id[i] = toupper(edit_student_id[i]);
                                }
                                
                                FILE *edit_student_file = fopen("students.txt", "r");
                                if (edit_student_file != NULL) {
                                    char lines[200][1000];
                                    int line_count = 0;
                                    int student_found = 0;
                                    int found_index = -1;
                                    int changes_made = 0;
                                    
                                    while (fgets(lines[line_count], sizeof(lines[line_count]), edit_student_file) && line_count < 200) {
                                        line_count++;
                                    }
                                    fclose(edit_student_file);
                                    
                                    for (int i = 0; i < line_count; i++) {
                                        char sid[20], fname[50], lname[50], nid[20], dob[15], pob[100], fos[100], phone[20], email[100], enroll[20], status[20];
                                        
                                        int field_count = 1;
                                        for (int j = 0; lines[i][j] != '\0' && lines[i][j] != '\n'; j++) {
                                            if (lines[i][j] == '|') field_count++;
                                        }
                                        
                                        if (field_count >= 11) {
                                            sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s",
                                                   sid, fname, lname, nid, dob, pob, fos, phone, email, enroll, status);
                                        } else if (field_count >= 7) {
                                            sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s",
                                                   sid, fname, lname, email, phone, enroll, status);
                                            strcpy(nid, "N/A");
                                            strcpy(dob, "N/A");
                                            strcpy(pob, "N/A");
                                            strcpy(fos, "N/A");
                                        } else {
                                            continue;
                                        }
                                        
                                        if (strcmp(sid, edit_student_id) == 0) {
                                            student_found = 1;
                                            found_index = i;
                                            printf("\n=== CURRENT STUDENT INFORMATION ===\n");
                                            printf("Student ID: %s (Cannot be changed)\n", sid);
                                            printf("1. First Name: %s\n", fname);
                                            printf("2. Last Name: %s\n", lname);
                                            printf("3. National ID: %s\n", nid);
                                            printf("4. Date of Birth: %s\n", dob);
                                            printf("5. Place of Birth: %s\n", pob);
                                            printf("6. Field of Study: %s\n", fos);
                                            printf("7. Mobile Phone: %s\n", phone);
                                            printf("8. Email: %s\n", email);
                                            printf("9. Status: %s\n", status);
                                            printf("Enrollment Date: %s (Cannot be changed)\n", enroll);
                                            
                                            printf("\n=== EDIT OPTIONS ===\n");
                                            printf("1. Edit First Name\n");
                                            printf("2. Edit Last Name\n");
                                            printf("3. Edit National ID\n");
                                            printf("4. Edit Date of Birth\n");
                                            printf("5. Edit Place of Birth\n");
                                            printf("6. Edit Field of Study\n");
                                            printf("7. Edit Mobile Phone\n");
                                            printf("8. Edit Email\n");
                                            printf("9. Edit Status (ACTIVE/INACTIVE/PROBATION)\n");
                                            printf("10. Edit Multiple Fields\n");
                                            printf("11. Cancel\n");
                                            printf("Enter your choice: ");
                                            
                                            int edit_choice;
                                            scanf("%d", &edit_choice);
                                            
                                            char new_value[200];
                                            int valid_input = 0;
                                            
                                            switch(edit_choice) {
                                                case 1:
                                                    do {
                                                        printf("Enter new first name: ");
                                                        scanf("%s", new_value);
                                                        if (strlen(new_value) >= 2 && strlen(new_value) <= 49) {
                                                            strcpy(fname, new_value);
                                                            printf("✓ First name updated to: %s\n", fname);
                                                            changes_made = 1;
                                                            valid_input = 1;
                                                        } else {
                                                            printf("Error: First name must be 2-49 characters.\n");
                                                        }
                                                    } while (!valid_input);
                                                    break;
                                                    
                                                case 2:
                                                    do {
                                                        printf("Enter new last name: ");
                                                        scanf("%s", new_value);
                                                        if (strlen(new_value) >= 2 && strlen(new_value) <= 49) {
                                                            strcpy(lname, new_value);
                                                            printf("✓ Last name updated to: %s\n", lname);
                                                            changes_made = 1;
                                                            valid_input = 1;
                                                        } else {
                                                            printf("Error: Last name must be 2-49 characters.\n");
                                                        }
                                                    } while (!valid_input);
                                                    break;
                                                    
                                                case 3:
                                                    do {
                                                        printf("Enter new national ID: ");
                                                        scanf("%s", new_value);
                                                        if (strlen(new_value) >= 5 && strlen(new_value) <= 19) {
                                                            strcpy(nid, new_value);
                                                            printf("✓ National ID updated to: %s\n", nid);
                                                            changes_made = 1;
                                                            valid_input = 1;
                                                        } else {
                                                            printf("Error: National ID must be 5-19 characters.\n");
                                                        }
                                                    } while (!valid_input);
                                                    break;
                                                    
                                                case 4:
                                                    do {
                                                        printf("Enter new date of birth (YYYY-MM-DD): ");
                                                        scanf("%s", new_value);
                                                        if (strlen(new_value) == 10 && new_value[4] == '-' && new_value[7] == '-') {
                                                            strcpy(dob, new_value);
                                                            printf("✓ Date of birth updated to: %s\n", dob);
                                                            changes_made = 1;
                                                            valid_input = 1;
                                                        } else {
                                                            printf("Error: Date must be in YYYY-MM-DD format.\n");
                                                        }
                                                    } while (!valid_input);
                                                    break;
                                                    
                                                case 5:
                                                    printf("Enter new place of birth: ");
                                                    getchar();
                                                    fgets(new_value, sizeof(new_value), stdin);
                                                    new_value[strcspn(new_value, "\n")] = 0;
                                                    if (strlen(new_value) >= 2 && strlen(new_value) <= 99) {
                                                        strcpy(pob, new_value);
                                                        printf("✓ Place of birth updated to: %s\n", pob);
                                                        changes_made = 1;
                                                    } else {
                                                        printf("Error: Place of birth must be 2-99 characters.\n");
                                                    }
                                                    break;
                                                    
                                                case 6:
                                                    printf("Enter new field of study: ");
                                                    getchar();
                                                    fgets(new_value, sizeof(new_value), stdin);
                                                    new_value[strcspn(new_value, "\n")] = 0;
                                                    if (strlen(new_value) >= 2 && strlen(new_value) <= 99) {
                                                        strcpy(fos, new_value);
                                                        printf("✓ Field of study updated to: %s\n", fos);
                                                        changes_made = 1;
                                                    } else {
                                                        printf("Error: Field of study must be 2-99 characters.\n");
                                                    }
                                                    break;
                                                    
                                                case 7:
                                                    do {
                                                        printf("Enter new mobile phone: ");
                                                        scanf("%s", new_value);
                                                        if (strlen(new_value) >= 7 && strlen(new_value) <= 19) {
                                                            strcpy(phone, new_value);
                                                            printf("✓ Mobile phone updated to: %s\n", phone);
                                                            changes_made = 1;
                                                            valid_input = 1;
                                                        } else {
                                                            printf("Error: Phone must be 7-19 characters.\n");
                                                        }
                                                    } while (!valid_input);
                                                    break;
                                                    
                                                case 8:
                                                    do {
                                                        printf("Enter new email: ");
                                                        scanf("%s", new_value);
                                                        if (strchr(new_value, '@') != NULL && strchr(new_value, '.') != NULL && strlen(new_value) <= 99) {
                                                            strcpy(email, new_value);
                                                            printf("✓ Email updated to: %s\n", email);
                                                            changes_made = 1;
                                                            valid_input = 1;
                                                        } else {
                                                            printf("Error: Invalid email format or too long.\n");
                                                        }
                                                    } while (!valid_input);
                                                    break;
                                                    
                                                case 9:
                                                    printf("Select new status:\n");
                                                    printf("1. ACTIVE\n");
                                                    printf("2. INACTIVE\n");
                                                    printf("3. PROBATION\n");
                                                    printf("Enter choice: ");
                                                    int status_choice;
                                                    scanf("%d", &status_choice);
                                                    switch(status_choice) {
                                                        case 1: strcpy(status, "ACTIVE"); break;
                                                        case 2: strcpy(status, "INACTIVE"); break;
                                                        case 3: strcpy(status, "PROBATION"); break;
                                                        default: printf("Invalid choice, status unchanged.\n"); break;
                                                    }
                                                    if (status_choice >= 1 && status_choice <= 3) {
                                                        printf("✓ Status updated to: %s\n", status);
                                                        changes_made = 1;
                                                    }
                                                    break;
                                                    
                                                case 10:
                                                    printf("\n=== EDIT MULTIPLE FIELDS ===\n");
                                                    printf("Leave blank to keep current value\n");
                                                    
                                                    printf("First name [%s]: ", fname);
                                                    getchar();
                                                    fgets(new_value, sizeof(new_value), stdin);
                                                    new_value[strcspn(new_value, "\n")] = 0;
                                                    if (strlen(new_value) > 0 && strlen(new_value) <= 49) {
                                                        strcpy(fname, new_value);
                                                        changes_made = 1;
                                                    }
                                                    
                                                    printf("Last name [%s]: ", lname);
                                                    fgets(new_value, sizeof(new_value), stdin);
                                                    new_value[strcspn(new_value, "\n")] = 0;
                                                    if (strlen(new_value) > 0 && strlen(new_value) <= 49) {
                                                        strcpy(lname, new_value);
                                                        changes_made = 1;
                                                    }
                                                    
                                                    printf("Mobile phone [%s]: ", phone);
                                                    fgets(new_value, sizeof(new_value), stdin);
                                                    new_value[strcspn(new_value, "\n")] = 0;
                                                    if (strlen(new_value) > 0 && strlen(new_value) <= 19) {
                                                        strcpy(phone, new_value);
                                                        changes_made = 1;
                                                    }
                                                    
                                                    printf("Email [%s]: ", email);
                                                    fgets(new_value, sizeof(new_value), stdin);
                                                    new_value[strcspn(new_value, "\n")] = 0;
                                                    if (strlen(new_value) > 0 && strchr(new_value, '@') != NULL && strlen(new_value) <= 99) {
                                                        strcpy(email, new_value);
                                                        changes_made = 1;
                                                    }
                                                    
                                                    if (changes_made) {
                                                        printf("✓ Multiple fields updated successfully!\n");
                                                    }
                                                    break;
                                                    
                                                case 11:
                                                    printf("Edit cancelled.\n");
                                                    break;
                                                    
                                                default:
                                                    printf("Invalid choice!\n");
                                                    break;
                                            }
                                            
                                            if (changes_made) {
                                                sprintf(lines[i], "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n", 
                                                        sid, fname, lname, nid, dob, pob, fos, phone, email, enroll, status);
                                                
                                                printf("\n=== UPDATED STUDENT INFORMATION ===\n");
                                                printf("Student ID: %s\n", sid);
                                                printf("Name: %s %s\n", fname, lname);
                                                printf("National ID: %s\n", nid);
                                                printf("Date of Birth: %s\n", dob);
                                                printf("Place of Birth: %s\n", pob);
                                                printf("Field of Study: %s\n", fos);
                                                printf("Mobile Phone: %s\n", phone);
                                                printf("Email: %s\n", email);
                                                printf("Status: %s\n", status);
                                                printf("Updated by: %s\n", staff_name);
                                            }
                                            break;
                                        }
                                    }
                                    
                                    if (student_found && changes_made && found_index >= 0) {
                                        edit_student_file = fopen("students.txt", "w");
                                        for (int i = 0; i < line_count; i++) {
                                            fputs(lines[i], edit_student_file);
                                        }
                                        fclose(edit_student_file);
                                        printf("\n✓ Student information updated successfully in the system!\n");
                                    } else if (!student_found) {
                                        printf("Error: Student with ID %s not found.\n", edit_student_id);
                                    }
                                } else {
                                    printf("Error: Could not access student records.\n");
                                }
                                break;
                                
                            case 3:
                                printf("\n=== ENTER STUDENT GRADES ===\n");
                                char grade_student_id[20], grade_course_code[20], grade_letter[5];
                                int grade_numeric;
                                int grade_valid_input = 0;
                                
                                do {
                                    printf("Enter student ID: ");
                                    scanf("%s", grade_student_id);
                                    
                                    if (strlen(grade_student_id) < 3) {
                                        printf("Error: Student ID must be at least 3 characters long.\n");
                                        grade_valid_input = 0;
                                    } else if (strlen(grade_student_id) > 19) {
                                        printf("Error: Student ID is too long (max 19 characters).\n");
                                        grade_valid_input = 0;
                                    } else {
                                        for (int i = 0; i < strlen(grade_student_id); i++) {
                                            grade_student_id[i] = toupper(grade_student_id[i]);
                                        }
                                        
                                        FILE *check_student_file = fopen("students.txt", "r");
                                        int student_exists = 0;
                                        if (check_student_file != NULL) {
                                            char line[1000];
                                            while (fgets(line, sizeof(line), check_student_file)) {
                                                char existing_id[20];
                                                sscanf(line, "%[^|]", existing_id);
                                                if (strcmp(existing_id, grade_student_id) == 0) {
                                                    student_exists = 1;
                                                    break;
                                                }
                                            }
                                            fclose(check_student_file);
                                        }
                                        
                                        if (!student_exists) {
                                            printf("Error: Student ID %s does not exist in the system.\n", grade_student_id);
                                            printf("Please check the student ID or register the student first.\n");
                                            grade_valid_input = 0;
                                        } else {
                                            grade_valid_input = 1;
                                        }
                                    }
                                } while (!grade_valid_input);
                                
                                do {
                                    printf("Enter course code: ");
                                    scanf("%s", grade_course_code);
                                    
                                    if (strlen(grade_course_code) < 3) {
                                        printf("Error: Course code must be at least 3 characters long.\n");
                                        grade_valid_input = 0;
                                        continue;
                                    }
                                    
                                    for (int i = 0; i < strlen(grade_course_code); i++) {
                                        grade_course_code[i] = toupper(grade_course_code[i]);
                                    }
                                    
                                    FILE *check_course_file = fopen("courses.txt", "r");
                                    int course_exists = 0;
                                    if (check_course_file != NULL) {
                                        char line[500];
                                        while (fgets(line, sizeof(line), check_course_file)) {
                                            char existing_code[20], course_name[100], instructor[50], course_type[20], status[20];
                                            int credits;
                                            sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%s", existing_code, course_name, instructor, &credits, course_type, status);
                                            if (strcmp(existing_code, grade_course_code) == 0 && strcmp(status, "ACTIVE") == 0) {
                                                course_exists = 1;
                                                printf("Course found: %s - %s\n", existing_code, course_name);
                                                break;
                                            }
                                        }
                                        fclose(check_course_file);
                                    }
                                    
                                    if (!course_exists) {
                                        printf("Error: Course code %s does not exist or is not active in the system.\n", grade_course_code);
                                        printf("Please check the course code or contact administration.\n");
                                        grade_valid_input = 0;
                                    } else {
                                        grade_valid_input = 1;
                                    }
                                } while (!grade_valid_input);
                                
                                FILE *check_grade_file = fopen("grades.txt", "r");
                                int grade_exists = 0;
                                if (check_grade_file != NULL) {
                                    char line[500];
                                    while (fgets(line, sizeof(line), check_grade_file)) {
                                        char existing_student[20], existing_course[20];
                                        sscanf(line, "%[^|]|%[^|]", existing_student, existing_course);
                                        if (strcmp(existing_student, grade_student_id) == 0 && strcmp(existing_course, grade_course_code) == 0) {
                                            grade_exists = 1;
                                            break;
                                        }
                                    }
                                    fclose(check_grade_file);
                                }
                                
                                if (grade_exists) {
                                    printf("Warning: A grade already exists for student %s in course %s.\n", grade_student_id, grade_course_code);
                                    printf("Do you want to add another grade entry? (Y/N): ");
                                    char confirm;
                                    scanf(" %c", &confirm);
                                    if (confirm != 'Y' && confirm != 'y') {
                                        printf("Grade entry cancelled.\n");
                                        break;
                                    }
                                }
                                
                                do {
                                    printf("Enter numeric grade (0-100): ");
                                    if (scanf("%d", &grade_numeric) != 1) {
                                        printf("Error: Please enter a valid number.\n");
                                        while (getchar() != '\n');
                                        grade_valid_input = 0;
                                        continue;
                                    }
                                    
                                    if (grade_numeric < 0 || grade_numeric > 100) {
                                        printf("Error: Grade must be between 0 and 100.\n");
                                        grade_valid_input = 0;
                                    } else {
                                        grade_valid_input = 1;
                                    }
                                } while (!grade_valid_input);
                                
                                if (grade_numeric >= 97) strcpy(grade_letter, "A+");
                                else if (grade_numeric >= 93) strcpy(grade_letter, "A");
                                else if (grade_numeric >= 90) strcpy(grade_letter, "A-");
                                else if (grade_numeric >= 87) strcpy(grade_letter, "B+");
                                else if (grade_numeric >= 83) strcpy(grade_letter, "B");
                                else if (grade_numeric >= 80) strcpy(grade_letter, "B-");
                                else if (grade_numeric >= 77) strcpy(grade_letter, "C+");
                                else if (grade_numeric >= 73) strcpy(grade_letter, "C");
                                else if (grade_numeric >= 70) strcpy(grade_letter, "C-");
                                else if (grade_numeric >= 67) strcpy(grade_letter, "D+");
                                else if (grade_numeric >= 65) strcpy(grade_letter, "D");
                                else strcpy(grade_letter, "F");
                                
                                printf("\n=== GRADE ENTRY SUMMARY ===\n");
                                printf("Student ID: %s\n", grade_student_id);
                                printf("Course Code: %s\n", grade_course_code);
                                printf("Numeric Grade: %d\n", grade_numeric);
                                printf("Letter Grade: %s\n", grade_letter);
                                printf("Entered by: %s\n", staff_name);
                                
                                printf("\nConfirm grade entry? (Y/N): ");
                                char grade_confirm;
                                scanf(" %c", &grade_confirm);
                                
                                if (grade_confirm == 'Y' || grade_confirm == 'y') {
                                    time_t t = time(NULL);
                                    struct tm tm = *localtime(&t);
                                    char datetime_str[30];
                                    sprintf(datetime_str, "%04d-%02d-%02d %02d:%02d:%02d", 
                                            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                                            tm.tm_hour, tm.tm_min, tm.tm_sec);
                                    
                                    FILE *grades_file = fopen("grades.txt", "a");
                                    if (grades_file != NULL) {
                                        fprintf(grades_file, "%s|%s|%d|%s|%s|%s\n", 
                                                grade_student_id, grade_course_code, grade_numeric, grade_letter, 
                                                datetime_str, staff_name);
                                        fflush(grades_file);
                                        fclose(grades_file);
                                        
                                        printf("\n✓ GRADE ENTRY SUCCESSFUL!\n");
                                        printf("Student ID: %s\n", grade_student_id);
                                        printf("Course Code: %s\n", grade_course_code);
                                        printf("Numeric Grade: %d\n", grade_numeric);
                                        printf("Letter Grade: %s\n", grade_letter);
                                        printf("Entry Date/Time: %s\n", datetime_str);
                                        printf("Entered by: %s\n", staff_name);
                                        printf("Grade has been successfully recorded in the system!\n");
                                    } else {
                                        printf("Error: Could not save grade information to file.\n");
                                    }
                                } else {
                                    printf("Grade entry cancelled.\n");
                                }
                                break;
                                
                            case 4:
                                printf("\n=== TRAINING EMPLOYEE REPORTS ===\n");
                                printf("Loading data into memory using linked lists...\n");
                                
                                Student* students_head = load_students();
                                Course* courses_head = load_courses();
                                Grade* grades_head = load_grades();
                                
                                if (students_head == NULL) {
                                    printf("Warning: Could not load student data.\n");
                                }
                                if (courses_head == NULL) {
                                    printf("Warning: Could not load course data.\n");
                                }
                                if (grades_head == NULL) {
                                    printf("Warning: Could not load grade data.\n");
                                }
                                
                                int report_choice;
                                while (1) {
                                    printf("\n=== EDUCATION STAFF REPORTS MENU ===\n");
                                    printf("Staff: %s | Position: %s\n", staff_name, staff_rank);
                                    printf("==========================================\n");
                                    printf("1.  Find Student Details (by name or ID)\n");
                                    printf("2.  Students Born in Date Range\n");
                                    printf("3.  Students by Field of Study\n");
                                    printf("4.  Students Born in Specific City\n");
                                    printf("5.  All Students List\n");
                                    printf("6.  Student Grade Report\n");
                                    printf("7.  Students with Failing Grades\n");
                                    printf("8.  Students by Status (Active/Inactive/Probation)\n");
                                    printf("9.  Return to Staff Menu\n");
                                    printf("Enter your choice: ");
                                    
                                    scanf("%d", &report_choice);
                                    
                                    switch (report_choice) {
                                        case 1:
                                            staff_report_find_student(students_head);
                                            break;
                                        case 2:
                                            staff_report_students_by_birth_date_range(students_head);
                                            break;
                                        case 3:
                                            staff_report_students_by_field_of_study(students_head);
                                            break;
                                        case 4:
                                            staff_report_students_by_birth_city(students_head);
                                            break;
                                        case 5:
                                            staff_report_all_students(students_head);
                                            break;
                                        case 6:
                                            staff_report_student_grades(students_head, courses_head, grades_head);
                                            break;
                                        case 7:
                                            staff_report_failing_students(students_head, grades_head);
                                            break;
                                        case 8:
                                            staff_report_students_by_status(students_head);
                                            break;
                                        case 9:
                                            printf("Returning to Staff Menu...\n");
                                            break;
                                        default:
                                            printf("Invalid choice! Please try again.\n");
                                            continue;
                                    }
                                    
                                    if (report_choice == 9) break;
                                    
                                    printf("\nPress Enter to continue...");
                                    getchar();
                                    getchar();
                                }
                                
                                free_students(students_head);
                                free_courses(courses_head);
                                free_grades(grades_head);
                                printf("Memory freed successfully.\n");
                                break;
                                
                            case 5:
                                printf("\n=== USER ACCOUNT SETTINGS ===\n");
                                printf("Current account: %s\n", staff_name);
                                printf("Position: %s\n", staff_rank);
                                
                                FILE *staff_info_file = fopen("staff.txt", "r");
                                if (staff_info_file != NULL) {
                                    char line[500];
                                    while (fgets(line, sizeof(line), staff_info_file)) {
                                        char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                        
                                        if (strcmp(user, username) == 0) {
                                            printf("Current Email: %s\n", email);
                                            printf("Current Phone: %s\n", phone);
                                            printf("Hire Date: %s (Admin-controlled)\n", hire);
                                            printf("Staff Rank: %s (Admin-controlled)\n", rank);
                                            break;
                                        }
                                    }
                                    fclose(staff_info_file);
                                }
                                
                                printf("\n=== WHAT YOU CAN MODIFY ===\n");
                                printf("✓ Password\n");
                                printf("✓ Email Address\n");
                                printf("✓ Phone Number\n");
                                printf("\n=== ADMIN-CONTROLLED (Cannot be changed) ===\n");
                                printf("✗ First Name, Last Name\n");
                                printf("✗ Username\n");
                                printf("✗ Hire Date\n");
                                printf("✗ Staff Rank/Position\n");
                                
                                printf("\n1. Change Password\n");
                                printf("2. Update Contact Information\n");
                                printf("3. View Complete Profile\n");
                                printf("4. Return to Staff Menu\n");
                                printf("Enter your choice: ");
                                
                                int staff_settings_choice;
                                scanf("%d", &staff_settings_choice);
                                
                                switch(staff_settings_choice) {
                                    case 1:
                                        printf("\n=== CHANGE PASSWORD ===\n");
                                        printf("Password Requirements:\n");
                                        printf("- Minimum 6 characters\n");
                                        printf("- Maximum 49 characters\n");
                                        printf("- No spaces allowed\n\n");
                                        
                                        printf("Enter current password: ");
                                        char staff_current_pass[50], staff_new_pass[50], staff_confirm_pass[50];
                                        scanf("%s", staff_current_pass);
                                        
                                        if (strcmp(staff_current_pass, password) == 0) {
                                            int staff_password_valid = 0;
                                            do {
                                                printf("Enter new password: ");
                                                scanf("%s", staff_new_pass);
                                                
                                                if (strlen(staff_new_pass) < 6) {
                                                    printf("Error: Password must be at least 6 characters long.\n");
                                                    continue;
                                                }
                                                if (strlen(staff_new_pass) > 49) {
                                                    printf("Error: Password must be less than 50 characters.\n");
                                                    continue;
                                                }
                                                if (strchr(staff_new_pass, ' ') != NULL) {
                                                    printf("Error: Password cannot contain spaces.\n");
                                                    continue;
                                                }
                                                if (strcmp(staff_new_pass, staff_current_pass) == 0) {
                                                    printf("Error: New password must be different from current password.\n");
                                                    continue;
                                                }
                                                
                                                staff_password_valid = 1;
                                            } while (!staff_password_valid);
                                            
                                            printf("Confirm new password: ");
                                            scanf("%s", staff_confirm_pass);
                                            
                                            if (strcmp(staff_new_pass, staff_confirm_pass) == 0) {
                                                FILE *staff_pass_file = fopen("staff.txt", "r");
                                                if (staff_pass_file != NULL) {
                                                    char lines[100][500];
                                                    int line_count = 0;
                                                    int found_index = -1;
                                                    
                                                    while (fgets(lines[line_count], sizeof(lines[line_count]), staff_pass_file) && line_count < 100) {
                                                        line_count++;
                                                    }
                                                    fclose(staff_pass_file);
                                                    
                                                    for (int i = 0; i < line_count; i++) {
                                                        char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                        sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                                        
                                                        if (strcmp(user, username) == 0) {
                                                            sprintf(lines[i], "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n", fname, lname, hire, rank, phone, email, user, staff_new_pass, status, term);
                                                            strcpy(password, staff_new_pass);
                                                            found_index = i;
                                                            break;
                                                        }
                                                    }
                                                    
                                                    if (found_index >= 0) {
                                                        staff_pass_file = fopen("staff.txt", "w");
                                                        for (int i = 0; i < line_count; i++) {
                                                            fputs(lines[i], staff_pass_file);
                                                        }
                                                        fclose(staff_pass_file);
                                                        printf("\n✓ Password updated successfully!\n");
                                                        printf("Your new password is now active.\n");
                                                    }
                                                } else {
                                                    printf("Error: Could not access staff records.\n");
                                                }
                                            } else {
                                                printf("Error: Passwords do not match!\n");
                                            }
                                        } else {
                                            printf("Error: Current password is incorrect!\n");
                                        }
                                        break;
                                        
                                    case 2:
                                        printf("\n=== UPDATE CONTACT INFORMATION ===\n");
                                        printf("You can only update the following information:\n");
                                        printf("- Email address\n");
                                        printf("- Phone number\n");
                                        printf("\nNote: Name, Username, Hire Date, and Rank can only be changed by Admin.\n\n");
                                        
                                        printf("1. Update Email Address\n");
                                        printf("2. Update Phone Number\n");
                                        printf("3. Update Both Email and Phone\n");
                                        printf("4. Return to Account Settings\n");
                                        printf("Enter your choice: ");
                                        
                                        int staff_contact_choice;
                                        scanf("%d", &staff_contact_choice);
                                        
                                        switch(staff_contact_choice) {
                                            case 1:
                                            case 2:
                                            case 3:
                                                {
                                                    FILE *staff_contact_file = fopen("staff.txt", "r");
                                                    if (staff_contact_file != NULL) {
                                                        char lines[100][500];
                                                        int line_count = 0;
                                                        int staff_found = 0;
                                                        int found_index = -1;
                                                        char current_email[100], current_phone[20];
                                                        char new_email[100], new_phone[20];
                                                        char staff_confirm = 'N';
                                                        
                                                        while (fgets(lines[line_count], sizeof(lines[line_count]), staff_contact_file) && line_count < 100) {
                                                            line_count++;
                                                        }
                                                        fclose(staff_contact_file);
                                                        
                                                        for (int i = 0; i < line_count; i++) {
                                                            char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                            sscanf(lines[i], "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                                            
                                                            if (strcmp(user, username) == 0) {
                                                                staff_found = 1;
                                                                found_index = i;
                                                                strcpy(current_email, email);
                                                                strcpy(current_phone, phone);
                                                                
                                                                printf("\n=== CURRENT INFORMATION ===\n");
                                                                printf("Current Email: %s\n", current_email);
                                                                printf("Current Phone: %s\n", current_phone);
                                                                printf("\n");
                                                                
                                                                if (staff_contact_choice == 1 || staff_contact_choice == 3) {
                                                                    printf("Enter new email address: ");
                                                                    scanf("%s", new_email);
                                                                    
                                                                    if (strchr(new_email, '@') == NULL || strchr(new_email, '.') == NULL) {
                                                                        printf("Error: Invalid email format! Email must contain @ and .\n");
                                                                        break;
                                                                    }
                                                                } else {
                                                                    strcpy(new_email, current_email);
                                                                }
                                                                
                                                                if (staff_contact_choice == 2 || staff_contact_choice == 3) {
                                                                    printf("Enter new phone number: ");
                                                                    scanf("%s", new_phone);
                                                                    
                                                                    if (strlen(new_phone) < 7) {
                                                                        printf("Error: Phone number too short! Must be at least 7 digits.\n");
                                                                        break;
                                                                    }
                                                                } else {
                                                                    strcpy(new_phone, current_phone);
                                                                }
                                                                
                                                                printf("\n=== CONFIRM CHANGES ===\n");
                                                                if (staff_contact_choice == 1 || staff_contact_choice == 3) {
                                                                    printf("New Email: %s\n", new_email);
                                                                }
                                                                if (staff_contact_choice == 2 || staff_contact_choice == 3) {
                                                                    printf("New Phone: %s\n", new_phone);
                                                                }
                                                                printf("Confirm changes? (Y/N): ");
                                                                scanf(" %c", &staff_confirm);
                                                                
                                                                if (staff_confirm == 'Y' || staff_confirm == 'y') {
                                                                    sprintf(lines[i], "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n", 
                                                                            fname, lname, hire, rank, new_phone, new_email, user, pass, status, term);
                                                                    
                                                                    printf("Contact information updated successfully!\n");
                                                                    
                                                                    if (staff_contact_choice == 1 || staff_contact_choice == 3) {
                                                                        printf("✓ Email updated: %s → %s\n", current_email, new_email);
                                                                    }
                                                                    if (staff_contact_choice == 2 || staff_contact_choice == 3) {
                                                                        printf("✓ Phone updated: %s → %s\n", current_phone, new_phone);
                                                                    }
                                                                } else {
                                                                    printf("Changes cancelled.\n");
                                                                    found_index = -1;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        
                                                        if (staff_found && found_index >= 0 && (staff_confirm == 'Y' || staff_confirm == 'y')) {
                                                            staff_contact_file = fopen("staff.txt", "w");
                                                            for (int i = 0; i < line_count; i++) {
                                                                fputs(lines[i], staff_contact_file);
                                                            }
                                                            fclose(staff_contact_file);
                                                        } else if (!staff_found) {
                                                            printf("Error: Staff account not found in system.\n");
                                                        }
                                                    } else {
                                                        printf("Error: Could not access staff records.\n");
                                                    }
                                                }
                                                break;
                                                
                                            case 4:
                                                printf("Returning to Account Settings...\n");
                                                break;
                                                
                                            default:
                                                printf("Invalid choice!\n");
                                                break;
                                        }
                                        break;
                                        
                                    case 3:
                                        printf("\n=== COMPLETE PROFILE ===\n");
                                        FILE *staff_profile_file = fopen("staff.txt", "r");
                                        if (staff_profile_file != NULL) {
                                            char line[500];
                                            while (fgets(line, sizeof(line), staff_profile_file)) {
                                                char fname[50], lname[50], hire[50], rank[50], phone[50], email[50], user[50], pass[50], status[20], term[20];
                                                sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", fname, lname, hire, rank, phone, email, user, pass, status, term);
                                                
                                                if (strcmp(user, username) == 0) {
                                                    printf("==========================================\n");
                                                    printf("PERSONAL INFORMATION (Admin-controlled):\n");
                                                    printf("==========================================\n");
                                                    printf("Full Name: %s %s\n", fname, lname);
                                                    printf("Username: %s\n", user);
                                                    printf("Staff Rank: %s\n", rank);
                                                    printf("Hire Date: %s\n", hire);
                                                    printf("Account Status: %s\n", status);
                                                    
                                                    printf("\n==========================================\n");
                                                    printf("CONTACT INFORMATION (You can modify):\n");
                                                    printf("==========================================\n");
                                                    printf("Email Address: %s\n", email);
                                                    printf("Phone Number: %s\n", phone);
                                                    
                                                    printf("\n==========================================\n");
                                                    printf("SECURITY:\n");
                                                    printf("==========================================\n");
                                                    printf("Password: ******** (You can change this)\n");
                                                    break;
                                                }
                                            }
                                            fclose(staff_profile_file);
                                        } else {
                                            printf("Error: Could not access profile information.\n");
                                        }
                                        printf("\nPress Enter to continue...");
                                        getchar(); getchar();
                                        break;
                                        
                                    case 4:
                                        printf("Returning to Staff Menu...\n");
                                        break;
                                        
                                    default:
                                        printf("Invalid choice!\n");
                                        break;
                                }
                                break;
                                
                            case 6:
                                printf("Logged out successfully.\n");
                                break;
                                
                            default:
                                printf("Invalid choice! Please try again.\n");
                                break;
                        }
                        
                        if(staff_choice == 6) {
                            printf("\n✅ Logged out successfully\n");
                            printf("Session ended. Thank you, %s!\n", staff_name);
                            // Security: Save login state
                            save_login_attempts(login_attempts, attempt_count);
                            break;
                        }
                    }
                } else {
                    // Security: Record failed attempt
                    if (staff_attempt != NULL) {
                        record_failed_attempt(staff_attempt, username);
                        save_login_attempts(login_attempts, attempt_count);
                    } else {
                        printf("Wrong username or password, or account is not active.\n");
                    }
                }
                break;
            case 4:
                printf("\n👋 Goodbye! Thank you for using the system.\n");
                // Security: Save all login attempts before exiting
                save_login_attempts(login_attempts, attempt_count);
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
                break;
        }
    }
    
    return 0;
}


Student* load_students() {
    FILE *file = fopen("students.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open students.txt file.\n");
        return NULL;
    }
    
    Student *head = NULL;
    Student *current = NULL;
    char line[1000];
    
    while (fgets(line, sizeof(line), file)) {
        Student *new_student = (Student*)malloc(sizeof(Student));
        if (new_student == NULL) {
            printf("Error: Memory allocation failed.\n");
            fclose(file);
            return head;
        }
        
        strcpy(new_student->student_id, "");
        strcpy(new_student->first_name, "");
        strcpy(new_student->last_name, "");
        strcpy(new_student->national_id, "");
        strcpy(new_student->date_of_birth, "");
        strcpy(new_student->place_of_birth, "");
        strcpy(new_student->field_of_study, "");
        strcpy(new_student->phone, "");
        strcpy(new_student->email, "");
        strcpy(new_student->enrollment_date, "");
        strcpy(new_student->status, "");
        
        int field_count = 1;
        for (int i = 0; line[i] != '\0' && line[i] != '\n'; i++) {
            if (line[i] == '|') field_count++;
        }
        
        if (field_count >= 11) {
            sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s",
                   new_student->student_id, new_student->first_name, new_student->last_name,
                   new_student->national_id, new_student->date_of_birth, new_student->place_of_birth,
                   new_student->field_of_study, new_student->phone, new_student->email,
                   new_student->enrollment_date, new_student->status);
        } else if (field_count >= 7) {
            sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s",
                   new_student->student_id, new_student->first_name, new_student->last_name,
                   new_student->email, new_student->phone, new_student->enrollment_date,
                   new_student->status);
            strcpy(new_student->national_id, "N/A");
            strcpy(new_student->date_of_birth, "N/A");
            strcpy(new_student->place_of_birth, "N/A");
            strcpy(new_student->field_of_study, "N/A");
        } else {
            free(new_student);
            continue;
        }
        
        new_student->next = NULL;
        
        if (head == NULL) {
            head = new_student;
            current = new_student;
        } else {
            current->next = new_student;
            current = new_student;
        }
    }
    
    fclose(file);
    return head;
}

Course* load_courses() {
    FILE *file = fopen("courses.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open courses.txt file.\n");
        return NULL;
    }
    
    Course *head = NULL;
    Course *current = NULL;
    char line[500];
    
    while (fgets(line, sizeof(line), file)) {
        Course *new_course = (Course*)malloc(sizeof(Course));
        if (new_course == NULL) {
            printf("Error: Memory allocation failed.\n");
            fclose(file);
            return head;
        }
        
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%s",
               new_course->course_code, new_course->course_name, new_course->instructor,
               &new_course->credits, new_course->course_type, new_course->status);
        
        new_course->next = NULL;
        
        if (head == NULL) {
            head = new_course;
            current = new_course;
        } else {
            current->next = new_course;
            current = new_course;
        }
    }
    
    fclose(file);
    return head;
}

Grade* load_grades() {
    FILE *file = fopen("grades.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open grades.txt file.\n");
        return NULL;
    }
    
    Grade *head = NULL;
    Grade *current = NULL;
    char line[500];
    
    while (fgets(line, sizeof(line), file)) {
        Grade *new_grade = (Grade*)malloc(sizeof(Grade));
        if (new_grade == NULL) {
            printf("Error: Memory allocation failed.\n");
            fclose(file);
            return head;
        }
        
        sscanf(line, "%[^|]|%[^|]|%d|%[^|]|%[^|]|%s",
               new_grade->student_id, new_grade->course_code, &new_grade->numeric_grade,
               new_grade->letter_grade, new_grade->date_time, new_grade->registered_by);
        
        new_grade->next = NULL;
        
        if (head == NULL) {
            head = new_grade;
            current = new_grade;
        } else {
            current->next = new_grade;
            current = new_grade;
        }
    }
    
    fclose(file);
    return head;
}

void free_students(Student* head) {
    Student* current = head;
    while (current != NULL) {
        Student* temp = current;
        current = current->next;
        free(temp);
    }
}

void free_courses(Course* head) {
    Course* current = head;
    while (current != NULL) {
        Course* temp = current;
        current = current->next;
        free(temp);
    }
}

void free_grades(Grade* head) {
    Grade* current = head;
    while (current != NULL) {
        Grade* temp = current;
        current = current->next;
        free(temp);
    }
}

Student* find_student_by_id(Student* head, char* student_id) {
    Student* current = head;
    while (current != NULL) {
        if (strcmp(current->student_id, student_id) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Course* find_course_by_code(Course* head, char* course_code) {
    Course* current = head;
    while (current != NULL) {
        if (strcmp(current->course_code, course_code) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

float calculate_gpa(Grade* grades_head, char* student_id) {
    Grade* current = grades_head;
    float total_points = 0.0;
    int total_courses = 0;
    
    while (current != NULL) {
        if (strcmp(current->student_id, student_id) == 0) {
            total_points += current->numeric_grade;
            total_courses++;
        }
        current = current->next;
    }
    
    if (total_courses == 0) return 0.0;
    return total_points / total_courses;
}

float calculate_course_average(Grade* grades_head, char* course_code) {
    Grade* current = grades_head;
    float total_points = 0.0;
    int total_students = 0;
    
    while (current != NULL) {
        if (strcmp(current->course_code, course_code) == 0) {
            total_points += current->numeric_grade;
            total_students++;
        }
        current = current->next;
    }
    
    if (total_students == 0) return 0.0;
    return total_points / total_students;
}

void generate_reports_menu(char* manager_name) {
    Student* students_head = load_students();
    Course* courses_head = load_courses();
    Grade* grades_head = load_grades();
    
    if (students_head == NULL || courses_head == NULL || grades_head == NULL) {
        printf("Error: Could not load required data files.\n");
        return;
    }
    
    int choice;
    while (1) {
        printf("\n=== COMPREHENSIVE REPORTS SYSTEM ===\n");
        printf("1.  Find Student Details (by name or ID)\n");
        printf("2.  Complete List of Students\n");
        printf("3.  Complete List of Courses\n");
        printf("4.  List of Deleted Courses\n");
        printf("5.  List of Active Courses\n");
        printf("6.  Grades of a Specific Student\n");
        printf("7.  Grades of Students in a Specific Course\n");
        printf("8.  Grades of Students in Course (Sorted by Last Name)\n");
        printf("9.  GPA of a Specific Student\n");
        printf("10. Average Grade of a Specific Course\n");
        printf("11. List of All Students Sorted by GPA\n");
        printf("12. Students Who Passed a Specific Course\n");
        printf("13. Students Who Failed a Specific Course\n");
        printf("14. List of All Probationary Students\n");
        printf("15. Probationary Students in a Specific Course\n");
        printf("16. Return to Group Manager Menu\n");
        printf("Enter your choice: ");
        
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                report_find_student(students_head);
                break;
            case 2:
                report_all_students(students_head);
                break;
            case 3:
                report_all_courses(courses_head);
                break;
            case 4:
                report_deleted_courses(courses_head);
                break;
            case 5:
                report_active_courses(courses_head);
                break;
            case 6:
                report_student_grades(students_head, courses_head, grades_head);
                break;
            case 7:
                report_course_grades(students_head, courses_head, grades_head);
                break;
            case 8:
                report_course_grades_sorted(students_head, courses_head, grades_head);
                break;
            case 9:
                report_student_gpa(students_head, grades_head);
                break;
            case 10:
                report_course_average(courses_head, grades_head);
                break;
            case 11:
                report_students_by_gpa(students_head, grades_head);
                break;
            case 12:
                report_students_passed_course(students_head, courses_head, grades_head);
                break;
            case 13:
                report_students_failed_course(students_head, courses_head, grades_head);
                break;
            case 14:
                report_probationary_students(students_head, grades_head);
                break;
            case 15:
                report_probationary_students_course(students_head, courses_head, grades_head);
                break;
            case 16:
                printf("Returning to Group Manager Menu...\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
                continue;
        }
        
        if (choice == 16) break;
        
        printf("\nPress Enter to continue...");
        getchar();
        getchar();
    }
    
    free_students(students_head);
    free_courses(courses_head);
    free_grades(grades_head);
}


void report_find_student(Student* students_head) {
    printf("\n=== FIND STUDENT DETAILS ===\n");
    printf("Search by:\n");
    printf("1. Student ID\n");
    printf("2. First Name\n");
    printf("3. Last Name\n");
    printf("4. National ID\n");
    printf("Enter your choice: ");
    
    int search_choice;
    scanf("%d", &search_choice);
    
    char search_term[100];
    Student* current = students_head;
    int found = 0;
    
    switch (search_choice) {
        case 1:
            printf("Enter Student ID: ");
            scanf("%s", search_term);
            while (current != NULL) {
                if (strcmp(current->student_id, search_term) == 0) {
                    printf("\n=== STUDENT FOUND ===\n");
                    printf("Student ID: %s\n", current->student_id);
                    printf("Name: %s %s\n", current->first_name, current->last_name);
                    printf("National ID: %s\n", current->national_id);
                    printf("Date of Birth: %s\n", current->date_of_birth);
                    printf("Place of Birth: %s\n", current->place_of_birth);
                    printf("Field of Study: %s\n", current->field_of_study);
                    printf("Mobile Phone: %s\n", current->phone);
                    printf("Email: %s\n", current->email);
                    printf("Enrollment Date: %s\n", current->enrollment_date);
                    printf("Status: %s\n", current->status);
                    found = 1;
                    break;
                }
                current = current->next;
            }
            break;
        case 2:
            printf("Enter First Name: ");
            scanf("%s", search_term);
            printf("\n=== SEARCH RESULTS ===\n");
            while (current != NULL) {
                if (strcasecmp(current->first_name, search_term) == 0) {
                    printf("ID: %s | Name: %s %s | Field: %s | Status: %s\n", 
                           current->student_id, current->first_name, current->last_name, 
                           current->field_of_study, current->status);
                    found = 1;
                }
                current = current->next;
            }
            break;
        case 3:
            printf("Enter Last Name: ");
            scanf("%s", search_term);
            printf("\n=== SEARCH RESULTS ===\n");
            while (current != NULL) {
                if (strcasecmp(current->last_name, search_term) == 0) {
                    printf("ID: %s | Name: %s %s | Field: %s | Status: %s\n", 
                           current->student_id, current->first_name, current->last_name,
                           current->field_of_study, current->status);
                    found = 1;
                }
                current = current->next;
            }
            break;
        case 4:
            printf("Enter National ID: ");
            scanf("%s", search_term);
            while (current != NULL) {
                if (strcmp(current->national_id, search_term) == 0) {
                    printf("\n=== STUDENT FOUND ===\n");
                    printf("Student ID: %s\n", current->student_id);
                    printf("Name: %s %s\n", current->first_name, current->last_name);
                    printf("National ID: %s\n", current->national_id);
                    printf("Date of Birth: %s\n", current->date_of_birth);
                    printf("Place of Birth: %s\n", current->place_of_birth);
                    printf("Field of Study: %s\n", current->field_of_study);
                    printf("Mobile Phone: %s\n", current->phone);
                    printf("Email: %s\n", current->email);
                    printf("Enrollment Date: %s\n", current->enrollment_date);
                    printf("Status: %s\n", current->status);
                    found = 1;
                    break;
                }
                current = current->next;
            }
            break;
        default:
            printf("Invalid choice!\n");
            return;
    }
    
    if (!found) {
        printf("No student found with the given criteria.\n");
    }
}

void report_all_students(Student* students_head) {
    printf("\n=== COMPLETE LIST OF STUDENTS ===\n");
    printf("%-8s %-12s %-12s %-12s %-12s %-20s %-15s %-12s %-20s %-12s %-8s\n", 
           "ID", "First", "Last", "National ID", "Birth Date", "Place of Birth", "Field of Study", "Phone", "Email", "Enrolled", "Status");
    printf("===========================================================================================================================================================================\n");
    
    Student* current = students_head;
    int count = 0;
    
    while (current != NULL) {
        char short_place[21], short_field[16], short_email[21];
        strncpy(short_place, current->place_of_birth, 20);
        short_place[20] = '\0';
        strncpy(short_field, current->field_of_study, 15);
        short_field[15] = '\0';
        strncpy(short_email, current->email, 20);
        short_email[20] = '\0';
        
        printf("%-8s %-12s %-12s %-12s %-12s %-20s %-15s %-12s %-20s %-12s %-8s\n",
               current->student_id, current->first_name, current->last_name,
               current->national_id, current->date_of_birth, short_place,
               short_field, current->phone, short_email, 
               current->enrollment_date, current->status);
        count++;
        current = current->next;
    }
    
    printf("===========================================================================================================================================================================\n");
    printf("Total Students: %d\n", count);
}

void report_all_courses(Course* courses_head) {
    printf("\n=== COMPLETE LIST OF COURSES ===\n");
    printf("%-10s %-30s %-20s %-8s %-12s %-10s\n", 
           "Code", "Course Name", "Instructor", "Credits", "Type", "Status");
    printf("================================================================================\n");
    
    Course* current = courses_head;
    int count = 0;
    
    while (current != NULL) {
        printf("%-10s %-30s %-20s %-8d %-12s %-10s\n",
               current->course_code, current->course_name, current->instructor,
               current->credits, current->course_type, current->status);
        count++;
        current = current->next;
    }
    
    printf("================================================================================\n");
    printf("Total Courses: %d\n", count);
}

void report_deleted_courses(Course* courses_head) {
    printf("\n=== LIST OF DELETED COURSES ===\n");
    printf("%-10s %-30s %-20s %-8s %-12s\n", 
           "Code", "Course Name", "Instructor", "Credits", "Type");
    printf("================================================================================\n");
    
    Course* current = courses_head;
    int count = 0;
    
    while (current != NULL) {
        if (strcmp(current->status, "DELETED") == 0) {
            printf("%-10s %-30s %-20s %-8d %-12s\n",
                   current->course_code, current->course_name, current->instructor,
                   current->credits, current->course_type);
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No deleted courses found.\n");
    } else {
        printf("Total Deleted Courses: %d\n", count);
    }
}

void report_active_courses(Course* courses_head) {
    printf("\n=== LIST OF ACTIVE COURSES ===\n");
    printf("%-10s %-30s %-20s %-8s %-12s\n", 
           "Code", "Course Name", "Instructor", "Credits", "Type");
    printf("================================================================================\n");
    
    Course* current = courses_head;
    int count = 0;
    
    while (current != NULL) {
        if (strcmp(current->status, "ACTIVE") == 0) {
            printf("%-10s %-30s %-20s %-8d %-12s\n",
                   current->course_code, current->course_name, current->instructor,
                   current->credits, current->course_type);
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No active courses found.\n");
    } else {
        printf("Total Active Courses: %d\n", count);
    }
}

void report_student_grades(Student* students_head, Course* courses_head, Grade* grades_head) {
    printf("\n=== STUDENT GRADES REPORT ===\n");
    printf("Enter Student ID: ");
    char student_id[20];
    scanf("%s", student_id);
    
    Student* student = find_student_by_id(students_head, student_id);
    if (student == NULL) {
        printf("Student with ID %s not found.\n", student_id);
        return;
    }
    
    printf("\n=== GRADES FOR %s %s (ID: %s) ===\n", 
           student->first_name, student->last_name, student_id);
    printf("%-10s %-30s %-8s %-8s %-20s\n", 
           "Course", "Course Name", "Grade", "Letter", "Date/Time");
    printf("================================================================================\n");
    
    Grade* current = grades_head;
    int count = 0;
    float total_points = 0;
    
    while (current != NULL) {
        if (strcmp(current->student_id, student_id) == 0) {
            Course* course = find_course_by_code(courses_head, current->course_code);
            char course_name[100] = "Unknown Course";
            if (course != NULL) {
                strcpy(course_name, course->course_name);
            }
            
            printf("%-10s %-30s %-8d %-8s %-20s\n",
                   current->course_code, course_name, current->numeric_grade,
                   current->letter_grade, current->date_time);
            
            total_points += current->numeric_grade;
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No grades found for this student.\n");
    } else {
        printf("Total Courses: %d\n", count);
        printf("Average Grade (GPA): %.2f\n", total_points / count);
    }
}
void report_course_grades(Student* students_head, Course* courses_head, Grade* grades_head) {
    printf("\n=== COURSE GRADES REPORT ===\n");
    printf("Enter Course Code: ");
    char course_code[20];
    scanf("%s", course_code);
    
    Course* course = find_course_by_code(courses_head, course_code);
    if (course == NULL) {
        printf("Course with code %s not found.\n", course_code);
        return;
    }
    
    printf("\n=== GRADES FOR %s (%s) ===\n", course->course_name, course_code);
    printf("%-10s %-25s %-8s %-8s %-20s\n", 
           "Student ID", "Student Name", "Grade", "Letter", "Date/Time");
    printf("================================================================================\n");
    
    Grade* current = grades_head;
    int count = 0;
    float total_points = 0;
    
    while (current != NULL) {
        if (strcmp(current->course_code, course_code) == 0) {
            Student* student = find_student_by_id(students_head, current->student_id);
            char student_name[100] = "Unknown Student";
            if (student != NULL) {
                sprintf(student_name, "%s %s", student->first_name, student->last_name);
            }
            
            printf("%-10s %-25s %-8d %-8s %-20s\n",
                   current->student_id, student_name, current->numeric_grade,
                   current->letter_grade, current->date_time);
            
            total_points += current->numeric_grade;
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No grades found for this course.\n");
    } else {
        printf("Total Students: %d\n", count);
        printf("Course Average: %.2f\n", total_points / count);
    }
}

void report_course_grades_sorted(Student* students_head, Course* courses_head, Grade* grades_head) {
    printf("\n=== COURSE GRADES REPORT (SORTED BY LAST NAME) ===\n");
    printf("Enter Course Code: ");
    char course_code[20];
    scanf("%s", course_code);
    
    Course* course = find_course_by_code(courses_head, course_code);
    if (course == NULL) {
        printf("Course with code %s not found.\n", course_code);
        return;
    }
    
    typedef struct {
        char student_id[20];
        char student_name[100];
        int numeric_grade;
        char letter_grade[5];
        char date_time[30];
    } TempGrade;
    
    TempGrade temp_grades[1000];
    int count = 0;
    
    Grade* current = grades_head;
    while (current != NULL && count < 1000) {
        if (strcmp(current->course_code, course_code) == 0) {
            Student* student = find_student_by_id(students_head, current->student_id);
            if (student != NULL) {
                strcpy(temp_grades[count].student_id, current->student_id);
                sprintf(temp_grades[count].student_name, "%s %s", student->first_name, student->last_name);
                temp_grades[count].numeric_grade = current->numeric_grade;
                strcpy(temp_grades[count].letter_grade, current->letter_grade);
                strcpy(temp_grades[count].date_time, current->date_time);
                count++;
            }
        }
        current = current->next;
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            char last1[50], last2[50];
            sscanf(temp_grades[j].student_name, "%*s %s", last1);
            sscanf(temp_grades[j+1].student_name, "%*s %s", last2);
            
            if (strcmp(last1, last2) > 0) {
                TempGrade temp = temp_grades[j];
                temp_grades[j] = temp_grades[j+1];
                temp_grades[j+1] = temp;
            }
        }
    }
    
    printf("\n=== GRADES FOR %s (%s) - SORTED BY LAST NAME ===\n", course->course_name, course_code);
    printf("%-10s %-25s %-8s %-8s %-20s\n", 
           "Student ID", "Student Name", "Grade", "Letter", "Date/Time");
    printf("================================================================================\n");
    
    float total_points = 0;
    for (int i = 0; i < count; i++) {
        printf("%-10s %-25s %-8d %-8s %-20s\n",
               temp_grades[i].student_id, temp_grades[i].student_name,
               temp_grades[i].numeric_grade, temp_grades[i].letter_grade,
               temp_grades[i].date_time);
        total_points += temp_grades[i].numeric_grade;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No grades found for this course.\n");
    } else {
        printf("Total Students: %d\n", count);
        printf("Course Average: %.2f\n", total_points / count);
    }
}

void report_student_gpa(Student* students_head, Grade* grades_head) {
    printf("\n=== STUDENT GPA REPORT ===\n");
    printf("Enter Student ID: ");
    char student_id[20];
    scanf("%s", student_id);
    
    Student* student = find_student_by_id(students_head, student_id);
    if (student == NULL) {
        printf("Student with ID %s not found.\n", student_id);
        return;
    }
    
    float gpa = calculate_gpa(grades_head, student_id);
    
    printf("\n=== GPA REPORT ===\n");
    printf("Student: %s %s (ID: %s)\n", student->first_name, student->last_name, student_id);
    printf("Status: %s\n", student->status);
    printf("GPA: %.2f\n", gpa);
    
    if (gpa < 60.0) {
        printf("Status: PROBATIONARY (GPA below 60.0)\n");
    } else if (gpa >= 90.0) {
        printf("Status: EXCELLENT (GPA 90.0 or above)\n");
    } else if (gpa >= 80.0) {
        printf("Status: GOOD (GPA 80.0 or above)\n");
    } else {
        printf("Status: SATISFACTORY\n");
    }
}

void report_course_average(Course* courses_head, Grade* grades_head) {
    printf("\n=== COURSE AVERAGE REPORT ===\n");
    printf("Enter Course Code: ");
    char course_code[20];
    scanf("%s", course_code);
    
    Course* course = find_course_by_code(courses_head, course_code);
    if (course == NULL) {
        printf("Course with code %s not found.\n", course_code);
        return;
    }
    
    float average = calculate_course_average(grades_head, course_code);
    
    Grade* current = grades_head;
    int student_count = 0;
    while (current != NULL) {
        if (strcmp(current->course_code, course_code) == 0) {
            student_count++;
        }
        current = current->next;
    }
    
    printf("\n=== COURSE AVERAGE REPORT ===\n");
    printf("Course: %s (%s)\n", course->course_name, course_code);
    printf("Instructor: %s\n", course->instructor);
    printf("Credits: %d\n", course->credits);
    printf("Type: %s\n", course->course_type);
    printf("Number of Students: %d\n", student_count);
    printf("Course Average: %.2f\n", average);
    
    if (student_count == 0) {
        printf("No students have taken this course yet.\n");
    }
}
void report_students_by_gpa(Student* students_head, Grade* grades_head) {
    printf("\n=== STUDENTS SORTED BY GPA ===\n");
    
    typedef struct {
        char student_id[20];
        char student_name[100];
        float gpa;
    } StudentGPA;
    
    StudentGPA student_gpas[1000];
    int count = 0;
    
    Student* current = students_head;
    while (current != NULL && count < 1000) {
        float gpa = calculate_gpa(grades_head, current->student_id);
        if (gpa > 0) {
            strcpy(student_gpas[count].student_id, current->student_id);
            sprintf(student_gpas[count].student_name, "%s %s", current->first_name, current->last_name);
            student_gpas[count].gpa = gpa;
            count++;
        }
        current = current->next;
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (student_gpas[j].gpa < student_gpas[j+1].gpa) {
                StudentGPA temp = student_gpas[j];
                student_gpas[j] = student_gpas[j+1];
                student_gpas[j+1] = temp;
            }
        }
    }
    
    printf("%-10s %-25s %-8s %-15s\n", "Student ID", "Student Name", "GPA", "Performance");
    printf("================================================================================\n");
    
    for (int i = 0; i < count; i++) {
        char performance[20];
        if (student_gpas[i].gpa >= 90.0) strcpy(performance, "EXCELLENT");
        else if (student_gpas[i].gpa >= 80.0) strcpy(performance, "GOOD");
        else if (student_gpas[i].gpa >= 60.0) strcpy(performance, "SATISFACTORY");
        else strcpy(performance, "PROBATIONARY");
        
        printf("%-10s %-25s %-8.2f %-15s\n",
               student_gpas[i].student_id, student_gpas[i].student_name,
               student_gpas[i].gpa, performance);
    }
    
    printf("================================================================================\n");
    printf("Total Students with Grades: %d\n", count);
}

void report_students_passed_course(Student* students_head, Course* courses_head, Grade* grades_head) {
    printf("\n=== STUDENTS WHO PASSED A COURSE ===\n");
    printf("Enter Course Code: ");
    char course_code[20];
    scanf("%s", course_code);
    
    Course* course = find_course_by_code(courses_head, course_code);
    if (course == NULL) {
        printf("Course with code %s not found.\n", course_code);
        return;
    }
    
    printf("\n=== STUDENTS WHO PASSED %s (%s) ===\n", course->course_name, course_code);
    printf("%-10s %-25s %-8s %-8s\n", "Student ID", "Student Name", "Grade", "Letter");
    printf("================================================================================\n");
    
    Grade* current = grades_head;
    int passed_count = 0;
    int total_count = 0;
    
    while (current != NULL) {
        if (strcmp(current->course_code, course_code) == 0) {
            total_count++;
            if (current->numeric_grade >= 60) {
                Student* student = find_student_by_id(students_head, current->student_id);
                char student_name[100] = "Unknown Student";
                if (student != NULL) {
                    sprintf(student_name, "%s %s", student->first_name, student->last_name);
                }
                
                printf("%-10s %-25s %-8d %-8s\n",
                       current->student_id, student_name, current->numeric_grade,
                       current->letter_grade);
                passed_count++;
            }
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    printf("Students Passed: %d\n", passed_count);
    printf("Total Students: %d\n", total_count);
    if (total_count > 0) {
        printf("Pass Rate: %.1f%%\n", (float)passed_count / total_count * 100);
    }
}

void report_students_failed_course(Student* students_head, Course* courses_head, Grade* grades_head) {
    printf("\n=== STUDENTS WHO FAILED A COURSE ===\n");
    printf("Enter Course Code: ");
    char course_code[20];
    scanf("%s", course_code);
    
    Course* course = find_course_by_code(courses_head, course_code);
    if (course == NULL) {
        printf("Course with code %s not found.\n", course_code);
        return;
    }
    
    printf("\n=== STUDENTS WHO FAILED %s (%s) ===\n", course->course_name, course_code);
    printf("%-10s %-25s %-8s %-8s\n", "Student ID", "Student Name", "Grade", "Letter");
    printf("================================================================================\n");
    
    Grade* current = grades_head;
    int failed_count = 0;
    int total_count = 0;
    
    while (current != NULL) {
        if (strcmp(current->course_code, course_code) == 0) {
            total_count++;
            if (current->numeric_grade < 60) {
                Student* student = find_student_by_id(students_head, current->student_id);
                char student_name[100] = "Unknown Student";
                if (student != NULL) {
                    sprintf(student_name, "%s %s", student->first_name, student->last_name);
                }
                
                printf("%-10s %-25s %-8d %-8s\n",
                       current->student_id, student_name, current->numeric_grade,
                       current->letter_grade);
                failed_count++;
            }
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    printf("Students Failed: %d\n", failed_count);
    printf("Total Students: %d\n", total_count);
    if (total_count > 0) {
        printf("Failure Rate: %.1f%%\n", (float)failed_count / total_count * 100);
    }
}

void report_probationary_students(Student* students_head, Grade* grades_head) {
    printf("\n=== PROBATIONARY STUDENTS REPORT ===\n");
    printf("%-10s %-25s %-8s %-10s\n", "Student ID", "Student Name", "GPA", "Status");
    printf("================================================================================\n");
    
    Student* current = students_head;
    int probation_count = 0;
    
    while (current != NULL) {
        float gpa = calculate_gpa(grades_head, current->student_id);
        
        if (gpa < 60.0 || strcmp(current->status, "PROBATION") == 0) {
            char student_name[100];
            sprintf(student_name, "%s %s", current->first_name, current->last_name);
            
            printf("%-10s %-25s %-8.2f %-10s\n",
                   current->student_id, student_name, gpa, current->status);
            probation_count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (probation_count == 0) {
        printf("No probationary students found.\n");
    } else {
        printf("Total Probationary Students: %d\n", probation_count);
    }
}

void report_probationary_students_course(Student* students_head, Course* courses_head, Grade* grades_head) {
    printf("\n=== PROBATIONARY STUDENTS IN A SPECIFIC COURSE ===\n");
    printf("Enter Course Code: ");
    char course_code[20];
    scanf("%s", course_code);
    
    Course* course = find_course_by_code(courses_head, course_code);
    if (course == NULL) {
        printf("Course with code %s not found.\n", course_code);
        return;
    }
    
    printf("\n=== PROBATIONARY STUDENTS IN %s (%s) ===\n", course->course_name, course_code);
    printf("%-10s %-25s %-8s %-8s %-8s\n", "Student ID", "Student Name", "GPA", "Grade", "Status");
    printf("================================================================================\n");
    
    Grade* current = grades_head;
    int count = 0;
    
    while (current != NULL) {
        if (strcmp(current->course_code, course_code) == 0) {
            Student* student = find_student_by_id(students_head, current->student_id);
            if (student != NULL) {
                float gpa = calculate_gpa(grades_head, current->student_id);
                
                if (gpa < 60.0 || strcmp(student->status, "PROBATION") == 0) {
                    char student_name[100];
                    sprintf(student_name, "%s %s", student->first_name, student->last_name);
                    
                    printf("%-10s %-25s %-8.2f %-8d %-8s\n",
                           current->student_id, student_name, gpa,
                           current->numeric_grade, student->status);
                    count++;
                }
            }
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No probationary students found in this course.\n");
    } else {
        printf("Total Probationary Students in Course: %d\n", count);
    }
}


void staff_report_find_student(Student* students_head) {
    printf("\n=== FIND STUDENT DETAILS ===\n");
    printf("Search by:\n");
    printf("1. Student ID\n");
    printf("2. First Name\n");
    printf("3. Last Name\n");
    printf("4. National ID\n");
    printf("Enter your choice: ");
    
    int search_choice;
    scanf("%d", &search_choice);
    
    char search_term[100];
    Student* current = students_head;
    int found = 0;
    
    switch (search_choice) {
        case 1:
            printf("Enter Student ID: ");
            scanf("%s", search_term);
            for (int i = 0; i < strlen(search_term); i++) {
                search_term[i] = toupper(search_term[i]);
            }
            while (current != NULL) {
                if (strcmp(current->student_id, search_term) == 0) {
                    printf("\n=== STUDENT FOUND ===\n");
                    printf("Student ID: %s\n", current->student_id);
                    printf("Name: %s %s\n", current->first_name, current->last_name);
                    printf("National ID: %s\n", current->national_id);
                    printf("Date of Birth: %s\n", current->date_of_birth);
                    printf("Place of Birth: %s\n", current->place_of_birth);
                    printf("Field of Study: %s\n", current->field_of_study);
                    printf("Mobile Phone: %s\n", current->phone);
                    printf("Email: %s\n", current->email);
                    printf("Enrollment Date: %s\n", current->enrollment_date);
                    printf("Status: %s\n", current->status);
                    found = 1;
                    break;
                }
                current = current->next;
            }
            break;
        case 2:
            printf("Enter First Name: ");
            scanf("%s", search_term);
            printf("\n=== SEARCH RESULTS ===\n");
            while (current != NULL) {
                if (strcasecmp(current->first_name, search_term) == 0) {
                    printf("ID: %s | Name: %s %s | Field: %s | Status: %s\n", 
                           current->student_id, current->first_name, current->last_name, 
                           current->field_of_study, current->status);
                    found = 1;
                }
                current = current->next;
            }
            break;
        case 3:
            printf("Enter Last Name: ");
            scanf("%s", search_term);
            printf("\n=== SEARCH RESULTS ===\n");
            while (current != NULL) {
                if (strcasecmp(current->last_name, search_term) == 0) {
                    printf("ID: %s | Name: %s %s | Field: %s | Status: %s\n", 
                           current->student_id, current->first_name, current->last_name,
                           current->field_of_study, current->status);
                    found = 1;
                }
                current = current->next;
            }
            break;
        case 4:
            printf("Enter National ID: ");
            scanf("%s", search_term);
            while (current != NULL) {
                if (strcmp(current->national_id, search_term) == 0) {
                    printf("\n=== STUDENT FOUND ===\n");
                    printf("Student ID: %s\n", current->student_id);
                    printf("Name: %s %s\n", current->first_name, current->last_name);
                    printf("National ID: %s\n", current->national_id);
                    printf("Date of Birth: %s\n", current->date_of_birth);
                    printf("Place of Birth: %s\n", current->place_of_birth);
                    printf("Field of Study: %s\n", current->field_of_study);
                    printf("Mobile Phone: %s\n", current->phone);
                    printf("Email: %s\n", current->email);
                    printf("Enrollment Date: %s\n", current->enrollment_date);
                    printf("Status: %s\n", current->status);
                    found = 1;
                    break;
                }
                current = current->next;
            }
            break;
        default:
            printf("Invalid choice!\n");
            return;
    }
    
    if (!found) {
        printf("No student found with the given criteria.\n");
    }
}

void staff_report_students_by_birth_date_range(Student* students_head) {
    printf("\n=== STUDENTS BORN IN DATE RANGE ===\n");
    char start_date[15], end_date[15];
    
    printf("Enter start date (YYYY-MM-DD): ");
    scanf("%s", start_date);
    printf("Enter end date (YYYY-MM-DD): ");
    scanf("%s", end_date);
    
    printf("\n=== STUDENTS BORN BETWEEN %s AND %s ===\n", start_date, end_date);
    printf("%-10s %-25s %-12s %-20s %-15s %-10s\n", 
           "ID", "Name", "Birth Date", "Place of Birth", "Field of Study", "Status");
    printf("================================================================================\n");
    
    Student* current = students_head;
    int count = 0;
    
    while (current != NULL) {
        if (strcmp(current->date_of_birth, start_date) >= 0 && 
            strcmp(current->date_of_birth, end_date) <= 0 &&
            strcmp(current->date_of_birth, "N/A") != 0) {
            
            char full_name[100];
            sprintf(full_name, "%s %s", current->first_name, current->last_name);
            
            printf("%-10s %-25s %-12s %-20s %-15s %-10s\n",
                   current->student_id, full_name, current->date_of_birth,
                   current->place_of_birth, current->field_of_study, current->status);
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No students found born in the specified date range.\n");
    } else {
        printf("Total Students Found: %d\n", count);
    }
}

void staff_report_students_by_field_of_study(Student* students_head) {
    printf("\n=== STUDENTS BY FIELD OF STUDY ===\n");
    char field_search[100];
    
    printf("Enter field of study (or part of it): ");
    getchar();
    fgets(field_search, sizeof(field_search), stdin);
    field_search[strcspn(field_search, "\n")] = 0;
    
    printf("\n=== STUDENTS IN FIELD: %s ===\n", field_search);
    printf("%-10s %-25s %-20s %-12s %-10s\n", 
           "ID", "Name", "Field of Study", "Birth Date", "Status");
    printf("================================================================================\n");
    
    Student* current = students_head;
    int count = 0;
    
    while (current != NULL) {
        if (strstr(current->field_of_study, field_search) != NULL ||
            strcasestr_custom(current->field_of_study, field_search) != NULL) {
            
            char full_name[100];
            sprintf(full_name, "%s %s", current->first_name, current->last_name);
            
            printf("%-10s %-25s %-20s %-12s %-10s\n",
                   current->student_id, full_name, current->field_of_study,
                   current->date_of_birth, current->status);
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No students found in the specified field of study.\n");
    } else {
        printf("Total Students Found: %d\n", count);
    }
}

void staff_report_students_by_birth_city(Student* students_head) {
    printf("\n=== STUDENTS BORN IN SPECIFIC CITY ===\n");
    char city_search[100];
    
    printf("Enter city name (or part of it): ");
    getchar();
    fgets(city_search, sizeof(city_search), stdin);
    city_search[strcspn(city_search, "\n")] = 0;
    
    printf("\n=== STUDENTS BORN IN: %s ===\n", city_search);
    printf("%-10s %-25s %-20s %-12s %-15s %-10s\n", 
           "ID", "Name", "Place of Birth", "Birth Date", "Field of Study", "Status");
    printf("================================================================================\n");
    
    Student* current = students_head;
    int count = 0;
    
    while (current != NULL) {
        if (strstr(current->place_of_birth, city_search) != NULL ||
            strcasestr_custom(current->place_of_birth, city_search) != NULL) {
            
            char full_name[100];
            sprintf(full_name, "%s %s", current->first_name, current->last_name);
            
            printf("%-10s %-25s %-20s %-12s %-15s %-10s\n",
                   current->student_id, full_name, current->place_of_birth,
                   current->date_of_birth, current->field_of_study, current->status);
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No students found born in the specified city.\n");
    } else {
        printf("Total Students Found: %d\n", count);
    }
}

void staff_report_all_students(Student* students_head) {
    printf("\n=== ALL STUDENTS LIST ===\n");
    printf("%-8s %-12s %-12s %-12s %-12s %-20s %-15s %-12s %-20s %-12s %-8s\n", 
           "ID", "First", "Last", "National ID", "Birth Date", "Place of Birth", "Field of Study", "Phone", "Email", "Enrolled", "Status");
    printf("===========================================================================================================================================================================\n");
    
    Student* current = students_head;
    int count = 0;
    
    while (current != NULL) {
        char short_place[21], short_field[16], short_email[21];
        strncpy(short_place, current->place_of_birth, 20);
        short_place[20] = '\0';
        strncpy(short_field, current->field_of_study, 15);
        short_field[15] = '\0';
        strncpy(short_email, current->email, 20);
        short_email[20] = '\0';
        
        printf("%-8s %-12s %-12s %-12s %-12s %-20s %-15s %-12s %-20s %-12s %-8s\n",
               current->student_id, current->first_name, current->last_name,
               current->national_id, current->date_of_birth, short_place,
               short_field, current->phone, short_email, 
               current->enrollment_date, current->status);
        count++;
        current = current->next;
    }
    
    printf("===========================================================================================================================================================================\n");
    printf("Total Students: %d\n", count);
}

void staff_report_student_grades(Student* students_head, Course* courses_head, Grade* grades_head) {
    printf("\n=== STUDENT GRADE REPORT ===\n");
    printf("Enter Student ID: ");
    char student_id[20];
    scanf("%s", student_id);
    
    for (int i = 0; i < strlen(student_id); i++) {
        student_id[i] = toupper(student_id[i]);
    }
    
    Student* student = find_student_by_id(students_head, student_id);
    if (student == NULL) {
        printf("Student with ID %s not found.\n", student_id);
        return;
    }
    
    printf("\n=== GRADES FOR %s %s (ID: %s) ===\n", 
           student->first_name, student->last_name, student_id);
    printf("%-10s %-30s %-8s %-8s %-20s\n", 
           "Course", "Course Name", "Grade", "Letter", "Date/Time");
    printf("================================================================================\n");
    
    Grade* current = grades_head;
    int count = 0;
    float total_points = 0;
    
    while (current != NULL) {
        if (strcmp(current->student_id, student_id) == 0) {
            Course* course = find_course_by_code(courses_head, current->course_code);
            char course_name[100] = "Unknown Course";
            if (course != NULL) {
                strcpy(course_name, course->course_name);
            }
            
            printf("%-10s %-30s %-8d %-8s %-20s\n",
                   current->course_code, course_name, current->numeric_grade,
                   current->letter_grade, current->date_time);
            
            total_points += current->numeric_grade;
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No grades found for this student.\n");
    } else {
        printf("Total Courses: %d\n", count);
        printf("Average Grade (GPA): %.2f\n", total_points / count);
    }
}

void staff_report_failing_students(Student* students_head, Grade* grades_head) {
    printf("\n=== STUDENTS WITH FAILING GRADES ===\n");
    printf("%-10s %-25s %-10s %-8s %-8s %-20s\n", 
           "Student ID", "Name", "Course", "Grade", "Letter", "Date");
    printf("================================================================================\n");
    
    Grade* current = grades_head;
    int count = 0;
    
    while (current != NULL) {
        if (current->numeric_grade < 60) {
            Student* student = find_student_by_id(students_head, current->student_id);
            char student_name[100] = "Unknown Student";
            if (student != NULL) {
                sprintf(student_name, "%s %s", student->first_name, student->last_name);
            }
            
            printf("%-10s %-25s %-10s %-8d %-8s %-20s\n",
                   current->student_id, student_name, current->course_code,
                   current->numeric_grade, current->letter_grade, current->date_time);
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No failing grades found.\n");
    } else {
        printf("Total Failing Grades: %d\n", count);
    }
}

void staff_report_students_by_status(Student* students_head) {
    printf("\n=== STUDENTS BY STATUS ===\n");
    printf("1. ACTIVE\n");
    printf("2. INACTIVE\n");
    printf("3. PROBATION\n");
    printf("Enter choice: ");
    
    int status_choice;
    scanf("%d", &status_choice);
    
    char target_status[20];
    switch(status_choice) {
        case 1: strcpy(target_status, "ACTIVE"); break;
        case 2: strcpy(target_status, "INACTIVE"); break;
        case 3: strcpy(target_status, "PROBATION"); break;
        default: 
            printf("Invalid choice!\n");
            return;
    }
    
    printf("\n=== %s STUDENTS ===\n", target_status);
    printf("%-10s %-25s %-15s %-12s %-20s\n", 
           "ID", "Name", "Field of Study", "Enrolled", "Phone");
    printf("================================================================================\n");
    
    Student* current = students_head;
    int count = 0;
    
    while (current != NULL) {
        if (strcmp(current->status, target_status) == 0) {
            char full_name[100];
            sprintf(full_name, "%s %s", current->first_name, current->last_name);
            
            printf("%-10s %-25s %-15s %-12s %-20s\n",
                   current->student_id, full_name, current->field_of_study,
                   current->enrollment_date, current->phone);
            count++;
        }
        current = current->next;
    }
    
    printf("================================================================================\n");
    if (count == 0) {
        printf("No %s students found.\n", target_status);
    } else {
        printf("Total %s Students: %d\n", target_status, count);
    }
}

