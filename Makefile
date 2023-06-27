SRCS=$(shell find src -name '*.c')
OBJS=$(SRCS:.c=.o)
SOBJ=$(OBJS:.o=.$(LIB_EXTENSION))
GCDAS=$(OBJS:.o=.gcda)
INSTALL?=install

ifdef POSTGRES_DECODE_COVERAGE
COVFLAGS=--coverage
endif

.PHONY: all install

all: $(SOBJ)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) $(COVFLAGS) $(CPPFLAGS) -o $@ -c $<

%.$(LIB_EXTENSION): %.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(PLATFORM_LDFLAGS) $(COVFLAGS)

install:
	$(INSTALL) -d $(INST_LIBDIR)
	$(INSTALL) $(SOBJ) $(INST_LIBDIR)
	rm -f $(OBJS) $(SOBJ) $(GCDAS)
