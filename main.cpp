#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>


#include <iostream>
#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include <curses.h>


using namespace std;

int initUart(int argc, const char * argv[])
{
    if (argc < 2 || argc > 3)
    {
        cerr << "usage: " << argv[0] << " device [bauds]" << endl;
        return 1;
    }

    string device = argv[1];
    unsigned long bauds = 9600;
    if (argc == 3)
    {
        char* result;
        bauds = strtoul(argv[2], &result, 10);
        if (*result != '\0')
        {
            cerr << "usage: " << argv[0] << " device [bauds]" << endl;
            return 1;
        }
    }

    int fd = open(argv[1], O_RDWR | O_NDELAY | O_NOCTTY);
    if (fd == -1)
    {
        perror((string("can't open ") + argv[1]).c_str());
        exit(errno);
    }

    struct termios config;
    if (tcgetattr(fd, &config) < 0)
    {
        perror("can't get serial attributes");
        exit(errno);
    }

    if (bauds != 5566)
    {

        if (cfsetispeed(&config, bauds) < 0 || cfsetospeed(&config, bauds) < 0)
        {
            perror("can't set baud rate");
            exit(errno);
        }
    }
    else
    {
        printf("Bypass bauds\n");

    }

    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    config.c_oflag = 0;
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= CS8;
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &config) < 0)
    {
        perror("can't set serial attributes");
        exit(errno);
    }

    return fd;

}

void* f(void* arg)
{
    int fd = *(int*)arg;

    printf("======== HyTerm ====== \n");

    char buffer[10240];
    while (true)
    {
        char * pbuf = buffer;
        errno = 0;
        int ret = read(fd, pbuf, sizeof(buffer));

        if (ret == 0)
        {
            perror("");
        }

        if (ret <= 0)
            continue;

        int n = ret;

        write(STDOUT_FILENO, buffer, n);
        fflush(stdout);

    }

    return NULL;
}

int main(int argc, const char * argv[])
{

    int fd = initUart(argc, argv);

    pthread_t pth;
    pthread_create(&pth, NULL,f,  &fd);

    char buffer[1024];
    while (true)
    {
        errno = 0;
        int n = 0;

        string input;

#ifdef DEBUG_EN
        printf("[%d][========== DEBUG---- cin ====]\n", ++fid);
#endif
        getline(cin, input);

        n = input.size();


        if (input == "x")
            continue;


        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, input.c_str());

#ifdef DEBUG_EN
        printf("buf(%d) = [%s]\n", n, buffer);
#endif

        if (input == "qq")
            break;


        buffer[n++] = 0xa;
        n = write(fd, buffer, n);

    }

    close(fd);
    return 0;
}
