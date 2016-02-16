include $(TOP)/build/header.mk

products_$(d) := reader core

reader_sources_$(d) += \
    reader.cpp

reader_precompiled_header_$(d) := 
reader_target_dir_$(d) := spec
reader_cxx_flags_$(d)  := -g -std=c++14 -I$(TOP)/src -I$(TOP)/lib/momentum/include
reader_ld_flags_$(d)   := -L$(BUILD_DIR)/lib -lvm

core_sources_$(d) += \
    core.cpp

core_precompiled_header_$(d) := 
core_target_dir_$(d) := spec
core_cxx_flags_$(d)  := -g -std=c++14 -I$(TOP)/src -I$(TOP)/lib/momentum/include
core_ld_flags_$(d)   := -L$(BUILD_DIR)/lib -lvm

include $(TOP)/build/footer.mk
