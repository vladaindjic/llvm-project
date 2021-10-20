// RUN: %libomp-compile-and-run | FileCheck %s
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
    printf("%" PRIu64 ": ancestor_level=%d task_type=%s=%d "
                      "parallel_id=%" PRIu64 " task_id=%" PRIu64 " "
                      "task_frame.exit=%p task_frame.enter=%p\n",
        ompt_get_thread_data()->value, ancestor_level, buffer,
        task_type, parallel_data->value, task_data->value,
        task_frame->exit_frame.ptr, task_frame->enter_frame.ptr);
  }
};


int main()
{
  unsigned int i;

  #pragma omp parallel for num_threads(1) schedule(dynamic)
  for (i = 0; i < 1; i++) {
    // innermost implicit task that corresponds to the worksharing-loop
    print_task_info_at(0);
    // initial task that corresponds to the initial master thread
    print_task_info_at(1);
  }

  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_begin'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_end'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_implicit_task'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_work'

  // CHECK: 0: NULL_POINTER=[[NULL:.*$]]

  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_initial_task_begin: parallel_id=[[INITIAL_PARALLEL_ID:[0-9]+]], task_id=[[INITIAL_TASK_ID:[0-9]+]], actual_parallelism=1, index=1, flags=1

  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_parallel_begin: parent_task_id={{[0-9]+}}, parent_task_frame.exit=[[NULL]], parent_task_frame.reenter=[[INITIAL_TASK_FRAME_ENTER:0x[0-f]+]], parallel_id=[[PARALLEL_ID:[0-9]+]], requested_team_size=1, codeptr_ra=0x{{[0-f]+}}, invoker={{[0-9]+}}

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_begin: parallel_id=[[PARALLEL_ID]], task_id=[[IMPLICIT_TASK_ID_0:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID]] task_id=[[IMPLICIT_TASK_ID_0]]
  // CHECK-SAME: task_frame.exit={{0x[0-f]+}}
  // CHECK-SAME: task_frame.enter=[[NULL]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=1 task_type=ompt_task_initial
  // CHECK-SAME: parallel_id=[[INITIAL_PARALLEL_ID]] task_id=[[INITIAL_TASK_ID]]
  // CHECK-SAME: task_frame.exit=[[NULL]]
  // CHECK-SAME: task_frame.enter=[[INITIAL_TASK_FRAME_ENTER]]

  return 0;
}
