#ifndef __TASK_RUNTIME_H__
#define __TASK_RUNTIME_H__

#include "realm.h"

using namespace Realm;

class Future {
public:
  Event realm_event;
  
public:
  Future()
  {
    realm_event = Event::NO_EVENT;
  }
  void wait()
  {
    realm_event.wait();
  }
};

class FutureList {
public:
  Event realm_event;
  
public:
  FutureList()
  {
    realm_event = Event::NO_EVENT;
  }
  FutureList(Future f_array[], int size)
  {
    realm_event = f_array[0].realm_event;
    for ( int i = 1; i < size; i++) {
      realm_event = Event::merge_events(realm_event, f_array[i].realm_event);
    }  
  }
  void add_future(Future &f) 
  {
    if (realm_event == Event::NO_EVENT ) {
      realm_event = f.realm_event;
    } else {
      Event tmp_event = Event::NO_EVENT;
      tmp_event = Event::merge_events(tmp_event, f.realm_event);
      realm_event = tmp_event;
    }
  }
  void wait()
  {
    realm_event.wait();
  }
};

class TaskRuntime {
private:
  int world_size;
  int rank;
  Runtime realm_rt;
  Event top_level_event;
  std::map<uintptr_t, Processor::TaskFuncID> task_id_map;
  Processor::TaskFuncID task_id_count;
  Processor comm_processor;
  std::vector<Processor> comp_processors;
  int comp_proc_id;

public:
  TaskRuntime() 
  {
    
  }
  
  int get_rank();
  
  int get_world_size();
  
  bool init(int *argc, char ***argv);
  
  bool register_task(Processor::TaskFuncPtr taskptr);
  
  void set_top_level_task(Processor::TaskFuncPtr taskptr);
  
  void start();
  
  Future launch_user_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, Future &f);
  
  Future launch_user_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, FutureList &fl);
  
  Future launch_user_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen);
  
  Future launch_mpi_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, Future &f);
  
  Future launch_mpi_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, FutureList &fl);
  
  Future launch_mpi_task(Processor::TaskFuncPtr taskptr, void *args, size_t arglen);
  
private:
  bool register_task_by_id(Processor::TaskFuncID taskid, Processor::TaskFuncPtr taskptr);
  
  void set_top_level_task_by_id(Processor::TaskFuncID task_id);
  
  Processor & get_comp_processor();
  
  Event spawn_realm_task(Processor::TaskFuncID task_id, void *args, size_t arglen, Processor &proc, Event &e);
  
  Future launch_task_internal(Processor::TaskFuncPtr taskptr, void *args, size_t arglen, Event &e, int task_type);
};

#endif