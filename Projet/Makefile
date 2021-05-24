CC=gcc
CFLAGS=-Wall -Wextra 
CFLAGS +=$(FINAL)
LDLIBS= -pthread -lm -lcrypto


TARGET ?= blockchain
SRC_DIRS ?= ./

SRCS := $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.s')
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBS) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS)

-include $(DEPS)
