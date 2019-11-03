#Aaron Steiner

#Webproxy Makefile

all: webproxy.c
	gcc webproxy.c -o webproxy -pthread

clean:
	$(RM) webproxy