------
Notes:
------
* This is the default linker script set
* Code will be placed in Flash Bank A as it is at the top of the memory map

-----------
Memory Map:
-----------
MFlashA512 (rx) : ORIGIN = 0x1a000000, LENGTH = 0x80000 /* 512K bytes */
MFlashB512 (rx) : ORIGIN = 0x1b000000, LENGTH = 0x80000 /* 512K bytes */
RamLoc32 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x8000 /* 32K bytes */
RamLoc40 (rwx) : ORIGIN = 0x10080000, LENGTH = 0xa000 /* 40K bytes */
RamAHB32 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000 /* 32K bytes */
RamAHB16 (rwx) : ORIGIN = 0x20008000, LENGTH = 0x4000 /* 16K bytes */
RamAHB_ETB16 (rwx) : ORIGIN = 0x2000c000, LENGTH = 0x4000 /* 16K bytes */