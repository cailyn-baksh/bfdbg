NAME = bfdbg
VERSION = v0.0.1
SRCS = $(filter-out %.swp,$(wildcard src/*))
OBJS = $(addsuffix .o,$(patsubst src/%,bin/%,$(SRCS)))
INCLUDES = include/
LIBS ?= ncurses

ifeq ($(origin CC),default)
CC = clang
endif

ifeq ($(origin LD),default)
LD = lld
endif

CFLAGS ?= -std=gnu2x -O3 -flto -Wall -Wextra

all: $(OBJS)
	$(CC) -o bin/$(NAME) -fuse-ld=$(LD) $^ $(addprefix -l,$(LIBS))

debug: CFLAGS += -DDEBUG -g
debug: all

bin/%.c.o: src/%.c | bin
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c -o $@ $^

bin:
	mkdir -p $@

clean:
	find bin/* -type f -delete

.PHONY: build clean FORCE
FORCE:
