<img src="https://github.com/prat-man/Brainfuck/blob/master/res/icon.ico" alt="Brainfuck Interpreter Logo" width="150">

# Brainfuck

Brainfuck is an esoteric programming language created in 1993 by Urban Müller, and is notable for its extreme minimalism.

The language consists of only eight simple commands and an instruction pointer. While it is fully Turing complete, it is not intended for practical use, but to challenge and amuse programmers. Brainfuck simply requires one to break commands into microscopic steps.

The language's name is a reference to the slang term brainfuck, which refers to things so complicated or unusual that they exceed the limits of one's understanding.

<br>

## Brainfuck Interpreter

This project provides a fast Brainfuck interpreter written in C.

It is implemented with multiple optimizations to increase execution speed.<br>
Default behaviour is to interpret and execute immediately.

It is cross-platform compatible, and has been tested on Windows, Linux, and macOS.

<br>

## Brainfuck Compiler

This project provides a fast Brainfuck compiler written in C.

It converts brainfuck code to highly optimized C code and then compiles the C code into machine executable file using GCC.
To compile brainfuck code, use the <code>-c</code> or <code>--compile</code> option.

If desired, brainfuck code can be translated to C code without compiling to executable using the <code>-x</code> or <code>--translate</code> option.

The generated C code is cross-platform compatible, and has been tested on Windows, Linux, and macOS.

<br>

## Usage

    brainfuck [options] <source file path>
    
#### Options

    -c
    --compile     Translate to C and compile to machine code [requires GCC]

    -x
    --translate   Translate to C but do not compile

    -t
    --tape        Size of interpreter tape [must be equal to or above 1000]

    -s
    --stack       Size of interpreter stack [must be equal to or above 100]

    -v
    --version     Show product version and exit

    -i
    --info        Show product information and exit

    -h
    --help        Show this help message and exit

__Note:__ Default behaviour is to interpret and execute immediately.

<br>

## Build

Run the following commands inside <code>src</code> directory.

    gcc main.c -o main.o -c -O3
    gcc stack.c -o stack.o -c -O3
    gcc -o brainfuck main.o stack.o -O3

<br>

## Optimizations

 * Jumps between [ and ]
 * Compacts and jumps consecutive > and <
 * Compacts and jumps consecutive + and -
 * Optimizes [-] to set(0)
 * Optimizes [<] to scan_left(0)
 * Optimizes [>] to scan_right(0)
 * Removes consecutive > and < if the net movement is zero
 * Removes consecutive + and - if the net change is zero
 * Removes consecutive + and - if it is immediately followed by an input operation ( , )

<br>

## Credits

The brainfuck interpreter/compiler icon is made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a>.
