#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
static void print_usage(const char* executable)
{
    fprintf(stderr, "USAGE: %s -s SIGNAL -p PID\n", executable);
    exit(EXIT_FAILURE);
}


int main (int argc, char* argv[])
{
    //write your code here
    if(argc >3 && argc <=5 ){
        if(argv[1][1] == 'p' ){//./send-signal -p 123 -s 123 input error
            print_usage( argv[0]);
        }
        int opt,pid,sig,ret;
        while((opt = getopt( argc, argv, "s:p:" ) )!= -1 ){
            switch ( opt ) {
                case 'p':
                     pid = atoi(optarg);
                     printf(" -%c %d\n",opt , pid);
                    break;
                case 's':
                     sig = atoi(optarg);
                     printf(" -%c %d",opt , sig);
                    break;
            }

        }
        ret = kill(pid, sig );
        if(ret == -1){
            perror("kill");
            exit(EXIT_FAILURE);
         }else{
            printf("kill %d\n", pid);
         }

    }else{
        print_usage( argv[0] );
    }
	return EXIT_SUCCESS;
}
