/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

typedef struct _node_st {
	dispatch_fn fn_ptr;
	void *arg;
	struct _node_st *next;
} _node;

typedef struct _queue_st {
	int num_tasks;
	_node *head, *tail;
} _queue;
void init_queue(_queue *q){
	q->num_tasks = 0;
	q->head = NULL;
	q->tail = NULL;
}
int queue_empty(_queue *q){
	while(q->head != q->tail){
		_node *temp = q->head->next;
		free(q->head);
		q->head = temp;
	}
	free(q->head);
	return q->num_tasks = 0;
}
void enqueue(_queue *q, _node *n){
	if(q->num_tasks == 0){
		q->head = q->tail = n;
		q->head->next = NULL;
		q->num_tasks++;
	} else if (q->num_tasks == 1){
		q->head->next = q->tail = n;
		q->num_tasks++;
	} else {
		q->tail->next = n;
		q->tail = n;
		q->num_tasks++;
	}
}
_node * dequeue(_queue *q){
	_node *temp = q->head;
	q->head = q->head->next;
	if(q->head == NULL){
		q->tail = NULL;
	}
	q->num_tasks--;
	return temp;
}


// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
   // you should fill in this structure with whatever you need
	_queue queue;
	pthread_mutex_t pool_mutex;
	pthread_cond_t pool_condition;
	pthread_t thread_id[MAXT_IN_POOL];
	int num_threads_in_pool;
	_Bool is_shutdown;
} _threadpool;

threadpool create_threadpool(int num_threads_in_pool) {
  _threadpool *pool;

  // sanity check the argument
  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (_threadpool *) malloc(sizeof(_threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }

 void *thread_main(void *threadpool){
	_threadpool *pool = (_threadpool *) threadpool;
	_node *task;
	while(1){
		pthread_mutex_lock(&pool->pool_mutex);
		while(pool->is_shutdown == 0 && pool->queue.num_tasks == 0){
			pthread_cond_wait(&(pool->pool_condition), &(pool->pool_mutex));
		}
		if(pool->is_shutdown){
			pthread_mutex_unlock(&(pool->pool_mutex));
			break;
		}
		task = dequeue(&pool->queue);
		pthread_mutex_unlock(&(pool->pool_mutex));
		(*(task->fn_ptr))(task->arg);
		free(task);
	}
// 	pool->num_threads_in_pool--;
	pthread_exit(NULL);
	return(NULL);
}


  // add your code here to initialize the newly created threadpool
  pool->num_threads_in_pool = num_threads_in_pool;
	pool->is_shutdown = 0;
  init_queue(&pool->queue);
	pthread_mutex_init(&pool->pool_mutex, NULL);
	pthread_cond_init(&pool->pool_condition, NULL);
	for (int i = 0; i < num_threads_in_pool; i++){
		pthread_create(&(pool->thread_id[i]), NULL, thread_main, (void*)pool);
	}

  return (threadpool) pool;
}



void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
	      void *arg) {
  _threadpool *pool = (_threadpool *) from_me;

  // add your code here to dispatch a thread
	_node *task = (_node *) malloc(sizeof(_node));
	task->fn_ptr = dispatch_to_here;
	task->arg = arg;
	pthread_mutex_lock(&pool->pool_mutex);
	enqueue(&pool->queue, task);
	pthread_cond_signal(&pool->pool_condition);
	pthread_mutex_unlock(&pool->pool_mutex);
}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  // add your code here to kill a threadpool
	pthread_mutex_lock(&pool->pool_mutex);
	pool->is_shutdown = 1;
	pthread_cond_broadcast(&pool->pool_condition);
	pthread_mutex_unlock(&pool->pool_mutex);
	for (int i = 0; i < pool->num_threads_in_pool; i++){
		pthread_join(pool->thread_id[i], NULL);
	}
	queue_empty(&pool->queue);
	pthread_mutex_destroy(&pool->pool_mutex);
	pthread_cond_destroy(&pool->pool_condition);
	free(pool);
}

