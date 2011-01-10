#ifndef MUTEXLOCKER_H
#define MUTEXLOCKER_H

#include "utils.h"
#include "mutex.h"

class MutexLocker {
	DISABLE_COPY(MutexLocker)
	MutexLocker() = delete;
private:
	Mutex& m_mutex;
public:
	explicit MutexLocker(Mutex& mutex) : m_mutex(mutex) {
		m_mutex.lock();
	}
	~MutexLocker() {
		m_mutex.unlock();
	}
};

#endif // MUTEXLOCKER_H