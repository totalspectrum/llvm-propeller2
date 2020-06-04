ENTRY(main)
SECTIONS
{
    . = 0x00400;
    .text :
    {
        *(.text.main);
        *(.text*);
    }
}