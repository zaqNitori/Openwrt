CC=mipsel-openwrt-linux-gcc
NET_INC=~/openwrt/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/monk/libnet/include/
NET_LIB=~/openwrt/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/monk/libnet/lib/
PCAP_INC=~/openwrt/build_dir/target-mipsel_24kc_musl/libpcap-1.9.1/
PCAP_LIB=~/openwrt/build_dir/target-mipsel_24kc_musl/libpcap-1.9.1/
header=none
target=none
ext=exe
des_ip=192.168.1.183
des_path=~/workspace/

make: 
	rm -rf $(target).exe
	$(CC) $(target).c -I $(NET_INC) -L $(NET_LIB) -lnet -I $(PCAP_INC) -L $(PCAP_LIB) -o $(target).exe

send:
	scp $(target).$(ext) root@$(des_ip):$(des_path)


