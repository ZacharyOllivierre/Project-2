CXX = g++
CXXFLAGS = -std=c++17 \
           -iframework/opt/homebrew/Cellar/qt/6.11.0/lib \
           -I/opt/homebrew/Cellar/qt/6.11.0/lib/QtSql.framework/Headers \
           -I/opt/homebrew/Cellar/qt/6.11.0/lib/QtCore.framework/Headers

LDFLAGS = -F/opt/homebrew/Cellar/qt/6.11.0/lib \
          -framework QtSql \
          -framework QtCore

SRC = Database.cpp main.cpp
OBJ = $(SRC:.cpp=.o) astar/astar.o dfs/dfs.o bfs/bfs.o
TARGET = main

TEST_SRC = traversal_tests.cpp
TEST_OBJ = traversal_tests.o astar/astar.o dfs/dfs.o bfs/bfs.o
TEST_TARGET = traversal_tests

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

astar/astar.o: astar/astar.cpp astar/astar.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

dfs/dfs.o: dfs/dfs.cpp dfs/dfs.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

bfs/bfs.o: bfs/bfs.cpp bfs/bfs.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---- Unit tests for DFS/BFS traversal reports ------------------------------
tests: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJ)
	$(CXX) $(TEST_OBJ) $(LDFLAGS) -o $(TEST_TARGET)

run-tests: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_OBJ) $(TEST_TARGET)
