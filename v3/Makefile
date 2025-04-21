all: tunnels tunnelc time
tunnels: tunnels.c
	gcc -o tunnels tunnels.c
tunnelc: tunnelc.c
	gcc -o tunnelc tunnelc.c
time:
	@echo "-----------------------------------"
	@echo "| $(shell date) |"
	@echo "-----------------------------------"
clean:
	rm tunnels tunnelc