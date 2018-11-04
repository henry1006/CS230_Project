#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include "hw.h"


// This is the main driver file for the bank simulation.
// The `main` function takes one argument, the name of a
// trace file to use.  The test directory contains a
// short sample trace that exercises at least one interesting
// case.

int main (int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s trace_file\n", argv[0]);
    exit(1);
  }

  int result        = 0;
  int atm_count     = 0;
  int account_count = 0;

  // open the trace file
  result = trace_open(argv[1]);
  if (result == -1) {
    printf("%s: could not open file %s\n", argv[0], argv[1]);
    exit (1);
  }

  // Get the number of ATMs and accounts:
  atm_count     = trace_atm_count();
  account_count = trace_account_count();
  trace_close();

  // This is a table of atm_out file descriptors. It will be used by
  // the bank process to communicate to each of the ATM processes.
  int atm_out_fd[atm_count];

  // This is a table of bank_in file descriptors. It will be used by
  // the bank process to receive communication from each of the ATM processes.
  int bank_in_fd[atm_count];

  // TODO: ATM PROCESS FORKING
  for (int i = 0; i < atm_count-1; i++){ //PARA 1
    int atmfd[2];
    atm_out_fd[i] = pipe(atmfd); //call ​pipe​ on array and store in the ith entry of ​atm_out_fd the to-the-ATM’s output file descriptor, ​to_atmfd[1]
    int to_bankfd[2];
    if (pipe(to_bankfd)==0){
      bank_in_fd[i] = to_bankfd[0]
    }
    if (fork() == 0){ //PARA 2
      close(atmfd[1]);
      close(to_bankfd[0]);
      int run = atm_run(result, to_bankfd, atmfd, i);
      if (run != SUCCESS){
        error_print();
        exit(0);
      }
    }
    else{ //Para 3
      close(atmfd);
      close(to_bankfd);
    }
  }

  // TODO: BANK PROCESS FORKING
  if (fork() == 0){
    bank_open(atm_count, account_count);
    int runResult = run_bank(bank_in_fd, atm_out_fd);
    if (runResult != SUCCESS){
      error_print();
      bank_dump();
      bank_close();
      exit(0);
    }
  }

  // Wait for each of the child processes to complete. We include
  // atm_count to include the bank process (i.e., this is not a
  // fence post error!)
  for (int i = 0; i <= atm_count; i++) {
    wait(NULL);
  }

  return 0;
}
