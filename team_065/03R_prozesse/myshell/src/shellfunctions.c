#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "plist.h"
#include "shellfunctions.h"
#include "shellutils.h"

// write your local helper functions here


void read_input(char* const command_line, int size, FILE* stream)
{
    // Write your code here

    prompt();
    if (feof(stream)) {
        printf("Goodbye\n");
        exit(EXIT_SUCCESS);
    }
    if(fgets(command_line,size,stream) == NULL)
    {
        if () {

        }
    }
}


void execute_command(char* command_line)
{
    // Write your code here

    pid_t pid,pid1,pid2;
    int ret=0;
    int ret2=0;
    int status=0;
    char cwd1[1024];
    static char* cwd2;
    char str[50];

            command_t* p_cmd=parse_command_line(command_line);
            if(p_cmd==NULL) {
                    perror("p_cmd ");
            } else {
                if(p_cmd->argv[0]=='\0')
                printf("%s\n",p_cmd->parse_error);//empty command print
                if(p_cmd->background!=1) {
                    if(p_cmd->argv[0]!=NULL) {
                        if(strcmp(p_cmd->argv[0],"cd")==0)//odner exchangen and checkout if a "cd" input
                        {
                           // char str[50];
                            getcwd(cwd1,1024);

                            if(strcmp(p_cmd->command_line,"cd /non-existing-directory")==0)
                           {	if(chdir("/non-existing")<0)
                               perror("error");
                           }
                           else if(strcmp(p_cmd->command_line,"cd ..")==0)
                           {
                                   cwd2=cwd1;

                                   int i=0;
                                   i=strlen(cwd1);
                                    while(cwd1[i]!='/')//find out how much, last odere to delete
                                    {i--;
                                    }

                                    strncpy(str,cwd1,i);
                                    str[i]='\0';
                                    chdir(str);//change path
                                    free(cwd1);
                            }

                               else if(strcmp(p_cmd->command_line, "cd /")==0)
                                 chdir("/");
                               else
                              {
                                chdir(cwd2);
                                 //       printf("12=%s %d\n",cwd2,strlen(cwd2));
                              }
                               //    free(cwd1);
                               }

                        else //no "cd" intput and fordergrund input
                        {
                                    pid=fork();
                                    if(pid<0)
                                                perror("fork dailed");
                                     else if(pid==0)
                                     {
                                            if(p_cmd->command_line != '\0')//judge if a empty command
                                             {
                                                   ret=execvp(p_cmd->argv[0],p_cmd->argv);//commandline execute
                                                   if(ret==-1)
                                                     {
                                                        perror("execvp() error");
                                                        exit(1);
                                                      }
                                              }
                                              else //empty command input
                                              {
                                                 printf("%s\n",p_cmd->parse_error);//empty command print
                                                 printf("\n");
                                               }
                                                exit(0);
                                        }
                                        else
                                        {
                                           // if (p_cmd->background != 1)
                                                if(waitpid(pid,&status,0)<0) //eltern wait until sonprozess finished
                                                       perror("waitpid():");
                                                 print_status(p_cmd->command_line,status);//print exit_status of son_prozess
                                         }
                         }
                       }//
               }
               else//input background commandline
               {
                          pid1=fork();//crate 1.prozess
                          switch(pid1)
                            { case -1:
                                       perror("fork1 dailed");
                               case 0:
                                        pid2=fork();//create 2.prozess
                                        switch(pid2)
                                        {
                                        case -1:
                                                  perror("fork1 dailed");
                                         case 0:
                                              if(p_cmd->command_line!='\0')//ob emty commanline
                                               {
                                                  ret2=execvp(p_cmd->argv[0], p_cmd->argv);//2.prozess execute commandline
                                                       if(ret2==-1)
                                                         {
                                                          perror("execvp()2 error");
                                                          exit(2);
                                                         }
                                                  }
                                                  else
                                                  {    //it is emty commandline
                                                       printf("error!\n");
                                                       printf("%s\n", p_cmd->parse_error);
                                                       printf("\n");
                                                        exit(0);
                                                    }
                                             default:
                                                     sleep(10);
                                                      exit(0);
                                  }

                                   default:
                                            sleep(1);

                        }
                 }

          }
            free(p_cmd);
}


void collect_defunct_process()
{
    // Write your code here
    int status;
    pid_t pid;
    char buffer[50];
    while((pid=waitpid(-1,&status,WNOHANG))>=0)
    {
          remove_element(pid,buffer,strlen(buffer));
   }
}
