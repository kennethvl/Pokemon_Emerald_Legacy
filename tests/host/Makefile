# Host-side regression tests for the pure game-rule logic (EXP / level-up / evolution / Rare Candy and
# the Pokémon Center Mom modifier services). Build and run with:  make -C tests/host
CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -O1 -I../../include

TESTS := test_level_up_logic test_mon_modifier_logic

test_level_up_logic: test_level_up_logic.c ../../src/level_up_logic.c ../../include/level_up_logic.h
	$(CC) $(CFLAGS) -o $@ test_level_up_logic.c ../../src/level_up_logic.c

test_mon_modifier_logic: test_mon_modifier_logic.c ../../src/mon_modifier_logic.c ../../include/mon_modifier_logic.h
	$(CC) $(CFLAGS) -o $@ test_mon_modifier_logic.c ../../src/mon_modifier_logic.c

.PHONY: check clean
check: $(TESTS)
	@set -e; for t in $(TESTS); do echo "== $$t"; ./$$t; done

clean:
	rm -f $(TESTS)

.DEFAULT_GOAL := check
