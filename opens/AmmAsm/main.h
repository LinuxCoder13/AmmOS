// I a sorry for this shit
void *amm_malloc(int __size) {
    if (__size <= 0) {
        return NULL;
    }

    int blocks_needed = (__size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int consec = 0;

    for (int i = 0; i < BLOCK_COUNT; ++i) {
        if (!bit_map[i]) {
            consec++;
            if (consec == blocks_needed) {
                int start = i - blocks_needed + 1;
                for (int j = start; j <= i; ++j) {
                    bit_map[j] = 1;
                }
                amm_malloc_count += 1;
                return (void*)((char*)MEMORY + (start * BLOCK_SIZE));
            }
        } 
        else {
            consec = 0;  
        }
    }
// 17 july
    return NULL;
}

void amm_free(void *ptr, int __size) {
    if (!ptr || __size <= 0) return;
    intptr_t offset = (uint8_t*)ptr - MEMORY;
    if (offset < 0 || offset >= MEMSIZE) return;

    int start = offset / BLOCK_SIZE;
    int blocks = (__size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (int i = start; i < start + blocks && i < BLOCK_COUNT; ++i) {
        bit_map[i] = 0;
    }
    amm_free_count++;
}