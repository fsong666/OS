#include "../include/server.h"

#include "pthread.h"
#include "ring_buffer.h"
#include "stdlib.h"
#include "string.h"
#include "string.h"
#include "unistd.h"
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>


unsigned int    client_number_count = 0;//

//! HINT: maybe global synchronization variables are needed
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t lock1 = PTHREAD_RWLOCK_INITIALIZER ;


int initialize_server()
{

    //! implement this function

    int server_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP   );
    if( server_socket < 0 ){
        perror("server socket error\n  ");
        return 0;
    }

    struct sockaddr_in server_addr;
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr= INADDR_ANY;
    server_addr.sin_port = htons( SERVER_PORT );

    if( bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr ) ) < 0 ){
        perror("bind failed" );
        return 0;
    }

    if( listen(server_socket, MAX_SOCKET_QUEUE ) < 0 ){
        perror("listen failed");
        return 0;
    }


    return server_socket;
}

//-----------------------------------------------------------------------------

int handshake(int file_descriptor)
{

    //! Read message from client
    char    message_buffer[MAX_MESSAGE_LENGTH];
    ssize_t message_lenght = 0;
    if((message_lenght = read(file_descriptor, message_buffer, MAX_MESSAGE_LENGTH - 1)) < 1)
    {
        return -1;
    }
    message_buffer[MAX_MESSAGE_LENGTH - 1] = '\0'; //! Enforce NULL Terminated string
    printf("Handshake: %s\n", message_buffer);

    //! Reply message to client
    const char server_message[] = SERVER_HANDSHAKE_MSG;
    if(write(file_descriptor, server_message, sizeof(server_message)) < 0)
    {
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------

void* handle_connection(void* socket)
{
    //! HINT: Synchronization is needed in this function

    pthread_mutex_lock( &mutex );

    int file_descriptor = *(int*)socket;
    free(socket); // can also be freed when thread exits

    //! get client number
    unsigned int client_number = client_number_count;//[0, 9]
    client_number_count++;

    printf("adding reader no %d\n", client_number);

    //! get reader number for thread
    int thread_reader = ringbuffer_add_reader(client_number);

    //! handshake
    if(handshake(file_descriptor) < 0)
    {
        printf("handshake failed \n");
        ringbuffer_remove_reader(&thread_reader, file_descriptor);
        close(file_descriptor);

        pthread_mutex_unlock( &mutex );
        return NULL;
    }

    void* buffer = malloc(sizeof(char) * (MAX_MESSAGE_LENGTH));
    if(buffer == NULL)
    {
        perror("malloc");
        ringbuffer_remove_reader(&thread_reader, file_descriptor);
        close(file_descriptor);

        pthread_mutex_unlock( &mutex );
        return NULL;
    }

    pthread_mutex_unlock( &mutex );

    //! main loop for each thread to continuesly read from file descriptor and handle input
    while(1)
    {

        //! clean the buffer
        memset(buffer, 0, MAX_MESSAGE_LENGTH);

        int   number_of_read_bytes = read(file_descriptor, buffer, MAX_MESSAGE_LENGTH - 1);
        char* message              = (char*)buffer;
        message[MAX_MESSAGE_LENGTH - 1]   = '\0';

        //! error handling for read
        if(number_of_read_bytes < 0)
        {
            ringbuffer_remove_reader(&thread_reader, client_number);
            perror("read");
            if(close(file_descriptor) < 0)
            {
                perror("close");
            }
            break;
        }
        else if(number_of_read_bytes == 0)
        {
            ringbuffer_remove_reader(&thread_reader, client_number);
            if(close(file_descriptor) < 0)
            {
                perror("close");
            }
            printf("closing connection\n");
            break;
        }
        //! handle clients input
        if(handle_input(client_number, message, file_descriptor, &thread_reader) != 0)
        {
            ringbuffer_remove_reader(&thread_reader, client_number);
            close(file_descriptor);
            break;
        }
    }
    free(buffer);

    return NULL;
}

//-----------------------------------------------------------------------------

int handle_input(int client_number, char* input, int socket, int* current_reader_pointer)
{

    const char* error_message_1       = "r:invalid input: short message";
    const char* error_message_2       = "r:invalid input: unknown message type";
    const char* write_error_message   = "r:nack";
    const char* write_success_message = "r:ack";

    //! check message length
    if(sizeof(input) < 2 * sizeof(char))
    {

        if(write(socket, error_message_1, strlen(error_message_1) + 1) < 0)
        {
            perror("write");
            return -1;
        }
        return 0;
    }

    //! check first two chars of message
    char control_character = input[0];
    char delimiter         = input[1];
    if(delimiter != ':' || !(control_character == 'g' || control_character == 's'))
    {
        printf("invalid input\n");
        if(write(socket, error_message_2, strlen(error_message_2) + 1) < 0)
        {
            perror("write");
            return -1;
        }
        return 0;
    }

    char* message = ++input;
    message++;

    //! handle GET request
    if(control_character == 'g')
    {
        char buffer[MAX_MESSAGE_LENGTH - 2];

        ringbuffer_read_element(current_reader_pointer, buffer, client_number);

        printf("client %d read: %s length %zu\n", client_number, buffer, strlen(buffer));

        char message[MAX_MESSAGE_LENGTH] = "r:";

        strcat(message, buffer);

        if(write(socket, message, strlen(message) + 1) < 0)
        {
            perror("write");
            return (-1);
        }
        //! handle set request
    }
    //! handle SET request
    else if(control_character == 's')
    {

        //! write in reingbuffer
        int write_ack = ringbuffer_write_element(message);

        //! write failed
        if(write_ack != 0)
        {
            if(write(socket, write_error_message, strlen(write_error_message) + 1) < 0)
            {
                perror("write");
                return -1;
            }
            printf("client %d write failed\n", client_number);
        }
        //! write success
        else
        {
            if(write(socket, write_success_message, strlen(write_success_message) + 1) < 0)
            {
                perror("write");
                return -1;
            }
            printf("client %d write: %s length %zu\n", client_number, message, strlen(message));
        }
    }
    else
    {
        printf("an unknown error occured\n");
        return (-1);
    }

    return 0;
}

//-----------------------------------------------------------------------------

void accept_connections(int socket_number)
{
//    (void) socket_number; //please delete  this line when you implement this function
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr= INADDR_ANY;
    client_addr.sin_port = 0;

    socklen_t client_addr_len = sizeof(client_addr);

    pthread_attr_t attr;

    if (pthread_attr_init( &attr ) != 0){
       perror("pthread_attr_init");
    }
    if( pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED ) != 0){
       perror("pthread_attr_setdetachstate");
    }

    while( 1 ){
         pthread_t thread_id;

        int* connected_socket = (int*) malloc(sizeof(int));
        //when client_addr try to connect with  the socket_number, accept return then *connected_socket
        //otherwise blockt main thread
       *connected_socket = accept(socket_number, (struct sockaddr *)&client_addr, &client_addr_len);

       int res = pthread_create(&thread_id, &attr, handle_connection, (void*)connected_socket);
       if(res != 0){
            perror("pthread_create error");
        }

    }

}
