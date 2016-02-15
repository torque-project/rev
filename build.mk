include $(TOP)/build/header.mk

products_$(d) := libvm.so

libvm.so_sources_$(d) += \
    reader.cpp \
		values/list.cpp \
	  values/boxed.cpp \
		values/symbol.cpp \
    values/vector.cpp \
	  values/map.cpp \
	  values/string.cpp

libvm.so_precompiled_header_$(d) := 
libvm.so_target_dir_$(d) := lib
libvm.so_cxx_flags_$(d)  := -std=c++14 -I$(TOP)/lib/momentum/include
libvm.so_ld_flags_$(d)   := -shared -undefined dynamic_lookup

include $(TOP)/build/footer.mk
