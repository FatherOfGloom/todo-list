#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define LIST_INIT_CAPACITY 256

#define return_defer(value) do { result = (value); goto defer; } while(0)

#define list_append(list, item) \
	do { \
	if ((list)->count >= (list)->capacity) { \
			(list)->capacity = (list)->capacity == 0 ? LIST_INIT_CAPACITY : (list)->capacity * 2; \
			(list)->items = realloc((list)->items, (list)->capacity * sizeof(*(list)->items)); \
			assert((list)->items != NULL && "list_append: error at realloc"); \
		} \
		(list)->items[(list)->count++] = (item); \
	} while (0)

#define list_append_many(list, new_items, items_count) \
	do { \
		if ((list)->count + items_count > (list)->capacity) { \
			if ((list)->capacity == 0) { \
				(list)->capacity = LIST_INIT_CAPACITY; \
			} \
			while ((list)->count + items_count > (list)->capacity) { \
				(list)->capacity *= 2; \
			} \
			(list)->items = realloc((list)->items, (list)->capacity * sizeof(*(list)->items)); \
			assert((list)->items != NULL && "list_append: error at realloc"); \
		} \
		memcpy((list)->items + (list)->count, new_items, items_count * sizeof(*(list)->items)); \
		(list)->count += items_count; \
	} while (0)

#define list_empty(list) \
	do { \
		(list)->capacity = LIST_INIT_CAPACITY; \
		(list)->count = 0; \
	} while (0)

#define list_peek(list) (assert((list)->count > 0), (list)->items[(list)->count - 1])

typedef struct StringBuilder {
	char *items;
	size_t count;
	size_t capacity; 
} StringBuilder;

#define SB_FORMAT "%.*s"

#define sb_append_buffer list_append_many

#define sb_empty list_empty

#define sb_append_cstr(sb, cstr) \
	do { \
		const char *string = (cstr); \
		size_t len = strlen(string); \
		list_append_many(sb, string, len); \
	} while (0)

#define sb_append_null(sb) list_append_many(sb, "", 1)

#define sb_append_new_line(sb) list_append(sb, '\n');

#define sb_print(sb) printf(SB_FORMAT, (int)(sb)->count, (sb)->items)

typedef enum {
	FT_REGULAR,
	FT_DIRECTORY,
	FT_OTHER,
} File_Type;

typedef struct FileInfo {
	StringBuilder file_name;
	char *file_format;
	File_Type type;
} FileInfo;

typedef struct Files {
	FileInfo* items;
	size_t count;
	size_t capacity;
} Files;

int write_file(const char *file_path, const char *buffer, size_t buffer_size);
int read_file(const char *file_path, StringBuilder *sb); 
int read_dir(const char *dir_path, Files *files);
char *get_file_format(const char *file_name);
int get_file_type(const char *file_path, File_Type *type);
bool file_exists(const char *file_path);
void free_files_list(Files *files);
bool file_exists(const char *file_name); 

#endif // UTILS_H
