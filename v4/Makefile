all: remotecmd.bin remotecmdc.bin
remotecmd.bin: remotecmd.c
	gcc -o remotecmd.bin remotecmd.c
remotecmdc.bin: remotecmdc.c
	gcc -o remotecmdc.bin remotecmdc.c
clean:
	rm remotecmd.bin remotecmdc.bin