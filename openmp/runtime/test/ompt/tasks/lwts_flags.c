// RUN: %libomp-compile-and-run | FileCheck %s
// REQUIRES: ompt

#include "callback.h"
#include <omp.h>


__attribute__ ((noinline)) // workaround for bug in icc
void print_task_info_at(int ancestor_level, int id)
{
#pragma omp critical
  {
    int task_type;
    char buffer[2048];
    ompt_data_t *parallel_data;
    ompt_data_t *task_data;
    ompt_get_task_info(ancestor_level, &task_type, &task_data, NULL, &parallel_data, NULL);
    format_task_type(task_type, buffer);
    printf("%" PRIu64 ": id=%d task_type=%s=%d "
                      "parallel_id=%" PRIu64 " task_id=%" PRIu64 "\n",
        ompt_get_thread_data()->value, id, buffer,
        task_type, parallel_data->value, task_data->value);
  }
};


__attribute__ ((noinline)) // workaround for bug in icc
void print_innermost_task_info(int id)
{
  print_task_info_at(0, id);
};


int main()
{
  //initial task

#pragma omp parallel num_threads(1)
  {
    // region R1
    print_innermost_task_info(0);

#pragma omp task
    {
      // task T
      print_innermost_task_info(1);

#pragma omp parallel num_threads(1)
      {
        // region R1
        print_innermost_task_info(2);
        // flags of task T
        print_task_info_at(1, 1);
      }

      // flags of T
      print_innermost_task_info(1);

    };

    // flags of R1
    print_innermost_task_info(0);

  }

  //print_task_info_at(0, 0);

  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_create'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_implicit_task'


  // CHECK: {{^}}0: NULL_POINTER=[[NULL:.*$]]
  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_initial_task_begin:

  // region R1
  // CHECK: {{^}}[[MASTER_ID]]: id=0
  // CHECK-SAME: task_type=ompt_task_implicit|ompt_task_undeferred=134217730

  // task T
  // CHECK: {{^}}[[MASTER_ID]]: id=1
  // CHECK-SAME: task_type=ompt_task_explicit|ompt_task_undeferred=134217732

  // region R2
  // CHECK: {{^}}[[MASTER_ID]]: id=2
  // CHECK-SAME: task_type=ompt_task_implicit|ompt_task_undeferred=134217730
  // flags of task T printed from region R2
  // CHECK: {{^}}[[MASTER_ID]]: id=1
  // CHECK-SAME: task_type=ompt_task_explicit|ompt_task_undeferred=134217732

  // task T
  // CHECK: {{^}}[[MASTER_ID]]: id=1
  // CHECK-SAME: ompt_task_explicit|ompt_task_undeferred=134217732

  // region R1
  // CHECK: {{^}}[[MASTER_ID]]: id=0
  // CHECK-SAME: task_type=ompt_task_implicit|ompt_task_undeferred=134217730

  return 0;
}
