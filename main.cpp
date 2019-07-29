#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <thread>


#include <iostream>
#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

//#include <curses.h>

#define NL_0A_EN


using namespace std;

class HyTermBase
{
    int uart_fd;
public:
    HyTermBase(void):
        uart_fd(-1)
    {

    };


    int initUart(const std::string & device, int bauds)
    {
        const char * c_device = device.c_str();

        int fd = open(c_device, O_RDWR | O_NDELAY | O_NOCTTY);
        if (fd == -1)
        {
            perror((string("can't open ") + device).c_str());
            exit(errno);
        }

        struct termios config;
        if (tcgetattr(fd, &config) < 0)
        {
            perror("can't get serial attributes");
            exit(errno);
        }

        if (bauds > 0)
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

        uart_fd = fd;
        return fd;

    }

    int close(void)
    {
        return ::close(uart_fd);
    }

    int inputProc(void)
    {
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

            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, input.c_str());

#ifdef DEBUG_EN
            printf("buf(%d) = [%s]\n", n, buffer);
#endif

            if (input == "qq")
                break;


            buffer[n++] = 0xa;
            n = write(uart_fd, buffer, n);

        }

        return 0;
    }

    virtual int outputProc(void)
    {
        int fd = uart_fd;

        printf("======== HyTerm Start ====== \n");

        char buffer[10240];
        while (true)
        {
            char * pbuf = buffer;
            int ret = read(fd, pbuf, sizeof(buffer));


            if (ret <= 0)
                continue;

            int n = ret;

            int n_fl = 0;
            char out_buf[10240] = {0};

            for (int i = 0; i < n; ++i)
            {
                char ch = *pbuf++;
                if (ch == 0 || ch == '\r')
                    continue;

                out_buf[n_fl++] = ch;

            }

            //write(STDOUT_FILENO, buffer, n);
            write(STDOUT_FILENO, out_buf, n_fl);
            fflush(stdout);

        }

        return 0;
    }

};

int main(int argc, const char * argv[])
{
    if (argc != 3)
    {
        cerr << "usage: " << argv[0] << " device [bauds]" << endl;
        return 1;
    }

    HyTermBase ht_base;

    string device = argv[1];
    int bauds = atoi(argv[2]);

    ht_base.initUart(argv[1], bauds);

    std::thread th_output(&HyTermBase::outputProc, &ht_base);
    ht_base.inputProc();
    ht_base.close();

    return 0;
}

