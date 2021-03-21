#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(char *argv[])
{
    int i, portno, sck, bytes, sent, received, total, message_size = 0;
    char host[25], *message, method[5], path[128], headers[256], query[256], body[256], response[4096], GET[5] = "GET", POST[5] = "POST";
    struct hostent *server;
    struct sockaddr_in serv_addr;

    printf("Enter Port No :");
    scanf("%d", &portno);

    if (portno == 0)
    {
        portno = 80;
    }

    printf("Enter HostName :");
    scanf("%s", host);

    printf("Enter method GET or POST :");
    scanf("%s", method);

    if (!(strcmp(method, GET)) && !(strcmp(method, POST)))
    {
        printf("Invalid Method. Try Again");
    }

    if (!strcmp(method, GET))
    {
        message_size += strlen("%s %s%s%s HTTP/1.1\r\n");
        message_size += strlen(method);

        printf("Enter Query String if any :");
        char temp[4];
        fgets(temp, sizeof temp, stdin);
        fgets(query, sizeof query, stdin);
        if (strcmp(query, "\n"))
            message_size += strlen(query);
    }
    else
    {
        message_size += strlen("%s %s HTTP/1.1\r\n");
        message_size += strlen(method);

        message_size += strlen("Content-Length: %d\r\n") + 10;
        message_size += strlen("\r\n");

        printf("Enter Body :");
        scanf("%s", body);
        message_size += strlen(body);
    }

    printf("Enter path :");
    scanf("%s", path);

    message_size += strlen(path);

    int headnum;
    printf("Enter the number of headers :");
    scanf("%d", &headnum);

    if (headnum > 0)
    {
        char *type, *value;
        for (i = 0; i < headnum; i++)
        {
            printf("Enter <headerType> <headerValue> :");
            scanf("%s %s", type, value);
            message_size += strlen(type);
            strcat(headers, type);
            message_size += strlen(": ");
            strcat(headers, ": ");

            message_size += strlen(value) + strlen("\r\n");
            strcat(headers, value);
            strcat(headers, "\r\n");
        }
    }

    message_size += strlen("\r\n");

    message = malloc(message_size);

    if (!strcmp(method, "GET"))
    {
        if (strcmp(query, "\n"))
            sprintf(message, "%s %s%s%s HTTP/1.1\r\n",
                    method,
                    strlen(path) > 0 ? path : "/",
                    "?",
                    query);
        else
            sprintf(message, "%s %s HTTP/1.1\r\n",
                    method,
                    strlen(path) > 0 ? path : "/");
        strcat(message, headers);
        strcat(message, "\r\n");
    }
    else
    {
        sprintf(message, "%s %s HTTP/1.1\r\n",
                method,
                strlen(path) > 0 ? path : "/");
        strcat(message, headers);
        sprintf(message + strlen(message), "Content-Length: %lu\r\n", strlen(body));
        strcat(message, "\r\n");
        strcat(message, body);
    }

    printf("\nRequest:\n%s\n", message);

    sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0)
    {
        printf("ERROR opening socket");
        exit(0);
    }

    server = gethostbyname(strlen(host) > 0 ? host : "localhost");
    if (server == NULL)
    {
        printf("ERROR, no such host");
        exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sck, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR connecting");
        exit(0);
    }

    total = strlen(message);
    sent = 0;
    do
    {
        bytes = write(sck, message + sent, total - sent);
        if (bytes < 0)
        {
            printf("ERROR writing message to socket");
            exit(0);
        }
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);

    memset(response, 0, sizeof(response));
    total = sizeof(response) - 1;
    received = 0;
    do
    {
        bytes = read(sck, response + received, total - received);
        if (bytes < 0)
        {
            printf("ERROR reading response from socket");
            exit(0);
        }
        if (bytes == 0)
            break;
        received += bytes;
    } while (received < total);

    if (received == total)
    {
        printf("ERROR storing complete response from socket");
        exit(0);
    }

    close(sck);

    printf("Response:\n%s\n", response);

    free(message);
    return 0;
}