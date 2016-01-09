# Parser for Mini-Pascal

## Usage
You can try directly by compiled executable or compile from source
### Run Compiled Executable
 Under folder [compiled_executable](compiled_executable)
```
./mini_pascal_compiler ../testcase/12_fib.p fib
java -jar jasmin.jar fib.j
java fib
```
More testcases, see [testcase](testcase)

### Compile by Yourself
You must install Lex and YACC first before compiling the source.
####  Library Dependency for Ubuntu
`$ sudo apt-get update && sudo apt-get install bison flex byacc`

#### Instructions
 1. go to folder [src](src)
 2. `$ make`
 3. `$ make run`
