#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "./utils.h"
#include "./lexer.h"
#include "./todolist.h"

TodoListNode *todolist_new_node(TodoListNode *prev, const char *entry) {
	TodoListNode *node = (TodoListNode *)malloc(sizeof(TodoListNode));
	if (node == NULL) {
		return NULL;
	}
	StringBuilder sb = {0};
	sb_append_cstr(&sb, entry);
	node->sb = sb;
	node->done_flag = false;
	node->prev = prev;
	node->next = NULL; 
	node->child = NULL;
	return node;
}

void todolist_free_rec(TodoListNode *root) {
	if (root == NULL) {
		return;
	}
	todolist_free_rec(root->next);
	todolist_free_rec(root->child);
	if (root->sb.items) {
		free(root->sb.items);
	}
	free(root);
}

void todolist_free(TodoList *todo_list) {
	todolist_free_rec(todo_list->head);
}

void todolist_serialize_rec(TodoListNode *root, StringBuilder *sb, int *iter) {
	if (root == NULL) {
		if (*iter != 0) {
			sb_append_cstr(sb, ",");
		}
		sb_append_cstr(sb, "#");
		*iter += 1;
		return;
	}
	if (*iter > 0) {
		sb_append_cstr(sb, ",");
	}
	sb_append_cstr(sb, "{entry:\"");
	sb_append_buffer(sb, root->sb.items, root->sb.count);
	sb_append_cstr(sb, "\",doneFlag:");
	sb_append_cstr(sb, root->done_flag ? "true" : "false");
	sb_append_cstr(sb, "}");
	*iter += 1;
	todolist_serialize_rec(root->next, sb, iter);
	todolist_serialize_rec(root->child, sb, iter);
}

char *todolist_serialize(TodoList *todo_list) {
	StringBuilder sb = {0};
	int iter = 0;
	todolist_serialize_rec(todo_list->head, &sb, &iter);
	sb_append_null(&sb);
	return sb.items;
}

void todolist_print_rec(TodoListNode *root, int *iter, int depth) {
	if (root == NULL) {
		return;
	}
	for (int i = 0; i < depth; ++i) {
		printf("    ");
	}
	printf("%d. ", *iter);
	if (root->done_flag) {
		printf("\033[1m");
		printf("\033[32m");
	}
	sb_print(&root->sb);
	if (root->done_flag) {
		printf("\033[0m");
	}
	if (root->child) {
		printf(":");
	}
	printf("\n");
	*iter += 1;
	todolist_print_rec(root->child, iter, depth + 1);
	todolist_print_rec(root->next, iter, depth);
}

void todolist_print(TodoList *todo_list) {
	int iter = 1;
	todolist_print_rec(todo_list->head, &iter, 0);
}

//THE DESIGN OF THIS IS VERY HUMAN (IM ASHAMED OF THIS)
TodoListNode *todolist_parse_rec(TodoListNode *root, Tokens *tokens, int *iter, int *error) {
	if (*error == -1) {
		return NULL;
	}
	TodoListNode *result = NULL;
	StringBuilder sb = {0};
	switch (tokens->items[*iter].type) {
		case (TOKEN_NULL):
			return NULL;
		case (TOKEN_BRACE_OPEN):
			*iter += 1;
			while(tokens->items[*iter].type != TOKEN_BRACE_CLOSE && *iter < tokens->count) {
				Token t = tokens->items[*iter];
				switch (t.type) {
					case(TOKEN_INVALID):
						goto error;
					case(TOKEN_COLON):
						break;
					case(TOKEN_STRING):
						if (result != NULL) {
							puts("Multiple string literals at one node");
							goto error;
						}
						sb_append_buffer(&sb, t.string + 1, t.string_len - 2);
						sb_append_null(&sb);
						result = todolist_new_node(root, sb.items);
						if (sb.items) free(sb.items);
						break;
					case(TOKEN_SYMBOL):
						if (!is_keyword(&t, "entry") && !is_keyword(&t, "doneFlag")) {
							goto error;
						}
						break;
					case(TOKEN_BOOL):
						if (result == NULL) {
							goto error;
						}
						if (*iter >= 2 && is_keyword(&tokens->items[*iter - 2], "doneFlag")) { 
							result->done_flag = is_keyword(&t, "true") ? true : false;
						}
						break;
					default:
						goto error;
						break;
				}
				*iter += 1;
			}
			if (result == NULL) {
				puts("todolist_parse_rec: invalid format");
				goto error;
			}
			break;
		case (TOKEN_INVALID):
			goto error;
		default:
			printf("Parsing error at token '%.*s'. Expected tokens: '#', ',', '{'\n", (int)tokens->items[*iter].string_len, tokens->items[*iter].string);
			goto error;
	}
	*iter += 1;
	result->next = todolist_parse_rec(result, tokens, iter, error);
	*iter += 1;
	result->child = todolist_parse_rec(result, tokens, iter, error);
	return result;
error:
	printf("Unexpected token '%.*s'\nUnable to parse todo tree\n", 
			(int)tokens->items[*iter].string_len, tokens->items[*iter].string);
	*error = -1;
	return NULL;
}

