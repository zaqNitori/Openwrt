#include <stdio.h>
#include <stdlib.h>
#include "libnet.h"

int main(int argc, char **argv) {

libnet_t *l;
char errbuf[LIBNET_ERRBUF_SIZE];

if (argc == 1 ) {
	fprintf(stderr, "Usage: %s device\n", argv[0]);
	exit(EXIT_FAILURE);
}
else {
	fprintf(stdout, "Device input is %s\n", argv[1]); 
}

l = libnet_init(LIBNET_RAW4, argv[1], errbuf);
if( l == NULL ) {
	fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
	exit(EXIT_FAILURE);
}

fprintf(stdout, "End!!!\n");
libnet_destroy(l);
return 0;
}

