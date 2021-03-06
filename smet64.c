#include <inttypes.h>
#include <string.h>

#include "smet64.h"

#define KEYBUFS 256
#define SMETMAGIC 0xb17217f7d1cf79ab /* ln(2) * 2^64 */

static void encode(uint64_t data[2],
                   const uint64_t RK[SMET64_NROUNDS+1],
                   const uint64_t C[SMET64_NROUNDS+1])
{
	uint64_t X, dL, dR;
	int i;

	dL = data[0];
	dR = data[1];
	
	for(i = 0; i < SMET64_NROUNDS; i += 2)
	{
		X = RK[i] - dR;
		dL ^= ((X << 11) ^ C[i]) + (X >> 2);
		X = RK[i+1] - dL;
		dR ^= ((X << 8) ^ C[i+1]) + (X >> 9);
	}
	X = RK[i] - dR;
	dL ^= ((X << 11) ^ C[i]) + (X >> 2);

	data[0] = dL;
	data[1] = dR;
}

static void round_keys(const uint64_t key_buf[KEYBUFS],
                       uint64_t RK[SMET64_NROUNDS+1],
                       uint64_t C[SMET64_NROUNDS+1], int mode)
{
	uint64_t data[2], Ca, Cb;
	int i, rkp;

	Ca = 0;
	Cb = SMETMAGIC;

	for(i = 0; i < SMET64_NROUNDS+1; ++i)
	{
		Ca += Cb;
		RK[i] = 0;
		C[i] = Ca;
	}
	
	for(rkp = 0, i = 0; i < KEYBUFS; i += 2)
	{
		data[0] = key_buf[i];
		data[1] = key_buf[i+1];
		encode(data, RK, C);
		RK[rkp] += data[0];
		rkp = (rkp + 1) % (SMET64_NROUNDS + 1);
		RK[rkp] += data[1];
		rkp = (rkp + 1) % (SMET64_NROUNDS + 1);
	}
	
	if(mode != 0)
	{
		uint64_t tmp;
		for(i = 0; i < SMET64_NROUNDS / 2; ++i)
		{
			tmp = RK[SMET64_NROUNDS - i];
			RK[SMET64_NROUNDS - i] = RK[i];
			RK[i] = tmp;
			tmp = C[SMET64_NROUNDS - i];
			C[SMET64_NROUNDS - i] = C[i];
			C[i] = tmp;
		}
	}
}

static void passphrase(const char *p,
                       uint64_t RK[SMET64_NROUNDS+1],
                       uint64_t C[SMET64_NROUNDS+1], int mode)
{
	uint64_t kb, key_buf[KEYBUFS];
	int pp, pl, i, j;
	unsigned char pb;
	
	kb = pl = strlen(p) + 1;

	for(pp = 0, i = 0; i < KEYBUFS; ++i)
	{
		key_buf[i] = kb;
		for(j = 0; j < 8; ++j)
		{
			pb = p[pp];
			kb = (kb << 8) | pb;
			pp = (pp + 1) % pl;
		}
	}
	
	round_keys(key_buf, RK, C, mode);
}

void smet64_encode_init(const char *p, struct smet64 *s64)
{
	passphrase(p, s64->RK, s64->C, 0);
}

void smet64_decode_init(const char *p, struct smet64 *s64)
{
	passphrase(p, s64->RK, s64->C, 1);
}

void smet64_block64(uint64_t block64[2], struct smet64 *s64)
{
	encode(block64, s64->RK, s64->C);
}

/* YAGNI...

void smet64_block32(uint32_t block32[4], struct smet64 *s64)
{
	uint64_t block64[2];
	
	block64[0] = block32[0];
	block64[0] = (block64[0] << 32) | block32[1]; 
	block64[1] = block32[2];
	block64[1] = (block64[1] << 32) | block32[3];
	encode(block64, s64->RK, s64->C);
	block32[0] = block64[0] >> 32;
	block32[1] = block64[0] & 0xffffffff;
	block32[2] = block64[1] >> 32;
	block32[3] = block64[1] & 0xffffffff;	
}

*/

void smet64_char(char block[16], struct smet64 *s64)
{
	uint64_t block64[2];
	int i, j;

	for(i = 0; i < 2; ++i)
	{
		unsigned char *b = (unsigned char *)&block[8 * i];
		uint64_t b64 = 0;
		for(j = 0; j < 8; ++j)
		{
			b64 = (b64 << 8) | (b[j] & 0xff);
		}
		block64[i] = b64;
	}	
	encode(block64, s64->RK, s64->C);
	for(i = 0; i < 2; ++i)
	{
		char *b = &block[8 * i];
		uint64_t b64 = block64[i];
		for(j = 7; j >= 0; --j)
		{
			b[j] = (b64 & 0xff);
			b64 >>= 8;
		}
	}	
}

