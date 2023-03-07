# Duino-AVR-emu
A tool to emulate the connection between Duino-Coin and an Arduino
# DISCLAIMER
This tool has been made for testing purposes only as it is very powerful. Using this tool to mine could get you banned from the duino-coin network. I am **not** responsible for any damage to your duino-coin account.
## Building
### Requirements
* g++
* make
* openssl
* socat - only required to create a virtual serial connection

To build Duino-AVR-emu navigate to the root of this project and run `make`. All output files will be stored under the `build/` directory.

## Using Duino-AVR-emu
`build/duino-avr-emu /dev/pts/X 256 1000`

This will create a connection on the virtual serial port /dev/pts/X. When running the AVR_Miner.py script from Duino-Coin the device connected to it (the emulator) will have a hashrate of 256 h/s and will take 1000 ms per share.

* Replace the first argument with the path to the virtual serial port created by socat
* Replace the second argument with the desired hashrate
* Replace the third argument with the time required for each share in ms

### Warning
Setting the hasrate too high and/or the hash time too low could get you banned from the Duino-Coin network.

After this run the AVR_Miner.py script from Duino-Coin. You may need to reconfigure the serial port in the Settings.cfg file.

## Creating the virtual serial port
In order to create the virtual serial port run: `socat -d -d pty,raw,echo=0 pty,raw,echo=0`
This will output something like this:

* 2023/01/16 17:55:43 socat[3185] N PTY is /dev/pts/3
* 2023/01/16 17:55:43 socat[3185] N PTY is /dev/pts/4

Open a new terminal and enter the path of the first output (/dev/pts/3) into the port argument of the emulator. Enter the second path inside the Duino-Coin's Settings.cfg.


## License
All files within this repo are released under the GNU GPL V3 License as per the LICENSE file stored in the root of this repo.
