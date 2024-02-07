#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>

#define OPTIONAL_ARGUMENT_IS_PRESENT \
    ((optarg == NULL && optind < argc && **(argv + optind) != '-') \
     ? (bool)(optarg = argv[optind++]) \
     : (optarg != NULL))

#define ARGV_REMAIN (optind < argc)
#define CONFIG_PATH "config" 
#define MAX_FILENAME_LEN 128
#define MAX_INPUT_LEN 256

typedef struct { 
    char current_file[MAX_FILENAME_LEN];
} Config;

typedef struct {
   void *next;
   char *data;
   bool done_flag;
} Node;

Node *head = NULL;

struct stat st = {0};

typedef enum {
    RED,
    GREEN 
} Color;

int validate_input_str(char *input, int max_input_len) {
    if (strlen(input) > max_input_len) {
        printf("Error: input string exceeds character limit of %d characters.\n", max_input_len);
        return 0;
    }
    return 1;
}

void print_colored(const char *str, const Color color) {
    switch (color) {
        case RED:
            printf("\033[1m");
            printf("\033[31m");
            break;
        case GREEN:
            printf("\033[1m");
            printf("\033[32m");
            break;
        default:
            puts("Error. Invalid color value");
            return;
    }
    printf("%s", str);
    printf("\033[0m");
}

Node *add_node(char *data) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        perror("Error at malloc() at add_node()");
        return NULL; 
    }
    /*
    memset(node->data, 0, MAX_INPUT_LEN);
    strcpy(node->data, data);
    */
    node->data = data;
    node->next = NULL;

    if (head == NULL) {
        head = node; 
        return node;
    } 

    Node *current = head;
    while (current->next != NULL) {
        current = current->next; 
    }
    current->next = node; 
    return node;
}

Node *get_node(int index) {
    if (head == NULL) {
        printf("Error. Cannot get_node from an empty list.");
        return NULL;
    }
    Node *current = head;
    for(int i = 0; current; ++i) {
        if (i == index) { 
            return current;
        }
        current = current->next;
    }
    printf("Error. Index out of range");
    return NULL;
}

Node *remove_node(int index) {
    if (head == NULL) { 
        printf("Remove_node: cannot remove from an empty list.\n");
        return NULL;
    }
    Node **indirect = &head;

    for(int i = 0; *indirect; ++i) { 
        if (i == index) { 
            Node *tmp = *indirect;
            *indirect = (*indirect)->next;
            return tmp;
        }
    }

    printf("Remove_node: index out of range.\n");
    return NULL;
}

void free_list(Node *list) {
    while(list) {
        Node *tmp = list;
        list = list->next;
        if (tmp->data) {
            free(tmp->data);
        }
        //list->data might be leaking
        free(tmp);
    }
}

void print_usage() {
    printf("Usage: todo [options] [option-arguments]\n"); 
    printf("Usage: todo [new-list-entry]\n");
    printf("Usage: todo\n");
    printf("Running todo with no arguments will print your current list.\n");
    printf("\nOptions:\n\t-h|--help\t\tDisplay usage.\n");
    printf("\t-l|--list\t\tDisplay existing lists.\n");
    printf("\t-l|--list <list-name>\tChoose an existing list or create a new one.\n");
}

bool file_exists(const char *fname) { 
    return access(fname, F_OK) == 0;
}

bool dir_exists(const char *path) {
    return stat(path, &st) == -1;
}

void trim_nlc(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = 0;
    }
}
char *strjoin(char **strs, int count, const char *sep) {
    if (count <= 0) { 
        return NULL;
    } 

    int dest_len = strlen(sep) * (count - 1);

    for (int i = 0; i < count; ++i) {
        dest_len += strlen(*(strs + i));
    }

    dest_len++;

    char *dest_str = malloc(sizeof(char) * dest_len);
    if (dest_str == NULL) {
        perror(dest_str);
        return NULL;
    }

    strcpy(dest_str, *strs);
    for (int i = 1; i < count; ++i) {
        strcat(dest_str, sep);
        strcat(dest_str, *(strs + i));
    }

    return dest_str;
}

Config *deserialize_config() {
    FILE *fptr = fopen(CONFIG_PATH, "rb");
    if (fptr == NULL) {
        return NULL;
    }
    Config *config = (Config *)malloc(sizeof(Config));
    if (config == NULL) {
        printf("Error at malloc(). L BOZO. Buy more RAM.\n");
        fclose(fptr);
        return NULL;
    }
    fread(config, sizeof(Config), 1, fptr);
    fclose(fptr);
    if (config) {
        printf("debug: config fname '%s'\n", config->current_file);
    }
    return config;
}

int serialize_config(const Config *config) { 
    FILE *fptr = fopen(CONFIG_PATH, "wb");
    if (fptr == NULL) {
        perror("Error at serialize_config()");
        return 1;
    }
    fwrite(config, sizeof(Config), 1, fptr); 
    fclose(fptr);
    return 0;
}

Config *init_config() {
    Config *config = (Config *)malloc(sizeof(Config));
    memset(config->current_file, 0, MAX_FILENAME_LEN);
    return config;
}

