all: baudot 

baudot: baudot.c
	$(CC) -o baudot baudot.c 

uppt: uppt.c
	$(CC) -o uppt uppt.c

clean:
	rm -f baudot.o baudot

