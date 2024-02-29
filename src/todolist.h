#ifndef TODOLIST_H
#define TODOLIST_H

#include "./utils.h"

#define CONFIG_NAME "config.xd"

typedef struct TodoListNode {	
	struct TodoListNode *next;
	struct TodoListNode *prev;
	struct TodoListNode *child;
	StringBuilder sb;
	bool done_flag;
} TodoListNode;

typedef struct TodoList {
	struct TodoListNode *head;
	struct TodoListNode *tail; 
	const char *current_file;
} TodoList;

// todo
int todolist_print_current_list(void);
// todo [entry]
int todolist_append_entry(const char *entry);
// todo -l [name] or todo --list [name]
int todolist_set_current_list(char *name);
// todo -l or todo --list
int todolist_print_all_list_names(const char *dir_path);
// call if -1 return val
void todolist_print_usage(void);

#endif // TODOLIST_H
