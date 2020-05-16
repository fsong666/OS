#include "client.h"
#include <string.h>
#include <arpa/inet.h>

pthread_t       reading_thread;
int             client_socket;
//Add your global variables here

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_id;

void connect_to_server()
{
    //write your code here

    client_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP   );
    if( client_socket == -1 ){
        perror("socket error\n  ");
    }

    struct sockaddr_in client_addr;
    memset((char *)&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr= inet_addr("127.0.0.1 ");

    // onverts  the  unsigned  integer hostlong from host byte order to network byte order
    client_addr.sin_port = htons( SERVER_PORT);

    printf("SERVER_PORT = %d\n",SERVER_PORT );

    int result = connect(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr ) );
    if (result < 0) {
           perror("connect failed\n");
    }

}

//-------------------------------------------------------------

void handshake()
{

    //write your code here
     char *sendBuffer = "Hello, I'm the client";
     char recvBuffer[MAX_MESSAGE_LENGTH];

     ssize_t result = send(client_socket, sendBuffer, strlen(sendBuffer) , 0);
     if (result < 0) {
            perror("send failed\n");
            exit_client(-1);
     }

     ssize_t res = recv(client_socket, recvBuffer, sizeof(recvBuffer) , 0);
     if (res < 0) {
            perror("recv failed\n");
            exit_client(-1);
     }
     print_reply(recvBuffer);

}

//-------------------------------------------------------------

void send_message()
{

    //write your code here
    char buffer[MAX_USER_INPUT];
    char* user_input = NULL;


    while(1){

       user_input = prompt_user_input(buffer, MAX_USER_INPUT);
       printf("user_input = %s", user_input);

       if( user_input[0] == '\n')// client execute Get
          get_request( );
       else
          set_request( user_input );
    }

    int res = pthread_join(thread_id, NULL );
    if(res != 0){
        perror("pthread_join error");
    }

}

//-------------------------------------------------------------

void set_request(char* message)
{
    //write your code here


    pthread_mutex_lock( &mutex );

    char new [MAX_MESSAGE_LENGTH]= "s:";
    char recvBuffer[MAX_MESSAGE_LENGTH] = {0};

    for(unsigned long i = 0; i< strlen(message); i++ ){
               new[i + 2] = message[i];
    }

    printf("SET send message = %s\n", new  );

    ssize_t result = send(client_socket, new, (strlen(new) + 2) , 0);
    if (result < 0) {
        perror("send failed\n");
        exit_client(-1);
   }

   if ((result = recv(client_socket, recvBuffer, sizeof(message) , 0)) < 0){
        perror("recv error!");
        exit_client(-1);
    }
   if(strcmp(recvBuffer, "r:nack") == 0){

        prompt_error();
   }
   if(strcmp(recvBuffer, "r:ack") != 0 && strcmp(recvBuffer, "r:nack") != 0){

        print_reply(recvBuffer);
        exit_client(-1);
   }

   printf("SET recve message from server = %s \n", recvBuffer  );
   pthread_mutex_unlock( &mutex);
}

//-------------------------------------------------------------

void get_request()
{
    pthread_mutex_lock( &mutex );//mutex lock

    char recvBuffer[MAX_MESSAGE_LENGTH] = {0};
    char *get ="g:";
    ssize_t result = send(client_socket, get, 3, 0);
    if (result < 0) {
       perror("send failed\n");
       exit_client(-1);
   }

   if ((result = recv(client_socket, recvBuffer, sizeof(recvBuffer) , 0)) < 0){

       perror("recv error!");
       exit_client(-1);
   }
   if(strcmp(recvBuffer, "r:nack") != 0){//client send new message to server

       print_reply(recvBuffer);

   }
  if(strcmp(recvBuffer, "r:nack") == 0){
       print_reply(recvBuffer);
   }

    pthread_mutex_unlock( &mutex); //mutex_unlock
}

//-------------------------------------------------------------

void* read_continously(void* unused)
{
    (void) unused; //Mark variable as used for the compiler :-)

    //write your code here

    while(1){

        sleep(READING_INTERVAL);// Thread wait hier for the cycle

        get_request();

    }


    //this method should not return so dont care about return value
    return NULL; 
}

//-------------------------------------------------------------

void start_reader_thread()
{
    //write your code here

    int res = pthread_create(&thread_id, NULL, read_continously, NULL);
    if(res != 0){
        perror("pthread_create error");
        pthread_exit(NULL);
    }

}
