all: baudot 

baudot: baudot.c
 $(CC) -o baudot baudot.c 

clean:
 rm -f baudot.o baudot

