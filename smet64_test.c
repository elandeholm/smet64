#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "smet64.h"

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
	for(i = 0; i < SMET64_NROUNDS+1; ++i)
	{
		printf("  %02d: %016"PRIx64" %016"PRIx64"\n", i, s64->RK[i], s64->C[i]);
	}
}

int main(int c, char **v)
{
	char *p = "my secret passwd";
	char block[16], facit[16];
	struct smet64 s64;
	
	if(c > 1)
	{
		p = v[1];
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

