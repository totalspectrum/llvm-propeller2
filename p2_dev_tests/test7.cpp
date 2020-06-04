/* compilation:
    ../build/bin/clang -target p2 -S -c test7.cpp -emit-llvm -o test7.ll
    ../build/bin/llc -march=p2 -filetype=asm -debug test7.ll -o test7.s
    ../build/bin/llc -march=p2 -filetype=obj -debug test7.ll -o test7.o
*/

void blink1() {

    asm("dirh #56");

    while(1) {
        asm("outh #56");
        asm("augd #39062");
        asm("waitx #256");
        asm("outl #56");
        asm("augd #39062");
        asm("waitx #256");
    }
}

void blink2() {

    asm("dirh #57");

    while(1) {
        asm("outh #57");
        asm("augd #19531");
        asm("waitx #128");
        asm("outl #57");
        asm("augd #19531");
        asm("waitx #128");
    }
}

int main() {

    asm("coginit #48, %0"
                : // no outputs
                : "r" (blink1)
        );

    asm("coginit #48, %0"
                : // no outputs
                : "r" (blink2)
        );

    while(1);

    return 0;
}