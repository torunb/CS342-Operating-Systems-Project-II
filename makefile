all: mps.c 
	gcc -Wall -g -o mps mps.c -pthread -lm

clean:
	rm -fr mps *~ output*