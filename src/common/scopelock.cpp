/**
 *******************************************************************************
 * scopelock.h                                                                 *
 *                                                                             *
 * Scope lock                                                                  *
 *   Implement pthread mutex that unlock when leaving scope                    *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "scopelock.h"

#include <errno.h>
/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */


/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

ScopeLock::ScopeLock(pthread_mutex_t * mutex) :
   m_pmutex(mutex)
{
    lock();
}

ScopeLock::~ScopeLock()
{
    if (m_pmutex) {
        unlock();
    }
}

bool ScopeLock::lock()
{
    if (m_pmutex) {
        while ( pthread_mutex_lock(m_pmutex) != 0 ) {
            if (errno==EOWNERDEAD) { 
                // TODO: check implementation on Linux
                pthread_mutex_consistent(m_pmutex);
            }
            else {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool ScopeLock::timedlock(struct timespec * tp)
{
    if ((m_pmutex) && tp) {
        return pthread_mutex_timedlock(m_pmutex, tp);
    }
    return false;
}
bool ScopeLock::trylock()
{
    if (m_pmutex) {
        return pthread_mutex_trylock(m_pmutex) == 0 ? true : false;
    }
    return false;
}

void ScopeLock::unlock()
{
    if (m_pmutex) {
        pthread_mutex_unlock(m_pmutex);
    }
}
/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */



