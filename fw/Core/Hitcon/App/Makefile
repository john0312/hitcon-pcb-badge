.PHONY: format

/tmp/test-tetris: *.cc *.h
	g++ -Wall -Wextra -pedantic -g -O0 -DHITCON_TEST_MODE -o /tmp/test-tetris -I.. test-tetris.cc TetrisGame.cc

format:
	clang-format -i *.cc *.h

test: /tmp/test-tetris
	/tmp/test-tetris
