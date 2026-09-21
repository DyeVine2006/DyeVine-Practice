/*
 * Student Grade Management System
 * ---------------------------------------------------------------------------
 * A small console program that keeps student records in a singly linked list,
 * loads them from a plain text file at start-up and writes them back after
 * every operation, so the data survives between runs.
 *
 * Features
 *   - add a student (inserted in alphabetical order by name)
 *   - search a student by exact name
 *   - modify the name, gender or any of the three scores (sum / average updated)
 *   - delete a student (with confirmation)
 *   - list every record
 *
 * Build
 *   gcc -std=c11 -Wall -Wextra -o stu_manage_sys Stu_manage_sys.c
 *   (GCC / Clang / MinGW-w64)
 *
 * Run
 *   stu_manage_sys                -> reads and writes Grade.txt in the current
 *                                    directory (created when it is missing)
 *   stu_manage_sys my_data.txt    -> uses the given file
 *   stu_manage_sys --help
 *
 * Data file format (one record per line, comma separated)
 *   name,gender,score_english,score_math,score_c,sum,average
 *
 * Author: DyeVine
 * License: MIT
 */

#define _CRT_SECURE_NO_WARNINGS

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define NAME_MAX_LEN 20
#define LINE_MAX_LEN 256
#define DATA_FILE_DEFAULT "Grade.txt"

/* Return codes for load_records(). */
enum { LOAD_OK = 0, LOAD_OUT_OF_MEMORY = -1 };

typedef struct student {
    char name[NAME_MAX_LEN];
    char gender[3];
    float score_english;
    float score_math;
    float score_c;
    float sum;
    float average;
    struct student *next;
} STU;

/* ---- file I/O ---------------------------------------------------------- */

static int read_line(char *buf, int size);
static char *trim(char *s);
static char *strip_bom(char *s);
static STU *load_records(const char *path, int *loaded, int *file_missing);
static int save_records(const STU *head, const char *path);
static int ensure_parent_directory(const char *path);
static int make_directory(const char *dir);

/* ---- list operations --------------------------------------------------- */

static int insert_sorted(STU *head, STU *node);
static STU *find_student(STU *head, const char *name, STU **prev_out);
static void remove_node(STU *node, STU *prev);
static void free_list(STU *head);
static void print_header(void);
static void print_student(const STU *s);

/* ---- menu operations --------------------------------------------------- */

static void do_add(STU *head);
static void do_search(STU *head);
static void do_modify(STU *head);
static void do_delete(STU *head);
static void do_display(STU *head);

/* ---- input helpers ----------------------------------------------------- */

static int read_choice(char *buf, int size);
static int read_name(const char *prompt, char *out, int size);
static int read_float(const char *prompt, float lo, float hi, float *out);
static int read_gender(const char *prompt, char *out, int size);
static void print_menu(void);

/* ======================================================================= */

int main(int argc, char *argv[])
{
    const char *path = DATA_FILE_DEFAULT;
    STU *head = NULL;
    int loaded = 0;
    int file_missing = 0;
    int running = 1;
    char line[LINE_MAX_LEN];

    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Usage: %s [data-file]\n", argv[0]);
            printf("  data-file   records to load and save (default: %s)\n",
                   DATA_FILE_DEFAULT);
            printf("\nExample data line:\n");
            printf("  Amy,F,100,97,92,289,96.333336\n");
            return 0;
        }
        path = argv[1];
    }

    head = load_records(path, &loaded, &file_missing);
    if (head == NULL) {
        fprintf(stderr, "Error: out of memory.\n");
        return 1;
    }

    printf("Welcome to the Student Grade Management System.\n");
    printf("Data file : %s\n", path);
    if (file_missing) {
        printf("Note      : the file did not exist, an empty list was created.\n");
    } else {
        printf("Loaded    : %d record(s)\n", loaded);
    }

    while (running) {
        int choice;

        print_menu();
        choice = read_choice(line, (int)sizeof(line));

        if (choice == EOF) {
            printf("\nEnd of input reached, saving and leaving.\n");
            break;
        }

        switch (choice) {
        case '1': do_add(head);     break;
        case '2': do_search(head);  break;
        case '3': do_modify(head);  break;
        case '4': do_delete(head);  break;
        case '5': do_display(head); break;
        case 'q':
        case 'Q': running = 0;      break;
        default:
            printf("Please enter 1 to 5, or q to quit.\n");
            break;
        }

        if (running && save_records(head, path) != 0) {
            fprintf(stderr, "Warning: could not write %s, changes are only in memory.\n",
                    path);
        }
    }

    free_list(head);
    printf("All changes were saved to %s\n", path);
    printf("Looking forward to your next visit!\n");
    return 0;
}

