#---------------------------------------------------------------------------------
# PS3 3D Test - Makefile
#
# A minimal real-3D sandbox (perspective camera + spinning cube) on the PSL1GHT
# SDK. Build with the Dockerized toolchain:
#   docker run --rm -v "$PWD":/src -w /src ghcr.io/02900/ps3-toolchain make
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

#---------------------------------------------------------------------------------
# Application metadata
#---------------------------------------------------------------------------------
TITLE		:=	PS3 Tiny3D Boilerplate
APPID		:=	PS33DTEST
CONTENTID	:=	UP0001-$(APPID)_00-0000000000000000
ICON0		:=	pkgfiles/ICON0.PNG
SFOXML		:=	sfo.xml

include $(PSL1GHT)/ppu_rules

#---------------------------------------------------------------------------------
# Directories
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data
INCLUDES	:=	include
PKGFILES	:=	pkgfiles

#---------------------------------------------------------------------------------
# Libraries to link
# Includes: Tiny3D, YA2D, libcurl, PolarSSL, MikMod, Mini18n
#---------------------------------------------------------------------------------
LIBS		:=	-lcurl -lya2d -lfont3d -ltiny3d -lsimdmath \
			-lgcm_sys -lrsx -lio -lsysutil -lrt -llv2 \
			-lpngdec -ljpgdec -lsysmodule -lm -lsysfs \
			-lnet -lfreetype -lz -lmikmod -laudio \
			-lpolarssl -lmini18n -lnetctl

#---------------------------------------------------------------------------------
# Compiler flags
#---------------------------------------------------------------------------------
CFLAGS		=	-O2 -Wall -mcpu=cell -std=gnu99 $(MACHDEP) $(INCLUDE)
CXXFLAGS	=	$(CFLAGS)
LDFLAGS		=	$(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# Debug logging (optional)
#---------------------------------------------------------------------------------
ifdef DEBUGLOG
CFLAGS		+=	-DENABLE_LOGGING
LIBS		+=	-ldbglogger
endif

#---------------------------------------------------------------------------------
# Library directories
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# Build rules
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)
export BUILDDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
PNGFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.png)))
JPGFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.jpg)))
MODFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.mod)))
S3MFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.s3m)))

ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(addsuffix .o,$(PNGFILES)) \
			$(addsuffix .o,$(JPGFILES)) \
			$(addsuffix .o,$(MODFILES)) \
			$(addsuffix .o,$(S3MFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES), -I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			$(LIBPSL1GHT_INC) \
			-I$(PORTLIBS)/include/freetype2 \
			-I$(CURDIR)/$(BUILD) -I$(PORTLIBS)/include

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
			$(LIBPSL1GHT_LIB) -L$(PORTLIBS)/lib

export OUTPUT	:=	$(CURDIR)/$(TARGET)

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo Cleaning...
	@rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).self EBOOT.BIN *.pkg

run: $(BUILD)
	ps3load $(OUTPUT).self

pkg: $(BUILD) $(OUTPUT).pkg

else

DEPENDS	:=	$(OFILES:.o=.d)

$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

%.bin.o: %.bin
	@echo $(notdir $<)
	@$(bin2o)

%.png.o: %.png
	@echo $(notdir $<)
	@$(bin2o)

%.jpg.o: %.jpg
	@echo $(notdir $<)
	@$(bin2o)

%.mod.o: %.mod
	@echo $(notdir $<)
	@$(bin2o)

%.s3m.o: %.s3m
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
