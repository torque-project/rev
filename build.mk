include $(TOP)/build/header.mk

FFI_CFLAGS := $(shell pkg-config --cflags-only-I libffi)
FFI_LFLAGS := $(shell pkg-config --libs libffi)

products_$(d) := libvm.so

libvm.so_sources_$(d) += \
    reader.cpp \
	  core.cpp \
		values/list.cpp \
	  values/boxed.cpp \
		values/symbol.cpp \
    values/vector.cpp \
	  values/map.cpp \
    values/set.cpp \
		values/var.cpp \
    values/ns.cpp \
	  values/binary.cpp \
	  values/string.cpp \
	  values/fn.cpp \
		values/type.cpp \
		values/protocol.cpp \
	  values/array.cpp

libvm.so_precompiled_$(d) :=
libvm.so_target_dir_$(d)  := lib
libvm.so_cxx_flags_$(d)   := -g -std=c++14 -I$(d)/lib/momentum/include $(FFI_CFLAGS)
libvm.so_ld_flags_$(d)    := -shared -undefined dynamic_lookup $(FFI_LFLAGS)

include $(TOP)/build/footer.mk
