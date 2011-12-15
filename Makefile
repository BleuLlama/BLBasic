# Makefile for BleuLlamaBasic
#
#  2011-01 Scott Lawrence
#

CC ?= gcc

CFLAGS += -Wall -pedantic -g -ggdb -I engine

# LDFLAGS += -lreadline

SRCSCOMMON := \
	engine/basic_errors.c \
	engine/basic_string.c \
	engine/basic_variable.c \
	engine/basic_tokenizer.c \
	engine/basic_evaluator.c

SRCS  := main.c $(SRCSCOMMON)
ESRCS := embed.c $(SRCSCOMMON)

OBJS := $(SRCS:%.c=%.o)
EOBJS := $(ESRCS:%.c=%.o)

TARG := blbasic
ETARG := blbasic_e

all: $(TARG) $(ETARG)


$(TARG):  $(OBJS)
	$(CC) -g -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

$(ETARG): $(EOBJS)
	$(CC) -g -o $@ $(EOBJS) $(LDFLAGS) $(LIBS)

clean:
	rm -rf $(TARG) $(OBJS) $(ETARG) $(EOBJS)


test: $(TARG)
	./$(TARG)

val: $(TARG)
	valgrind ./$(TARG)
