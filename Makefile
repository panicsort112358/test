CXXFLAGS = -std=c++20 -Wall -Wextra -O2

basic: examples/basic.cpp test.hpp
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: test clean

test: basic
	./basic

install: test
	install -d /usr/local/include
	cp test.hpp /usr/local/include/test.hpp

clean:
	rm -f basic