#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


const char PORT[]     = "9000";
const int  BACKLOG    = 4;
const char FILENAME[] = "/var/tmp/aesdsocketdata";

const int RETURN_FAILURE = -1;

bool signal_caught = false;

// TODO: signal handler
void sighandler(int signum){
    if (signum == SIGINT || signum == SIGTERM){
        signal_caught = true;
    }
}


int setup_socket(){
    struct addrinfo hints;
    struct addrinfo *servinfo; // will point to the results

    memset(&hints, 0, sizeof hints);

    hints.ai_family   = AF_UNSPEC;   // don't care if ip4 or ip6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags    = AI_PASSIVE;  // fill in my ip for me

    int status = getaddrinfo(NULL, PORT, &hints, &servinfo);

    if (status != 0){
        syslog(LOG_ERR, "ERROR: getaddrinfo failed, rc = %d, error = %s\n", status, gai_strerror(status));
        exit(RETURN_FAILURE);
    }
 
    int socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    if (socket_fd < 0) {
        syslog(LOG_ERR, "socket() failed");
        exit(RETURN_FAILURE);
    }

    int reuseaddr = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    status = bind(socket_fd, servinfo->ai_addr, sizeof(struct sockaddr));
    freeaddrinfo(servinfo);
    if (status < 0)
    {
        syslog(LOG_ERR, "bind() failed");
        exit(RETURN_FAILURE);
    }


    //Listen
    status = listen(socket_fd, BACKLOG);
    if (status < 0){
        syslog(LOG_ERR, "listen() failed");
        exit(RETURN_FAILURE);
    }

    return socket_fd;
}

int send_file_content(int client_fd){
    FILE *f = fopen(FILENAME, "rb");

    if (f == NULL){
        return -1;
    }

    char buf[1024] = { 0 };
    int total_bytes_read = 0;
    int total_bytes_sent = 0;
    size_t bytes_read = 0;
    while ( (bytes_read = fread(buf, sizeof(buf[0]), sizeof buf, f)) > 0){
        int bytes_sent = 0;
        if (bytes_read < sizeof buf && ferror(f)){
            fclose(f);
            return -1;
        }
        if (bytes_read < sizeof buf && feof(f)){
            bytes_sent = send(client_fd, buf, bytes_read, 0);
            if (bytes_sent == -1) {
                syslog(LOG_ERR, "(send_file_contents): cannot send over socket: %s\n", strerror(errno));
            }
            
            total_bytes_read += bytes_read;
            total_bytes_sent += bytes_sent;
            break;
        }
        bytes_sent = send(client_fd, buf, bytes_read, 0);
        if (bytes_sent == -1) {
            syslog(LOG_ERR, "(send_file_contents): cannot send over socket: %s\n", strerror(errno));
        }
        total_bytes_read += bytes_read;
        total_bytes_sent += bytes_sent;
        memset(buf, 0, sizeof buf);
    }
    fclose(f);
    if (total_bytes_sent != total_bytes_read){
        syslog(LOG_ERR, "(send_file_contents): total_bytes_sent = %d, total_bytes_read = %d\n",
                total_bytes_sent,
                total_bytes_read);
        return -1;
    }
    return total_bytes_read;
}


void daemonize()
{
    pid_t childpid = fork();
    if (childpid == -1){
        perror("fork");
        exit(RETURN_FAILURE);
    }
    if (childpid > 0){
        exit(EXIT_SUCCESS);
    }
    else if (childpid == 0){
        // We are in the child
        // setsid()
        pid_t sidpid = setsid();
        if (sidpid < 0) {
            perror("setsid");
            exit(RETURN_FAILURE);
        }
        // chdir("/")
        int rc = chdir("/");
        if (rc < 0){
            perror("chdir");
            exit(RETURN_FAILURE);
        }

        // redir stdin, stdout, stderr to /dev/null
        int devnull_fd = open("/dev/null", O_RDWR);
        if (devnull_fd == -1){
            perror("open devnull");
            exit(RETURN_FAILURE);
        }
        dup2(devnull_fd, STDOUT_FILENO);
        dup2(devnull_fd, STDERR_FILENO);
    }
}

