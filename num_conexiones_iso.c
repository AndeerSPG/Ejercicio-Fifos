#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

//tuberias sin nombre

#define READING_CHANNEL (0)
#define WRITTING_CHANNEL (1)
#define STUDENT_ACCOUNT_FILTER "acaf00"

main() { /* who | grep acaf00 | wc -l */
  int pfd1[2], pfd2[2], pid;

  // Create first pipe
  if (pipe(pfd1) == -1) error("pipe 1");

  // Get how many connections are right now and save it in one extreme of the pipe
  switch (fork()) {
    case -1: error("fork"); break;
    case 0: // who
      // Close writting channel
      if (close(WRITTING_CHANNEL) == -1) error("close");
      // Duplicate writting channel to use it as standard writting output
      if (dup(pfd1[1]) != WRITTING_CHANNEL) error("dup");
      close(pfd1[0]); close(pfd1[1]);

      execlp("who", "who", NULL);
      error("execlp");
      break;
  }

  // Create second pipe
  if (pipe(pfd2) == -1) error("pipe 2");

  // Filter the connection
  switch (fork()) {
    case -1: error("fork"); break;
    case 0: // grep
      // Close reading channel
      if (close(READING_CHANNEL) == -1) error("close");
      // Duplicate reading channel to use it as standard reading output
      if (dup(pfd1[0]) != READING_CHANNEL) error("dup");
      close(pfd1[0]); close(pfd1[1]); // Close first pipe

      // Close writting channel
      if (close(WRITTING_CHANNEL) == -1) error("close");
      // Duplicate writting channel to use it as standard writting output
      if (dup(pfd2[1]) != WRITTING_CHANNEL) error("dup");
      close(pfd2[0]); close(pfd2[1]); // Close second pipe

      execlp("grep", "grep", STUDENT_ACCOUNT_FILTER, NULL);
      error("execlp");
      break;
  }

  // Close first pipe
  close(pfd1[0]); close(pfd1[1]);

  // Count how many lines are in the content got from the other extreme of the pipe2
  switch (fork()) {
    case -1: error("fork"); break;
    case 0: // wc -l
      // Close reading channel
      if (close(READING_CHANNEL) == -1) error("close");
      // Duplicate reading channel to use it as standard reading output
      if (dup(pfd2[0]) != READING_CHANNEL) error("dup");
      close(pfd2[0]); close(pfd2[1]); // Close second pipe

      execlp("wc", "wc", "-l", NULL);
      error("execlp");
      break;
  }

  close(pfd2[0]); close(pfd2[1]); // Close second pipe

  //Wait until all the childs finish
  while ((pid = wait(NULL)) != -1) ;

  return 0;
}
