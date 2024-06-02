.PHONY: format

format:
	clang-format -i *.cc *.h

/tmp/test-display: test-display.cc display.cc
	gcc -DHITCON_TEST_MODE -o /tmp/test-display test-display.cc display.cc

test: /tmp/test-display
	/tmp/test-display
