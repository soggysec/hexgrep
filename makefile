SHELL := /bin/bash
CC := cc
INSTALL := install

app := hexgrep
prefix=/usr/local

$(app): $(app).c $(app).h
# 	@$(CC) -lrt -Wall -O2 -o $(app) $(app).c
	@$(CC) -Wall -O2 -o $(app) $(app).c

all: $(app)

install: $(app)
	@$(INSTALL) -Dm755 $(app) $(DESTDIR)$(prefix)/bin/$(app)

clean:
	@if [[ -e $(app) ]]; then rm $(app); fi

.PHONY: all install clean
