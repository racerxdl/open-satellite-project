all: aptdec

binfolder:
	@echo Creating folder bin
	@mkdir -p bin

dsp: binfolder
	@echo Compiling dsp.c
	@gcc -c lib/dsp.c -o bin/dsp.o

aptdec: binfolder dsp
	@echo Compiling aptdec

clean:
	rm -fr bin