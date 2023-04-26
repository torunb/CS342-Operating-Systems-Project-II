all: mps.c 
	gcc -Wall -g -o mps mps.c -lpthread

clean:
	rm -fr mps *~ output*