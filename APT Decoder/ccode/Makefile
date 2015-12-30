all: aptdec

binfolder:
	@echo Creating folder bin
	@mkdir -p bin

wav: binfolder
	@echo Compiling wav.c
	@gcc -c -g lib/wav.c -o bin/wav.o

dsp: binfolder
	@echo Compiling dsp.c
	@gcc -c -g lib/dsp.c -o bin/dsp.o

aptdec: binfolder dsp wav
	@echo Compiling aptdec
	gcc -g aptdec.c bin/dsp.o bin/wav.o -lm -o bin/aptdec

clean:
	rm -fr bin