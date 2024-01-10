## cubsh

### Description
Implementation of a custom UNIX shell that supports up to one level of pipe redirection.

### Installation
The project comes with a Makefile which will generate an executable called cubsh.

```
╭─cbabste@mufasa ~/Code/cubsh ‹main●› 
╰─$ clear

╭─cbabste@mufasa ~/Code/cubsh ‹main●› 
╰─$ make
gcc -c cubsh.c -o cubsh.o
gcc -o cubsh cubsh.o
╭─cbabste@mufasa ~/Code/cubsh ‹main●› 
╰─$
```

### Usage
```
╭─cbabste@mufasa ~/Code/cubsh ‹main●› 
╰─$ ./cubsh 

Welcome to cubsh
cubsh $ ls
Makefile	README.md	cubsh		cubsh.c		cubsh.o
cubsh $ exit

Goodbye. Thanks for using cubsh.
╭─cbabste@mufasa ~/Code/cubsh ‹main●› 
╰─$ 
```