CC      = gcc
CFLAGS  = -Wall -Wextra -g -fsanitize=address,undefined -std=c11 \
          -I../components/bmp388 \
          -I../components/bmp388/include \
          -I../components/i2c_bus/include
LDLIBS  = -lm                # math library, for fabsf
 
# Every test_*.c becomes a binary of the same name (minus extension).
TEST_SRCS := $(wildcard test_*.c)
TEST_BINS := $(patsubst %.c,build/%,$(TEST_SRCS))
 
.PHONY: test clean
test: $(TEST_BINS)
	@echo "== running tests =="
	@for b in $(TEST_BINS); do echo "-> $$b"; ./$$b || exit 1; done
	@echo "== all tests passed =="
 
build/%: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)
 
clean:
	rm -rf build