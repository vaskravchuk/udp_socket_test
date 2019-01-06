#include <stdio.h>

#ifdef _WIN32
    #include <winsock2.h> // before Windows.h, else Winsock 1 conflict
    #include <ws2tcpip.h> // needed for ip_mreq definition for multicast
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib") // need for linking
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <time.h>
    #include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MSGBUFSIZE 256
#define ASCII_START 32
#define ASCII_END 126
#define LISTEN_CMD "listen"
#define SEND_CMD "send"

#define PORT "-p"
#define IP "-ip"
#define DELAY "-d"
#define COUNT "-c"
#define SIZE "-s"

int listen_udp(int argc, char *argv[]);
int send_udp(int argc, char *argv[]);
void show_help(char *name);
char* generate_random_string(int size);

int main(int argc, char *argv[]) {
    int result = 1;

    if (argc > 1 && strcmp(argv[1], LISTEN_CMD) == 0) {
        result = listen_udp(argc - 2, argv + 2);
    }
    else if (argc > 1 && strcmp(argv[1], SEND_CMD) == 0) {
        result = send_udp(argc - 2, argv + 2);
    }

    if (result != 0) {
        show_help(argv[0]);
    }

    return result;
}

void show_help(char *name) {
    printf("Examples:\n"
           "%s listen -p port\n"
           "%s send -ip ip -p port -s data_size -c data_count -d delay_sec", name, name);
}

int listen_udp(int argc, char *argv[]) {
    int port = 0;

    for (int i = 0; i < argc - 1; ++i) {
        char *cmd = argv[i];
        if (strcmp(cmd, PORT) == 0) {
            port = atoi(argv[++i]); // 0 if error, which is an invalid port
        } else {
            return 1;
        }
    }

    if (!port) {
        return 1;
    }

    printf("Listening: '%d'\n", port);

#ifdef _WIN32
    // Initialize Windows Socket API with given VERSION.
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        return 1;
    }
#endif

    // create what looks like an ordinary UDP socket
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    // allow multiple sockets to use the same PORT number
    u_int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        return 1;
    }

    // set up destination address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(port);

    // bind to receive address
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    int recieved = 0;
    printf("Recieved: %d", recieved);
    // now just enter a read-print loop
    while (1) {
        char msgbuf[MSGBUFSIZE];
        int addrlen = sizeof(addr);
        int nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, &addrlen);
        if (nbytes < 0) {
            perror("recvfrom");
            return 1;
        }
        msgbuf[nbytes] = '\0';
        recieved++;
        printf("\33[2K\r" "Recieved: %d", recieved);
        fflush(stdout);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

int send_udp(int argc, char *argv[]) {
    char* ip = NULL;
    int port = 0;
    int data_size = 0;
    int count = 0;
    float delay = 0;

    for (int i = 0; i < argc - 1; ++i) {
        char *cmd = argv[i];
        if (strcmp(cmd, PORT) == 0) {
            port = atoi(argv[++i]); // 0 if error, which is an invalid port
        } else if (strcmp(cmd, IP) == 0) {
            ip = argv[++i]; // 0 if error, which is an invalid port
        } else if (strcmp(cmd, SIZE) == 0) {
            data_size = atoi(argv[++i]); // 0 if error, which is an invalid port
        } else if (strcmp(cmd, COUNT) == 0) {
            count = atoi(argv[++i]); // 0 if error, which is an invalid port
        } else if (strcmp(cmd, DELAY) == 0) {
            delay = atof(argv[++i]); // 0 if error, which is an invalid port
        } else {
            return 1;
        }
    }

    if (!port || !ip || !data_size || !count || delay < 0) {
        return 1;
    }

#ifdef _WIN32
    //
    // Initialize Windows Socket API with given VERSION.
    //
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        return 1;
    }
#endif

    // create what looks like an ordinary UDP socket
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    // set up destination address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    printf("Sending to '%s:%d' '%d'\n"
           "Packet size: '%d'\n"
           "Packets amount: '%d'\n"
           "Sending delay: '%fs'\n",
           ip, port, data_size, count, delay);
    // now just sendto() our destination!
    int sent = 0;
    for (int i = 0; i < count; ++i) {
        char *message = generate_random_string(data_size);
        int nbytes = sendto(fd, message, data_size, 0, (struct sockaddr*) &addr, sizeof(addr));
        free(message);
        if (nbytes < 0) {
            perror("sendto");
            return 1;
        }

        sent++;
        printf("\33[2K\r" "Sent: %d", sent);
        fflush(stdout);

#ifdef _WIN32
        Sleep(delay * 1000); // Windows Sleep is milliseconds
#else
        usleep(delay * 1000000); // Unix sleep is seconds
#endif
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

char* generate_random_string(int size) {
    int i;
    char *res = malloc(size + 1);
    for(i = 0; i < size; i++) {
        res[i] = (char) (rand()%(ASCII_END-ASCII_START))+ASCII_START;
    }
    res[i] = '\0';
    return res;
}