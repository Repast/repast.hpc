#!/usr/bin/env python

import sys, os, string

def parse_line(line):
    idx1 = line.find("(")
    name = line[:idx1]
    idx2 = line.find("):")
    args = line[idx1 + 1 : idx2].split(",")
    return (name, args)


def create_code(sig):
    t = string.Template("""TEST_F(Errors, $name) {
        $name$t r_error$args;
        ASSERT_TRUE(string(r_error.what()).size() > 0);
        try {
          throw r_error;
          FAIL();
        } catch (std::exception& e) {
          ASSERT_TRUE(string(e.what()).size() > 0);
        }
      }""")
      
    args = ""
    name_t = ""
    for arg in sig[1]:
        arg = arg.strip()
        if arg.startswith("int"):
            args += (", 0");
        elif arg.startswith("double"):
            args += (", 0.0")
        elif arg.startswith("std::string") or arg.startswith("const std::string"):
            args += (", \"foo\"")
        elif arg.startswith("std::vector<std::string>"):
            args += ", std::vector<string>{\"foo\"}"
        elif arg.startswith("std::vector<int>"):
            args += ", std::vector<int>{0}"
        elif arg.startswith("std::vector<double>"):
            args += ", std::vector<double>{0.0}"
        elif arg.startswith("T"):
            args += ", 1"
            name_t = "<int>"
        elif len(arg) > 0:
            args += ", " + arg
    
    if len(args) > 0:
        args = "(" + args[2:] + ")"
    
    
    return t.substitute(name = sig[0], t=name_t, args=args)
    

def run(f):
    errors = []
    with open(f) as f_in:
        for line in f_in:
            line = line.strip()
            if line.startswith("Repast_Error"):
                errors.append(parse_line(line))
    
    code = []
    for error in errors:
        code.append(create_code(error))
    
    for code in code:
        print(code)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: create_error_tests_code.py [path to RepastErrors.h]")
    else:
        run(sys.argv[1])

