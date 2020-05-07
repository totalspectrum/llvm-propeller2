# Propeller 2 LLVM Backend

The goal of this project is to write an LLVM backend to generate code for the Propeller 2 microcontroller. Need to do the following things from a high level:

1. ~~figure out how to add a new target machine in the LLVM environment~~
    - https://llvm.org/docs/WritingAnLLVMBackend.html#preliminaries seems to have some starting points
1. define a register format to use the COG memory register space, cache
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
1. use the custom backend (the goal of this project, based probably on the MIPS target) will convert the LLVM IR code to PASM. 
1. use fastspin (or whatever Parallax's official assembler will be) to compile the assembly code into an executable elf to load
