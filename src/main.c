#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include "./todolist.h"
#include "utils.h"

#define OPTIONAL_ARGUMENT_IS_PRESENT \
    ((optarg == NULL && optind < argc && **(argv + optind) != '-') \
     ? (bool)(optarg = argv[optind++]) \
     : (optarg != NULL))

#define ARGV_REMAIN (optind < argc)

static char path[PATH_MAX];

typedef enum {
	STATE_ERROR = 0,
	STATE_PRINT_LIST,
	STATE_NEW_ENTRY,
	STATE_PRINT_ALL_LISTS,
	STATE_USAGE,
	STATE_INCOMPATIBLE_OPTS,
	STATE_SET_LIST,
	STATE_DEFAULT,
	STATE_DONE
} Program_State;

bool ensure_single_opt(bool *flag, Program_State *state) {
	if (*flag == true) {
		*state = STATE_INCOMPATIBLE_OPTS;
		return true;
	} else {
		*flag = true;
		return false;
	}
}

int main(int argc, char **argv) {
	getcwd(path, sizeof(path));
	const char *short_options = "hl::d:";
	const struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "list", optional_argument, 0, 'l' },
		{ "done", required_argument, 0, 'd' },
		{ 0, 0, 0, 0 }
	};

	int result = 0;
	int option = 0;
	int option_index = 0;
	Program_State state = STATE_DEFAULT; 
	StringBuilder user_entry = {0};
	bool single_option_flag = false;

	while ((option = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (option) {
			case 'h':
				if (!ensure_single_opt(&single_option_flag, &state)) {
					state = STATE_USAGE;
				}
				break;
			case 'l':
				if (ensure_single_opt(&single_option_flag, &state)) {
					break;
				}
				if (OPTIONAL_ARGUMENT_IS_PRESENT) {
					state = STATE_SET_LIST; 
					user_entry.count = 0;
					sb_append_cstr(&user_entry, optarg);
					sb_append_null(&user_entry);
				} else {
					state = STATE_PRINT_ALL_LISTS;
				}
				break;
			case 'd':
				ensure_single_opt(&single_option_flag, &state);
				state = STATE_DONE;
				break;
			case '?':
				ensure_single_opt(&single_option_flag, &state);
				state = STATE_ERROR;
				break;
			default:
				state = STATE_ERROR;
				break;
		}
	}

	if (optind == 1) {
		if (ARGV_REMAIN) {
			state = STATE_NEW_ENTRY;
		} else if (argc == 1) {
			state = STATE_PRINT_LIST;
		}
	}


	switch (state) {
		case STATE_DEFAULT:
			puts("UNREACHABLE");
			return_defer(-1);
		case STATE_PRINT_LIST:
			result = todolist_print_current_list();
			break;
		case STATE_PRINT_ALL_LISTS:
			result = todolist_print_all_list_names(path);
			break;
		case STATE_ERROR:
			return_defer(-1);
		case STATE_INCOMPATIBLE_OPTS:
			puts("Incompatible options");
			return_defer(-1);
		case STATE_DONE:
			//todolist_mark_done();
			puts("UNIMPLEMENTED");
			return_defer(-1);
		case STATE_NEW_ENTRY:
			user_entry.count = 0;
			while (*++argv) {
				sb_append_cstr(&user_entry, *argv);
				if (*(argv + 1)) {
					sb_append_cstr(&user_entry, " ");
				}
			}
			sb_append_null(&user_entry);
			result = todolist_append_entry(user_entry.items);
			todolist_print_current_list();
			break;
		case STATE_SET_LIST:
			result = todolist_set_current_list(user_entry.items);
			break;
		case STATE_USAGE:
			todolist_print_usage();
			break;
	}

defer:
	if (result == -1) {
		todolist_print_usage();
	}
	if (user_entry.items) free(user_entry.items);
	return result;
}
