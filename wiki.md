# Propeller 2 LLVM Backend

The goal of this project is to write an LLVM backend to generate code for the Propeller 2 microcontroller. Need to do the following things from a high level (in no particular order)

1. ~~figure out how to add a new target machine in the LLVM environment~~
    - https://llvm.org/docs/WritingAnLLVMBackend.html#preliminaries seems to have some starting points
1. ~~define a register format to use the COG memory register space~~ (see below)
1. ~~edit this machine to translate a basic program that is just main returning a constant.~~
    - minimum requires defining out the calling convention, ~~load~~, ~~store~~, and ~~ret instruction lowering~~.
    - the same page as above should get us there.
    - some great resources (though on opposite ends of the spectrum of sparse and dense):
        - https://jonathan2251.github.io/lbd/llvmstructure.html
        - https://llvm.org/devmtg/2012-04-12/Slides/Workshops/Anton_Korobeynikov.pdf
        - https://llvm.org/devmtg/2014-04/PDFs/Talks/Building%20an%20LLVM%20backend.pdf
1. ~~edit this machine to generate PASM code for basic ALU operations~~
    - expand to do all math instructions and provide custom nodes for operations propeller doesn't natively support
1. ~~edit this machine to support basic use of special registers (OUTx, DIRx, and INx)~~
1. ~~edit this machine to add some basic startup code to start the cog and execute a blinking LED program~~
    - what as actually needed here was to implement the linker backend and write a simple linker script, p2.ld
1. ~~add more propeller instructions to support branching~~
1. implement calling conventions to call functions (especially recursion)
    - ~implement passing arguments by registers~
    - ~implement passing arguments by stack~
    - ~implement passing byval arguments (structs and classes)~
    - implement variable argument functions
        - I think this will require spooling up the C standard library
1. add support for starting cogs at specific memory location
    - ~~implement basic cog starting for cogs that do not require a stack~~
    - implement including setq to pass the stack pointer and then assign the stack pointer to that value. Some startup code will be needed.
1. expand on the rest of the propeller instruction set
1. create clang extensions for special directives and being able to write directly to I/O regsiters, etc.
1. port the necessary functions from the c standard library to make c/c++ useful.
    - This can be done using llvm-clib, implementing the necessary functions for P2

