#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define MAX_BUF 200
#define ULL_BITS (sizeof(unsigned long long) * 8)

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
// parse hex string to bigN
struct bigN* bigN_parse_hex(struct bigN *a, char *buf);
// display hex
void bigN_display_hex(struct bigN *a);
// display bytes in hex to debug
void bigN_display_byte(struct bigN *a);
// a <= b (deep copy)
struct bigN* bigN_copy(struct bigN *a, struct bigN *b);
// a <= -a
struct bigN* bigN_neg(struct bigN *a);
// a <= a + b
struct bigN* bigN_add(struct bigN *a, struct bigN *b);
// a <= a + 1
struct bigN* bigN_add1(struct bigN *a);
// a <= a + (-b)
struct bigN* bigN_sub(struct bigN *a, struct bigN *b);
// compare (a > b)
// int bigN_compare(struct bigN *a, struct bigN *b);

struct bigN* bigN_init(struct bigN *a)
{
	a->len = 1ULL;
	a->num = malloc(2 * sizeof(unsigned long long));
	a->num[0] = 0ULL;
	a->num[1] = 0ULL;
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

struct bigN* bigN_extend(struct bigN *a, unsigned long long len)
{
	unsigned long long *backup_num = a->num;
	unsigned long long backup_len = a->len;
	a->len = len;
	a->num = realloc(backup_num, sizeof(unsigned long long) * len);

	// sign extend
	unsigned long long sign_val = (a->num[backup_len - 1] >> (ULL_BITS - 1) ? ULLONG_MAX : 0ULL);
	for(; backup_len < a->len; backup_len++){
		a->num[backup_len] = sign_val;
	}
	return a;
}

struct bigN* bigN_parse_hex(struct bigN *a, char *buf_arg)
{
	int buf_len = strlen(buf_arg);
	int len = buf_len / 16 + 2;
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
		int start = (buf_len - 17 >= 0 ? buf_len - 17 : 0);
		int end = buf_len;
		char *endptr;
		unsigned long long val = strtoull(buf + start, &endptr, 16);
		a->num[i] = val;
		buf[start] = '\0';
	}

	if(neg_flag){
		bigN_neg(a);
	}

	return a;
}

struct bigN* bigN_neg(struct bigN *a)
{
	int i = 0;
	for(; i < a->len; i++){
		a->num[i] = ~a->num[i];
	}
	return a;
}

struct bigN* bigN_add1(struct bigN *a)
{
	// full adder, we want to add 1, set carry to 1
	unsigned long long sum = 0;
	unsigned long long carry = 1;
	unsigned long long next_carry = 0;

	int i = 0;
	for(; i < a->len; i++){
		if(a->num[i] > ULLONG_MAX - carry){
			// overflow
			next_carry = 1;
		}
		a->num[i] = a->num[i] + carry;
		carry = next_carry;
		next_carry = 0;
	}
	return a;
} 

struct bigN* bigN_add(struct bigN *a, struct bigN *b)
{
	// sign extend
	unsigned long long max_len;
	if(a->len > b->len){
		max_len = a->len;
	}
	else{
		max_len = b->len;
	}
	bigN_extend(a, max_len);
	bigN_extend(b, max_len);

	unsigned long long sum = 0;
	unsigned long long carry = 0;
	unsigned long long next_carry = 0;

	int i = 0;
	for(; i < max_len; i++){
		unsigned long long num_a = a->num[i], num_b = b->num[i];
		if(num_a == ~0ULL && num_b == ~0ULL){
			// overflow
			next_carry = 1;
		}
		else if(num_a >= num_b && a->num[i] > ULLONG_MAX - carry - b->num[i]){
			// overflow
			next_carry = 1;
		}
		else if(b->num[i] > ULLONG_MAX - carry - a->num[i]){
			// overflow
			next_carry = 1;
		}
		a->num[i] = a->num[i] + carry + b->num[i];
		carry = next_carry;
		next_carry = 0;
	}
	return a;
}

struct bigN* bigN_sub(struct bigN *a, struct bigN *b)
{
	struct bigN c;
	bigN_init(&c);
	bigN_copy(&c, b);
	bigN_neg(&c);
	bigN_add(a, &c);
	bigN_free(&c);
	return a;
}

struct bigN* bigN_copy(struct bigN *a, struct bigN *b)
{
	bigN_free(a);
	a->len = b->len;
	a->num = malloc(a->len * sizeof(unsigned long long));
	memcpy(a->num, b->num, a->len * sizeof(unsigned long long));
	return a;
}

void bigN_display_byte(struct bigN *a)
{
	int i = a->len - 1;
	printf("0x");
	for(; i >= 0; i--){
		printf("%016llx", a->num[i]);
	}
	puts("");
}

void bigN_display_hex(struct bigN *a)
{
	if(a->num[a->len - 1] >> (ULL_BITS - 1)){
		// neg
		struct bigN b;
		bigN_init(&b);
		bigN_copy(&b, a);
		bigN_neg(&b);
		printf("-");
		bigN_display_byte(&b);
		bigN_free(&b);
	}
	else{
		bigN_display_byte(a);
	}
}

int main()
{
	struct bigN a, b;
	struct bigN a1, b1;
	bigN_init(&a);
	bigN_init(&b);
	bigN_init(&a1);
	bigN_init(&b1);
	// input hex, i'm lazy to convert hex to dec
	char buffer[MAX_BUF + 1];
	fgets(buffer, MAX_BUF, stdin);
	bigN_parse_hex(&a, buffer);
	fgets(buffer, MAX_BUF, stdin);
	bigN_parse_hex(&b, buffer);

	bigN_copy(&a1, &a);
	bigN_copy(&b1, &b);

	puts("a & b");
	bigN_display_hex(&a1);
	bigN_display_hex(&b1);
	puts("");

	puts("(a - b)");
	bigN_sub(&a1, &b1);
	bigN_display_hex(&a1);

	bigN_free(&a);
	bigN_free(&b);

	return 0;
}
