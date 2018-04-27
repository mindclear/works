#ifndef TIMER_H_

#include <functional>
#include <stdint.h>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <mutex>

class Timer;
typedef std::function<void()> TimerCallback;
typedef std::shared_ptr<Timer> TimerPtr;
typedef std::list<TimerPtr> TimerList;
typedef TimerList::iterator TimerListItor;

class Timer
{
public:
	Timer(int32_t timerid, TimerCallback cb, int64_t when, uint32_t interval)
		: timerid_(timerid),
		  callback_(cb),
		  expire_(when),
		  interval_(interval),
		  repeat_(interval > 0)
	{}

	void run() const { callback_(); }

	int32_t get_timerid() const { return timerid_; }
	int64_t expiration() const { return expire_; }
	bool repeat() const { return repeat_; }
	void restart() { expire_ += interval_; }

	// for TimerWheel
	void set_wheel_index(int32_t wheel_index) { wheel_index_ = wheel_index; }
	int32_t get_wheel_index() const { return wheel_index_; }
	void set_itor(TimerListItor itor) { itor_ = itor; }
	TimerListItor get_itor() const { return itor_; }

private:
	int32_t timerid_;
	TimerCallback callback_;
	int64_t expire_;
	uint32_t interval_;
	bool repeat_;

	// for TimerWheel
	int32_t wheel_index_;
	TimerListItor itor_;

};

class TimerWheel
{
public:
	TimerWheel();

	int32_t setTimer(TimerCallback cb, int64_t when, uint32_t interval);
	void cancelTimer(int32_t timerid);
	void update();

	void debug();

private:
	// unsafe, must be lock befor call
	int32_t addAndFetchTimerid();
	// unsafe, must be lock before call
	void addTimer(TimerPtr timer);
	// unsafe, must be lock before call
	void moveTimerList(int32_t wheel_index);

private:
	std::mutex mutex_;
	std::vector<TimerList> v_timers_;
	std::map<int32_t, TimerPtr> m_timers_;

	// atomic timerid
	int32_t num_since_created_;

	// current pointer
	int64_t last_cursor_;
};

#endif // TIMER_H_
