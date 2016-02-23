include build/core.mk

CC  := clang
CXX := clang++

$(eval $(call SUBDIR, .))
$(eval $(call SUBDIR, tools))
$(eval $(call SUBDIR, test))

include build/targets.mk
