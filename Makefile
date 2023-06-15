CC=g++

SOURCES=$(wildcard src/config/*.cpp src/entity/*.cpp src/network/*.cpp \
	src/protocol/*.cpp src/repo/*.cpp src/router/*.cpp \
	src/security/*.cpp src/uuid/*.cpp src/cli/*.cpp)

BUILD?=build

OBJECTS=$(SOURCES:src/%.cpp=$(BUILD)/%.o)

FLAGS=-lcrypto -ljsoncpp -lncurses
CFLAGS=-std=c++2a -w

MODULES=$(wildcard src/*)

define generate_deps
    $(wildcard src/$(1)/*.h*)
endef

.PHONY: build_tree test clean

ducange: 
	@echo $(call generate_deps,router)

all: build

build: 	build_tree server client $(BUILD)/test

build_tree: $(MODULES:src/%=$(BUILD)/%)

$(MODULES:src/%=$(BUILD)/%):
	mkdir -v -p $@

$(BUILD)/config/%.o: src/config/%.cpp $(wildcard src/config/*.h)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/entity/%.o: src/entity/%.cpp $(wildcard src/entity/*.h)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/network/%.o: src/network/%.cpp $(wildcard src/network/*.h) \
		$(call generate_deps,entity)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/protocol/%.o: src/protocol/%.cpp $(wildcard src/protocol/*.h) \
		$(call generate_deps,entity) \
		$(call generate_deps,security) \
		$(call generate_deps,network) \
		$(call generate_deps,defer)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/repo/%.o: src/repo/%.cpp $(wildcard src/repo/*.h) \
		$(call generate_deps,entity) \
		$(call generate_deps,router) \
		$(call generate_deps,security) \
		$(call generate_deps,uuid)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/router/%.o: src/router/%.cpp $(wildcard src/router/*.h) \
		$(call generate_deps,entity) \
		$(call generate_deps,network)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/security/%.o: src/security/%.cpp $(wildcard src/security/*.h) \
		$(call generate_deps,defer)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/uuid/%.o: src/uuid/%.cpp $(wildcard src/uuid/*.h)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/cli/%.o: src/cli/%.cpp $(wildcard src/cli/*.h) \
		$(call generate_deps,entity)
	$(CC) $(CFLAGS) -c -o $@ $<


# Main executables
server: $(BUILD)/server
$(BUILD)/server: ./src/cmd/server.cpp $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(FLAGS)

client: $(BUILD)/client
$(BUILD)/client: ./src/cmd/client.cpp $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(FLAGS)

clean:
	rm -rv $(BUILD)/*

generator: $(BUILD)/generator
$(BUILD)/generator: src/cmd/generator.cpp $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(FLAGS)
	@echo generating...
	@./build/generator

test/%test.o: peer/%test.cpp test.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/test: $(wildcard test/*test.cpp) $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(FLAGS)

test: build_tree $(BUILD)/test
	@echo running tests...
	@./build/test

