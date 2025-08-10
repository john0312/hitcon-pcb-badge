.PHONY: format

/tmp/test-tetris: *.cc *.h
	g++ -Wall -Wextra -pedantic -g -O0 -DHITCON_TEST_MODE -o /tmp/test-tetris -I.. test-tetris.cc TetrisGame.cc

/tmp/test-bouncing: *.cc *.h
	g++ -Wall -Wextra -pedantic -g -O0 -DHITCON_TEST_MODE -o /tmp/test-bouncing -I.. test-bouncing.cc BouncingDVDApp.cc

format:
	clang-format -i *.cc *.h

test: /tmp/test-tetris /tmp/test-bouncing
	/tmp/test-tetris
	/tmp/test-bouncing
