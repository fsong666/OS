#include "../include/ring_buffer.h"
#include "../include/reads_list.h"
#include "../include/server.h"

#include "pthread.h"
#include "semaphore.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

typedef struct ring_buffer_element
{
    //! HINT: something is missing here
    char text[MAX_MESSAGE_LENGTH] ;
    int             reader_count;// to_read

} ring_buffer_element_t;


static ring_buffer_element_t ring_buffer[RINGBUFFER_SIZE];

unsigned int current_writer = 0;

unsigned int number_of_readers = 0;// clients

//! HINT: maybe global synchronization variables are needed

pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------------

int ringbuffer_write_element(char* text)
{
    //! HINTS: Check if thread can read a new element, and synchronization will be needed
    /* ... */

    pthread_mutex_lock( &mutex2 );

    if( ring_buffer[current_writer % RINGBUFFER_SIZE].reader_count == 0  ){

        //! Write element
        strcpy(ring_buffer[current_writer % RINGBUFFER_SIZE].text, text);

        ring_buffer[current_writer % RINGBUFFER_SIZE].reader_count = number_of_readers;

        reads_list_increment_all();

        current_writer++;

    }else {
        pthread_mutex_unlock( &mutex2 );
        return -1;
    }

    pthread_mutex_unlock( &mutex2 );

    return 0;
}

//-----------------------------------------------------------------------------

void ringbuffer_read_element(int* current_reader, char* buffer, unsigned int client_number)
{

    int reader = *current_reader;
    //! HINT: Check if thread can read a new element & synchronization will be needed
    /* ... */

      pthread_mutex_lock( &mutex2 );

    int reads2 = reads_list_get_reads(client_number);
    if( ring_buffer[reader % RINGBUFFER_SIZE].reader_count == 0 && reads2 == 0 ){

        char* msg = "nack";
        strcpy(buffer, msg);

    }else if( current_writer > 0 ){

        ring_buffer[reader % RINGBUFFER_SIZE].reader_count--;//to_read--

        reads_list_decrement(client_number);//reads--

        //! Read Element
        strcpy(buffer, (const char*)ring_buffer[reader % RINGBUFFER_SIZE].text);

        //! HINT: notify the writer

        //! Update reader count
        (*current_reader)++;
    }

    pthread_mutex_unlock( &mutex2 );

    return;
}

//-----------------------------------------------------------------------------

int ringbuffer_add_reader(unsigned int client_number)
{
    //! HINT: synchronization is needed in this function

    pthread_mutex_lock( &mutex2 );

    if(reads_list_insert_element(client_number) != 0)
    {
        exit(EXIT_FAILURE);
    }

    number_of_readers++;
    int new_reader = current_writer;

    pthread_mutex_unlock( &mutex2 );

    return new_reader;
}

//-----------------------------------------------------------------------------

void ringbuffer_remove_reader(int* current_reader, unsigned int client_number)
{
    //! HINT: synchronization is needed in this function

    pthread_mutex_lock( &mutex2 );

    int reads = reads_list_get_reads(client_number);

    pthread_mutex_unlock( &mutex2 );// prevent deadlock

    //! perform all unfinished reads for the disconnected client
    while(reads != 0)
    {
        char buffer[MAX_MESSAGE_LENGTH];

        ringbuffer_read_element(current_reader, buffer, client_number);// prevent deadlock
        reads = reads_list_get_reads(client_number);
    }

    pthread_mutex_lock( &mutex2 );

    reads_list_remove_reader(client_number);
    number_of_readers--;

    pthread_mutex_unlock( &mutex2 );

    return;
}
