#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#define NROUNDS 20
#define KEYBUFS 256
#define SMETMAGIC 0xb17217f7d1cf79ab /* ln(2) * 2^64 */

static void encode( uint64_t data[2],
										const uint64_t RK[NROUNDS+1],
										const uint64_t C[NROUNDS+1])
{
	uint64_t X, dL, dR;
	int i;

	dL = data[0];
	dR = data[1];
	
	for(i = 0; i < NROUNDS; i += 2)
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

static void round_keys( const uint64_t key_buf[KEYBUFS],
												uint64_t RK[NROUNDS+1],
												uint64_t C[NROUNDS+1], int mode)
{
	uint64_t data[2], Ca, Cb;
	int i, rkp;

	Ca = 0;
	Cb = SMETMAGIC;

	for(i = 0; i < NROUNDS+1; ++i)
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
		rkp = (rkp + 1) % (NROUNDS + 1);
		RK[rkp] += data[1];
		rkp = (rkp + 1) % (NROUNDS + 1);
	}
	
	if(mode != 0)
	{
		uint64_t tmp;
		for(i = 0; i < NROUNDS / 2; ++i)
		{
			tmp = RK[NROUNDS - i];
			RK[NROUNDS - i] = RK[i];
			RK[i] = tmp;
			tmp = C[NROUNDS - i];
			C[NROUNDS - i] = C[i];
			C[i] = tmp;
		}
	}
}

static void passphrase(const char *p,
												uint64_t RK[NROUNDS+1], 
												uint64_t C[NROUNDS+1], int mode)
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


struct smet64
{
	uint64_t RK[NROUNDS+1];
	uint64_t C[NROUNDS+1];
};

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

void print_block(const char *pre, const char block[16])
{
	int i;
	printf("%s", pre);
	
	for(i = 0; i < 16; ++i)
	{
		printf("%02x", (unsigned char)block[i]);
	}
	puts("");
}

void print_s64(const struct smet64 *s64)
{
	int i;
	
	puts("smet64 RK[], C[]");
	for(i = 0; i < NROUNDS+1; ++i)
	{
		printf("  %02d: %016"PRIx64" %016"PRIx64"\n", i, s64->RK[i], s64->C[i]);
	}
}

int main(int c, char **v)
{
	char *p = "my secret passwd";
	char block[16], facit[16];
	struct smet64 s64;
	
	if(c > 0)
	{
		p = v[0];
	}

	memset(block, -1, sizeof(block));
	memset(facit, -1, sizeof(facit));
//	strcpy(block, "abcdefghijklmno");
//	strcpy(facit, block);
	print_block("plain: ", block);	
//	print_s64(&s64);
	smet64_decode_init(p, &s64);
	smet64_char(block, &s64);
	print_block("  dec: ", block);
	smet64_char(block, &s64);
	memset(block, -1, sizeof(block));
	smet64_encode_init(p, &s64);
	smet64_char(block, &s64);
	print_block("  enc: ", block);
//	print_s64(&s64);
	smet64_decode_init(p, &s64);
	smet64_char(block, &s64);
	print_block("  dec: ", block);
	printf("encryption/decryption verification: %s\n",
		memcmp(facit, block, sizeof(block)) ? "FAIL!" : "ok");

	return 0;
}

