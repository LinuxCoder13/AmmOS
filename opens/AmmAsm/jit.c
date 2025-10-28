#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    unsigned char *buffer = malloc(size);
    if (!buffer) {
        perror("malloc");
        fclose(f);
        return 1;
    }

    fread(buffer, 1, size, f);
    fclose(f);

    void *jit = mmap(NULL, size + 1, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | 0x20, -1, 0);
    if (jit == MAP_FAILED) {
        perror("mmap");
        free(buffer);
        return 1;
    }

    memcpy(jit, buffer, size);
    ((__uint8_t *)jit)[size] = 0xC3;
    free(buffer);

    printf("Loaded %zu bytes to JIT memory, executing...\n", size);

    int (*func)() = jit;
    int result = func();
    printf("Program returned: %d\n", result);

    munmap(jit, size);
    return 0;
}
