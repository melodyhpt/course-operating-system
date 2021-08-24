#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


int main()
{
  while(1)
  {
    char input[1024], *temp, prog[1024], args[10][1024];
    printf(">");
    gets(input);

    //parse string into a program name and arguments
    temp = strtok(input, " ");
    strcpy(prog, temp);
    temp = strtok(NULL, " ");
    
    int num_arg = 0;
    while(temp!=NULL)
    {
      strcpy(args[num_arg], temp);
      num_arg++;
      temp = strtok(NULL, " ");
    }

    //see if there is &
    if(strcmp(args[num_arg-1], "&") == 0) num_arg--;
    else strcpy(args[num_arg], "");

    //fork child process
    pid_t pid;
    pid = fork();

    if (pid< 0) //error occurred
    {
      fprintf(stderr, "Fork Failed");
      exit(-1);
    }
    else if (pid == 0) //child process->execute command
    {
      if(strcmp(args[num_arg], "&") == 0)
      {
        pid_t pid2;
        pid2 = fork();

        if(pid2 == 0) sleep(5);
        else exit(1);
      }

      if(num_arg == 0) execlp(prog, prog, NULL);
      else if(num_arg == 1) execlp(prog, prog, args[0], NULL);
      else if(num_arg == 2) execlp(prog, prog, args[0], args[1], NULL);
    }
    else  //parent process
    {
      wait(NULL);
    }
  }
}