void todolist_tokenize(Tokens *token_list, const char *string) {
	token_list->count = 0;
	Lexer l = lexer_new(string, strlen(string));
	Token t = lexer_next(&l);
	while (t.type != TOKEN_END) {
		if (t.type != TOKEN_COMMA) {
			list_append(token_list, t);
		}
		t = lexer_next(&l);
	}
}
int todolist_parse(TodoList *todo_list, const char *string) {
	Tokens tokens = {0};
	int iter = 0;
	int error = 0;
	todolist_tokenize(&tokens, string);
	todo_list->head = todolist_parse_rec(NULL, &tokens, &iter, &error);
	if (tokens.items) {
		free(tokens.items);
	}
	if (todo_list->head == NULL) {
		todo_list->tail = NULL;
		return error;
	}
	TodoListNode *tmp = todo_list->head;
	while (tmp->next) {
		tmp = tmp->next;
	}
	todo_list->tail = tmp;
	return error;
}

TodoList todolist_deserialize(const char *string) {
	TodoListNode *result = NULL;
	TodoList todo_list = {0};
	int err = todolist_parse(&todo_list, string); 
	if (err == -1) {
		if (todo_list.head) {
			todolist_free(&todo_list);
		}
		todo_list.head = NULL;
	}
	return todo_list;
}

void todolist_append(TodoList *todo_list, const char *string) {
	TodoListNode *node = todolist_new_node(todo_list->tail, string);
	if (todo_list->tail == NULL) {
		todo_list->head = todo_list->tail = node;
		return;
	}
	todo_list->tail->next = node;
	todo_list->tail = node;
}

//Works only with "." dir_path
//Otherwise readdir gives errno: 2 for some reason ( T-T)
int todolist_get_files(const char *dir_path, Files *files) {
	int result = 0;
	Files tmp_files = {0};
	files->count = 0;
	int err = read_dir(dir_path, &tmp_files);
	if (err != 0) {
		puts("Unable to read directory");
		free_files_list(&tmp_files);
		return_defer(err);
	}
	for (int i = 0; i < tmp_files.count; ++i) {
		FileInfo fi = tmp_files.items[i];
		if (fi.file_format && !strcmp(fi.file_format, "gloom")) {
			list_append(files, fi);
		} else {
			if (fi.file_name.items) free(fi.file_name.items);
			if (fi.file_format) free(fi.file_format);
		}
	}
	if (files->count == 0) {
		puts("No saved Lists.");
		printf("Save folder: '%s'.\n", dir_path);
		return_defer(-1);
	}
defer:
	if (tmp_files.items) free(tmp_files.items);
	return result;
}

void todolist_print_usage(void) {
    printf("Usage: todo [options] [option-arguments]\n"); 
    printf("Usage: todo [new-list-entry]\n");
    printf("Usage: todo\n");
    printf("Running todo with no arguments will print your current list.\n");
    printf("\nOptions:\n\t-h|--help\t\tDisplay usage.\n");
    printf("\t-l|--list\t\tDisplay existing lists.\n");
    printf("\t-l|--list <list-name>\tChoose an existing list or create a new one.\n");
}

