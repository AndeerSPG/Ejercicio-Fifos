#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define READING_CHANNEL (0)
#define WRITTING_CHANNEL (1)
#define ACCOUNT_LIST_FILE "/etc/passwd"
#define STUDENT_ACCOUNT_FILTER "acaf00"

// Tuberias con nombre

int main(){
  int pid;
  unlink("my_fifo"); // Remove if already exists

  if (mkfifo("my_fifo", 0644) == -1) perror("mkfifo");

  // Filter account list to get only those starting by "acaf00" and put the result in the fifo (waiting to write)
  switch (pid = fork()) {
    case -1: error("fork"); break;
    case 0: /* grep ACCOUNT_LIST_FILE */
      // Close regular writting channel
      if (close(WRITTING_CHANNEL) == -1) error("close");

      // Use fifo as standard writting output (channel 1)
      if (open("my_fifo", O_WRONLY) != WRITTING_CHANNEL) perror("open");

      // Execute grep
      execlp("grep", "grep", STUDENT_ACCOUNT_FILTER, ACCOUNT_LIST_FILE, NULL);
      perror("grep");
      break;
  }

  // Count number of lines are in the got from fifo (waiting to read)
  switch (pid = fork()) {
    case -1: error("fork"); break;
    case 0: /* wc -l */
      // Close regular reading channel
      if (close(READING_CHANNEL) == -1) perror("close");

      // Use fifo as standard reading output (channel 0)
      if (open("my_fifo", O_RDONLY) != READING_CHANNEL) perror("open");

      // Execute wc
      execlp("wc", "wc", "-l", NULL);
      perror("execlp");
      break;
  }

  //Wait until all the childs finish
  while ((pid = wait(NULL)) != -1) ;

  // Remove fifo
  unlink("my_fifo");
  
  return 0;
}

