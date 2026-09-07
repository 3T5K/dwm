####################################
### dwm - dynamic window manager ###
####################################

VERSION  := 6.8
EXE      := dwm
CC       := gcc

BLD_DIR  := build
SRC_DIR  := src
CFG_DIR  := config
INS_DIR  := /usr/local
BIN_DIR  := $(INS_DIR)/bin
MAN_DIR  := $(INS_DIR)/share/man/man1

NAMES    := drw dwm util
SOURCES  := $(addprefix $(SRC_DIR)/,$(addsuffix .c,$(NAMES)))
OBJECTS  := $(addprefix $(BLD_DIR)/,$(addsuffix .o,$(NAMES)))
CONFIGS  := $(patsubst $(CFG_DIR)/%,%,$(shell find $(CFG_DIR) -type f | sort))
DWMCFG   := $(SRC_DIR)/config.h

INCLUDES := -I/usr/include/freetype2 \
			-I/usr/X11R6/include \
			-I$(CFG_DIR)
CPPFLAGS := -D_DEFAULT_SOURCE \
			-D_BSD_SOURCE \
			-D_XOPEN_SOURCE=700L \
			-DVERSION=\"$(VERSION)\" \
			-DXINERAMA
CFLAGS   := -std=c99 -pedantic -Wall \
			-Wno-unused-function \
			-Wno-deprecated-declarations -Os \
			$(INCLUDES) $(CPPFLAGS)
LDFLAGS  := -L/usr/X11R6/lib -lX11 \
			-lXinerama \
			-lfontconfig -lXft

ERR = $(shell mkdir -p $(BLD_DIR) 2>&1)
ifneq ($(ERR),)
$(error $(ERR))
endif

CFG_IDX := $(shell HIDDEN= $(SRC_DIR)/trackdir.sh $(CFG_DIR) $(BLD_DIR))
ifneq ($(filter ERROR:%,$(CFG_IDX)),)
$(error $(CFG_IDX))
endif

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $(CFLAGS) $<

$(DWMCFG): $(CFG_IDX)
	@if test -n "$(strip $(CONFIGS))"; then \
		printf '#include <%s>\n' $(CONFIGS) > $@; \
	else \
		cp $(SRC_DIR)/config.def.h $@; \
	fi

$(OBJECTS): $(DWMCFG)

$(BLD_DIR)/$(EXE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

build: $(BLD_DIR)/$(EXE)

dist:
	@tar --create \
		--gzip \
		--verbose \
		--file $(BLD_DIR)/$(EXE)-$(VERSION).tar.gz \
		--exclude $(BLD_DIR) \
		--exclude-vcs \
		--transform 's#^.#$(EXE)-$(VERSION)#' .

get-applied:
	@git log --oneline --root \
		| grep '^\([[:digit:]]\|[[:alpha:]]\)\+ applied patch:' \
		| sed 's/applied patch//'

get-added:
	@git log --oneline --root \
		| grep '^\([[:digit:]]\|[[:alpha:]]\)\+ added patch:' \
		| sed 's/added patch//'

clean:
	rm -f $(BLD_DIR)/*

clean-src:
	rm -f $(SRC_DIR)/*.orig $(SRC_DIR)/*.rej

# This target can't be run before $(DWMCFG) in a single
# make invocation due to it removing metadata generated
# by trackdir.sh which $(DWMCFG) relies on.
clean-all: clean-src
	find $(BLD_DIR) -type f -exec rm -f {} +

install: build
	mkdir -p $(BIN_DIR) $(MAN_DIR)
	cp -f $(BLD_DIR)/$(EXE) $(BIN_DIR)
	sed "s/VERSION/$(VERSION)/g" \
		< $(SRC_DIR)/dwm.1 \
		> $(MAN_DIR)/$(EXE).1
	chmod 755 $(BIN_DIR)/$(EXE)
	chmod 644 $(MAN_DIR)/$(EXE).1

uninstall:
	rm -f $(BIN_DIR)/$(EXE) $(MAN_DIR)/$(EXE).1

.PHONY: build dist get-applied get-added clean clean-src clean-all install uninstall
.DEFAULT_GOAL := build
