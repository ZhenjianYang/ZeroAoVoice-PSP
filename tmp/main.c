#include <stdio.h>

#define TEST 24
#define _STR(X) #X
#define STR(X) _STR(X)


int main() {
	printf("Hello \n" STR(TEST));
	return 0;
}