/* ======================================================================= */
/* file I/O                                                                */
/* ======================================================================= */

/*
 * Load every record from `path`.
 * Returns a sentinel head on success - including when the file is empty or
 * does not exist yet - or NULL when memory runs out.
 * `loaded` counts the records read; `file_missing` reports an absent file.
 */
static STU *load_records(const char *path, int *loaded, int *file_missing)
{
    FILE *fp;
    STU *head;
    char line[LINE_MAX_LEN];

    *loaded = 0;
    *file_missing = 0;

    head = (STU *)malloc(sizeof(STU));
    if (head == NULL) {
        return NULL;
    }
    head->name[0] = '\0';
    head->gender[0] = '\0';
    head->next = NULL;

    fp = fopen(path, "r");
    if (fp == NULL) {
        *file_missing = 1;
        printf("Data file \"%s\" not found, starting with an empty list.\n", path);
        return head;
    }

    while (fgets(line, (int)sizeof(line), fp) != NULL) {
        STU *node;
        char name[NAME_MAX_LEN];
        char gender[3];
        float e, m, c, s, a;
        int fields;
        char *p = trim(line);

        if (*p == '\0') {
            continue; /* skip blank lines */
        }

        fields = sscanf(p, "%19[^,],%2[^,],%f,%f,%f,%f,%f",
                        name, gender, &e, &m, &c, &s, &a);
        if (fields != 7) {
            fprintf(stderr, "Warning: skipping malformed line: %s\n", p);
            continue;
        }

        node = (STU *)malloc(sizeof(STU));
        if (node == NULL) {
            fclose(fp);
            free_list(head);
            return NULL;
        }

        strncpy(node->name, name, NAME_MAX_LEN - 1);
        node->name[NAME_MAX_LEN - 1] = '\0';
        strncpy(node->gender, gender, 2);
        node->gender[2] = '\0';
        node->score_english = e;
        node->score_math = m;
        node->score_c = c;
        /* Recompute instead of trusting the values stored in the file. */
        node->sum = e + m + c;
        node->average = node->sum / 3.0f;
        node->next = NULL;

        if (insert_sorted(head, node)) {
            (*loaded)++;
        } else {
            free(node);
        }
    }

    fclose(fp);
    return head;
}

/* Write the whole list back, creating the parent directory if needed. */
static int save_records(const STU *head, const char *path)
{
    const STU *p;
    FILE *fp;

    if (ensure_parent_directory(path) != 0) {
        return -1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        return -1;
    }

    for (p = head->next; p != NULL; p = p->next) {
        if (fprintf(fp, "%s,%s,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                    p->name, p->gender,
                    (double)p->score_english, (double)p->score_math,
                    (double)p->score_c, (double)p->sum, (double)p->average) < 0) {
            fclose(fp);
            return -1;
        }
    }

    return (fclose(fp) == 0) ? 0 : -1;
}

/*
 * Create the parent directory of `path`, including intermediate levels,
 * when it does not exist yet.
 * Returns 0 when the directory exists afterwards (or is not needed).
 */
static int ensure_parent_directory(const char *path)
{
    char dir[LINE_MAX_LEN];
    char *sep;
    size_t len;

    if (path == NULL) {
        return 0;
    }

    len = strlen(path);
    if (len == 0 || len >= sizeof(dir)) {
        return 0; /* let fopen() report the problem */
    }
    memcpy(dir, path, len + 1);

    sep = strrchr(dir, '/');
#ifdef _WIN32
    {
        char *back = strrchr(dir, '\\');
        if (back != NULL && (sep == NULL || back > sep)) {
            sep = back;
        }
    }
#endif
    if (sep == NULL || sep == dir) {
        return 0; /* plain file name or root directory */
    }
    *sep = '\0';

    /* Walk the path and create every missing component in turn. */
    for (char *p = dir + 1; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            char saved = *p;
            *p = '\0';
            if (strlen(dir) > 0 && dir[strlen(dir) - 1] == ':') {
                /* Drive letter such as "C:" - not a directory of its own. */
                *p = saved;
                continue;
            }
            if (make_directory(dir) != 0) {
                *p = saved;
                return -1;
            }
            *p = saved;
        }
    }

    return make_directory(dir);
}

