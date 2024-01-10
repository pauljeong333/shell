CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -D_GNU_SOURCE -std=gnu99 -Werror
PROMPT = -DPROMPT
CC = gcc

33SH = 33sh
33NOPROMPT = 33noprompt
33SH_HEADERS = common.h parse.h check_error.h jobs.h jobs.h
33SH_OBJS = sh.c common.c parse.c check_error.c jobs.c

EXECS = $(33SH) $(33NOPROMPT)

.PHONY: all clean

all: $(EXECS)

$(33SH): $(33SH_HEADERS) $(33SH_OBJS)
	$(CC) $(CFLAGS) $(33SH_OBJS) $(PROMPT) -o $@

$(33NOPROMPT): $(33SH_HEADERS) $(33SH_OBJS)
	$(CC) $(CFLAGS) $(33SH_OBJS) -o $@

clean:
	rm -f $(EXECS)