include $(TOP)/build/header.mk

products_$(d) := libvm.so

libvm.so_sources_$(d) += \
    reader.cpp \
	  core.cpp \
		values/list.cpp \
	  values/boxed.cpp \
		values/symbol.cpp \
    values/vector.cpp \
	  values/map.cpp \
		values/var.cpp \
    values/ns.cpp \
	  values/string.cpp

libvm.so_precompiled_header_$(d) := 
libvm.so_target_dir_$(d) := lib
libvm.so_cxx_flags_$(d)  := -g -std=c++14 -I$(TOP)/lib/momentum/include
libvm.so_ld_flags_$(d)   := -shared -undefined dynamic_lookup

include $(TOP)/build/footer.mk
