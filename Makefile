#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)

APP_NAME      := Five Nights at Freddy's 3 - Wii U Edition
APP_SHORTNAME := FNaF3 Wii U
APP_AUTHOR    := Eitan1414 and contributors

include $(DEVKITPRO)/wut/share/wut_rules

PKGCONF := $(PORTLIBS_PATH)/wiiu/bin/powerpc-eabi-pkg-config

#-------------------------------------------------------------------------------
# Project layout
#-------------------------------------------------------------------------------
TARGET      := fnaf3-wiiu
BUILD       := build
SOURCES     := source source/game source/platform source/renderer
DATA        := data data/audio
INCLUDES    := include
CONTENT     := .wuhb-content
ARTWORK_DIR := .wiiu-artwork
ICON        := $(ARTWORK_DIR)/icon.png
TV_SPLASH   := $(ARTWORK_DIR)/boot-tv.png
DRC_SPLASH  := $(ARTWORK_DIR)/boot-drc.png
PC_BB_ARCHIVE := source/generated/phantom_bb_pc_assets.c.xz
PC_BB_SOURCE  := source/phantom_bb_pc_assets.c
PC_BB_SHA256  := 67d94ae9783764d50ddf8b9a6c2c28b0df526a3e4b6158b1ac87c0c33ed4e9cf
PC_PUPPET_B64     := source/generated/phantom_puppet_pc_assets.fixed.b64
PC_PUPPET_ARCHIVE := .phantom_puppet_pc_assets.c.xz
PC_PUPPET_SOURCE  := source/phantom_puppet_pc_assets.c
PC_PUPPET_SHA256  := b3d88dbbce637601acb63be72f470081379dff207e7c632dce4da7d4c3428810

#-------------------------------------------------------------------------------
# Compiler and linker options
#-------------------------------------------------------------------------------
CFLAGS   := -g -Wall -Wextra -O2 -ffunction-sections $(MACHDEP)
CFLAGS   += $(INCLUDE) -D__WIIU__ -D__WUT__
CXXFLAGS := $(CFLAGS)
ASFLAGS  := -g $(ARCH)
LDFLAGS  := -g $(ARCH) $(RPXSPECS) -Wl,-Map,$(notdir $*.map)
LIBS     := $(shell $(PKGCONF) --libs sdl2) -lz -lwut -lstdc++ -lm
LIBDIRS  := $(PORTLIBS) $(WUT_ROOT)

#-------------------------------------------------------------------------------
# Standard devkitPro/wut build rules
#-------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CFILES   += phantom_bb_pc_assets.c
CFILES   += phantom_puppet_pc_assets.c
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES_BIN := $(addsuffix .o,$(BINFILES))
export OFILES_SRC := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES     := $(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifneq (,$(strip $(CONTENT)))
export APP_CONTENT := $(TOPDIR)/$(CONTENT)
endif

ifneq (,$(strip $(ICON)))
export APP_ICON := $(TOPDIR)/$(ICON)
else ifneq (,$(wildcard $(TOPDIR)/$(TARGET).png))
export APP_ICON := $(TOPDIR)/$(TARGET).png
else ifneq (,$(wildcard $(TOPDIR)/icon.png))
export APP_ICON := $(TOPDIR)/icon.png
endif

ifneq (,$(strip $(TV_SPLASH)))
export APP_TV_SPLASH := $(TOPDIR)/$(TV_SPLASH)
else ifneq (,$(wildcard $(TOPDIR)/tv-splash.png))
export APP_TV_SPLASH := $(TOPDIR)/tv-splash.png
else ifneq (,$(wildcard $(TOPDIR)/splash.png))
export APP_TV_SPLASH := $(TOPDIR)/splash.png
endif

ifneq (,$(strip $(DRC_SPLASH)))
export APP_DRC_SPLASH := $(TOPDIR)/$(DRC_SPLASH)
else ifneq (,$(wildcard $(TOPDIR)/drc-splash.png))
export APP_DRC_SPLASH := $(TOPDIR)/drc-splash.png
else ifneq (,$(wildcard $(TOPDIR)/splash.png))
export APP_DRC_SPLASH := $(TOPDIR)/splash.png
endif

.PHONY: all clean artwork pc-bb-assets pc-puppet-assets $(BUILD)

all: $(BUILD)

artwork:
	@bash tools/prepare_wiiu_artwork.sh

pc-bb-assets:
	@test -f "$(PC_BB_ARCHIVE)"
	@printf '%s  %s\n' "$(PC_BB_SHA256)" "$(PC_BB_ARCHIVE)" | sha256sum -c -
	@xz -dc "$(PC_BB_ARCHIVE)" > "$(PC_BB_SOURCE)"
	@grep -q "const TextureRle gPhantomBBCameraTexture" "$(PC_BB_SOURCE)"
	@grep -q "const JumpscareSequence gPhantomBBRealJumpscare" "$(PC_BB_SOURCE)"
	@echo "Restored authentic PC Phantom BB camera sprite and jumpscare"

pc-puppet-assets:
	@test -f "$(PC_PUPPET_B64)"
	@base64 -d "$(PC_PUPPET_B64)" > "$(PC_PUPPET_ARCHIVE)"
	@printf '%s  %s\n' "$(PC_PUPPET_SHA256)" "$(PC_PUPPET_ARCHIVE)" | sha256sum -c -
	@xz -t "$(PC_PUPPET_ARCHIVE)"
	@xz -dc "$(PC_PUPPET_ARCHIVE)" > "$(PC_PUPPET_SOURCE)"
	@grep -q "const JumpscareSequence gPhantomPuppetPcAnimation" "$(PC_PUPPET_SOURCE)"
	@echo "Restored authentic PC Phantom Puppet attack animation"

$(BUILD): artwork pc-bb-assets pc-puppet-assets
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(ARTWORK_DIR) .wuhb-content $(TARGET).wuhb $(TARGET).rpx $(TARGET).elf $(TARGET).map
	@rm -f "$(PC_BB_SOURCE)" "$(PC_PUPPET_SOURCE)" "$(PC_PUPPET_ARCHIVE)"

else

DEPENDS := $(OFILES:.o=.d)

.PHONY: all weaken-pc-bb-fallbacks
all: $(OUTPUT).wuhb

$(OUTPUT).wuhb: $(OUTPUT).rpx
$(OUTPUT).rpx: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES) weaken-pc-bb-fallbacks

# The generated PSX assets remain as fallbacks in source, but the supplied
# original PC BB sheet provides strong replacements for these two symbols.
# Weaken only the two fallback definitions before the final link.
weaken-pc-bb-fallbacks: phantom_assets.o jumpscare_assets.o
	@powerpc-eabi-objcopy --weaken-symbol=gPhantomBBCameraTexture phantom_assets.o
	@powerpc-eabi-objcopy --weaken-symbol=gPhantomBBRealJumpscare jumpscare_assets.o

$(OFILES_SRC): $(HFILES_BIN)

%.bin.o %_bin.h: %.bin
	@echo $(notdir $<)
	@$(bin2o)

%.b64.o %_b64.h: %.b64
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
