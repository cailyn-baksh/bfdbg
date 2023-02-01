NAME = bfdbg
VERSION = v0.0.1
SRCS = $(filter-out %.swp,$(wildcard src/*))
OBJS = $(addsuffix .o,$(patsubst src/%,bin/%,$(SRCS)))
INCLUDES = include/
LIBS = ncurses

CC = clang
CFLAGS = -std=gnu2x -Os -Wall

debug: CFLAGS += -DDEBUG -g
debug: all

all: $(OBJS)
	$(CC) -o bin/$(NAME) $^ $(addprefix -l,$(LIBS))

bin/%.c.o: src/%.c | bin
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c -o $@ $^

bin:
	mkdir -p $@

clean:
	find bin/* -type f -delete

.PHONY: build clean FORCE
FORCE:
