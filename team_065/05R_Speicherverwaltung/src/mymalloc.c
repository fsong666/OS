#include "mymalloc.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colorCodes.h"

chunk_t* root = NULL;

//===================================================================================================================//

//! --------------------------------
//! Write your helper functions here
//! --------------------------------

void set_block( memory_block_t* block , size_t size)
{
    block->next = MAGIC_NUMBER;
    block->previous = MAGIC_NUMBER;
    block->size = size;
}
void insert_free_block(memory_block_t* tmp , size_t size)
{
    memory_block_t* new_block =NULL;
    new_block = tmp + 1 + size / sizeof(memory_block_t);
    new_block->size = tmp->size - size -sizeof(memory_block_t);

    if( tmp->previous == NULL && tmp->next == NULL){
        root->free_space_list_entry_point = new_block;
        new_block->next = NULL;
        new_block->previous = NULL;
    }else if (tmp->previous == NULL){// discrete first block
        root->free_space_list_entry_point = new_block;
        new_block->next = tmp->next;
        new_block->previous = NULL;
        tmp->next->previous = new_block;
    }else if (tmp->next == NULL){// discrete last block
        new_block->next = NULL;
        new_block->previous = tmp->previous;
        tmp->previous->next = new_block;
    }else{//to belegte block bettween two free block

        new_block->next = tmp->next;
        tmp->next->previous = new_block;

        tmp->previous->next = new_block;
        new_block->previous = tmp->previous;
    }
}
size_t sum_free( chunk_t* root)
{
    memory_block_t* tmp = root->free_space_list_entry_point;
    if( tmp->next == NULL){
    size_t sum = tmp->size;
    return sum;
    }
    size_t sum =0;
    while(tmp->next != NULL){
        sum = sum + tmp->size;
        tmp = tmp->next;
        if(tmp->next == NULL){
        }
    }
    sum = sum + tmp->size;
    return sum;
}
int RightAddress(void* memory_location) {
    if(root != NULL && memory_location >= (void*)(root + 1)&&
       memory_location < (void*)(root + 1 + root->size / sizeof(memory_block_t))){
       return 1;
    }
     
    return 0;
}
void free_inLow( memory_block_t* block)
{
    block->next = root->free_space_list_entry_point;
    root->free_space_list_entry_point->previous = block;
    root->free_space_list_entry_point = block;
    block->previous = NULL;
}
void free_inBetween( memory_block_t* block, memory_block_t* hoch)
{
    hoch->previous->next = block;
    block->previous = hoch->previous;
    block->next = hoch;
    hoch->previous = block;
}
void free_inEnde( memory_block_t* block, memory_block_t* last_free)
{
    last_free ->next = block;
    block->previous =  last_free;
    block->next = NULL;
}


void merge()
{
    memory_block_t* tmp = root->free_space_list_entry_point;
    while( tmp->next != NULL){
        if(tmp->next ==( tmp + tmp->size/sizeof(memory_block_t) + 1)){//merge certain
            memory_block_t* mergeBlock = tmp->next;
            tmp->size = tmp->size + tmp->next->size + sizeof(memory_block_t);
            if(mergeBlock->next != NULL ){
                tmp->next = mergeBlock->next;
                mergeBlock->next->previous = tmp;

            }else{ //the last block to merge
                tmp->next = NULL;
                break; //after merge,no tmp->next,so jump loop
            }
        }
        tmp = tmp->next;
    }
    if(tmp->previous != NULL){//for the last continuous three free block to merge
        tmp= tmp->previous;
        if(tmp->next ==( tmp + tmp->size/sizeof(memory_block_t) + 1)){
            tmp->size = tmp->size + tmp->next->size + sizeof(memory_block_t);
            tmp->next = NULL;
        }
    }

}


//===================================================================================================================//

/**
 * @brief get_page_size returns page size in bytes
 * @return page size in byte
 */
