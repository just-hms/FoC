CC=g++
SOURCES=./src/config/*.cpp ./src/entity/*.cpp ./src/network/*.cpp ./src/protocol/*.cpp ./src/repo/*.cpp ./src/router/*.cpp ./src/security/*.cpp ./src/uuid/*.cpp ./src/cli/*.cpp
BUILD=./build
FLAGS=-lcrypto -ljsoncpp -std=c++2a -w -lncurses

.PHONY: test clean

build: 	build_server build_client

build_server: $(SOURCES)
	@echo building server...				
	@$(CC) -o ./$(BUILD)/server ./src/cmd/server.cpp $(SOURCES) $(FLAGS)

build_client: $(SOURCES)
	@echo building client...				
	@$(CC) -o ./$(BUILD)/client ./src/cmd/client.cpp $(SOURCES) $(FLAGS)

clean:
	rm -v ./$(BUILD)/*

generate: $(SOURCES)
	@echo building generator...
	$(CC) -o ./$(BUILD)/generator ./src/cmd/generator.cpp $(SOURCES) $(FLAGS)
	@echo generating...
	@./build/generator
				

test: $(SOURCES)
	@echo building tests...
	$(CC) -o ./$(BUILD)/test ./test/test.cpp ./test/*_test.cpp $(SOURCES) $(FLAGS)
	@echo running tests...
	@./build/test