int main(int argc, char** argv){
    const bool daemon_mode = (argc > 1 && strcmp(argv[1], "-d") == 0);

    
    openlog(NULL, 0, LOG_USER);

    syslog(LOG_INFO, "aesdsocket started (%s). %s, pid = %d", daemon_mode ? "daemon mode" : "interactive mode",
           argv[0], getpid());
    

    // Socket stuff
    int sockfd = setup_socket();

    if (daemon_mode){
        daemonize();
    }

    // Signal stuff
    int  rc_signal      = 0;
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = sighandler;

    rc_signal = sigaction(SIGTERM, &new_action, NULL);
    if (rc_signal != 0){
        syslog(LOG_ERR, "Error %d (%s) registrering for SIGTERM", errno, strerror(errno));
        exit(RETURN_FAILURE);
    }
    rc_signal = sigaction(SIGINT,  &new_action, NULL);
    if (rc_signal != 0){
        syslog(LOG_ERR, "Error %d (%s) registrering for SIGINT", errno, strerror(errno));
        exit(RETURN_FAILURE);
    }
    syslog(LOG_INFO, "Signal handlers installed");

    
    syslog(LOG_INFO, "setup_socket done, sockfd = %d", sockfd);
    
    struct sockaddr client_addr = { 0 };
    socklen_t client_addr_len   = sizeof(client_addr);
    bool failure                = false;
    while (true){
        memset(&client_addr, 0, sizeof client_addr);
        int client_fd = accept(sockfd, &client_addr, &client_addr_len);
        if (client_fd < 0){
            if (errno == EINTR){
                signal_caught = true;
                close(client_fd);
                break;
            }
            else{
                syslog(LOG_ERR, "Failed on accept(), errno = %d: %s", errno, strerror(errno));
                failure = true;
                close(client_fd);
                break;
            }
        }

        const char *ip = inet_ntoa(((struct sockaddr_in*)&client_addr)->sin_addr);
        syslog(LOG_INFO, "Accepted connection from %s", ip);

        FILE* outfile = fopen(FILENAME, "a");
        if (outfile == NULL){
            syslog(LOG_ERR, "Failed to open outfile %s\n", FILENAME);
            close(client_fd);
            failure = true;
            break;
        }
        char buf[1024] = { 0 };
        ssize_t bytes = 0;
        ssize_t total_bytes = 0;
        while (    (bytes = recv(client_fd, buf, sizeof(buf), 0)  ) > 0    ){
            size_t res_writetofile = fwrite(buf, sizeof buf[0], (size_t) bytes, outfile);
            if (res_writetofile < bytes){
                syslog(LOG_ERR, "Failed to write to file %s, errno = %d: %s", FILENAME, errno, strerror(errno));
                fclose(outfile);
                failure = true;
                break;
            }

            fflush(outfile);
            total_bytes += bytes;

            if (buf[bytes-1] == '\n'){
                if (send_file_content(client_fd) < 0){
                    syslog(LOG_ERR, "ERROR: could not send file contents");
                }
            }
            memset(buf, 0, sizeof buf);
        }
        if (bytes < 0){
            if (errno == EINTR){
                signal_caught = true;
                break;
            }
            else{
                syslog(LOG_ERR, "Failed on recv(), errno = %d: %s", errno, strerror(errno));
                failure = true;
                break;
            }
        }

        fclose(outfile);
        syslog(LOG_INFO, "Closed connection from %s", ip);
        close(client_fd);
        if (failure == true || signal_caught == true){
            break;
        }
        if (signal_caught == true){
            break;
        }
    }

    if (signal_caught == true){
        syslog(LOG_INFO, "Caught signal, exiting\n");
        int unlink_res = unlink(FILENAME);
        if (unlink_res == -1){
            syslog(LOG_ERR, "Failed to unlink outfile %s, errno = %d: %s", FILENAME, errno, strerror(errno));
        }
    }
    
    close(sockfd);
    closelog();
    return EXIT_SUCCESS;
}
