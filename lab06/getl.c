#include <stdio.h>
#include <stdlib.h>

int main() {
	for(int iteration = 0; iteration < 10; iteration++) {
		char *str = NULL;
		size_t len = 0;
		getline(&str, &len, stdin);
		printf("%ld: %s", len, str);
		free(str);
	}
	return 0;
}
