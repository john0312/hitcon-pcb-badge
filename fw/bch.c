#include <stdio.h>
#include <stdlib.h>

#define n 15 // codeword length
#define k 7 // information bits
#define t 2 // correcting capability

#define p1 0b10011u
#define p2 0b11111u
#define g  0b111010001u

typedef unsigned int u16;
typedef struct div_res{
	u16 q, r;
}div_res;

static u16 add(u16 a, u16 b){
	return a ^ b;
}

static u16 minus(u16 a, u16 b){
	return a ^ b;
}

static u16 mul(u16 a, u16 b){
	u16 res = 0;
	for(int i=0;i<n;i++){
		if(b & (1<<i))
			res = add(res, a<<i);
	}
	return res;
}

static div_res divide(u16 a, u16 b){
	// a = b * q + r
	div_res res = (div_res){
		.q = 0,
		.r = 0,
	};
	u16 high = n;
	for(;high>=0;high--) if(b & (1<<high)) break;

	for(int i=n-high;i>=0;i--){
		if(a & (1<<(i+high))) {
			res.q |= 1<<i;
			a = minus(a, b<<i);
		}
	}
	res.r = a;
	return res;
}

u16 enc(u16 data){
	return mul(data, g);
}

static u16 ror(u16 x, u16 r){
	u16 mask = (1<<r)-1;
	return ((x & mask) << (16-r)) | (x>>r);
}

static u16 rol(u16 x, u16 r){
	return ror(x, (16-r)&0xf);
}

u16 dec(u16 cipher){
	div_res res = divide(cipher, g);
	if (!__builtin_popcount(res.r)) {
		return res.q;
	}

	u16 cnt = 0, recd = cipher;
	while(__builtin_popcount(res.r) > t){
		if(cnt > n) // fail
			return 0;
		recd = rol(recd, 1);
		res = divide(recd, g);
		cnt++;
	}
	recd = add(recd, res.r);
	recd = ror(recd, cnt);
	return divide(cipher, g).q;
}


#if 0
static void print_bin(u16 x){
	for(int i=15;i>=0;i--) printf("%c", ((1<<i)&x) ? '1': '0');
	printf("\n");
}

int main(){
	print_bin(divide(0b1000u, 0b10u).q);
	print_bin(divide(0b1000u, 0b10u).r);

	u16 data = 0b1101000u;
	print_bin(data);
	u16 cipher = enc(data);
	print_bin(cipher);
	u16 error = 0b10000001;
	cipher ^= error;

	u16 recd = dec(cipher);
	print_bin(recd);
}
#endif