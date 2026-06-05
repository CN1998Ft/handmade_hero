CC := cl
CFLAGS := -Zi -FC # -Fe:
BUILD_DIR := ../build
LIBS := user32.lib Gdi32.lib
SRC := ../code/win_handmade.cpp
# TOCLEAN := $(wildcard $(BUILD_DIR)/*.obj) $(wildcard $(BUILD_DIR)/*.ilk)\
# 		   $(wildcard $(BUILD_DIR)/*.exe) $(wildcard $(BUILD_DIR)/*.pdb)

# This works for all machine, don't know why the above does not work on other
# windows machine.
TOCLEAN := $(BUILD_DIR)/*.obj $(BUILD_DIR)/*.ilk $(BUILD_DIR)/*exe \
		   $(BUILD_DIR)/*.pdb

.PHONY: clean all

all: $(BUILD_DIR)/win_handmade.exe

$(BUILD_DIR)/win_handmade.exe: $(BUILD_DIR)
	@cd $(BUILD_DIR); \
		$(CC) $(CFLAGS) $(SRC) $(LIBS)

$(BUILD_DIR): win_handmade.cpp
	@mkdir $(BUILD_DIR)

clean:
	rm -rf $(TOCLEAN)