void trim_nlc(char *string) {
    int len = strlen(string);
    if (len > 0 && string[len - 1] == '\n') {
        string[len - 1] = 0;
    }
}

void todolist_validate_filename(char *file_name) {
	if (file_name == NULL || strlen(file_name) == 0) {
		return;
	}
	int len = strlen(file_name);
	for (int i = 0; i < len; ++i) {
		if (file_name[i] == ' ') {
			file_name[i] = '_';
		}
	}
}
//For now returns current file_name
//TODO: add config struct if > 1 fields will be needed
char *todolist_parse_config(const char *string) {
	char *result = NULL;
	char mut_string[strlen(string) + 1];
	strcpy(mut_string, string);
	char *tokens[256];
	int iter = -1;
	if (strlen(string) == 0) {
		puts("Config file is empty. Run todo --list [name]. To create a new list.");
		return_defer(NULL);
	}
	char *token = strtok(mut_string, "=");
	while (token != NULL && iter < 256) {
		trim_nlc(token);
		char *buffer = (char *)malloc(sizeof(char) * strlen(token) + 1);
		strcpy(buffer, token);
		tokens[++iter] = buffer;
		token = strtok(NULL, "=");
	}
	if (iter < 1) {
		return_defer(NULL);
	}
	if (!strcmp(*tokens, "current") && tokens[1]) {
		int result_len = strlen(tokens[1]) + 1; 
		result = (char *)malloc(sizeof(char) * result_len);
		strcpy(result, tokens[1]);
	} else {
		//invalid format
		return_defer(NULL);
	}
defer:
	for (int i = 0; i < iter + 1; ++i) {
		//debug
		//printf(" i %d iter %d tokens[i] '%s' \n", i, iter, tokens[i] ? tokens[i] : "NULL");
		if (tokens[i]) free(tokens[i]);
	}
	return result;
}

int todolist_get_config(StringBuilder *current_list_path) {
	int result = 0;
	char *file_name = NULL;
	StringBuilder sb_config = {0};
	if(!file_exists(CONFIG_NAME)) {
		puts("No lists set to current.");
		//caller must print_usage();
		return_defer(-1);
	}
	if (read_file(CONFIG_NAME, &sb_config) != 0) {
		puts("Error at reading config file.");
		return_defer(-1);
	}
	sb_append_null(&sb_config);
	file_name = todolist_parse_config(sb_config.items);
	if (file_name == NULL) {
		puts("Unable to parse config.");
		return_defer(-1);
	}
	sb_append_cstr(current_list_path, file_name);
defer:
	if (file_name) free(file_name);
	if (sb_config.items) free(sb_config.items);
	return result;
}

int todolist_get_current_list(TodoList *list) {
	int result = 0;
	StringBuilder sb_todolist_path = {0};
	StringBuilder sb_todolist = {0};
	if (todolist_get_config(&sb_todolist_path) == -1) {
		return_defer(-1);
	}
	sb_append_cstr(&sb_todolist_path, ".gloom");
	sb_append_null(&sb_todolist_path);
	if (!file_exists(sb_todolist_path.items)) {
		printf("Current todo list '%.*s' is empty.", (int)sb_todolist_path.count, sb_todolist_path.items); 
		return_defer(-1);
	}
	if (read_file(sb_todolist_path.items, &sb_todolist) != 0) {
		printf("Error at reading file '%.*s'.", (int)sb_todolist_path.count, sb_todolist_path.items);
		return_defer(-1);
	}
	sb_append_null(&sb_todolist);
	*list = todolist_deserialize(sb_todolist.items);
	if (list->head == NULL) {
		puts("Error at todolist_deserialize.");
		return_defer(-1);
	}
defer:
	if (sb_todolist_path.items) free(sb_todolist_path.items);
	if (sb_todolist.items) free(sb_todolist.items);
	return result;
}

