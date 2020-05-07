# Propeller 2 LLVM Backend

The goal of this project is to write an LLVM backend to generate code for the Propeller 2 microcontroller. Need to do the following things from a high level:

1. ~~figure out how to add a new target machine in the LLVM environment~~
    - https://llvm.org/docs/WritingAnLLVMBackend.html#preliminaries seems to have some starting points
1. define a register format to use the COG memory register space
1. edit this machine to translate a basic program that is just main returning a constant.
    - minimum requires defining out the calling convention, load, store, and ret instruction lowering.
    - the same page as above should get us there.
    - some other resources:
        - https://jonathan2251.github.io/lbd/llvmstructure.html
        - http://llvm.org/devmtg/2009-10/Korobeynikov_BackendTutorial.pdf
        - https://llvm.org/devmtg/2014-04/PDFs/Talks/Building%20an%20LLVM%20backend.pdf
1. edit this machine to generate PASM code for basic ALU operations
1. edit this machine to support basic use of special registers (OUTx, DIRx, and INx)
1. edit this machine to add some basic startup code to start the cog and execute a blinking LED program
1. add more propeller instructions to support branching
1. implement calling conventions to call functions (especially recursion)
1. add support for starting cogs at specific memory location (with the same startup code as above)
1. expand on the rest of the propeller instruction set
1. port the necessary functions from the c standard library to make c/c++ useful.

The high level of how this will work: 
1. use clang to compile c/c++ source into LLVM's IR language. Eventually any LLVM front end should work
1. use the custom backend (the goal of this project, using MIPS, AVR, and ARC targets as references) will convert the LLVM IR code to PASM. 
1. use fastspin (or whatever Parallax's official assembler will be) to compile the assembly code into an executable elf to load. Eventually should be able to compile the PASM to and elf/binary directly from LLVM

## Cog Layout
The simplest way to make LLVM compatible with propeller is to divide the cog memory into various sections for various compiler features. This section defines that layout.

### Register Definition
Cog memory is 512 longs. The last 16 are special registers (0x1f0 - 0x1ff, so we will use the previous 16 registers (0x1e1 - 0x1ef) as general purpose registers for the compiler. So r0 = 0x1e0, r1 = 0x1e1, etc. We'll probably also need a stack pointer and maybe a link register. We'll deal with this later. 

### Stack
We'll need a stack for calling functions, etc. This will be the previous 256 longs will be a fixed stack for calls and such. Eventually maybe there should be a way to use the LUT RAM for stack space to not use up cog RAM. So, the stack starts at 0x1df and grows down to 0x0e0.

The remaining 224 longs of cog RAM should be used as a cache for loops, etc. Eventually, there should be a way to specify a function or variable to be cached in cog RAM so it never needs to be fetched. 

## Calling Convention
For starters, we will use a simple calling convention using the above registers to pass and return values for functions.

### Passing Arguments
- All 8 and 16 bit arguments are promoted to 32 bits
- registers r12-r15 are used to pass first 4 arguments, 
- remaining arguments are passed via the stack

### Return Value
- Functions will return values using r15.