int set_current_file(char *file_name) {
    if (!validate_input_str(file_name, MAX_FILENAME_LEN)) {
        return 1;
    }
    Config *config = deserialize_config();
    if (config == NULL) {
        config = init_config();
        strcpy(config->current_file, file_name);
    }

    if (!file_exists(file_name)) {
        FILE *new_fptr = fopen(file_name, "w");
        if (new_fptr == NULL) {
            perror("Unable to create a new file");
            return 1;
        }
        printf("list '%s' is successfully created.\n", file_name);
        fclose(new_fptr);
    } else {
        printf("list '%s' is set to current.\n", file_name);
    }
    serialize_config(config);
    free(config);
    return 0;
}

int fprint_list(const char *file_name) {
    if (head == NULL) {
        perror("Cannot save an empty list");
        return 1;
    }

    FILE *fptr = fopen(file_name, "w");
    if (fptr == NULL) { 
        perror("Error at fprint_list() at fopen()");
        return 1;
    }

    Node *current = head;
    while (current) {
        fprintf(fptr, "%s\n", current->data);
        current = current->next;
    }
    fclose(fptr);
    return 0;
}

int get_current_list() {
    Config *config = deserialize_config();
    if (config == NULL) { 
        printf("No lists currently open.\n");
        print_usage();
        return 1;
    }
    char *file_name = config->current_file;
    FILE *fptr = fopen(file_name, "r");
    if (fptr == NULL) {
        printf("Current todo list '%s' doesn't exist anymore.\n", file_name);
        printf("The corresponding file is missing from the data folder.\n");
        printf("Run todo --list %s to create a new todo list with the same name.\n", file_name);
        printf("Run todo --help to display usage.\n");
        free(config);
        return 1;
    }
    char *line = NULL;
    size_t len = 0;

    while(getline(&line, &len, fptr) != -1) {
        char *tmp = malloc((strlen(line) + 1) * sizeof(char)); 
        strcpy(tmp, line);
        trim_nlc(tmp); 
        if (!add_node(tmp)) {
            printf("Unable to add_node('%s'\n", tmp);
        }
    }
    free(config);
    fclose(fptr);
    if(line) {
        free(line);
    }
    return 0;
}

int add_todo_list_entry(char *entry) {
    if (get_current_list()) {
        return 1;
    }
    char *entrycpy = malloc((strlen(entry) + 1) * sizeof(char));
    strcpy(entrycpy, entry);
    add_node(entrycpy);
    Config *config = deserialize_config();
    if (config == NULL) {
        free_list(head);
        return 1;
    }
    fprint_list(config->current_file);
    free_list(head);
    free(config);
    return 0;
}

int print_todo_list() {
    if (get_current_list()) {
        return 1;
    }
    if (head == NULL) {
        printf("Current todo list is empty.\n");
        return 1;
    }
    Node *current = head;
    int index = 1;
    int text_set = 0;
    while (current != NULL) {
        printf("%d. %s\n", index++, current->data);
        current = current->next;
    }
    free_list(head);
    return 0;
}


int main(int argc, char **argv) {
    FILE *fptr;
    char *file_name;

    // read more about struct initializers
    const Config config = { "" };
    const char *short_options = "hl::d:";
    const struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        { "list", optional_argument, 0, 'l' },
        { "done", required_argument, 0, 'd' },
        { 0, 0, 0, 0 }
    };

    int option = 0;
    int option_index = 0;
    bool single_option_flag = false;

    while ((option = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        if (single_option_flag) {
            break;
        }
        switch (option) {
            case 0:
                if (long_options[option_index].flag != 0) {
                    break;
                }
                printf("option %s ", long_options[option_index].name);
                if (OPTIONAL_ARGUMENT_IS_PRESENT) {
                    printf("with arg %s", optarg);
                }
                printf("\n");
                break;
            case 'h':
                single_option_flag = true;
                print_usage();
                break;
            case 'l':
                single_option_flag = true;
                if (OPTIONAL_ARGUMENT_IS_PRESENT) {
                    set_current_file(optarg);
                } else {
                    Config *config = deserialize_config(); 
                    if (config == NULL) {
                        puts("Current todo list is not set.");
                        puts("Run todo --list 'name' to create a new list or choose an existing one");
                    } else {
                        printf("Current todo list: '%s'\n", config->current_file);
                    }
                    printf("Your todo lists:\n...\n");
                    free(config);
                    //print
                }
                break;
            case 'd':
                single_option_flag = true;
                printf("Marking done. with arg: %s\n", optarg);
                // parse numbers
                break;
            case '?':
                print_colored("Specified option was not found.\n", RED);
                print_colored("Run --help to display usage.\n", RED);
                exit(EXIT_FAILURE);
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    if (optind == 1) {
        if (ARGV_REMAIN) {
            char *entry  = strjoin(argv + optind, argc - optind, " ");
            if (entry == NULL) {
                puts("Error at strjoin().");
                exit(EXIT_FAILURE);
            }
            if (!validate_input_str(entry, MAX_INPUT_LEN)) {
                free(entry);
                exit(EXIT_FAILURE);
            }
            if (add_todo_list_entry(entry)) {
                free(entry); 
                exit(EXIT_FAILURE);
            }
            free(entry);
        } else {
            if (argc == 1) {
                print_todo_list();
            }
        }
    }

    exit(EXIT_SUCCESS);
}