size_t get_page_size(void)
{
    return sysconf(_SC_PAGESIZE);
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief open_file: opens a file with the size of 1 page for that we want to manage the memory
 * @return file descriptor
 */
int open_file(void)
{
    // put your code here
	int fd = open(FILE_NAME, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	if(fd < 0){
		perror("open file error:");
		exit(-1);
	}
    void *ppage = mmap(START_ADDRESS, 4096, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, fd, 0);
	if(ppage == MAP_FAILED){
		perror("mmap error:");
		exit(-1);
	}
    int ret = posix_fallocate(fd, 0, get_page_size());
	if(ret < 0){
		perror("fallocate error");
		exit(-1);
	}
    // init page
    int *tmp= (int*)ppage;//change ppage to 64bit addressline
    for(unsigned int n=0; n < 4096 / sizeof(int); n++){
        *(tmp++)=0;
    }

    //init root
    root = (chunk_t*)ppage;
    root->file = fd;
    root->size = get_page_size() - sizeof(chunk_t);

    //init first memory_block
    memory_block_t *first=(memory_block_t *)(root + 1);
    first->previous = NULL;
    first->next = NULL;
    first->size = root->size - sizeof(memory_block_t);
    root->free_space_list_entry_point = first;
    return fd;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief my_malloc allocates memory that is backed by a file
 * @param size: size of memory to allocate in bytes
 * @return pointer to allocated memory
 */
void* my_malloc(size_t size)
{  // how much memory_blockpoint to reservieren
    --size;
    size /= sizeof(memory_block_t);
    ++size;
    size = size * sizeof(memory_block_t);
    size_t sum = sum_free(root);
    memory_block_t* free_block = root->free_space_list_entry_point;

    while( size <= sum){
        if( size <= free_block->size ){
            if(free_block->previous == NULL && free_block->next == NULL){ // only a block in memory
                insert_free_block(free_block,  size);
                set_block(free_block , size);
                break;
            }else if(free_block->previous == NULL){//in discrete first block
                if(size < free_block->size - sizeof(memory_block_t)){//certain,ob a new memory_block_t insert
                    insert_free_block(free_block,  size );
                    set_block(free_block , size);
                    break;
                }else{
                    root->free_space_list_entry_point = free_block->next;//no a new memory_block insert
                    free_block->next->previous = NULL;
                    if(size == free_block->size - sizeof(memory_block_t)){//just a sizeof(memory_block_t) to full wird
                       set_block(free_block , size + 24);
                    }else if( size == free_block->size ){//just same as size of free block
                        set_block(free_block , size );
                    }
                     break;
                }
            }else if(free_block->next == NULL){//in discrete last block
                insert_free_block(free_block,  size);
                set_block(free_block , size);
                break;
            }else{//betwen blocklist and laction fit
                if(size < free_block->size - sizeof(memory_block_t)){
                    insert_free_block(free_block,  size);
                    set_block(free_block, size);
                    break;
                }else{
                      free_block->previous->next = free_block->next;
                      free_block->next->previous = free_block->previous;
                      set_block(free_block , size);
                      break;
                }
            }
        }
        free_block = free_block->next;
    }
    if(size > sum){
        errno = ENOMEM;
        printf("error\n");
        return NULL;
    }
    return free_block + 1;

}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief my_free: releases memory that was allocated by my_free
 * @param memory_location: pointer to memory that should be freed
 */
void my_free(void* memory_location)
{
    // put your code here...
   memory_block_t* block = (memory_block_t*)memory_location - 1 ;
    if( block == (memory_block_t*)root ){
        perror("wurzerlnote");
        exit(-1);
    }

    if(RightAddress(memory_location) == 1){
        memory_block_t* block = (memory_block_t*)memory_location - 1 ;

        if(block->next == MAGIC_NUMBER &&
           block->previous == MAGIC_NUMBER){

            memory_block_t* last_free= root->free_space_list_entry_point;
            while(last_free->next != NULL){
                last_free = last_free->next; //find last free block
            }
            if(block < root->free_space_list_entry_point){
                free_inLow( block );        //locate below first free block
            }else if (block < last_free){
                   memory_block_t* hoch = root->free_space_list_entry_point;
                   while( hoch < block){
                       hoch = hoch->next;
                   }
                   // between first and last free space
                   free_inBetween(block, hoch);

            }else{// in end of liste
                free_inEnde(block, last_free);
            }

            merge( );

        }else{
            printf("error my_free: no solche memory to free ");
        }
    }else {
            printf("error my_free: uncorrect addresse\n");
    }
}

