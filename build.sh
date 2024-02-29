#!/bin/bash
set -xe
gcc -o ./bin/todo src/main.c src/lexer.c src/todolist.c src/utils.c 
set +xe
echo "build.sh: success."
