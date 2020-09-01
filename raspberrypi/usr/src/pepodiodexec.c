/*
 licence GPLv2 ; this scripts designed by IZAMUKARERA 2011.11.5
 pepodioexec.c help diod for daemon contorl digital-Input to ANDDIO
*/
#include <stdio.h>
#include <stdlib.h>
void exec_cmd(char *cmd) {
  char *cmds[4];
  cmds[0] = "/bin/sh";
  cmds[1] = cmd;
  cmds[2] = 0;
  execv(cmds[0], cmds);
}

int main (int argc, char *argv[]) {
  int pid;
  switch( pid = fork() ) {
    case 0:
      exec_cmd(argv[1]);
    break;
    case 1:
      return 0;
    break;
  }
}
