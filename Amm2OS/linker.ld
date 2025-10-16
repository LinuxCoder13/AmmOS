ENTRY(_start) SECTIONS { 
    . = 0x7C00;
    .text : { *(.text) } 
    .data : { *(.data) }
    .bss : { *(.bss) }
    . = ALIGN(0x1000); 
}
