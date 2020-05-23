/* compilation:
    ../build/bin/clang -target p2 -S -c test6.cpp -emit-llvm -o test6.ll
    ../build/bin/llc -march=p2 -filetype=asm -debug test6.ll -o test6.s
    ../build/bin/llc -march=p2 -filetype=obj -debug test6.ll -o test6.o
*/

int main() {

    asm("dirh #56");

    return 0;
}