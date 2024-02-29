## TODO
This is a small project in c language that was done for studying purposes and is a work in progress. Its basically a prototype.
This ptoject represents a simple cli tool for linux (for now) that helps you to create and manage your todo lists in console with simple commands.
It is important to note that I am studying and a lot of solutions are greatly inspired by tsodings streams, and are not unique to this project (handling of file io, dinamic arrays and StringBuilder and a lexer).

## CONTENTS
# This project consists of several modules:
1. It parses CLI options and arguments using getopt.
2. Binary tree data structure implementation with next and child pointers for managing todo list with sub-items.
3. A custom serialization for this tree data structure that pretty much resembles json/
4. A custom lexer and parser for deserialization of strings in this format (Although the parser is not a generalized solution and works only for this specific purpose).
5. Dinamic array and StringBuilder implementation.

## USAGE
# To print your current list run:
todo 

# To add a new entry to a current list run: 
todo [entry-string]

# To use options run:
todo [options] <option-arguments>

# Available options:
-h|--help               Display usage.
-l|--list               Display existing lists.                                                                                                             
-l|--list <list-name>   Choose an existing list or create a new one.

As stated above this is a work in progress so new features will be added.
