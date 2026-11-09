####################################
### dwm - dynamic window manager ###
####################################

VERSION    ?= 6.8
EXE        ?= dwm

DWM_THEME  ?=

PKG_CONFIG ?= pkg-config

BLD_DIR	   ?= build
SRC_DIR	   ?= src
CFG_DIR	   ?= config
INS_DIR	   ?= /usr/local
BIN_DIR	   ?= bin
MAN_DIR	   ?= share/man

PC_DEPS    := freetype2 x11 xft xinerama fontconfig x11-xcb xcb-res

         CFLAGS   ?= -Werror -Os
override CFLAGS   += -std=c99 -Wall -Wpedantic
override CFLAGS   += -Wno-unused-result
override CFLAGS   += -Wno-unused-function
override CFLAGS   += -Wno-constant-logical-operand
override CFLAGS   += -Wno-deprecated-declarations

		 CPPFLAGS ?=
override CPPFLAGS += $(shell $(PKG_CONFIG) --cflags $(PC_DEPS))
override CPPFLAGS += -I$(CFG_DIR)
override CPPFLAGS += -D_DEFAULT_SOURCE -D_BSD_SOURCE -DXINERAMA
override CPPFLAGS += -D_XOPEN_SOURCE=700L -DVERSION=\"$(VERSION)\"

ifneq ($(strip $(DWM_THEME)),)
override CPPFLAGS += -DDWM_THEME=$(DWM_THEME)
endif

LDFLAGS    ?=
LDLIBS     := $(shell $(PKG_CONFIG) --libs $(PC_DEPS))

NAMES      := drw dwm util
SOURCES    := $(addprefix $(SRC_DIR)/,$(addsuffix .c,$(NAMES)))
OBJECTS    := $(addprefix $(BLD_DIR)/,$(addsuffix .o,$(NAMES)))
CONFIGS    := $(patsubst $(CFG_DIR)/%,%,$(shell find $(CFG_DIR) -type f 2>/dev/null | sort))
BLD_CFG    := $(SRC_DIR)/config.h
CFG_IDX    := $(BLD_DIR)/.$(CFG_DIR).idx

$(CFG_DIR) $(BLD_DIR):
	mkdir -p $@

_FORCE:
$(CFG_IDX): _FORCE | $(CFG_DIR) $(BLD_DIR)
	@HIDDEN= $(SRC_DIR)/trackdir.sh $(CFG_DIR) $(BLD_DIR) 1>/dev/null

$(BLD_CFG): $(CFG_IDX)
	@if test -n "$(strip $(CONFIGS))"; then \
		printf '#include <%s>\n' $(CONFIGS) > $@; \
	else \
		cp $(SRC_DIR)/config.def.h $@; \
	fi

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c | $(BLD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OBJECTS): $(BLD_CFG)

$(BLD_DIR)/$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $(OBJECTS) 

$(EXE): $(BLD_DIR)/$(EXE)

dist:
	@tar --create \
		--gzip \
		--verbose \
		--file $(BLD_DIR)/$(EXE)-$(VERSION).tar.gz \
		--exclude $(BLD_DIR) \
		--exclude-vcs \
		--transform 's#^.#$(EXE)-$(VERSION)#' .

get-applied:
	@git log --oneline base-$(VERSION)..HEAD \
		| grep '^\([[:digit:]]\|[[:alpha:]]\)\+ applied patch:' \
		| sed 's/applied patch//'

get-added:
	@git log --oneline base-$(VERSION)..HEAD \
		| grep '^\([[:digit:]]\|[[:alpha:]]\)\+ added patch:' \
		| sed 's/added patch//'

get-config:
	@git log --oneline base-$(VERSION)..HEAD \
		| grep '^\([[:digit:]]\|[[:alpha:]]\)\+ config:' \
		| sed 's/config//'

clean-bld:
	rm -rf $(BLD_DIR)

clean-src:
	rm -f $(BLD_CFG) $(SRC_DIR)/*.orig $(SRC_DIR)/*.rej

clean: clean-bld clean-src

install: $(EXE)
	mkdir -p $(INS_DIR)/$(BIN_DIR) $(INS_DIR)/$(MAN_DIR)/man1
	cp -f $(BLD_DIR)/$(EXE) $(INS_DIR)/$(BIN_DIR)/$(EXE)
	sed "s/VERSION/$(VERSION)/g" \
		< $(SRC_DIR)/dwm.1 \
		> $(INS_DIR)/$(MAN_DIR)/man1/$(EXE).1
	chmod 755 $(INS_DIR)/$(BIN_DIR)/$(EXE)
	chmod 644 $(INS_DIR)/$(MAN_DIR)/man1/$(EXE).1

uninstall:
	rm -f $(INS_DIR)/$(BIN_DIR)/$(EXE) \
		  $(INS_DIR)/$(MAN_DIR)/man1/$(EXE).1

.PHONY: _FORCE $(EXE) dist \
		get-applied get-added get-config \
		clean-bld clean-src clean \
		install uninstall

.DEFAULT_GOAL := $(EXE)
