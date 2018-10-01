#include <stdint.h>
typedef struct PutBitContext {
    uint32_t bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
    int size_in_bits;
} PutBitContext;

int entropic_enc(uint8_t *hops, uint8_t *bits, int line_width);
