CXX      = g++
CXXFLAGS = -std=c++20 -fno-pie -no-pie -I../ldf
LDFLAGS  = -lCatch2Main -lCatch2

TEST_SRCS := $(wildcard test_*.cpp)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)

.PHONY: test clean disasm exec

# Catch2 test target — picks up all test_*.cpp files
test: tests
	./tests

tests: $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Legacy targets
vpath ../ldf
main: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main

disasm: main
	objdump -xsSD main > main.dis

exec: main
	setarch $(shell uname -m) -R ./main

clean:
	rm -f $(TEST_OBJS) tests main main.dis
