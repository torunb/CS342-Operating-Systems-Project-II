all: mps.c 
	gcc -Wall -g -o mps mps.c -pthread -lm
	gcc -Wall -g -o mps_cv mps_cv.c -pthread -lm

clean:
	rm -fr mps *~ output*
	rm -fr mps_cv *~ output*