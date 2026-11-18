# Host-side regression tests for the pure EXP / level-up / evolution / Rare Candy logic.
# Build and run with:  make -C tests/host
CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -O1 -I../../include

test_level_up_logic: test_level_up_logic.c ../../src/level_up_logic.c ../../include/level_up_logic.h
	$(CC) $(CFLAGS) -o $@ test_level_up_logic.c ../../src/level_up_logic.c

.PHONY: check clean
check: test_level_up_logic
	./test_level_up_logic

clean:
	rm -f test_level_up_logic

.DEFAULT_GOAL := check
