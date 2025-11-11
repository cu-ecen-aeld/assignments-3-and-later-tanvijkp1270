#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    int ret;
    //struct timespec remaining_time;
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    struct thread_data* thread_func_args = (struct thread_data *)thread_param;

    thread_func_args->thread_complete_success = true;

    /*do
    {
        ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &thread_func_args->ts_obtain, &thread_func_args->ts_obtain); 

    }while(ret == EINTR || ret == 0);*/

    printf("Sleep time val obtain of thread %lu,  %ld \t %ld \n ",pthread_self(),thread_func_args->ts_obtain.tv_sec,thread_func_args->ts_obtain.tv_nsec);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &thread_func_args->ts_obtain, NULL); 

    if (ret == 0)
    {

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        printf("Sleep time elapsed obtain of thread %lu,  %ld  \n",pthread_self(),end_time.tv_nsec-start_time.tv_nsec);
        if(pthread_mutex_lock(thread_func_args->mutex) != 0)
        {
            perror("mutex lock failed");
            thread_func_args->thread_complete_success = false;
            return thread_param;
        }
        else
        {
            printf("Mutex locked \n");
        }

    }
    else if(ret == EINTR)
    {
        perror("clock_nanosleep interrupted by signal");
        //printf("Remaining sleep time for obtain  %ld \t %ld ",remaining_time.tv_sec,remaining_time.tv_nsec);
        thread_func_args->thread_complete_success = false;

        return thread_param;
    }
    else
    {
        perror("clock_nanosleep failed");
        thread_func_args->thread_complete_success = false;

        return thread_param;
    }
    
    
    /*do
    {
        ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &thread_func_args->ts_release, &thread_func_args->ts_release); 

    }while(ret == EINTR || ret == 0);*/
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &thread_func_args->ts_release, NULL); 

    printf("Sleep time val release of thread %lu ,  %ld \t %ld \n",pthread_self(),thread_func_args->ts_release.tv_sec,thread_func_args->ts_release.tv_nsec);

    if (ret == 0) 
    {
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        printf("Sleep time elapsed release of thread %lu,  %ld \n ",pthread_self(),end_time.tv_nsec-start_time.tv_nsec);

        if(pthread_mutex_unlock(thread_func_args->mutex) != 0)
        {
            perror("mutex unlock failed");
            thread_func_args->thread_complete_success = false;

            return thread_param;
        }
        else
        {
            printf("Mutex unlocked \n");
        }
    }
    else if(ret == EINTR)
    {
        perror("clock_nanosleep interrupted by signal");
        //printf("Remaining sleep time for release %ld \t %ld ",remaining_time.tv_sec,remaining_time.tv_nsec);
        thread_func_args->thread_complete_success = false;

        return thread_param;
    }
    else
    {
        perror("clock_nanosleep failed");
        thread_func_args->thread_complete_success = false;

        return thread_param;
    } 
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    int ret = 0;

    if(wait_to_obtain_ms < 0 || wait_to_release_ms < 0)
    {
        perror("Pass valid arguments");
        return false;
    }

    struct timespec ts_obtain,ts_release;
    ts_obtain.tv_sec = wait_to_obtain_ms / 1000;
    ts_obtain.tv_nsec = (wait_to_obtain_ms % 1000) * 1000000;
    
    ts_release.tv_sec = wait_to_release_ms / 1000;
    ts_release.tv_nsec = (wait_to_release_ms % 1000) * 1000000;
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    
    struct thread_data *tdata = (struct thread_data *)malloc(sizeof(struct thread_data));

    if(tdata == NULL)
    {
        perror("Malloc failed");
        free(tdata);
        return false;
    }

    tdata->mutex = mutex;
    tdata->ts_obtain = ts_obtain;
    tdata->ts_release = ts_release;
    tdata->thread_complete_success = false;

    //ret = pthread_mutex_init(mutex, NULL);
    
    /*if( ret != 0)
    {
        perror("Error in mutex init");
        free(tdata);
        return false;
    }*/

    ret = pthread_create(thread, NULL, threadfunc, (void *)tdata);
    
    if( ret != 0)
    {
        perror("Error in thread creation");
        free(tdata);
        return false;
    }
    
    
    return true;
}

