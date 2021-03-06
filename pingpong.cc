#include "mpi_future.h"
#include <unistd.h>

typedef struct revise_args_s{
  int count;
  int *buf;
}revise_args_t;

#define LOOP 200

void revise(const void *args, size_t arglen,
              const void *userdata, size_t userlen, Realm::Processor p)
{
  printf("revise\n");
  const revise_args_t *revise_args = (const revise_args_t*)args;
  int *array = (int *)revise_args->buf;
  int count = revise_args->count;
  printf("array %p, count %d\n", array, count);
  for (int i = 0; i < count; i++) {
     array[i] += 1;
  }
}

void pingpong(const void *args, size_t arglen,
              const void *userdata, size_t userlen, Realm::Processor p)
{ 
  int rank, world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("ping pong rank %d\n", rank);
  
  
  MPI_Future send_future[LOOP];
  MPI_Future recv_future[LOOP];
  MPI_Future revise_future[LOOP];
  int array[10];
  
  if (rank == 0) {
    for (int i = 0; i < 10; i++) {
      array[i] = 10;
    }
    for (int s = 0; s < LOOP; s++) {
      if (s == 0) {
        MPI_Fsend(array, 10, MPI_INT, 1, 0, MPI_COMM_WORLD, NULL, 0, &(send_future[s]));
      } else {
        MPI_Fsend(array, 10, MPI_INT, 1, 0, MPI_COMM_WORLD, &(recv_future[s-1]), 1, &(send_future[s]));
      }
      MPI_Frecv(array, 10, MPI_INT, 1, 0, MPI_COMM_WORLD, &(send_future[s]), 1, &(recv_future[s]));
    }
    MPI_Waitfuture(&(recv_future[LOOP-1]));
    for (int i = 0; i < 10; i++) {
      printf("%d ", array[i]);
    }
    printf("\n");
  } else {
    for (int s = 0; s < LOOP; s++) {
      if (s == 0) {
        MPI_Frecv(array, 10, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL, 0, &(recv_future[s]));
      } else {
        MPI_Frecv(array, 10, MPI_INT, 0, 0, MPI_COMM_WORLD, &(send_future[s-1]), 1, &(recv_future[s]));
      }
      revise_args_t revise_args;
      revise_args.buf = array;
      revise_args.count = 10;
      printf("buf %p, ct %d, size %d\n", array, revise_args.count, sizeof(int));
      MPI_Fexecute(revise, &revise_args, sizeof(revise_args), &(recv_future[s]), 1, &(revise_future[s]));
      MPI_Fsend(array, 10, MPI_INT, 0, 0, MPI_COMM_WORLD, &(revise_future[s]), 1, &(send_future[s]));
    }
    MPI_Waitfuture(&(send_future[LOOP-1]));
  }
  
}

int main(int argc, char **argv)
{
  int provided = 0;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, &provided);
  
  MPI_Init_fmodule(argc, argv);
  MPI_Register_codelet(pingpong);
  MPI_Register_codelet(revise);
  MPI_Set_top(pingpong);
  MPI_Start_fmodule();
  
  MPI_Finalize();
  return 0;
}
