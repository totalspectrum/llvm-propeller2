# Propeller 2 LLVM Backend

The goal of this project is to write an LLVM backend to generate code for the Propeller 2 microcontroller. Need to do the following things from a high level:

1. ~~figure out how to add a new target machine in the LLVM environment~~
    - https://llvm.org/docs/WritingAnLLVMBackend.html#preliminaries seems to have some starting points
1. ~~define a register format to use the COG memory register space~~ (see below)
1. edit this machine to translate a basic program that is just main returning a constant.
    - minimum requires defining out the calling convention, load, store, and ret instruction lowering.
    - the same page as above should get us there.
    - some great resources (though on opposite ends of the spectrum of sparse and dense):
        - https://jonathan2251.github.io/lbd/llvmstructure.html
        - https://llvm.org/devmtg/2012-04-12/Slides/Workshops/Anton_Korobeynikov.pdf
        - https://llvm.org/devmtg/2014-04/PDFs/Talks/Building%20an%20LLVM%20backend.pdf
1. edit this machine to generate PASM code for basic ALU operations
1. edit this machine to support basic use of special registers (OUTx, DIRx, and INx)
1. edit this machine to add some basic startup code to start the cog and execute a blinking LED program
1. add more propeller instructions to support branching
1. implement calling conventions to call functions (especially recursion)
1. add support for starting cogs at specific memory location (with the same startup code as above)
1. expand on the rest of the propeller instruction set
1. create clang extensions for special directives and being able to write directly to I/O regsiters, etc.
1. port the necessary functions from the c standard library to make c/c++ useful.

The high level of how this will work: 
1. use clang to compile c/c++ source into LLVM's IR language. Eventually any LLVM front end should work
1. use the custom backend (the goal of this project, using MIPS, AVR, and ARC targets as references) will convert the LLVM IR code to PASM. 
1. use fastspin (or whatever Parallax's official assembler will be) to compile the assembly code into an executable elf to load. Eventually should be able to compile the PASM to and elf/binary directly from LLVM

## Getting Started
See README.md, with the following notes: 
- when running `cmake`, run `cmake -G "Unix Makefiles" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=P2 ../llvm`
- building for the first time will take quite a bit of time, ~20 min.
- to run one of the examples in p2_dev_tests, run in two steps (from p2_dev_tests/)
    - `../build/bin/clang -target mips-unknown-linux-gnu -S -c <file>.cpp -emit-llvm -o <file>.ll` (Eventually need to create a P2 target in clang). This compiles C down to LLVM IR language. 
    - `../build/bin/llc -march=p2 -debug -filetype=asm <file>.ll -o test2.s`. This will output an assembly file with P2 instructions. There's still work to be done to clean it up into something compilable with fastspin. Eventually should be able to compile PASM directly to objects to remove the need for fastspin entirely.
- The presentation by Anton Korobeynikov above gives a pretty high level but informative overview of how a backend works and the various components. This along with the other documents linked above help form a complete picture of how stuff works.

## Cog Layout
The simplest way to make LLVM compatible with propeller is to divide the cog memory into various sections for various compiler features. This section defines that layout.

### Register Definition
Cog memory is 512 longs. The last 16 are special registers (0x1f0 - 0x1ff, so we will use the previous 16 registers (0x1e0 - 0x1ef) as general purpose registers for the compiler. So r0 = 0x1e0, r1 = 0x1e1, etc. We'll also need a stack pointer and maybe a link register. We'll deal with this later. 2 more registers will be reserved for this.

### Stack
We'll need a stack for calling functions, etc. The first 256 longs of the look-up RAM will be a fixed stack for calls and such. So, the stack starts at 0x0 and grows up to 0x0ff. Initially we'll just use COG RAM so that a typical architecture's load/store instructions can translate directly to rdlong/wrlong for memory read/writes, but eventually we'll add distinction between memories so that stack operatiosn use rdlut/wrlut, and normal memory operations needing the hub RAM will use rdlong/wrlong.

The remaining cog RAM should be used as a cache for loops, etc. Eventually, there should be a way to specify a function or variable to be cached in cog RAM so it never needs to be fetched. This will likely need a clang extension.

## Calling Convention
For starters, we will use a simple calling convention using the above registers to pass and return values for functions.

### Passing Arguments
- All 8 and 16 bit arguments are promoted to 32 bits
- registers r0-r3 are used to pass first 4 arguments, remaining arguments are passed via the stack

### Return Value
- Functions will return values using r15.
