all: raymake run convert

raymake: vector.c plane.c sphere.c light.c ray.c
	gcc -g ray.c vector.c plane.c sphere.c light.c -lm

run:
	./a.out > a.out.log

clean:
	rm a.out
	rm a.out.log

convert:
	magick test1.ppm test1.png
