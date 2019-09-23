#include "mpi_future.h"
#include "task_runtime.h"

TaskRuntime internal_rt;

typedef struct sendrecv_task_args_s {
  int flag;
  int source;
  int dest;
  void *buf;
  int count;
  int tag;
  MPI_Comm comm;
  MPI_Datatype datatype;
}sendrecv_task_args_t;

typedef Processor::TaskFuncPtr ompi_codelet_t;

int ompi_future_wait(ompi_future_t *future);
int ompi_fsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, ompi_future_t *infuture, int infuture_size, ompi_future_t *outfuture);
int ompi_frecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, ompi_future_t *infuture, int infuture_size, ompi_future_t *outfuture);
int ompi_init_fmodule(int argc, char **argv);
int ompi_register_codelet(ompi_codelet_t codelet);
int ompi_set_top(ompi_codelet_t codelet);
int ompi_start_fmodule();
int ompi_fexecute(ompi_codelet_t codelet, void *args, size_t arglen, ompi_future_t *infuture, int infuture_size, ompi_future_t *outfuture);

int MPI_Fsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Future *infuture, int infuture_size, MPI_Future *outfuture)
{
  return ompi_fsend(buf, count, datatype, dest, tag, comm, infuture, infuture_size, outfuture);
}

int MPI_Frecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Future *infuture, int infuture_size, MPI_Future *outfuture)
{
  return ompi_frecv(buf, count, datatype, source, tag, comm, infuture, infuture_size, outfuture);
}

int MPI_Waitfuture(MPI_Future *future)
{
  return ompi_future_wait(future);
}

int MPI_Init_fmodule(int argc, char **argv)
{
  return ompi_init_fmodule(argc, argv);
}

int MPI_Register_codelet(MPI_Codelet codelet)
{
  return ompi_register_codelet(codelet);
}

int MPI_Start_fmodule()
{
  return ompi_start_fmodule();
}

int MPI_Set_top(MPI_Codelet codelet)
{
  return ompi_set_top(codelet);
}

int MPI_Fexecute(MPI_Codelet codelet, void *args, size_t arglen, MPI_Future *infuture, int infuture_size, MPI_Future *outfuture)
{
  return ompi_fexecute(codelet, args, arglen, infuture, infuture_size, outfuture);
}

void sendrecv_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  const sendrecv_task_args_t *sendrecv_args = (const sendrecv_task_args_t*)args;
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  if (sendrecv_args->flag == 0) {
    //printf("rank %d, dest %d, send buf %p\n", rank, sendrecv_args->dest, sendrecv_args->buf);
    void *buf = sendrecv_args->buf;
    int count = sendrecv_args->count;
    MPI_Datatype datatype = sendrecv_args->datatype;
    MPI_Comm comm = sendrecv_args->comm;
    int tag = sendrecv_args->tag;
    int dest = sendrecv_args->dest;
    MPI_Send(buf, count, datatype, dest, tag, comm);
  //  printf("rank %d, dst %d, send done buf %p\n", rank, dest, buf);
  } else {
  //  printf("rank %d, src %d, recv buf %p\n", rank, sendrecv_args->source, sendrecv_args->buf);
    void *buf = sendrecv_args->buf;
    int count = sendrecv_args->count;
    MPI_Datatype datatype = sendrecv_args->datatype;
    MPI_Comm comm = sendrecv_args->comm;
    int tag = sendrecv_args->tag;
    int source = sendrecv_args->source;
    MPI_Recv(buf, count, datatype, source, tag, comm, MPI_STATUS_IGNORE);
  //  printf("rank %d, src %d, recv done buf %p\n", rank, source, buf);
  }
}

int ompi_future_wait(ompi_future_t *future)
{
  future->f.wait();
  return 0;
}

int ompi_fsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, ompi_future_t *infuture, int infuture_size, ompi_future_t *outfuture)
{
  sendrecv_task_args_t args;
  args.flag = 0;
  args.dest = dest;
  args.buf = (void *)buf;
  args.count = count;
  args.tag = tag;
  args.comm = comm;
  args.datatype = datatype;
  if (infuture == NULL) {
    outfuture->f = internal_rt.launch_mpi_task(sendrecv_task, &args, sizeof(args));
  } else {
    assert(infuture_size != 0);
    if (infuture_size == 1) {
      outfuture->f = internal_rt.launch_mpi_task(sendrecv_task, &args, sizeof(args), infuture->f);
    } else {
      
    }
  }
  return 0;
}

int ompi_frecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, ompi_future_t *infuture, int infuture_size, ompi_future_t *outfuture)
{
  sendrecv_task_args_t args;
  args.flag = 1;
  args.source = source;
  args.buf = buf;
  args.count = count;
  args.tag = tag;
  args.comm = comm;
  args.datatype = datatype;
  if (infuture == NULL) {
    outfuture->f = internal_rt.launch_mpi_task(sendrecv_task, &args, sizeof(args));
  } else {
    assert(infuture_size != 0);
    if (infuture_size == 1) {
      outfuture->f = internal_rt.launch_mpi_task(sendrecv_task, &args, sizeof(args), infuture->f);
    } else {
      
    }
  }
  return 0;
}

int ompi_init_fmodule(int argc, char **argv)
{
  internal_rt.init(&argc, &argv);
  internal_rt.register_task(sendrecv_task);
  return 0;
}

int ompi_register_codelet(ompi_codelet_t codelet)
{
  internal_rt.register_task(codelet);
  return 0;
}

int ompi_set_top(ompi_codelet_t codelet)
{
  internal_rt.register_task(codelet);
  internal_rt.set_top_level_task(codelet);
}

int ompi_start_fmodule()
{
  internal_rt.start();
  return 0;
}

int ompi_fexecute(ompi_codelet_t codelet, void *args, size_t arglen, ompi_future_t *infuture, int infuture_size, ompi_future_t *outfuture)
{
 // printf("argelen %d\n", arglen);
  if (infuture == NULL) {
    outfuture->f = internal_rt.launch_user_task(codelet, args, arglen);
  } else {
    assert(infuture_size != 0);
    if (infuture_size == 1) {
      outfuture->f = internal_rt.launch_user_task(codelet, args, arglen, infuture->f);
    } else {
      
    }
  }
  return 0;
}
