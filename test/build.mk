include $(TOP)/build/header.mk

products_$(d) := reader core

reader_sources_$(d) += \
    reader.cpp

reader_needs_$(s)       := lib/libvm.so
reader_precompiled_$(d) := 
reader_target_dir_$(d)  := spec
reader_cxx_flags_$(d)   := -g -std=c++14 -I$(TOP)/src -I$(TOP)/lib/momentum/include
reader_ld_flags_$(d)    := -L$(BUILD_DIR)/lib -lvm

core_sources_$(d) += \
    core.cpp

core_needs_$(s)       := lib/libvm.so
core_precompiled_$(d) := 
core_target_dir_$(d)  := spec
core_cxx_flags_$(d)   := -g -std=c++14 -I$(TOP)/src -I$(TOP)/lib/momentum/include
core_ld_flags_$(d)    := -L$(BUILD_DIR)/lib -lvm

include $(TOP)/build/footer.mk
