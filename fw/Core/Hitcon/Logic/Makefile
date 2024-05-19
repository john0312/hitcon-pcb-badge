.PHONY: format

format:
	clang-format -i *.c *.h

/tmp/test-game: test-game.cc game.cc
	g++ -DHITCON_TEST_MODE -o /tmp/test-game test-game.cc game.cc

/tmp/test-infrared: test-infrared.cc infrared.cc
	gcc -DHITCON_TEST_MODE -o /tmp/test-infrared test-infrared.cc infrared.cc

test: /tmp/test-game /tmp/test-infrared
	/tmp/test-infrared
	/tmp/test-game
