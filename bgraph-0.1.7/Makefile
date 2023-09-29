# By Pierre Sarrazin <http://sarrazip.com/>
# This file is in the public domain.

PACKAGE = bgraph
VERSION = 0.1.7

TARGET = coco

LIBSRC = \
	clipping.c \
	compute16ColorModeByteAddress.c \
	computePmode4ByteAddress.c \
	divBy3Table.c \
	drawCatmullRomPath.c \
	Drawer_16colors.c \
	Drawer_2colors.c \
	Drawer_4colors.c \
	Drawer.c \
	Drawer_initForHSCREEN.c \
	Drawer_initForPMODE.c \
	ellipse.c \
	filledRectangle.c \
	getAddressAndMaskPmode4.c \
	getPixel16ColorMode.c \
	getPixel256x16.c \
	getPixel320x16.c \
	getPixelPmode4.c \
	getPixelSG12.c \
	getPixelSG24.c \
	getPixelSG8.c \
	getPixelSG.c \
	getSG12PixelAddress.c \
	getSG24PixelAddress.c \
	getSG8PixelAddress.c \
	setPixel16ColorMode.c \
	setPixel256x16.c \
	setPixel320x16.c \
	line.c \
	lores.c \
	Painter.c \
	rectangle.c \
	setPixel256x16.c \
	setPixelPmode4.c \
	setPixelSG12.c \
	setPixelSG24.c \
	setPixelSG8.c \
	setPixelSG.c \
	sgMasks.c \
	sgmode.c \
	shiftLeftBy5.c \
	times6times32.c
LIBHEADERS = \
	bgraph_private.h \
	catmull-rom.h \
	clipping.h \
	clipping_private.h \
	Drawer.h \
	lores.h \
	Painter.h \
	Primitive.h \
	semigr.h
STATICLIB = lib$(PACKAGE).a

# Main demo.
DEMOSRC = $(PACKAGE).c
DEMOBIN = $(PACKAGE).bin

ifeq "$(TARGET)" "coco"

# CoCo 3 demo.
CC3DEMOSRC = cc3demo.c
CC3DEMOBIN = cc3demo.bin

# Driver used vby the CoCo 3 demo.
CC3DRVSRC = cc3graph.c
CC3DRVHEADERS = cc3graph.h
CC3DRVBIN = cc3graph.drv

# My Driver
CC4DRVSRC = cc4graph.c
CC4DRVHEADERS = cc4graph.h
CC4DRVBIN = cc4graph.drv

# Catmull-Rom spline demo.
CMDEMOSRC = cmdemo.c
CMDEMOBIN = $(CMDEMOSRC:.c=.bin)

DRIVER_ENTRY = C000
DRIVER_END = F09D  # do not let program grow to overwrite SECB's HPRINT font (see SECB_HPRINT_FONT in cc3graph.c)

endif

DEMODSK = $(PACKAGE).dsk

DISTFILES = README.md Makefile $(LIBHEADERS) $(LIBSRC) $(DEMOSRC) $(CC3DEMOSRC) $(CC3DRVSRC) $(CC4DRVSRC) $(CC3DRVHEADERS) $(CMDEMOSRC) testclip.c

ORG = #--org=2800
INTERMEDIATE = #--intermediate
NDEBUG = -DNDEBUG
OPTIM =
WERROR = -Werror
CFLAGS_PERMISSIVE = --$(TARGET) $(NDEBUG) $(INTERMEDIATE) $(OPTIM) -DDRIVER_ENTRY=0x$(DRIVER_ENTRY)
CFLAGS = $(CFLAGS_PERMISSIVE) $(WERROR)
LDFLAGS = $(ORG) $(INTERMEDIATE)

DEMOOBJ = $(DEMOSRC:.c=.o)
CC3DEMOOBJ = $(CC3DEMOSRC:.c=.o)
CC3DRVOBJ = $(CC3DRVSRC:.c=.o)
CC4DRVOBJ = $(CC4DRVSRC:.c=.o)
CMDEMOOBJ = $(CMDEMOSRC:.c=.o)
LIBOBJ = $(LIBSRC:.c=.o)

CMOC = cmoc
LWAR = lwar
TAR = tar
PERL = perl
WRITECOCOFILE = writecocofile

all:  dsk

$(DEMOBIN): $(DEMOOBJ) $(STATICLIB)
	$(CMOC) -o $(DEMOBIN) --$(TARGET) $(LDFLAGS) $(DEMOOBJ) -L. -l$(PACKAGE)

ifeq "$(TARGET)" "coco"

