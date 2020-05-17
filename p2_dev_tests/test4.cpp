/* compilation:
    ../build/bin/clang -target p2 -S -c test4.cpp -emit-llvm -o test4.ll
    ../build/bin/llc -march=p2 -filetype=asm -debug test4.ll -o test4.s
    ../build/bin/llc -march=p2 -filetype=obj -debug test4.ll -o test4.o
*/

void test() {
    int a = 0;
    int b = 3;
    for (int i = 0; i < 25; i++) {
        if (i % b)
            a = i-b;
    }
}