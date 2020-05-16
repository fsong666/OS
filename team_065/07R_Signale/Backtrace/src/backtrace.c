#include "backtrace.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
typedef void (*sighandler)( int);
//-----------------------------------------------------------------------------
void print_backtrace(void)
{
    //write your code here
    void *buffer[MAXIMAL_BACKTRACE_DEPTH] = { NULL };
    char **trace = NULL;
    int size = backtrace(buffer, MAXIMAL_BACKTRACE_DEPTH);
    // returns the number of addresses returned in buffer
    trace = backtrace_symbols(buffer, size);//hier buffer is void *const *buffer
    if (NULL == trace) {
        perror("back_symbols");
        exit(EXIT_FAILURE);
    }
    printf("-------------------------------------------------------\n\n");
    printf("[DEPTH] FILE(SYMBOL+OFFSET) [ADDRESS]\n");
    printf("--------------------------------------------------------\n");
    for (int i = 0; i < size; ++i) {
        printf("[%d] %s\n",i, trace[i]);
    }
    free(trace);
    printf("----------done-----------------------------------------\n");

}

//-----------------------------------------------------------------------------

void signal_handler(int sig)
{
    //write your code here
    if(sig == SIGINT){
           printf("interrupt by keyboard signum = %d\n",sig);
           printf("Received Signal: %d\n",sig);
           print_backtrace();
           exit(sig);
       }
       if(sig == SIGQUIT){
           printf("quit by keyboard signum = %d\n",sig);
           printf("Received Signal: %d\n",sig);
           print_backtrace();
           exit(sig);
       }
       if(sig == SIGILL){
           printf("illegal instruction = %d\n",sig);
           printf("Received Signal: %d\n",sig);
           print_backtrace();
           exit(sig);
       }
       if(sig == SIGTERM){
           printf("termination = %d\n",sig);
           printf("Received Signal: %d\n",sig);
           print_backtrace();
           exit(sig);
       }
       if(sig == SIGABRT){
           printf("abnormal termination = %d\n",sig);
           printf("Received Signal: %d\n",sig);
           print_backtrace();
           exit(sig);
       }
       if(sig == SIGFPE){
           printf("floating point exception = %d\n",sig);
           printf("Received Signal: %d\n",sig);
           print_backtrace();
           exit(sig);
       }
       if(sig == SIGSEGV){
           printf("segmention fault = %d\n",sig);
           printf("Received Signal: %d\n",sig);
           print_backtrace();
           exit(sig);
       }
}

//-----------------------------------------------------------------------------

void initialize_signal_handler(void)
{
    sighandler  p = &signal_handler;
       if(signal(SIGINT, p) == SIG_ERR){
               perror("SIGNAL error\n");
       }
       if(signal(SIGQUIT, p) == SIG_ERR){
               perror("SIGNAL error\n");
       }
       if(signal(SIGILL, p) == SIG_ERR){
               perror("SIGNAL error\n");
       }
       if(signal(SIGTERM, p) == SIG_ERR){
               perror("SIGNAL error\n");
       }
       if(signal(SIGABRT, p) == SIG_ERR){
               perror("SIGNAL error\n");
       }
       if(signal(SIGFPE, p) == SIG_ERR){
               perror("SIGNAL error\n");
       }
       if(signal(SIGSEGV, p) == SIG_ERR){
               perror("SIGNAL error\n");
       }
}
