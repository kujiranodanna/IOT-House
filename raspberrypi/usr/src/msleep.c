/* licence GPLv2 ; this is milliseconds to sleep by Isamu.Yamauchi 2011.6.25 update 2017.12.2 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char *argv[]){
  unsigned int msec;
  if (argc < 2) {
	printf("Usage: msleep ,miliseconds\n");
	exit (1);
  }
  msec = atoi(argv[1]);
  msleep(msec);
  exit(0);
}

int msleep(int ms)
{
	struct timeval timeout;
	timeout.tv_sec = ms / 1000;
	timeout.tv_usec = (ms % 1000) * 1000;
	if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
		perror("msleep");
		return -1;
	}
	return 0;
}
