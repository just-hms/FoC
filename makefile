CC=g++
SOURCES=./src/config/*.cpp ./src/entity/*.cpp ./src/network/*.cpp ./src/protocol/*.cpp ./src/repo/*.cpp ./src/router/*.cpp ./src/security/*.cpp
BUILD=./build
FLAGS=-lcrypto -ljsoncpp -g -std=c++2a -w

.PHONY: test clean

build: 			build_server build_client

build_server: 	$(SOURCES)
				@$(CC) -o ./$(BUILD)/server ./src/cmd/server.cpp $(SOURCES) $(FLAGS)

build_client: 	$(SOURCES)
				@$(CC) -o ./$(BUILD)/client ./src/cmd/client.cpp $(SOURCES) $(FLAGS)

run:
				@./$(BUILD)/server &
				@./$(BUILD)/client
				@pkill -f "./$(BUILD)/server"

clean:
				rm -v ./$(BUILD)/*

test:			$(SOURCES)
				@echo building tests...
				$(CC) -o ./$(BUILD)/test ./test/test.cpp ./test/*_test.cpp $(SOURCES) $(FLAGS)
				@echo running tests...
				@./build/test

