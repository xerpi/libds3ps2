IOP_BIN  = ds3ps2.irx
IOP_OBJS = ds3ps2.o exports.o imports.o

IOP_CFLAGS += -Wall -fno-builtin-printf -fno-builtin-memcpy

all: $(IOP_BIN)

# A rule to build imports.lst.
imports.o : imports.lst
	echo "#include \"irx_imports.h\"" > build-imports.c
	cat $< >> build-imports.c
	$(IOP_CC) $(IOP_CFLAGS) -c build-imports.c -o $@
	-rm -f build-imports.c

# A rule to build exports.tab.
exports.o : exports.tab
	echo "#include <irx.h>" > build-exports.c
	cat $< >> build-exports.c
	$(IOP_CC) $(IOP_CFLAGS) -c build-exports.c -o $@
	-rm -f build-exports.c

clean:
	rm -f -r $(IOP_OBJS) $(IOP_BIN)

install: $(IOP_BIN)
	cp $(IOP_BIN) $(PS2SDK)/iop/irx
	cp ds3ps2.h   $(PS2SDK)/iop/include
	cp ds3ps2.h   $(PS2SDK)/ee/include

#FOR ME!!
copy: $(IOP_BIN)
	cp $(IOP_BIN) /media/$(USER)/MLG
	sync

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.iopglobal
