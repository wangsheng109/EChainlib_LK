/*
    File:       OSCond.cpp

    Contains:   Implementation of OSCond class
    
    

*/

#include "OSCond.h"
#include "OSMutex.h"


#if __PTHREADS_MUTEXES__
#include <sys/time.h>
#endif


OSCond::OSCond()
{
#ifdef WIN32
	fWaitCount = 0;		
    fCondition = ::CreateEvent(NULL, FALSE, FALSE, NULL);
//    Assert(fCondition != NULL);
#elif __PTHREADS_MUTEXES__

        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);
        int ret = pthread_cond_init(&fCondition, &cond_attr);
        Assert(ret == 0);

#else
	/*
    fCondition = mycondition_alloc();
    Assert(fCondition != NULL);
	*/
#endif
}

OSCond::~OSCond()
{
#ifdef WIN32
    int32 theErr = ::CloseHandle(fCondition);
//    Assert(theErr == TRUE);
#elif __PTHREADS_MUTEXES__
    pthread_cond_destroy(&fCondition);
#else
	/*
    Assert(fCondition != NULL);
    mycondition_free(fCondition);
	*/
#endif
}

/*
 * �������ܣ��ȴ�
 * ���������inMutex - ������
 *           inTimeoutInMilSecs -  0 :������ֱ�����ź�
 *                               ��0:������ֱ�����źŻ�inTimeoutInMilSecs΢���ʱ
 *           ʹ��ǰ��Ӧ���OSMutex::Lock��OSMutex::Unlock��
 * �����������
 * ����ֵ  ����
 */
#if __PTHREADS_MUTEXES__
void OSCond::TimedWait(OSMutex* inMutex, Int32 inTimeoutInMilSecs)
{
    struct timespec ts;
    struct timeval tv;
    struct timezone tz;
    int sec, usec;
    
    //These platforms do refcounting manually, and wait will release the mutex,
    // so we need to update the counts here

    
    if (inTimeoutInMilSecs == 0)
        (void)pthread_cond_wait(&fCondition, &inMutex->fMutex);
    else
    {
        gettimeofday(&tv, &tz);
        sec = inTimeoutInMilSecs / 1000;
        inTimeoutInMilSecs = inTimeoutInMilSecs - (sec * 1000);
        Assert(inTimeoutInMilSecs < 1000);
        usec = inTimeoutInMilSecs * 1000;
        ts.tv_sec = tv.tv_sec + sec;
        ts.tv_nsec = (tv.tv_usec + usec) * 1000;
        Assert(ts.tv_nsec < 2000000000);
        if(ts.tv_nsec > 999999999)
        {
             ts.tv_sec++;
             ts.tv_nsec -= 1000000000;
        }
        (void)pthread_cond_timedwait(&fCondition, &inMutex->fMutex, &ts);
    }

    
}

#endif
