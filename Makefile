CC = clang
#CFLAGS = -std=c2x -Werror -Wall -Wextra -pedantic
CFLAGS = -g -std=c2x -Wall -Wextra
CSRC = Qview.c
COBJ := $(CSRC:%.c=%.o)
OBJ := $(COBJ)

#DEPS = test.h

all: Qview

Qview: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -g -c $< -o $@

clean:
	rm -rf *.o Qview
