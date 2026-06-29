all: hello blah

hello:
	echo "Hello World"
blah: src/scanner.c
	cc src/scanner.c -o scanner1
