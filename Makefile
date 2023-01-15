CPP = g++

all: prep duino-avr-emu

prep:
	mkdir -p build

duino-avr-emu: src/*.cpp
	$(CPP) $^ -lcrypto -o build/$@