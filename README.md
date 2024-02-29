# TODO
This is a small project in c language that was done for studying purposes and is a work in progress. Its basically a prototype.
This ptoject represents a simple CLI tool for linux (for now) that helps you to create and manage your todo lists in console with simple commands.
It is important to note that I am studying and a lot of solutions are greatly inspired by tsodings streams, and are not unique to this project (handling of file io, dinamic arrays and StringBuilder and a lexer).


## This project consists of several modules:
- It parses CLI options and arguments using getopt.
- Binary tree data structure implementation with next and child pointers for managing todo list with sub-items.
- A custom serialization for this tree data structure that pretty much resembles json/
- A custom lexer and parser for deserialization of strings in this format (Although the parser is not a generalized solution and works only for this specific purpose).
- Dinamic array and StringBuilder implementation.

## Installation

- For now you can only build from source.

## Usage

#### To print your current list run:
```bash
todo
```

#### To add a new entry to a current list run: 
 ```bash
todo [entry-string]
```

#### To use options run:
 ```bash
todo [options] <option-arguments>
```

#### To display options and usage run:

```bash
todo --help
```


 As stated above this is a work in progress so new features will be added.

## Existing bugs
 - The config/data folder is not managed properly so you cannot use it from path.
 - Some new line/indenting issues in the output.

## Incoming features 
 - Marking list items as done.
 - Deleting list items/sub-items.
 - Inserting sub-items.
 - Deleting saved lists with commands instead of doing it manually.
 - Tree-like visualization of sub-items.
