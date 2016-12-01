First off, I wrote this code while high out of my mind on pain
killers. So I hope you enjoy deciphering my code as much as I enjoyed
writing it. Good luck. :)

Threadpool structure - Contains a mutex and conditional variable for
synchronization. It contains a task queue implemented using a linked
list. It uses a FIFO policy. An array of pthread_t type contains the
thread IDs. This along with the num_threads_in_pool tracker are used
to iterate through all of the threads. Finaly there is a flag to
detect shutdown.

Critical Sections - Anytime the threadpool was accessed a mutex lock
was aquired. This mainly revolved around enqueueing a task (for
dispatch) and dequeueing a task (in thread_main).

A thread is blocked while it is waiting on a task to perform. The
dispatch function wakes up a thread once a new task is enqueued.
