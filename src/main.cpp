#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <openssl/sha.h>
#include <bitset>

struct termios tty;
int serialPort;
char buffer[256];
char lastblockhash[41] = {0};
char newblockhash[41] = {0};
char diffStr[4] = {0};
char DUCOID[23] = {0};
uint16_t difficulty = 0;
uint16_t ducos1result = 0;
const uint16_t job_maxsize = 104;
uint8_t job[job_maxsize];

void initSerial(const char *path)
{
    serialPort = open(path, O_RDWR);

    if (serialPort < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    if (tcgetattr(serialPort, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    if (tcsetattr(serialPort, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
}

void setup(const char *serial)
{
    std::srand(std::time(nullptr));
    int uuid1 = std::rand();
    std::srand(std::time(nullptr) * 2);
    int uuid2 = std::rand();

    sprintf(DUCOID, "DUCOID%8X%8X", uuid1, uuid2);

    initSerial(serial);
}

uint16_t ducos1a(char *lastblockhash, char *newblockhash, uint16_t difficulty, int hash_time)
{
    for (int i = 0; newblockhash[i] != 0; i++)
    {
        if (newblockhash[i] >= 'a' && newblockhash[i] <= 'f')
        {
            newblockhash[i] -= 'a';
            newblockhash[i] += 'A';
        }
        i++;
    }

    uint8_t final_len = 40 >> 1;
    for (uint8_t i = 0, j = 0; j < final_len; i += 2, j++)
        job[j] = ((((newblockhash[i] & 0x1F) + 9) % 25) << 4) + ((newblockhash[i + 1] & 0x1F) + 9) % 25;

    for (uint16_t ducos1res = 0; ducos1res < difficulty * 100 + 1; ducos1res++)
    {
        uint8_t hash_bytes[20];
        char str[46] = {0};
        int dsize = sprintf(str, "%s%d", lastblockhash, ducos1res);
        SHA1((unsigned char *)str, dsize, hash_bytes);
        if (memcmp(hash_bytes, job, 20) == 0)
            return ducos1res;
        usleep(hash_time * 1000);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: build/duino-avr-emu [PORT] [HASHRATE]\n");
        exit(0);
    }

    setup(argv[1]);

    uint32_t time = 0;
    uint16_t numShares = 0;
    float hashrate = (float)atoi(argv[2]);
    int hashtime = 1000 / (int)hashrate;

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        while (buffer[0] == 0)
            read(serialPort, buffer, sizeof(buffer));

        memcpy(lastblockhash, buffer, 40);
        memcpy(newblockhash, buffer + 41, 40);
        memcpy(diffStr, buffer + 82, 4);

        uint16_t res = ducos1a(lastblockhash, newblockhash, atoi(diffStr), hashtime);
        std::bitset<16> resb(res);
        float tmp = (float)res / hashrate * 1000000.0;
        time = (int)tmp;
        std::bitset<32> elapsedTime(time);
        int rsize = sprintf(buffer, "%s,%s,%s\n", resb.to_string().c_str(), elapsedTime.to_string().c_str(), DUCOID);
        write(serialPort, buffer, rsize);
        numShares++;
    }
}