make: sha1.c
	gcc sha1.c -o sha1 -lbsd

.PHONY: clean
clean:
	rm -f sha1
