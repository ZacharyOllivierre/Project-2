CXX = g++
CXXFLAGS = -std=c++17 \
           -iframework/opt/homebrew/Cellar/qt/6.11.0/lib \
           -I/opt/homebrew/Cellar/qt/6.11.0/lib/QtSql.framework/Headers \
           -I/opt/homebrew/Cellar/qt/6.11.0/lib/QtCore.framework/Headers

LDFLAGS = -F/opt/homebrew/Cellar/qt/6.11.0/lib \
          -framework QtSql \
          -framework QtCore

SRC = Database.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
