MEMORY
{
  null (rwx): ORIGIN = 0x00001000, LENGTH = 4K
  rom (rx) : ORIGIN = 0x08013000, LENGTH = 40K
  ram (rwx) : ORIGIN = 0x20002900, LENGTH = 28K
}

/* _estack = ORIGIN(ram)+LENGTH(ram)-0x100; */

SECTIONS
  {
    .text : {
      *(.entry)
      *(.text*)        /* Program code */
      *(.rodata*)      /* Read only data */
    } >rom

    .data : {
      *(.data)        /* Data memory */
    } >ram AT >rom

  .bss : {
    *(.bss)         /* Zero-filled run time allocate data memory */
    } >ram

  .rel.plt : { *(.rel.plt)  } > rom
  .plt : { *(.plt) } > rom
  .got : { *(.got.plt) *(.got) } > ram /* relocation fixed by memory write! */
  .dynsym : { *(.dynsym) } > null
  .dynstr : { *(.dynstr) } > null

  }  

