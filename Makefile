CC := clang

tests/build/stringbuffertests: bin/munit.o bin/string_buffer.o bin/string_buffer_tests.o
	$(CC) $^ -o $@

bin/munit.o: munit/munit.h munit/munit.c
	mkdir -p bin
	$(CC) -c munit/munit.c -Imunit -o $@

bin/string_buffer.o: src/string_buffer.c include/string_buffer.h
	mkdir -p bin
	$(CC) -c src/string_buffer.c -Iinclude -o $@

bin/string_buffer_tests.o: tests/src/string_buffer_tests.c include/string_buffer.h
	mkdir -p bin
	$(CC) -c tests/src/string_buffer_tests.c -Iinclude -Imunit -o $@
