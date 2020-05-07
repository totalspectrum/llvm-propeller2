/* compilation:
    ~/Github/llvm-propeller2/build/bin/clang -target mips-unknown-linux-gnu -c test1.cpp -emit-llvm -o test1.bc
    ~/Github/llvm-propeller2/build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm test1.bc -o test1.cpu0.s
    ~/Github/llvm-propeller2/build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj test1.bc -o test1.cpu0.o

    ~/Github/llvm-propeller2/build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm test1.bc -o test1.cpu0.s
    ~/Github/llvm-propeller2/build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj test1.bc -o test1.cpu0.o
*/

int main() {
    return 1;
}

