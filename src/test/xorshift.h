#ifndef _XORSHIFT_H
#define _XORSHIFT_H

/* From https://en.wikipedia.org/wiki/Xorshift */


#define mixseed(state, x) state[3] = x; xorshift128(state)

static inline uint32_t xorshift128(uint32_t state[static 4])
{
	uint32_t t = state[3];

	t ^= t << 11;
	t ^= t >> 8;
	state[3] = state[2]; state[2] = state[1]; state[1] = state[0];
	t ^= state[0];
	t ^= state[0] >> 19;
	state[0] = t;

	return t;
}

static inline uint64_t xorshift64star(uint64_t state[static 1])
{
	uint64_t x = state[0];

	x ^= x >> 12; // a
	x ^= x << 25; // b
	x ^= x >> 27; // c
	state[0] = x;

	return x * 0x2545F4914F6CDD1D;
}

static inline unsigned long strhash(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
#endif
