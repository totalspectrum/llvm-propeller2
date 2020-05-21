/* compilation:
    ../build/bin/clang -target p2 -S -c test5.cpp -emit-llvm -o test5.ll
    ../build/bin/llc -march=p2 -filetype=asm -debug test5.ll -o test5.s
    ../build/bin/llc -march=p2 -filetype=obj -debug test5.ll -o test5.o
*/

int sum(int a, int b, int c, int d, int e) {
    return a + b + c + d + e;
}

int main() {
    int d = sum(1, 2, 3, 4, 5);
    return d;
}