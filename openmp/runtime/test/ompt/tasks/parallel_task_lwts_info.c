// RUN: %libomp-compile-and-run | FileCheck %s
// REQUIRES: ompt

#include "callback.h"
#include <omp.h>


int trap_ompt_get_task_info_internal(int level) {
  int task_type;
  ompt_data_t *parallel_data;
  ompt_data_t *task_data;
  return ompt_get_task_info(level, &task_type, &task_data, NULL,
                     &parallel_data, NULL);
}

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
    printf("%" PRIu64 ": ancestor_level=%d id=%d task_type=%s=%d "
                      "parallel_id=%" PRIu64 " task_id=%" PRIu64 "\n",
        ompt_get_thread_data()->value, ancestor_level, id, buffer,
        task_type, parallel_data->value, task_data->value);
  }
};

int main()
{
  int condition = 0;

  //initial task
  print_task_info_at(0, 0);

#pragma omp parallel num_threads(2)
  {
    // region 0
    print_task_info_at(0, 1);

#pragma omp barrier

#pragma omp master
#pragma omp task
    {
      // task 0
      // executed by worker thread
      print_task_info_at(0, 2);

#pragma omp task
      {
        // task 1
        // executed by master thread
        print_task_info_at(0, 3);

        print_task_info_at(0, 3);
        // scheduling parent is implicit task of the parallel region
        print_task_info_at(1, 1);
        print_task_info_at(2, 0);

#pragma omp parallel num_threads(1)
        {
          // region 1
          print_task_info_at(0, 4);

#pragma omp parallel num_threads(1)
          {
            // region 2
            print_task_info_at(0, 5);

            // region 2
            print_task_info_at(0, 5);
            // region 1
            print_task_info_at(1, 4);
            // task 1
            print_task_info_at(2, 3);
            // region 0
            print_task_info_at(3, 1);
            // initial task
            print_task_info_at(4, 0);

            print_task_info_at(0, 5);
          }

          print_task_info_at(0, 4);
        }

        print_task_info_at(0, 3);

        OMPT_SIGNAL(condition);

      };

      OMPT_WAIT(condition, 1);

      // executed by worker
      print_task_info_at(0, 2);
    };

#pragma omp barrier

    print_task_info_at(0, 1);

  }

  print_task_info_at(0, 0);

  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_create'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_implicit_task'


  // CHECK: {{^}}0: NULL_POINTER=[[NULL:.*$]]
  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_initial_task_begin: parallel_id=[[PARALLEL_ID_0:[0-9]+]], task_id=[[TASK_ID_0:[0-9]+]], actual_parallelism=1, index=1, flags=1
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=0 task_type=ompt_task_initial=1
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_0]] task_id=[[TASK_ID_0]]

  // region 0
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_parallel_begin: parent_task_id=[[TASK_ID_0]],
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_begin: parallel_id=[[PARALLEL_ID_1]], task_id=[[TASK_ID_1:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=1 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_1]]

  // task 0
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: parent_task_id=[[TASK_ID_1]]
  // CHECK-SAME: new_task_id=[[TASK_ID_2:[0-9]+]]
  // CHECK: [[THREAD_ID_1:[0-9]+]]: ompt_event_task_schedule:
  // CHECK-SAME: second_task_id=[[TASK_ID_2]]
  // CHECK: {{^}}[[THREAD_ID_1]]: ancestor_level=0 id=2 task_type=ompt_task_explicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_2]]

  // task 1
  // CHECK: {{^}}[[THREAD_ID_1]]: ompt_event_task_create: parent_task_id=[[TASK_ID_2]]
  // CHECK-SAME: new_task_id=[[TASK_ID_3:[0-9]+]]
  // CHECK: [[MASTER_ID]]: ompt_event_task_schedule:
  // CHECK-SAME: second_task_id=[[TASK_ID_3]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=3 task_type=ompt_task_explicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_3]]

  // check hierarchy
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=3 task_type=ompt_task_explicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_3]]
  // scheduling parent is implicit task of the parallel region executed by
  // initial master thread
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=1 id=1 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_1]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=2 id=0 task_type=ompt_task_initial
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_0]] task_id=[[TASK_ID_0]]

  // region 1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_parallel_begin: parent_task_id=[[TASK_ID_3]],
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_2:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_begin: parallel_id=[[PARALLEL_ID_2]], task_id=[[TASK_ID_4:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=4 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_2]] task_id=[[TASK_ID_4]]

  // region 2
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_parallel_begin: parent_task_id=[[TASK_ID_4]],
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_3:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_begin: parallel_id=[[PARALLEL_ID_3]], task_id=[[TASK_ID_5:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=5 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_3]] task_id=[[TASK_ID_5]]

  // check hierarchy
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=5 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_3]] task_id=[[TASK_ID_5]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=1 id=4 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_2]] task_id=[[TASK_ID_4]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=2 id=3 task_type=ompt_task_explicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_3]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=3 id=1 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_1]]
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=4 id=0 task_type=ompt_task_initial
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_0]] task_id=[[TASK_ID_0]]

  // region 2
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=5 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_3]] task_id=[[TASK_ID_5]]

  // region 1
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=4 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_2]] task_id=[[TASK_ID_4]]

  // task 1
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=3 task_type=ompt_task_explicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_3]]

  // task 0
  // CHECK: {{^}}[[THREAD_ID_1]]: ancestor_level=0 id=2 task_type=ompt_task_explicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_2]]

  // region 0
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=1 task_type=ompt_task_implicit
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_1]] task_id=[[TASK_ID_1]]

  // initial task
  // CHECK: {{^}}[[MASTER_ID]]: ancestor_level=0 id=0 task_type=ompt_task_initial=1
  // CHECK-SAME: parallel_id=[[PARALLEL_ID_0]] task_id=[[TASK_ID_0]]

  return 0;
}
