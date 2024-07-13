
ALL: s c

s:server.cpp
	g++ $^ -o $@ -g -lpthread

c:client.cpp
	g++ $^ -o $@ -g -lpthread

clean:
	rm -rf s c