/* Create one directory; an already existing directory counts as success. */
static int make_directory(const char *dir)
{
#ifdef _WIN32
    if (_mkdir(dir) == 0) {
        return 0;
    }
    return (errno == EEXIST) ? 0 : -1;
#else
    if (mkdir(dir, 0755) == 0) {
        return 0;
    }
    if (errno == EEXIST) {
        return 0;
    }
    {
        struct stat st;
        if (stat(dir, &st) == 0 && S_ISDIR(st.st_mode)) {
            return 0;
        }
    }
    return -1;
#endif
}

/* ======================================================================= */
/* list operations                                                         */
/* ======================================================================= */

/*
 * Insert `node` so that names stay in ascending order.
 * Returns 1 on success, 0 when the name is already present.
 */
static int insert_sorted(STU *head, STU *node)
{
    STU **link = &head->next;

    while (*link != NULL) {
        int cmp = strcmp((*link)->name, node->name);
        if (cmp == 0) {
            printf("A student named \"%s\" already exists.\n", node->name);
            return 0;
        }
        if (cmp > 0) {
            break;
        }
        link = &(*link)->next;
    }

    node->next = *link;
    *link = node;
    return 1;
}

/* Find a node by exact name; `prev_out` optionally receives its predecessor. */
static STU *find_student(STU *head, const char *name, STU **prev_out)
{
    STU *prev = head;
    STU *p = head->next;

    while (p != NULL) {
        if (strcmp(p->name, name) == 0) {
            if (prev_out != NULL) {
                *prev_out = prev;
            }
            return p;
        }
        prev = p;
        p = p->next;
    }

    if (prev_out != NULL) {
        *prev_out = NULL;
    }
    return NULL;
}

/* Unlink `node` (whose predecessor is `prev`) from the list and free it. */
static void remove_node(STU *node, STU *prev)
{
    if (node == NULL || prev == NULL) {
        return;
    }
    prev->next = node->next;
    free(node);
}

static void free_list(STU *head)
{
    STU *p = head;

    while (p != NULL) {
        STU *next = p->next;
        free(p);
        p = next;
    }
}

/* ======================================================================= */
/* printing                                                                */
/* ======================================================================= */

static void print_header(void)
{
    printf("%-16s %-6s %9s %9s %9s %9s %9s\n",
           "Name", "Gender", "English", "Math", "C", "Sum", "Average");
    printf("-------------------------------------------------------------------------------\n");
}

static void print_student(const STU *s)
{
    printf("%-16s %-6s %9.2f %9.2f %9.2f %9.2f %9.2f\n",
           s->name, s->gender,
           (double)s->score_english, (double)s->score_math,
           (double)s->score_c, (double)s->sum, (double)s->average);
}

/* ======================================================================= */
/* menu operations                                                         */
/* ======================================================================= */

static void do_add(STU *head)
{
    char name[NAME_MAX_LEN];
    char gender[3];
    float e, m, c;
    STU *node;

    if (!read_name("Name: ", name, (int)sizeof(name))) {
        return;
    }
    if (!read_gender("Gender (M/F): ", gender, (int)sizeof(gender))) {
        return;
    }
    if (!read_float("Grade of English: ", 0.0f, 100.0f, &e) ||
        !read_float("Grade of Math   : ", 0.0f, 100.0f, &m) ||
        !read_float("Grade of C      : ", 0.0f, 100.0f, &c)) {
        return;
    }

    node = (STU *)malloc(sizeof(STU));
    if (node == NULL) {
        printf("Error: out of memory.\n");
        return;
    }

    strncpy(node->name, name, NAME_MAX_LEN - 1);
    node->name[NAME_MAX_LEN - 1] = '\0';
    strncpy(node->gender, gender, 2);
    node->gender[2] = '\0';
    node->score_english = e;
    node->score_math = m;
    node->score_c = c;
    node->sum = e + m + c;
    node->average = node->sum / 3.0f;
    node->next = NULL;

    if (insert_sorted(head, node)) {
        printf("\nRecord added:\n");
    } else {
        /* insert_sorted() already explained the reason (duplicate name). */
        printf("The record was not added.\n");
        free(node);
        return;
    }
    print_header();
    print_student(node);
}

