
ENTRY(_start)  

MEMORY {
    ram (rw) : ORIGIN = 0x80000000, LENGTH = 64K
}

SECTIONS {
 
    .text : {
        *(.text)
    } > ram


    .data : {
        *(.data)
    } > ram


    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    } > ram


    .stack : {
        __stack_start = .;
        . += 16K;  // 栈大小16KB
        __stack_end = .;
    } > ram
}