CFLAGS += -Wall -Wextra -O3 -g3

all:	ustardict

ustardict:
	$(CC) $(CFLAGS) ustardict.c -o ustardict

clean:
	rm -vf *~ *.o *.out ustardict