static void do_search(STU *head)
{
    char name[NAME_MAX_LEN];
    STU *found;

    if (!read_name("Name to search: ", name, (int)sizeof(name))) {
        return;
    }

    found = find_student(head, name, NULL);
    if (found == NULL) {
        printf("Not found: \"%s\"\n", name);
        return;
    }

    print_header();
    print_student(found);
}

static void do_modify(STU *head)
{
    char name[NAME_MAX_LEN];
    char buf[LINE_MAX_LEN];
    STU *prev = NULL;
    STU *p;

    if (!read_name("Name to modify: ", name, (int)sizeof(name))) {
        return;
    }

    p = find_student(head, name, &prev);
    if (p == NULL) {
        printf("Not found: \"%s\"\n", name);
        return;
    }

    printf("Current record:\n");
    print_header();
    print_student(p);

    for (;;) {
        int choice;
        float value;

        printf("\n1: Change the Name\n"
               "2: Change the Gender\n"
               "3: Change the Grade of English\n"
               "4: Change the Grade of Math\n"
               "5: Change the Grade of C Language\n"
               "q: Back to the main menu\n"
               "Enter 1 to 5, or q: ");

        choice = read_choice(buf, (int)sizeof(buf));
        if (choice == EOF) {
            return;
        }

        switch (choice) {
        case 'q':
        case 'Q':
            printf("Record updated.\n");
            return;

        case '1': {
            char new_name[NAME_MAX_LEN];

            if (!read_name("New name: ", new_name, (int)sizeof(new_name))) {
                return;
            }
            if (strcmp(new_name, p->name) == 0) {
                printf("The name is unchanged.\n");
                return;
            }
            if (find_student(head, new_name, NULL) != NULL) {
                printf("A student named \"%s\" already exists.\n", new_name);
                return;
            }

            /* Rename first, then re-insert the same node at its new place. */
            prev->next = p->next;
            p->next = NULL;
            strncpy(p->name, new_name, NAME_MAX_LEN - 1);
            p->name[NAME_MAX_LEN - 1] = '\0';
            if (!insert_sorted(head, p)) {
                printf("Error: could not re-insert the record.\n");
            } else {
                printf("Name changed to \"%s\"; the list was re-sorted.\n", p->name);
            }
            printf("Search for the record again if you want to keep editing.\n");
            return;
        }

        case '2':
            if (!read_gender("New gender (M/F): ", buf, (int)sizeof(buf))) {
                return;
            }
            p->gender[0] = buf[0];
            p->gender[1] = '\0';
            printf("Gender updated.\n");
            break;

        case '3':
        case '4':
        case '5':
            if (!read_float("New score: ", 0.0f, 100.0f, &value)) {
                return;
            }
            if (choice == '3') {
                p->score_english = value;
            } else if (choice == '4') {
                p->score_math = value;
            } else {
                p->score_c = value;
            }
            p->sum = p->score_english + p->score_math + p->score_c;
            p->average = p->sum / 3.0f;
            printf("Score updated:\n");
            print_header();
            print_student(p);
            break;

        default:
            printf("Please enter 1 to 5, or q.\n");
            break;
        }
    }
}

static void do_delete(STU *head)
{
    char name[NAME_MAX_LEN];
    char buf[LINE_MAX_LEN];
    STU *prev = NULL;
    STU *p;

    if (!read_name("Name to delete: ", name, (int)sizeof(name))) {
        return;
    }

    p = find_student(head, name, &prev);
    if (p == NULL) {
        printf("Not found: \"%s\"\n", name);
        return;
    }

    print_header();
    print_student(p);

    printf("Delete this record? (y/N): ");
    if (read_choice(buf, (int)sizeof(buf)) == EOF) {
        return;
    }
    if (buf[0] != 'y' && buf[0] != 'Y') {
        printf("Cancelled, nothing was deleted.\n");
        return;
    }

    remove_node(p, prev);
    printf("Deleted \"%s\".\n", name);
}

