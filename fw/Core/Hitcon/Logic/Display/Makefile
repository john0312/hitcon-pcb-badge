.PHONY: format

format:
	clang-format -i *.cc *.h

/tmp/test-editor: *.cc *.h
	g++ -g -O0 -DHITCON_TEST_MODE -o /tmp/test-editor -I../.. test-editor.cc editor.cc display.cc

/tmp/test-display: *.cc *.h
	g++ -g -O0 -DHITCON_TEST_MODE -o /tmp/test-display -I../.. test-display.cc editor.cc display.cc

test: /tmp/test-display /tmp/test-editor
	/tmp/test-display
	/tmp/test-editor
