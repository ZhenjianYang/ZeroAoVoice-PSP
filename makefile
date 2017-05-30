
export DEBUG
export WAV
export OGG

all: loader za_voice test

loader:
	make -f makefile.loader

za_voice:
	make -f makefile.za_voice

test:
	make -f makefile.test

loader.clean:
	make -f makefile.loader clean

za_voice.clean:
	make -f makefile.za_voice clean

test.clean:
	make -f makefile.test clean

rebuild: clean all

clean: loader.clean za_voice.clean test.clean

