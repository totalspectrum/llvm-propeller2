ENTRY(_entry)
SECTIONS {
    . = 0x00000;
    .text : {
        *(.text._entry);
    }

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

    .data : {
        *(.data*);
    }

    .bss : {
        *(.bss*);
    }
}