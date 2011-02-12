# Makefile for BleuLlamaBasic
#
#  2011-01 Scott Lawrence
#

CC = GCC

CFLAGS += -Wall -pedantic -g -ggdb -I engine

# LDFLAGS += -lreadline

SRCS := main.c \
	\
	engine/basic_errors.c \
	engine/basic_string.c \
	engine/basic_variable.c \
	engine/basic_tokenizer.c \
	engine/basic_evaluator.c

OBJS := $(SRCS:%.c=%.o)

TARG := blbasic

all: $(TARG)


$(TARG):  $(OBJS)
	$(CC) -g -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	rm -rf $(TARG) $(OBJS)


test: $(TARG)
	./$(TARG)

val: $(TARG)
	valgrind ./$(TARG)
