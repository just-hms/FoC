CC=g++
SOURCES=./src/config/*.cpp ./src/network/*.cpp ./src/protocol/*.cpp 
BUILD=./build
FLAGS=-lcrypto -ljsoncpp -g

build: 			build_server build_client

build_server: 	$(SOURCES)
				$(CC) -o ./$(BUILD)/server ./src/cmd/server.cpp $(SOURCES) $(FLAGS)

build_client: 	$(SOURCES)
				$(CC) -o ./$(BUILD)/client ./src/cmd/client.cpp $(SOURCES) $(FLAGS)

clean:
				rm -v ./$(BUILD)/*


