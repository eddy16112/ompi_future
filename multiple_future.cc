#include "mpi_future.h"
#include <unistd.h>

typedef struct revise_args_s{
  int count;
  int *buf;
}revise_args_t;

void revise(const void *args, size_t arglen,
              const void *userdata, size_t userlen, Realm::Processor p)
{
  const revise_args_t *revise_args = (const revise_args_t*)args;
  int *array = (int *)revise_args->buf;
  int count = revise_args->count;
  printf("array %p, count %d\n", array, count);
  for (int i = 0; i < count; i++) {
    printf("%d, ", array[i]);
  }
  printf("\n");
}

void multiple_future(const void *args, size_t arglen,
              const void *userdata, size_t userlen, Realm::Processor p)
{ 
  int rank, world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("multiple_future rank %d\n", rank);
  
  if (rank == 0) {
    int array[10];
    MPI_Future recv_future[10];
    MPI_Future revise_future;
    array[0] = 0;
    for (int i = 1; i < world_size; i++) {
      MPI_Frecv(&(array[i]), 1, MPI_INT, i, i, MPI_COMM_WORLD, NULL, 0, &(recv_future[i-1]));
    }
    revise_args_t revise_args;
    revise_args.buf = array;
    revise_args.count = world_size;
    MPI_Fexecute(revise, &revise_args, sizeof(revise_args), recv_future, world_size-1, &revise_future);
    MPI_Waitfuture(&revise_future);
  } else {
    MPI_Future send_future;
    if (rank == 1) {
    //  sleep(5);
    }
    MPI_Fsend(&rank, 1, MPI_INT, 0, rank, MPI_COMM_WORLD, NULL, 0, &send_future);
    MPI_Waitfuture(&send_future);
  }
  
}

int main(int argc, char **argv)
{
  int provided = 0;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, &provided);
  
  MPI_Init_fmodule(argc, argv);
  MPI_Register_codelet(multiple_future);
  MPI_Register_codelet(revise);
  MPI_Set_top(multiple_future);
  MPI_Start_fmodule();
  
  MPI_Finalize();
  return 0;
}
