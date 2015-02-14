#include <stdlib.h>
#include <string.h>

#include "transform.h"
#include "udpmask.h"

#define mask_unit       unsigned long long
#define MASK_UNIT_LEN   ((int) sizeof(mask_unit))

unsigned char *mask = NULL;
size_t mask_len = 0;
int mask_loaded = 0;

int load_mask(const char *smask)
{
    int smask_len = strlen(smask);

    if (smask_len < 1) {
        return -1;
    }

    int smask_len_mul = smask_len / MASK_UNIT_LEN;

    if (smask_len_mul * MASK_UNIT_LEN < smask_len) {
        smask_len_mul++;
    }

    mask_len = smask_len_mul * MASK_UNIT_LEN;

    mask = (unsigned char *) malloc(mask_len);

    for (size_t i = 0; i < mask_len; i++) {
        mask[i] = (unsigned char) smask[i % smask_len];
    }

    mask_loaded = 1;

    return 0;
}

void unload_mask(void)
{
    free(mask);
    mask = NULL;
    mask_loaded = 0;
}

int transform(__attribute__((unused)) enum um_mode mode,
              const unsigned char *buf, size_t buflen,
              unsigned char *outbuf, size_t *outbuflen,
              int tlimit)
{
    size_t bufplen;

    if (tlimit < 0) {
        bufplen = buflen;
    } else {
        bufplen = (size_t) tlimit < buflen ? (size_t) tlimit : buflen;
    }

    // Mask

    size_t bufplen_mul = bufplen / MASK_UNIT_LEN;
    size_t mask_len_mul = mask_len / MASK_UNIT_LEN;

    for (size_t i = 0; i < bufplen_mul; i++) {
        ((mask_unit *) outbuf)[i] = ((mask_unit *) buf)[i] ^
                                    ((mask_unit *) mask)[i % mask_len_mul];
    }

    for (size_t i = bufplen_mul * MASK_UNIT_LEN; i < bufplen; i++) {
        outbuf[i] = buf[i] ^ mask[i % mask_len];
    }

    // Copy

    if (buf == outbuf) {
        return 0;
    }

    size_t copylen = buflen - bufplen;

    if (copylen > 0) {
        memcpy((void *) (outbuf + bufplen), (void *) (buf + bufplen), copylen);
    }

    *outbuflen = buflen;

    return 0;
}
