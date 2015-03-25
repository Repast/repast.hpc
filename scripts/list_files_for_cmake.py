#!/usr/bin/env python

import sys, os

def run(input_dir):
    if input_dir[-1:] == '/':
        input_dir = input_dir[:-1]
    exts = ['.cpp', '.h', '.hpp']
    head, tail = os.path.split(input_dir)
    for f in os.listdir(input_dir):
        index = f.rfind(".")
        if index != -1 and f[index:] in exts:
            print(tail + "/" + f)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: list_files_for_cmake.py [directory]")
    else:
        run(sys.argv[1])

