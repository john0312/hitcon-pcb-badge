.PHONY: format

/tmp/test-tetris: *.cc *.h
	g++ -g -O0 -DHITCON_TEST_MODE -o /tmp/test-tetris -I.. test-tetris.cc TetrisGame.cc ../Logic/Display/display.cc ../Logic/Display/editor.cc

format:
	clang-format -i *.cc *.h

test: /tmp/test-tetris
	/tmp/test-tetris
