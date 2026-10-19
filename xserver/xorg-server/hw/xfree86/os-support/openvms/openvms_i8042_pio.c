/* Original x86-64 instructions for the public i8042 port interface.
 * Called only through SYS$CMKRNL_64, never directly in user mode. */
#include "openvms_i8042.h"
#if !defined(__clang__) || !defined(__x86_64)
#error CLANG_X86_64_REQUIRED
#endif
uint32_t XORG_I8042_PIO(uint32_t operation, uint32_t value)
{
    uint16_t port;
    uint8_t byte;
    if (operation == XORG_I8042_READ_STATUS || operation == XORG_I8042_READ_DATA) {
        port = operation == XORG_I8042_READ_STATUS ? 0x64 : 0x60;
        __asm__ __volatile__("inb %%dx, %%al" : "=a"(byte) : "d"(port) : "memory");
        return byte;
    }
    if (value > UINT8_MAX) return UINT32_MAX;
    if (operation == XORG_I8042_WRITE_COMMAND) {
        /* Exclude reset, self-test, output-port, and interrupt injection. */
        switch (value) {
        case 0x20: case 0x60: case 0xa7: case 0xa8:
        case 0xad: case 0xae: case 0xd4: break;
        default: return UINT32_MAX;
        }
        port = 0x64;
    } else if (operation == XORG_I8042_WRITE_DATA) port = 0x60;
    else return UINT32_MAX;
    byte = (uint8_t)value;
    __asm__ __volatile__("outb %%al, %%dx" : : "a"(byte), "d"(port) : "memory");
    return 0;
}