$(CC3DEMOBIN): $(CC3DEMOOBJ)
	$(CMOC) -o $(CC3DEMOBIN) --$(TARGET) $(LDFLAGS) $(CC3DEMOOBJ)

# The driver is generated as a raw binary file, not as a DECB BIN file.
# It is made to be loaded as is by cc3demo.c.
# --void-target avoids using the DECB C run-time, which might do more than needed upon exit.
# --no-relocate allows the optimizer to generate shorter code.
#
$(CC3DRVBIN): $(CC3DRVOBJ) $(STATICLIB)
	$(CMOC) --raw --void-target -o $(CC3DRVBIN) --org=$(DRIVER_ENTRY) --limit=$(DRIVER_END) --no-relocate $(LDFLAGS) $(CC3DRVOBJ) -L. -l$(PACKAGE)

$(CMDEMOBIN): $(CMDEMOOBJ) $(STATICLIB)
	$(CMOC) -o $(CMDEMOBIN) --$(TARGET) $(LDFLAGS) $(CMDEMOOBJ) -L. -l$(PACKAGE)
	
$(CC4DRVBIN): $(CC4DRVOBJ) $(STATICLIB)
	$(CMOC) --raw --void-target -o $(CC4DRVBIN) --org=$(DRIVER_ENTRY) --limit=$(DRIVER_END) --no-relocate $(LDFLAGS) $(CC4DRVOBJ) -L. -l$(PACKAGE)

endif

# Program that tests clipping.c.
testclip.bin: testclip.o $(STATICLIB)
	$(CMOC) -o $@ --$(TARGET) testclip.o $(LDFLAGS) -L. -l$(PACKAGE)

$(STATICLIB): $(LIBOBJ)
	$(LWAR) --create $(STATICLIB) $(LIBOBJ)

%.o: %.c
	$(CMOC) -c $(CFLAGS) -o $@ $<

# Exception(s) to the above rule for file(s) that generate tolerated warnings.
#
ellipse.o: ellipse.c
	$(CMOC) -c $(CFLAGS_PERMISSIVE) -o $@ $<

clean:
	rm -f $(DEMOBIN) $(DEMOOBJ) $(CC3DEMOBIN) $(CC3DEMOOBJ) $(CC3DRVBIN) $(CC3DRVOBJ) $(CMDEMOBIN) $(CMDEMOOBJ) testclip.bin testclip.o $(STATICLIB) $(LIBOBJ)

.PHONY: dsk
dsk: $(DEMODSK)

$(DEMODSK): $(DEMOBIN) $(CC3DEMOBIN) $(CC3DRVBIN) $(CC4DRVBIN) $(CMDEMOBIN) testclip.bin
ifeq "$(TARGET)" "coco"
	$(PERL) -e 'print chr(255) x (35*18*256)' > $(DEMODSK)
	$(WRITECOCOFILE) $(DEMODSK) $(DEMOBIN)
	$(WRITECOCOFILE) $(DEMODSK) $(CC3DEMOBIN)
	$(WRITECOCOFILE) $(DEMODSK) $(CC3DRVBIN)
	$(WRITECOCOFILE) $(DEMODSK) $(CC4DRVBIN)
	$(WRITECOCOFILE) $(DEMODSK) $(CMDEMOBIN)
	$(WRITECOCOFILE) $(DEMODSK) testclip.bin
endif

.PHONY: dist
dist:
	$(TAR) -czf $(PACKAGE)-$(VERSION).tar.gz --transform 's,^,$(PACKAGE)-$(VERSION)/,' $(DISTFILES)

distcheck: dist
	mkdir ,distcheck
	$(TAR) -C ,distcheck -xzf $(PACKAGE)-$(VERSION).tar.gz
	$(MAKE) -C ,distcheck/$(PACKAGE)-$(VERSION) all
	rm -fr ,distcheck
	@echo "SUCCESS: $(PACKAGE)-$(VERSION).tar.gz ready for distribution"

# Automatic prerequisite generation.
%.d: %.c
	$(CMOC) --deps-only $(CFLAGS) $<

.PHONY: cleandeps
cleandeps:
	rm -f $(DEMOSRC:.c=.d) $(CC3DEMOSRC:.c=.d) $(CC3DRVSRC:.c=.d) $(CC4DRVSRC:.c=.d) $(CMDEMOSRC:.c=.d) $(LIBSRC:.c=.d) testclip.d

include $(DEMOSRC:.c=.d)
ifeq "$(TARGET)" "coco"
include $(CC3DEMOSRC:.c=.d)
include $(CC3DRVSRC:.c=.d)
include $(CMDEMOSRC:.c=.d)
endif
include $(LIBSRC:.c=.d)
include testclip.d
