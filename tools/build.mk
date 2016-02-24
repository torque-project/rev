include $(TOP)/build/header.mk

products_$(d) := booti

booti_sources_$(d) += \
    repl.cpp

booti_precompiled_header_$(d) :=
booti_target_dir_$(d) := bin
booti_cxx_flags_$(d)  := -g -std=c++14 -I$(TOP)/src -I$(TOP)/lib/momentum/include
booti_ld_flags_$(d)   := -L$(BUILD_DIR)/lib -lvm -lreadline

include $(TOP)/build/footer.mk
