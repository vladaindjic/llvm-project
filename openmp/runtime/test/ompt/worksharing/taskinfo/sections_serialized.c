// RUN: %libomp-compile-and-run | %sort-threads | FileCheck %s
// REQUIRES: ompt

#include "callback.h"
#include <omp.h>


__attribute__ ((noinline)) // workaround for bug in icc
void print_task_info_at(int ancestor_level)
{
#pragma omp critical
  {
    int task_type;
    char buffer[2048];
    ompt_data_t *parallel_data;
    ompt_data_t *task_data;
    ompt_frame_t *task_frame;
    ompt_get_task_info(ancestor_level, &task_type, &task_data, &task_frame,
                       &parallel_data, NULL);
    format_task_type(task_type, buffer);
    printf("%" PRIu64 ": ancestor_level=%d task_type=%s=%d parallel_id=%" PRIu64
           " task_id=%" PRIu64 " task_frame.exit=%p task_frame.enter=%p\n",
        ompt_get_thread_data()->value, ancestor_level, buffer,
        task_type, parallel_data->value, task_data->value,
        task_frame->exit_frame.ptr, task_frame->enter_frame.ptr);
  }
};

volatile int wait = 0;

void spin_wait() {
  while (wait == 0);
}

void signal() {
  wait = 1;
}

int main()
{
#pragma omp parallel sections num_threads(1)
  {
#pragma omp section
    {
      // implicit task info
      print_task_info_at(0);
      // initial task info
      print_task_info_at(1);
    }
  }

  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_create'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_implicit_task'


  // CHECK: {{^}}0: NULL_POINTER=[[NULL:.*$]]
  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_initial_task_begin: parallel_id=[[INITIAL_PARALLEL_ID:[0-9]+]], task_id=[[INITIAL_TASK_ID:[0-9]+]], actual_parallelism=1, index=1, flags=1

  // region 0
  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_parallel_begin
  // CHECK-SAME: parent_task_frame.exit=[[NULL]], parent_task_frame.reenter=[[INITIAL_TASK_FRAME_ENTER:0x[0-f]+]],
  // CHECK-SAME: parallel_id=[[PARALLEL_ID:[0-9]+]]

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_begin: parallel_id=[[PARALLEL_ID]], task_id=[[TASK_ID:[0-9]+]]

  // information about implicit task (exit frame should be set, while enter should be NULL)
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID]] task_id=[[TASK_ID]]
  // CHECK-SAME: task_frame.exit={{0x[0-f]+}}
  // CHECK-SAME: task_frame.enter=[[NULL]]

  // information about initial task (exit frame should be NULL, while enter frame shoule be set)
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=1 task_type=ompt_task_initial
  // CHECK-SAME: parallel_id=[[INITIAL_PARALLEL_ID]] task_id=[[INITIAL_TASK_ID]]
  // CHECK-SAME: task_frame.exit=[[NULL]]
  // CHECK-SAME: task_frame.enter=[[INITIAL_TASK_FRAME_ENTER]]


  return 0;
}