The high level of how this will work:
1. use clang to compile c/c++ source into LLVM's IR language. Eventually any LLVM front end should work
1. use the custom backend (the goal of this project, using MIPS, AVR, and ARC targets as references) will convert the LLVM IR code to PASM.
1. ~~use fastspin (or whatever Parallax's official assembler will be) to compile the assembly code into an executable elf to load.~~ Compile the PASM to an elf/binary directly from LLVM using clang and lld.

In principle, all the plumbing should already exist to run clang with the correct arguments and get an loadable binary out.

## Why are you writing yet another compiler for the Propeller 2? Fastspin works great

Yes, fastspin is great, but we need more.

Propeller (and Propeller 2) are power chips that can do A LOT of a small, simple, and power efficient package. The high flexibility allows it to be used in a very wide variety of applications without having to include a lot of support hardware. As such, it should be used widely in industry, but it's not. I think there are several reasons for its lack of adoption, but one of the biggest ones is the lack of a modern toolchain and lack of modern language support. Propeller 1 addressed this with PropGCC, but it was several years after the release of Propeller 1 and built around GCC 4, which is outdated in the modern world. Additionally, there appears to be a game of chicken going on between Parallax and the Propeller community, where Parallax is focused on Spin and the development of Propeller hardware, so they are hoping the community steps up (again) and develops the tools they desire, while the community is hoping to see something official come out and not put too much effort into developing something that might be pushed aside by an "official" toolchain. As a result, we have a few toolchain that are not quite good enough by industry standards (sorry to those who work on them, I know these tools take a lot of work and I do appreciate all the work that has been put in so far) that don't fully support C/C++ (like fastspin), and some that do have full C++ support, but do not support the full functionality of Propeller hardware (like RISCV-P2), and some in between, like p2gcc, which is more or less a bandaid for make use of PropGCC for Propeller 2 (p2gcc also doesn't support the most "standard" P2 library which makes code developed with it not very portable). While these are excellent tools to demonstrate the capabilities of the hardware, they make developing scalable products difficult if not impoosible. There has also been several requests for various language support (microPython, Arduino, Rust, etc), all of which will require developing a compiler for Propeller's architecture.

This project aims to solve all of the problems listed above. LLVM is a modern toolchain used by many companies around the world, developed and supported primarily by Apple at this point. It has an intermediate representation that frontends (such as clang for C/C++/Objective C) compile down to, and target specific backends the compile the IR down to target machine instructions. The majority of the work to of the backend is baked into LLVM as it, and the P2 target is another backend (same as the existing x86, AArch64, MIPS, AVR, etc etc backends) that provides basic information (such as regsiters, instruction encoding, and ABI information) to connect the dots between the various compiler passes that LLVM does. Once complete, it will provide access to the full functionality of several langauges for Propeller.

I am developing this project with two main goals in mind:
1. create as much backward compatability as possible with PropGCC projects. This won't be completely possible due to a few differences, but the hope is that porting those P1 projects to P2 will be easy.
2. provide a tool that the community finds useful, regardless of adoption by Parallax as a formal tool. I know there's been some gripe on forums about the community's hard work not being adopted as much as people hope, but I am not pushing this for formal adoption. We'll just see what happens.

## Getting Started
See README.md, with the following notes:
- when running `cmake`, run `cmake -G "Unix Makefiles" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=P2 ../llvm`
- building for the first time will take quite a bit of time, ~20 min.
- to run one of the examples in p2_dev_tests, run in two steps (from p2_dev_tests/)
    - `../build/bin/clang -target mips-unknown-linux-gnu -S -c <file>.cpp -emit-llvm -o <file>.ll` (Eventually need to create a P2 target in clang). This compiles C down to LLVM IR language.
    - `../build/bin/llc -march=p2 -debug -filetype=asm <file>.ll -o test2.s`. This will output an assembly file with P2 instructions. There's still work to be done to clean it up into something compilable with fastspin. Eventually should be able to compile PASM directly to objects to remove the need for fastspin entirely.
- The presentation by Anton Korobeynikov above gives a pretty high level but informative overview of how a backend works and the various components. This along with the other documents linked above help form a complete picture of how stuff works.
- This has all the definitions for instruction info creation: https://github.com/llvm-mirror/llvm/blob/master/include/llvm/Target/TargetSelectionDAG.td

## Cog Layout
The simplest way to make LLVM compatible with propeller is to divide the cog memory into various sections for various compiler features. This section defines that layout.

### Register Definition
Cog memory is 512 longs. The last 16 are special registers (0x1f0 - 0x1ff, so we will use the previous 16 registers (0x1e0 - 0x1ef) as general purpose registers for the compiler. So r0 = 0x1e0, r1 = 0x1e1, etc. We'll also need a stack pointer and maybe a link register. We'll deal with this later. 2 more registers will be reserved for this.

### Stack
We'll need a stack for calling functions, etc. The first 256 longs of the look-up RAM will be a fixed stack for calls and such. So, the stack starts at 0x0 and grows up to 0x0ff. Initially we'll just use HUB RAM so that a typical architecture's load/store instructions can translate directly to rdlong/wrlong for memory read/writes, but eventually we'll add distinction between memories so that stack operatiosn use rdlut/wrlut, and normal memory operations needing the hub RAM will use rdlong/wrlong.

The stack is organized starting at address 0x00000 (since the cog can't exectute from that hub address so it doesn't take up program space to use it) and can continue to 0x003ff before begining to use program space. The stack will always grow up and each frame grows up within the stack. We won't have a distinct frame pointer, in each frame we should read in the current stack pointer and offset it locally to the desired stack slot.

The remaining cog RAM should be used as a cache for loops, common functions, any math functions we want a fast implementation of, etc. Eventually, there should be a way to specify a function or variable to be cached in cog RAM so it never needs to be fetched from the hub. This will likely need a clang extension.

## Calling Convention
For starters, we will use a simple calling convention using the above registers to pass and return values for functions.

### Passing Arguments
- All 8 and 16 bit arguments are promoted to 32 bits
- registers r0-r3 are used to pass first 4 arguments, remaining arguments are passed via the stack
- byval arguments will be passed via the stack

### Return Value
- Functions will return values using r15.
- byvals will be returned via the stack

When calling a function, the caller will increment sp to allocate space for the arguments that require stack space. It will then call the function (using propeller's CALL instruction) (TBD on how this will affect recursive calls, will probably reuqire implementing a link register that gets stored on the software stack). The callee will allocate the stack space it needs for it's local variables and return values. This it will do it's stuff and before returning (using the RET instruction), it will de-allocate the stack space.

## Program and memory organization

This section describes how a program should be organized in hub memory. Below is a simple diagram.

| 0x00000                | 0x00200     | 0x00400                        | 0x7fffff (or 0x7cffff?)          |
|------------------------|-------------|--------------------------------|----------------------------------|
| Re-usable startup code | Cog 0 stack | Start of generic program space | End of memory (on Rev B silicon) |

Hub memory 0x00000-0x003ff will be used as stack space for the cog 0 cog, as well as contain the startup code needed (like setting up the UART interface, etc). we start at 0x400 since that's the first address than can be used for hub executiong. Starting a new cog requires setting PTRA (using SETQ) to the start of the stack space to use for that cog. The first stack slot should store the pointer to the function to run, the second stack slot should store an optional parameter that will be the first argument into the function. A new cog should always start by copying the startup code at 0x00000 and executing it

The end of the memory space (not yet described in detail) will be bss/rodata for static data and heap space for dynamic allocation, but dynamic allocation should be kept to a minimum (as with most embedded systems).

### Startup code

Right now, the only thing the startup code does is setup the stack pointer for the cog and jump to the starting function of the cog. If the cog is cog 0, this is hardcoded to set `sp` to 0x00200 and jump to 0x00400. Otherwise, it pulls the initial stack pointer value and jump location from PTRA.

## Linking/Loading Programs

The linker script (p2.ld) does 2 things:
1. place `main()` at the start of .text, before anything else
1. place `_start()` at 0x00000 so that it is the first thing that executes
1. place .text at 0x00400, the first hub execution mode address.

The resulting elf can use Eric Smith's loadp2 (which supports elf binaries) and can be loaded onto the P2 Eval Board.
