CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined
RELEASE_FLAGS = -std=c11 -O3 -DNDEBUG

SRCDIR = src
TESTDIR = tests
BUILDDIR = build
INCDIR = include

SOURCES = $(wildcard $(SRCDIR)/**/*.c $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)

TARGET = databse
TEST_TARGET = test_database

.PHONY: all clean test debug RELEASE_FLAGS

all: debug

debug: CFLAGS = $(RELEASE_FLAGS)
debug: $(TARGET)

release: CFLAGS = $(RELEASE_FLAGS)
release: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(SRCDIR) -c -o $@ $<

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES) $(filter-out $(BUILDDIR)/main.o,$(OBJECTS))
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(SRCDIR) -o $@ $^

clean:
	rm -rf $(BUILDDIR) $(TARGET) $(TEST_TARGET)