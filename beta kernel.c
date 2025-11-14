#include <bits/libc-header-start.h>
#define VGA_BUFFER ((volatile char*)0xb8000)
#define VGA_COLS 80
#define VGA_ROWS 25

void print(const char *str);
void print_nl();
char getchar();
void shell();
unsigned char inb(unsigned short port);
int strcmp(const char *a, const char *b);

// Minimal ASCII map for US QWERTY (expand for full support)
char scancode_ascii[128] = {
    0, 27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,0,0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static int cursor = 0;

void print(const char *str) {
    while (*str) {
        if (*str == '\n') {
            print_nl();
        } else {
            VGA_BUFFER[cursor * 2] = *str;
            VGA_BUFFER[cursor * 2 + 1] = 0x0F;
            cursor++;
        }
        str++;
    }
}

void print_nl() {
    cursor += VGA_COLS - (cursor % VGA_COLS);
}

char getchar() {
    unsigned char status, scancode;
    while (1) {
        do {
            status = inb(0x64);
        } while ((status & 1) == 0);
        scancode = inb(0x60);
        if (scancode < 128 && scancode_ascii[scancode]) {
            char c = scancode_ascii[scancode];
            if (c == '\n' || (c >= 32 && c <= 126))
                return c;
        }
    }
}

void shell() {
    char cmd[64];
    int cmd_i;
    print("Chocolate Shell OS\nType 'help', 'about', or 'echo <text>'\n\n");
    while (1) {
        print("> ");
        cmd_i = 0;
        while (1) {
            char c = getchar();
            if (c == '\n') {
                break;
            } else if ((c == '\b' || c == 127) && cmd_i > 0) {
                if (cursor > 0) cursor--;
                VGA_BUFFER[cursor * 2] = ' ';
                VGA_BUFFER[cursor * 2 + 1] = 0x0F;
                cmd_i--;
            } else if (cmd_i < (int)sizeof(cmd) - 1 && c >= 32 && c <= 126) {
                VGA_BUFFER[cursor * 2] = c;
                VGA_BUFFER[cursor * 2 + 1] = 0x0F;
                cursor++;
                cmd[cmd_i++] = c;
            }
        }
        cmd[cmd_i] = 0;
        print_nl();

        if (!strcmp(cmd, "help")) {
            print("Available: help, echo, about\n");
        } else if (!strcmp(cmd, "about")) {
            print("Chocolate OS Shell 2025!\n");
        } else if (!strncmp(cmd, "echo ", 5)) {
            print(cmd+5); print("\n");
        } else if (cmd[0] == 0) {
            // do nothing for empty
        } else {
            print("Unknown command.\n");
        }
    }
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}
int strncmp(const char *a, const char *b, int n) {
    while (n-- && *a && *b && (*a == *b)) { a++; b++; }
    return (n>=0) ? (*(unsigned char*)a - *(unsigned char*)b) : 0;
}

void kernel_main() {
    for (int i = 0; i < VGA_COLS * VGA_ROWS; ++i) {
        VGA_BUFFER[i * 2] = ' ';
        VGA_BUFFER[i * 2 + 1] = 0x07;
    }
    cursor = 0;
    shell();
}
