CXX = g++
CXXFLAGS = -std=c++17 -Wall
OBJ = main.o dataBase.o megatron.o
TARGET = program

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

main.o: main.cpp dataBase.h megatron.h
	$(CXX) $(CXXFLAGS) -c main.cpp

dataBase.o: dataBase.cpp dataBase.h
	$(CXX) $(CXXFLAGS) -c dataBase.cpp

megatron.o: megatron.cpp megatron.h
	$(CXX) $(CXXFLAGS) -c megatron.cpp

clean:
	rm -f $(OBJ) $(TARGET)