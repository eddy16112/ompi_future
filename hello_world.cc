#include "mpi_future.h"
#include <unistd.h>

typedef struct hello_args_s {
  int id;
}hello_args_t;

void hello_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  const hello_args_t *hello_args = (const hello_args_t*)args;
  int id = hello_args->id;
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("rank %d, hello task %d,  pid %llx\n", rank, id, p.id);
}

void helloworld(const void *args, size_t arglen,
              const void *userdata, size_t userlen, Realm::Processor p)
{ 
  
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  MPI_Future future_comp[10];
  
  for (int i = 0; i < 10; i++) {
    hello_args_t hello_args;
    hello_args.id = i;
    if (i == 0) {
      MPI_Fexecute(hello_task, &hello_args, sizeof(hello_args), NULL, 0, &(future_comp[i]));
    } else {
      MPI_Fexecute(hello_task, &hello_args, sizeof(hello_args), &(future_comp[i-1]), 1, &(future_comp[i]));
    }
    printf("rank %d, launch task %d\n", rank, i);
  }
  
  MPI_Waitfuture(&(future_comp[9]));
  
}

int main(int argc, char **argv)
{
  int provided = 0;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, &provided);
  
  MPI_Init_fmodule(argc, argv);
  MPI_Register_codelet(helloworld);
  MPI_Register_codelet(hello_task);
  MPI_Set_top(helloworld);
  MPI_Start_fmodule();
  
  MPI_Finalize();
  return 0;
}