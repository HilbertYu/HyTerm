#include <stdio.h>
#include <errno.h>


#include <iostream>
#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

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

    if (cfsetispeed(&config, bauds) < 0 || cfsetospeed(&config, bauds) < 0)
    {
        perror("can't set baud rate");
        exit(errno);
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

int main(int argc, const char * argv[])
{

    int fd = initUart(argc, argv);

    // close (fd);
    // return 0;

    char buffer[1024];
    int fid = 0;
    while (true)
    {
        errno = 0;
        int n = 0;

        {
            int rem = sizeof(buffer) - 1;
            memset(buffer, 0, sizeof(buffer));

            char *pbuf = buffer;
            while (1)
            {
                int tmp = read(fd, pbuf, rem);
                printf("tmp = %d\n", tmp);
                if (tmp <= 0)
                    break;

                n += tmp;
                pbuf += tmp;
                rem -= tmp;
            }
        }

        if (n < 0)
        {
            perror("");
        }
        else
        {
            write(STDOUT_FILENO, buffer, n);
            printf("OUT n = %d\n", n);
#if 0
            if (n == 3)
            {
                printf("%x,%x,%x\n",
                    buffer[0],
                    buffer[1],
                    buffer[2]);

            }
#endif


        }



        fflush(stdout);



#if 1
        //        int n;
        string input;
        printf("[%d][========== DEBUG---- cin ====]\n", ++fid);
        getline(cin, input);
        n = input.size();

        if (input == "x")
            continue;


        memset(buffer, 0, sizeof(buffer));


        //
        strcpy(buffer, input.c_str());
        printf("buf(%d) = [%s]\n", n, buffer);

        if (input == "qq")
            break;


#endif
        //       continue;
        //    n = 0;
        //        buffer[n++] = 0xd;
        buffer[n++] = 0xa;
        //
        //
        // //n = write(fd, &ent, 1);
        n = write(fd, buffer, n);

    }

    close(fd);
    return 0;
}
