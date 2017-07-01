all: client server

%: %.cc
	g++ -std=c++11 $< -o $@

%: %.c
	gcc $< -o $@

client: client.c id.c

clean:
	rm -f client server c_*.log
