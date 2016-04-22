include $(TOP)/build/header.mk

products_$(d) := booti

booti_needs_$(s)       := lib/libvm.so
booti_sources_$(d)     := repl.cpp
booti_precompiled_$(d) :=
booti_target_dir_$(d)  := bin
booti_cxx_flags_$(d)   := -D_DEBUG -g -std=c++14 -I$(d)/../src -I$(d)/../lib/momentum/include
booti_ld_flags_$(d)    := -L$(BUILD_DIR)/lib -lvm -lreadline

include $(TOP)/build/footer.mk
