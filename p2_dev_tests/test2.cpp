/* compilation:
    ~/Github/llvm-propeller2/build/bin/clang -target mips-unknown-linux-gnu -S -c test2.cpp -emit-llvm -o test2.ll
    ~/Github/llvm-propeller2/build/bin/llc -march=p2 -filetype=asm test2.bc -o test2.s
    ~/Github/llvm-propeller2/build/bin/llc -march=p2 -filetype=obj test2.bc -o test2.o
*/

int main() {
    int a = 5;
    int b = 2;
    int c = a+b;
}

