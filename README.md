<img src="https://github.com/prat-man/Brainfuck/blob/master/res/icon.ico" alt="Brainfuck Interpreter Logo" width="150">

# Brainfuck

Brainfuck is an esoteric programming language created in 1993 by Urban Müller, and is notable for its extreme minimalism.

The language consists of only eight simple commands and an instruction pointer. While it is fully Turing complete, it is not intended for practical use, but to challenge and amuse programmers. Brainfuck simply requires one to break commands into microscopic steps.

The language's name is a reference to the slang term brainfuck, which refers to things so complicated or unusual that they exceed the limits of one's understanding.

<br>

## Brainfuck Interpreter

This project presents a fast Brainfuck interpreter written in C.

It is implemented with multiple optimizations to increase execution speed. It is cross-platform compatible, and has been tested on Windows and Linux.

<br>

## Usage

    brainfuck [options] <source file path>
    
#### Options

    -t
    -tape       Size of interpreter tape [must be equal to or above 10000]

    -s
    -stack      Size of interpreter stack [must be equal to or above 1000]

    -v
    -version    Show product version and exit

    -i
    -info       Show product information and exit

    -h
    -help       Show this help message and exit

<br>

## Build

Run the following commands inside <code>src</code> directory.

    gcc main.c -o main.o -c
    gcc stack.c -o stack.o -c
    gcc -o brainfuck main.o stack.o

<br>

## Optimizations

 * Jumps between [ and ]
 * Compacts and jumps consecutive > and <
 * Compacts and jumps consecutive + and -
 * Optimizes [-] to set(0)
 * Optimizes [<] to scan_left(0)
 * Optimizes [>] to scan_right(0)

<br>

## Credits

The brainfuck interpreter icon is made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a>.
