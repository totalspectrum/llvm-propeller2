/* compilation:
    ../build/bin/clang -target p2 -S -c test6.cpp -emit-llvm -o test6.ll
    ../build/bin/llc -march=p2 -filetype=asm -debug test6.ll -o test6.s
    ../build/bin/llc -march=p2 -filetype=obj -debug test6.ll -o test6.o
*/

int x = 0;

int main() {

    while(1) {
        asm("dirh #56");
        asm("augd #156250");
        asm("waitx #0");
        asm("dirl #56");
        asm("augd #156250");
        asm("waitx #0");
    }
    return 0;
}