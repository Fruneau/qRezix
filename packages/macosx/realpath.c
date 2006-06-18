#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	char absolutePath[PATH_MAX];
	if (argc != 2) {
		printf("Usage: realpath path\n");
		return 1;
	}

	realpath(argv[1], absolutePath);
	printf("%s\n", absolutePath);
	return 0;
}
