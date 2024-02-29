#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "./utils.h"

static int file_size(FILE *fptr, size_t *size) {
	long saved = ftell(fptr);
	if (saved < 0) {
		return errno;
	} 
	if (fseek(fptr, 0, SEEK_END) < 0) {
		return errno;
	}
	long result = ftell(fptr);
	if (result < 0) {
		return errno;
	}
	if (fseek(fptr, saved, SEEK_SET) < 0) {
		return errno;
	}
	*size = (size_t)result;
	return 0;
}

int write_file(const char *file_path, const char *buffer, size_t buffer_size) {
	int result = 0;
	FILE *fptr = NULL; 
	fptr = fopen(file_path, "wb");
	if (fptr == NULL) {
		return_defer(errno);
	}
	fwrite(buffer, 1, buffer_size, fptr); 
	if (ferror(fptr)) {
		return_defer(errno);
	}
defer:
	if (fptr) {
		fclose(fptr);
	}
	return result;
}

int read_file(const char *file_path, StringBuilder *sb) { 
	int result = 0;
	FILE *fptr = NULL;
	fptr = fopen(file_path, "r");
	if (fptr == NULL) {
		return_defer(errno);
	}
	size_t size;
	int err = file_size(fptr, &size);
	if (err != 0) {
		return_defer(err);
	}
	char *buffer = (char *)malloc(size * sizeof(char));
	if (buffer == NULL) {
		return_defer(errno);
	}
	fread(buffer, size, 1, fptr);
	if (ferror(fptr)) {
		return_defer(errno);
	}
	sb_append_buffer(sb, buffer, size);
defer: 
	if (fptr) {
		fclose(fptr);
	}
	if (buffer) {
		free(buffer);
	}
	return result;
}

void free_files_list(Files *files) {
	for (int i = 0; i < files->count; ++i) { 
		if (files->items[i].file_name.items) free(files->items[i].file_name.items);
		if (files->items[i].file_format) free(files->items[i].file_format);
	}
	if (files->items) free(files->items);
}

FileInfo new_file_info(const char *file_name) {
	FileInfo result = {0};
	sb_append_cstr(&result.file_name, file_name);
	/*
	int file_path_len = strlen(file_name) + 3;
	char file_path[file_path_len];
	file_path[0] = 0;
	snprintf(file_path, file_path_len, "./%s", file_name);
	*/
	get_file_type(file_name, &result.type);
	result.file_format = get_file_format(file_name);
	return result;
}

//TODO: returns errno 2, when at "." dirent when reading a subfolder
//temporarily not reporting an error in such occasion and it seems to work...
int read_dir(const char *dir_path, Files *files) {
	int result = 0;
	DIR *dir = NULL;
	dir = opendir(dir_path);
	if (dir == NULL) {
		puts("Error at opendir");
		printf("Errno: %d (%s)\nPath: '%s'\n", errno, strerror(errno), dir_path);
		return_defer(errno);
	}
	errno = 0;
	struct dirent *ent = readdir(dir);
	while (ent != NULL) {
		FileInfo f_info = new_file_info(ent->d_name);
		list_append(files, f_info);
		ent = readdir(dir);
	}
	if (errno != 0) {
		//printf("Errno: %d (%s)\nPath: '%s'\n", errno, strerror(errno), dir_path);
		//return_defer(errno);
	}
defer: 
	if (dir) {
		closedir(dir);
	}
	return result;
}

char *get_file_format(const char *file_name) {
	char *result = NULL;
	char file_name_mut[strlen(file_name) + 1];
	strcpy(file_name_mut, file_name);
	char *string_token = strtok(file_name_mut, ".");
	char *prev_token = NULL;
	while (string_token != NULL) {
		prev_token = string_token;
		string_token = strtok(NULL, ".");
	}
	if (prev_token == NULL) {
		return_defer(NULL);
	}
	if (!strcmp(prev_token, file_name)) {
		return_defer(NULL);
	}
	result = (char *)malloc(sizeof(char) * strlen(prev_token) + 1);
	if (result == NULL) {
		return_defer(NULL);
	}
	strcpy(result, prev_token);
defer:
	return result;
}

int get_file_type(const char *file_path, File_Type *type) {
	int result = 0;
	struct stat st = {0};
	if (stat(file_path, &st) < 0) {
		return_defer(errno);
	}
	if (S_ISREG(st.st_mode)) {
		*type = FT_REGULAR;
	} else if (S_ISDIR(st.st_mode)) {
		*type = FT_DIRECTORY;
	} else { 
		*type = FT_OTHER;
	}
defer:
	return result;
}

bool file_exists(const char *file_name) { 
    return access(file_name, F_OK) == 0;
}
