/* Copyright 2018 Stanford University, NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "task_runtime.h"
#include <mpi.h>
#include <unistd.h>

// Other experiments:
// - Run computation out of Zero-Copy memory
// - Use partitioning API
// - Add profiling

TaskRuntime rt;

typedef struct hello_args_s {
  int id;
}hello_args_t;

void hello_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  const hello_args_t *hello_args = (const hello_args_t*)args;
  int id = hello_args->id;
  printf("rank %d, hello task %d,  pid %llx\n", rt.get_rank(), id, p.id);
}

void top_level_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{ 
  int my_rank = rt.get_rank();
  int size = rt.get_world_size();
  
  
  Future future_comp[10];
  
  for (int i = 0; i < 10; i++) {
    hello_args_t hello_args;
    hello_args.id = i;
    if (i == 0) {
      future_comp[i] = rt.launch_user_task(hello_task, &hello_args, sizeof(hello_args));
    } else {
      future_comp[i] = rt.launch_user_task(hello_task, &hello_args, sizeof(hello_args), future_comp[i-1]);
    }
  }
  
  future_comp[9].wait();
  
}

int main(int argc, char **argv)
{
  int provided = 0;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, &provided);
  
  rt.init(&argc, &argv);

  rt.register_task(top_level_task);
  rt.register_task(hello_task);
  rt.set_top_level_task(top_level_task);

  rt.start();
  
  MPI_Finalize();
  return 0;
}
