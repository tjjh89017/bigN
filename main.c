#include <stdio.h>
#include <stdlib.h>

#define MAX_BUF 200

struct bigN {
	unsigned long long len;
	unsigned long long *num;
};
// init to zero, and len = 1
struct bigN* bigN_init(struct bigN *a);
// set to zero, and len = 1
struct bigN* bigN_clear(struct bigN *a);
// free memory
void bigN_free(struct bigN *a);
// sign extend (for now, only bigger)
struct bigN* bigN_extend(struct bigN *a, unsigned long long len);
// parse int to bigN
struct bigN* bigN_parse_int(struct bigN *a, const int val);
// parse hex string to bigN
struct bigN* bigN_parse_hex(struct bigN *a, const char *buf);
// display hex
void bigN_display_hex(struct bigN *a, char *buf);
// a <= b (deep copy)
struct bigN* bigN_copy(struct bigN *a, struct bigN *b);
// a <= -a
struct bigN* bigN_neg(struct bigN *a);
// a <= a + b
struct bigN* bigN_add(struct bigN *a, struct bigN *b);
// a <= a + (-b)
struct bigN* bigN_sub(struct bigN *a, struct bigN *b);
// compare (a > b)
int bigN_compare(struct bigN *a, struct bigN *b);

struct bigN* bigN_init(struct bigN *a)
{
	a->len = 1ULL;
	a->num = malloc(1 * sizeof(unsigned long long));
	a->num[0] = 0ULL;
	return a;
}

void bigN_free(struct bigN *a)
{
	free(a->num);
	a->len = 0;
}

struct bigN* bigN_clear(struct bigN *a)
{
	bigN_free(a);
	return bigN_init(a);
}

struct bigN* bigN_parse_int(struct bigN *a, const int val)
{
	bigN_clear(a);

	// positive number or not
	if(val < 0){
		a->num = ~(-(unsigned long long)val) + 1;
	}
	else{
		a->num = (unsigned long long)val;
	}
	
	return a;
}

struct bigN* bigN_parse_hex(struct bigN *a, const char *buf_arg)
{
	int buf_len = strlen(buf_arg);
	int len = buf_len / 16 + 1;
	int neg_flag = 0;
	char *buf = buf_arg;

	if(buf_len > 0 && buf[0] == '-'){
		neg_flag = 1;
		buf = buf_arg + 1;
		buf_len--;
	}

	bigN_clear(a);
	bigN_extend(a, len);

	// for end to begin of buffer, convert every 16 byte HEX to 1 unsigned long long
	int i = 0;
	for(; buf_len >= 0 && i < len; buf_len -= 16, i++){
		int start = (buf_len - 16 >= 0 ? buf_len - 16 : 0);
		int end = buf_len;
		char *endptr;
		unsigned long long val = strtoull(buf + start, &endptr, 16);
		a->num[i] = val;
	}

	if(neg_flag){
		bigN_neg(a);
	}

	return a;
}

int main()
{
	struct bigN a, b;
	// input hex, i'm lazy to convert hex to dec
	char buffer[MAX_BUF + 1];
	fgets(buffer, MAX_BUF, stdin);
	bigN_parse_hex(a, buffer);
	fgets(buffer, MAX_BUF, stdin);
	bigN_parse_hex(b, buffer);

	bigN_display_hex(a, buffer);
	puts(buffer);
	bigN_display_hex(b, buffer);
	puts(buffer);

	bigN_add(a, b);
	bigN_display_hex(a, buffer);
	puts(buffer);

	return 0;
}
