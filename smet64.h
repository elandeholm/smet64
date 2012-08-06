#define SMET64_NROUNDS 20

struct smet64
{
	uint64_t RK[SMET64_NROUNDS+1];
	uint64_t C[SMET64_NROUNDS+1];
};

void smet64_encode_init(const char *p, struct smet64 *s64);
void smet64_decode_init(const char *p, struct smet64 *s64);
void smet64_block64(uint64_t block64[2], struct smet64 *s64);
void smet64_char(char block[16], struct smet64 *s64);

