FILES = $(wildcard *.c) $(wildcard *.h)

wii2serial: $(FILES)
	clang -o wii2serial -arch x86_64 -lwiiuse *.c