int todolist_print_current_list(void) {
	int result = 0;
	TodoList todo_list = {0};
	if (todolist_get_current_list(&todo_list) == -1) {
		puts("Couldn't get current list.");
		return_defer(-1);
	}
	todolist_print(&todo_list);
defer:
	if (todo_list.head) todolist_free(&todo_list);
	return result;
}

int todolist_append_entry(const char *entry) {
	int result = 0;
	TodoList todo_list = {0};
	StringBuilder sb_todolist_path = {0};
	StringBuilder sb_todolist = {0};
	char *buffer = NULL;
	if (todolist_get_config(&sb_todolist_path) == -1) {
		return_defer(-1);
	}
	sb_append_cstr(&sb_todolist_path, ".gloom");
	sb_append_null(&sb_todolist_path);
	if (file_exists(sb_todolist_path.items)) {
		if (read_file(sb_todolist_path.items, &sb_todolist) != 0) {
			printf("Error at reading file '%.*s'\n", (int)sb_todolist_path.count, sb_todolist_path.items);
			return_defer(-1);
		}
		sb_append_null(&sb_todolist);
		todo_list = todolist_deserialize(sb_todolist.items);
	} 
	todolist_append(&todo_list, entry);
	buffer = todolist_serialize(&todo_list);
	if (write_file(sb_todolist_path.items, buffer, strlen(buffer)) != 0) {
		printf("Writing to file '%.*s' failed.", (int)sb_todolist_path.count, sb_todolist_path.items);
	}
defer:
	if (sb_todolist_path.items) free(sb_todolist_path.items);
	if (sb_todolist.items) free(sb_todolist.items);
	if (todo_list.head) todolist_free(&todo_list);
	if (buffer) free(buffer);
	return result;
}

int todolist_print_all_list_names(const char *dir_path) {
	int result = 0;
	Files files = {0};
	StringBuilder sb_todolist_path = {0};
	printf("Your current list: ");
	if (todolist_get_config(&sb_todolist_path) == 0) {
		sb_append_null(&sb_todolist_path);
		printf("'");
		sb_print(&sb_todolist_path);
		printf("'\n");
	}
	puts("Your todo lists:");
	if (todolist_get_files(dir_path, &files) != 0) {
		return_defer(-1);
	}
	for (int i = 0; i < files.count; ++i) {
		printf("%d. ", i + 1);
		sb_print(&files.items[i].file_name);
	}
	printf("\n");
defer:
	if (sb_todolist_path.items) free(sb_todolist_path.items);
	if (files.items) free_files_list(&files);
	return result;
}

int todolist_set_current_list(char *name_entry) {
	int result = 0;
	StringBuilder sb_todolist_path = {0};
	char name[strlen(name_entry) + 1];
	strcpy(name, name_entry);
	todolist_validate_filename(name);
	printf("List '%s' is set to current\n", name);
	sb_todolist_path.count = 0;
	sb_append_cstr(&sb_todolist_path, "current=");
	sb_append_cstr(&sb_todolist_path, name);
	sb_append_null(&sb_todolist_path);
	if (write_file(CONFIG_NAME, sb_todolist_path.items, sb_todolist_path.count) != 0) {
		puts("Writing to config file failed.");
		return_defer(-1);
	}
defer:
	if (sb_todolist_path.items) free(sb_todolist_path.items);
	return result;
}

// todo->read_config->if(file_exists(file_name))->read_file->deserialize(string)->print(tree)
//						else print todolist empty->print_usage();
// todo [entry]->read_config->get_files->if(file_exists(file_name)->read_file->deserialize(string)->append(entry)->serialize(tree)->write_file(tree)
//						if (config empty): no files open; print_usage;
//						if(file_exists(file_name) == false): print(given file doesnt exist, creating a new list with a corresponding name)->append(entry)->ser_write
//										
// todo -l/--list ->get_files()->if(files)->print(files)
//		read_config->print(current)
// todo -l/--list [entry] ->file_exists(file_name)->write_config(entry)->print("current list is set to 'name'");
//						if(!file_exists(file_name))->write_config(entry)->print(list 'entry' doesnt exit. New list created)->get_files->print(your current lists: ...)
