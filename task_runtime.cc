#include <unistd.h>
#include <mpi.h>
#include "task_runtime.h"

enum {
  MPI_TASK = 1000,
  USER_TASK = 1001,
};

////////////////////////////////////////////////////////////////////////
//
// class TaskRuntime
//

// public

int TaskRuntime::get_rank()
{
  return rank;
}

int TaskRuntime::get_world_size()
{
  return world_size;
}

bool TaskRuntime::init(int *argc, char ***argv)
{
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  realm_rt.init(argc, argv);
  
  Machine::ProcessorQuery query(Machine::get_machine());
  query.only_kind(Processor::LOC_PROC);
  comm_processor = query.first();
  comp_processors.clear();
  Processor proc = comm_processor;
  if (query.count() == 1) {
    comp_processors.push_back(proc);
  } else {
    for (int i = 1; i < query.count(); i++) {
      proc = query.next(proc);
      comp_processors.push_back(proc);
    }
  }
  printf("rank %d, comm processor %llx\n", rank, comm_processor.id);
  for (std::vector<Processor>::iterator it = comp_processors.begin() ; it != comp_processors.end(); it++) {
    printf("rank %d, comp processor %llx\n", rank, it->id);
  }
  
  task_id_count = Processor::TASK_ID_FIRST_AVAILABLE;
  printf("taskid %d\n", task_id_count);
  
  return true;
}

bool TaskRuntime::register_task(Processor::TaskFuncPtr taskptr)
{
  task_id_map.insert(std::make_pair((uintptr_t)taskptr, task_id_count));
  bool rt_value = register_task_by_id(task_id_count, taskptr);
  task_id_count ++;
  return rt_value;
}



void TaskRuntime::set_top_level_task(Processor::TaskFuncPtr taskptr)
{
  std::map<uintptr_t, Processor::TaskFuncID>::iterator it = task_id_map.find((uintptr_t)taskptr);
  if (it != task_id_map.end()) {
    Processor::TaskFuncID task_id = it->second;
    set_top_level_task_by_id(task_id);
  } else {
    printf("can not find task %p\n", taskptr);
  }
}

void TaskRuntime::start()
{
  // request shutdown once that task is complete
  realm_rt.shutdown(top_level_event);

  // now sleep this thread until that shutdown actually happens
  realm_rt.wait_for_shutdown();
}

Future TaskRuntime::launch_user_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, Future &f)
{
  return launch_task_internal(taskptr, args, arglen, f.realm_event, USER_TASK);
}

Future TaskRuntime::launch_user_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, FutureList &fl)
{
  return launch_task_internal(taskptr, args, arglen, fl.realm_event, USER_TASK);
}

Future TaskRuntime::launch_user_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen)
{
  Event e = Event::NO_EVENT;  
  return launch_task_internal(taskptr, args, arglen, e, USER_TASK);
}

Future TaskRuntime::launch_mpi_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, Future &f)
{
  return launch_task_internal(taskptr, args, arglen, f.realm_event, MPI_TASK);
}

Future TaskRuntime::launch_mpi_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, FutureList &fl)
{
  return launch_task_internal(taskptr, args, arglen, fl.realm_event, MPI_TASK);
}

Future TaskRuntime::launch_mpi_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen)
{
  Event e = Event::NO_EVENT;  
  return launch_task_internal(taskptr, args, arglen, e, MPI_TASK);
}

// private

bool TaskRuntime::register_task_by_id(Processor::TaskFuncID taskid, Processor::TaskFuncPtr taskptr)
{
  return realm_rt.register_task(taskid, taskptr);
}

void TaskRuntime::set_top_level_task_by_id(Processor::TaskFuncID task_id)
{
  Processor p = Processor::NO_PROC;
  {
    Machine::ProcessorQuery query(Machine::get_machine());
    query.only_kind(Processor::LOC_PROC);
    p = query.first();
  }
  assert(p.exists());

  // collective launch of a single task - everybody gets the same finish event
  top_level_event = realm_rt.collective_spawn(p, task_id, NULL, 0);
}

Processor & TaskRuntime::get_comp_processor()
{
  Processor & proc = comp_processors[comp_proc_id];
  comp_proc_id ++;
  comp_proc_id = comp_proc_id % comp_processors.size();
  return proc;
}

Event TaskRuntime::spawn_realm_task(Processor::TaskFuncID task_id, void *args, size_t arglen, Processor &proc, Event &e)
{
  Event r_e = Event::NO_EVENT;
  if (e == Event::NO_EVENT) {
    r_e = proc.spawn(task_id, args, arglen);
  } else {
    r_e = proc.spawn(task_id, args, arglen, e);
  }
  return r_e;
}

Future TaskRuntime::launch_task_internal(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, Event &e, int task_type)
{
  std::map<uintptr_t, Processor::TaskFuncID>::iterator it = task_id_map.find((uintptr_t)taskptr);
  if (it == task_id_map.end()) {
    assert(0);
  }
  Processor::TaskFuncID task_id = it->second;
  Future future; 
  Processor proc;
  if (task_type == USER_TASK) {
    proc = get_comp_processor(); 
  //  printf("rank %d, launch comp task internal processor %llx\n", rank, proc.id);
  } else {
    proc = comm_processor;
 //   printf("rank %d, launch comm task internal processor %llx\n", rank, proc.id);
  }
  future.realm_event = spawn_realm_task(task_id, args, arglen, proc, e);
  return future;
}