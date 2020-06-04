/* compilation:
    ../build/bin/clang -target p2 -S -c test6.cpp -emit-llvm -o test6.ll
    ../build/bin/llc -march=p2 -filetype=asm -debug test6.ll -o test6.s
    ../build/bin/llc -march=p2 -filetype=obj -debug test6.ll -o test6.o
*/

void blink() {
    asm("outh #56");
    asm("augd #39062");
    asm("waitx #256");
    asm("outl #56");
    asm("augd #39062");
    asm("waitx #256");
}

int main() {

    asm("dirh #56");

    while(1) {
        blink();
    }
    return 0;
}