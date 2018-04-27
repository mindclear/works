#ifndef LOCK_H_

#include <mutex>

class LockGuard
{
public:
	explicit LockGuard(std::mutex& t_mutex)
		: mutex_(t_mutex)
	{
		mutex_.lock();
	}
	~LockGuard()
	{
		mutex_.unlock();
	}
private:
	std::mutex& mutex_;
};

#endif // LOCK_H_