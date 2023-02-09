#include <stdio.h>
#include <stdlib.h>
#include <libnet.h>
#include <stdint.h>

int main()
{
    libnet_t *l;
    char errbuf[LIBNET_ERRBUF_SIZE];
    char payload[] = "hello libnet :D";
//    u_int8_t mac_broadcast_addr[6] = {0x40, 0xee, 0x15, 0xd0, 0x43, 0x94};
    u_int8_t mac_broadcast_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  u_int8_t src_addr[6];
    struct libnet_ether_addr *src_mac_addr;
    int bytes_written, i;

    l = libnet_init(LIBNET_LINK, "wlan0", errbuf);
    if (l == NULL) {
        fprintf(stderr, "libnet_init () failed: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    src_mac_addr = libnet_get_hwaddr(l);
    if (src_mac_addr == NULL) {
        fprintf(stderr, "Couldn't get own IP address: %s\n",libnet_geterror(l));
        libnet_destroy(l);
        exit(EXIT_FAILURE);
    }

    printf("My MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",\
        src_mac_addr->ether_addr_octet[0],\
        src_mac_addr->ether_addr_octet[1],\
        src_mac_addr->ether_addr_octet[2],\
        src_mac_addr->ether_addr_octet[3],\
        src_mac_addr->ether_addr_octet[4],\
        src_mac_addr->ether_addr_octet[5]);

	memcpy(src_addr, src_mac_addr, ETHER_ADDR_LEN);
    
//    if( libnet_build_ethernet(mac_broadcast_addr, src_addr, ETHERTYPE_LOOPBACK, (uint8_t*)payload, sizeof(payload), l, 0)) {
    if (libnet_autobuild_ethernet(mac_broadcast_addr, ETHERTYPE_LOOPBACK, l) == -1) {
        fprintf(stderr, "Error building Ethernet header: %s\n", libnet_geterror(l));
        libnet_destroy(l);
        exit(EXIT_FAILURE);
    }

    bytes_written = libnet_write(l);
    if(bytes_written != -1) {
        printf("%d bytes written.\n", bytes_written);
    }
    else {
        fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
    }

    libnet_destroy(l);
    return 0;
}
