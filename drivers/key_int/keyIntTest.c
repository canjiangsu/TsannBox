#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "key_int.h"

int main(int argc, char **argv)
{
	int i;
	int ret;
	int fd;
	char cKeyVal;

	fd = open("/dev/"DEVICE_NAME, 0);
	if (fd < 0) {
		printf("can't open /dev/%s\n", DEVICE_NAME);
		return -1;
	}

	while (1) {
		ret = read(fd, &cKeyVal, sizeof(cKeyVal));
		if (ret < 0) {
			printf("read err!\n");
			continue;
		}
		printf("Read key:%d.\n", cKeyVal);
	}

	close(fd);
	return 0;
}


