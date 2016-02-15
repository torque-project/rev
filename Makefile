include build/core.mk

CC  := clang
CXX := clang++

$(eval $(call SUBDIR, .))
$(eval $(call SUBDIR, test))

include build/targets.mk

test: $(BUILD_DIR)/spec/reader
	$(BUILD_DIR)/spec/reader
