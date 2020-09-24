

CFLAGS = -Wall -Werror -g

all: build

build:
	gcc $(CFLAGS) utility.c info.c output.c myls.c -o myls

clean:
	rm -f myls

