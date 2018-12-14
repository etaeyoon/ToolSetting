#!/bin/sh
rm -rf cscope.file cscope.out
rm -rf tags

find . \( -name '*.c' -o -name '*.cpp' -o -name '*.cc' -o -name '*.h' -o -name '*.s' -o -name '*.S' \) -print > cscope.file
ctags -R

cscope -i cscope.file
