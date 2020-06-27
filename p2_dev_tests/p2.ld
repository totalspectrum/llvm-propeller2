ENTRY(_start)
SECTIONS
{
    . = 0x00100;
    .text : {
        *(.text._start);
    }

    . = 0x00400;
    .text : {
        *(.text.main);
        *(.text*);
    }

    .rodata : {
        *(.rodata*);
    }

    .bss : {
        *(.bss);
    }
}