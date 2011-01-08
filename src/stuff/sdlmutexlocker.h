#ifndef SDLMUTEXLOCKER_H
#define SDLMUTEXLOCKER_H

#include "utils.h"
#include <SDL_mutex.h>

class SDLMutexLocker {
	DISABLE_COPY(SDLMutexLocker)
	SDLMutexLocker() = delete;
private:
	SDL_mutex* m_mutex;
public:
	SDLMutexLocker(SDL_mutex* mutex) : m_mutex(mutex) {
		SDL_LockMutex(m_mutex);
	}
	~SDLMutexLocker() {
		SDL_UnlockMutex(m_mutex);
	}
};

#endif // SDLMUTEXLOCKER_H