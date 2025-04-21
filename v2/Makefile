all: pings.o pingc.o tunnels tunnelc time
pings.o: pings.c
	gcc -o pings pings.c
pingc.o: pingc.c
	gcc -o pingc pingc.c
tunnels: tunnels.c
	gcc -o tunnels tunnels.c
tunnelc: tunnelc.c
	gcc -o tunnelc tunnelc.c
time:
	@echo "-----------------------------------"
	@echo "| $(shell date) |"
	@echo "-----------------------------------"
clean:
	rm pings pingc tunnels tunnelc