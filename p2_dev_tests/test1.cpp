/* compilation:
    ~/Github/llvm-propeller2/build/bin/clang -target mips-unknown-linux-gnu -S -c test1.cpp -emit-llvm -o test1.ll
    ~/Github/llvm-propeller2/build/bin/llc -march=p2 -filetype=asm test1.bc -o test1.s
    ~/Github/llvm-propeller2/build/bin/llc -march=p2 -filetype=obj test1.bc -o test1.o
*/

int main() {
    int a = 5;
    int b = 2;

    int c = a + b;      // c = 7
    //int d = b + 1;      // d = 3

    return (c);
}

