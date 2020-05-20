/* compilation:
    ../build/bin/clang -target p2 -S -c test5.cpp -emit-llvm -o test5.ll
    ../build/bin/llc -march=p2 -filetype=asm -debug test5.ll -o test5.s
    ../build/bin/llc -march=p2 -filetype=obj -debug test5.ll -o test5.o
*/

int sum(int a, int b) {
    return a + b;
}

int c = 10;

int main() {
    // int a = 2;
    // int b = a + c;

    int d = 0;sum(2, 4);
    return d;
}