#ifndef MPI_FUTURE_H
#define MPI_FUTURE_H

#include <mpi.h>
#include <realm.h>
#include "task_runtime.h"

typedef struct ompi_future_s {
  Future f;
}ompi_future_t;

typedef ompi_future_t MPI_Future;

typedef Realm::Processor::TaskFuncPtr MPI_Codelet;

int MPI_Fsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Future *infuture, int infuture_size, MPI_Future *outfuture);

int MPI_Frecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Future *infuture, int infuture_size, MPI_Future *outfuture);

int MPI_Waitfuture(MPI_Future *future);

int MPI_Fexecute(MPI_Codelet codelet, void *args, size_t arglen, MPI_Future *infuture, int infuture_size, MPI_Future *outfuture);

int MPI_Init_fmodule(int argc, char **argv);

int MPI_Register_codelet(MPI_Codelet codelet);

int MPI_Start_fmodule();

int MPI_Set_top(MPI_Codelet codelet);

#endif