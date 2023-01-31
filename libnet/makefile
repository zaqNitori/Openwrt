CC=mipsel-openwrt-linux-gcc
INCLUDE=~/openwrt/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/monk/libnet/include/
LIBRARY=~/openwrt/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/monk/libnet/lib/
header=none
target=none

make: 
	rm $(target).exe
	$(CC) $(target).c -I $(INCLUDE) -L $(LIBRARY) -lnet -o $(target).exe

