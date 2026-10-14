CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
MACOS_SDK ?= $(shell if test -d /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk; then echo /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk; fi)
ifneq ($(MACOS_SDK),)
CXXFLAGS += -isysroot $(MACOS_SDK)
endif

all: build/ravenwood

build/ravenwood: src/main.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) src/main.cpp -o build/ravenwood

run: build/ravenwood
	./build/ravenwood

clean:
	rm -rf build

.PHONY: all run clean