static void do_display(STU *head)
{
    const STU *p;
    int count = 0;

    print_header();
    for (p = head->next; p != NULL; p = p->next) {
        print_student(p);
        count++;
    }
    printf("-------------------------------------------------------------------------------\n");
    printf("%d record(s) in total.\n", count);
}

/* ======================================================================= */
/* input helpers                                                           */
/* ======================================================================= */

/*
 * Read one whole line and return its first non-blank character, '\n' for an
 * empty line, or EOF. Reading whole lines avoids the leftover-character bugs
 * that appear when scanf() and getchar() are mixed.
 */
static int read_choice(char *buf, int size)
{
    char *p;

    if (fgets(buf, size, stdin) == NULL) {
        return EOF;
    }

    p = trim(buf);
    if (*p == '\0') {
        buf[0] = '\0';
        return '\n';
    }

    memmove(buf, p, strlen(p) + 1);
    return (unsigned char)buf[0];
}

/* Read one trimmed line. Returns 0 on EOF. */
static int read_line(char *buf, int size)
{
    if (fgets(buf, size, stdin) == NULL) {
        return 0;
    }
    trim(buf);
    return 1;
}

static int read_name(const char *prompt, char *out, int size)
{
    char buf[LINE_MAX_LEN];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!read_line(buf, (int)sizeof(buf))) {
            return 0; /* EOF */
        }
        if (buf[0] == '\0') {
            printf("The name cannot be empty.\n");
            continue;
        }
        if (strlen(buf) > (size_t)(size - 1)) {
            printf("The name is too long (at most %d characters).\n", size - 1);
            continue;
        }
        strncpy(out, buf, (size_t)(size - 1));
        out[size - 1] = '\0';
        return 1;
    }
}

/* Accept M/F (any case) and store the uppercase letter in `out`. */
static int read_gender(const char *prompt, char *out, int size)
{
    char buf[LINE_MAX_LEN];

    if (size < 2) {
        return 0;
    }
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!read_line(buf, (int)sizeof(buf))) {
            return 0; /* EOF */
        }
        if (buf[0] != '\0') {
            char g = (char)toupper((unsigned char)buf[0]);
            if (g == 'M' || g == 'F') {
                out[0] = g;
                out[1] = '\0';
                return 1;
            }
        }
        printf("Please enter M or F.\n");
    }
}

static int read_float(const char *prompt, float lo, float hi, float *out)
{
    char buf[LINE_MAX_LEN];
    char extra;

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!read_line(buf, (int)sizeof(buf))) {
            return 0; /* EOF */
        }
        if (sscanf(buf, "%f %c", out, &extra) == 1) {
            if (*out >= lo && *out <= hi) {
                return 1;
            }
            printf("Please enter a number between %.0f and %.0f.\n",
                   (double)lo, (double)hi);
            continue;
        }
        printf("Invalid number, please try again.\n");
    }
}

static void print_menu(void)
{
    printf("\nEnter 1 to 5, or q to quit:\n"
           "1: Input a student's information\n"
           "2: Inquire a student's information\n"
           "3: Modify a student's information\n"
           "4: Delete a student's information\n"
           "5: Display all the students' information\n"
           "q: Quit (all changes are already saved)\n"
           "> ");
    fflush(stdout);
}

/* ======================================================================= */
/* small utilities                                                         */
/* ======================================================================= */

/* Strip leading and trailing whitespace in place; return the new start. */
static char *trim(char *s)
{
    char *end;

    s = strip_bom(s);
    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }
    if (*s == '\0') {
        return s;
    }

    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return s;
}

/*
 * Drop a UTF-8 byte order mark if the text starts with one.
 * Editors on Windows (Notepad, older Excel exports) like to add it, and it
 * would otherwise turn the first name or the first menu command into garbage.
 */
static char *strip_bom(char *s)
{
    static const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };

    if ((unsigned char)s[0] == bom[0] &&
        (unsigned char)s[1] == bom[1] &&
        (unsigned char)s[2] == bom[2]) {
        return s + 3;
    }
    return s;
}
