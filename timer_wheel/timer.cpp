#include "timer.h"
#include <assert.h>
#include "lock_guard.h"

#ifdef  _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)

#define OFFSET(n) (TIME_NEAR + (n) * TIME_LEVEL)
#define INDEX(cursor, n) ((cursor >> (TIME_NEAR_SHIFT + (n) * TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)

#define GRANULARITY	10 // centisecond: 1/10 second

int64_t getGranularityTime()
{
#ifdef _WIN32
	struct timeb tp;
	ftime(&tp);
	return tp.time * GRANULARITY + tp.millitm * GRANULARITY / 1000;
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * GRANULARITY + tv.tv_usec * GRANULARITY / 1000000;
#endif
}

TimerWheel::TimerWheel()
	: v_timers_(TIME_NEAR + 4 * TIME_LEVEL),
	  num_since_created_(0),
	  last_cursor_(getGranularityTime())
{}

int32_t TimerWheel::setTimer(TimerCallback cb, int64_t when, uint32_t interval)
{
	LockGuard guard(mutex_);
	int64_t expire_ticks = when * GRANULARITY;
	if (last_cursor_ >= expire_ticks) //earlier than last_cursor_
		return 0;

	TimerPtr timer(new Timer(addAndFetchTimerid(), cb, when * GRANULARITY, interval));
	m_timers_.insert(std::make_pair(timer->get_timerid(), timer));
	addTimer(timer);
	return timer->get_timerid();
}

void TimerWheel::cancelTimer(int32_t timerid)
{
	LockGuard guard(mutex_);
	auto timer_itor = m_timers_.find(timerid);
	if (timer_itor == m_timers_.end())
		return;

	TimerPtr timer = timer_itor->second;
	assert(timer);
	assert(timer->get_timerid() == timerid);
	
	uint32_t wheel_index = timer->get_wheel_index();
	assert(wheel_index >= 0 && wheel_index < v_timers_.size());

	v_timers_[wheel_index].erase(timer->get_itor());
	m_timers_.erase(timer_itor);
}

void TimerWheel::update()
{
	int64_t current_time = getGranularityTime();
	TimerList expire_list;
	{
		LockGuard guard(mutex_);
		while (last_cursor_ < current_time)
		{
			++last_cursor_;
			int32_t cur_index = last_cursor_ & TIME_NEAR_MASK;
			printf("%d\n", cur_index);

			TimerList& t_list = v_timers_[cur_index];
			if (!t_list.empty())
			{
				for (TimerPtr& timer : t_list)
				{
					if (timer->repeat()) // repeat timer
					{
						timer->restart();
						addTimer(timer); // timer will move to new place that not same with cur_index
					}
					else
					{
						m_timers_.erase(timer->get_timerid());
					}
				}

				// out of date timer
				expire_list.splice(expire_list.end(), t_list);
			}
				
			if (0 == cur_index) // one loop finish
			{
				int level = 0;
				int64_t mask = TIME_NEAR;
				do
				{
					int32_t wheel_index = OFFSET(level) + INDEX(last_cursor_, level);
					if (wheel_index != 0)
					{
						moveTimerList(wheel_index);
						break;
					}
					++level;
					mask <<= TIME_LEVEL_SHIFT;
				} while ((last_cursor_ & (mask - 1)) == 0); // one loop finish
			}
		}
	}
	
	for (TimerPtr& timer : expire_list)
		timer->run();
}

void TimerWheel::debug()
{
	for (int i = 0; i < v_timers_.size(); ++i)
	{
		if (!v_timers_[i].empty())
		{
			printf("%d %d\n", i, v_timers_[i].size());
		}
	}
}

int32_t TimerWheel::addAndFetchTimerid()
{
	// to do 
	// use atomic
	return ++num_since_created_;
}

void TimerWheel::addTimer(TimerPtr timer)
{
	int32_t wheel_index = 0;
	int64_t expire_ticks = timer->expiration();
	int64_t diff_ticks = expire_ticks - last_cursor_;
	if (diff_ticks < TIME_NEAR)
	{
		wheel_index = expire_ticks & TIME_NEAR_MASK;
	}
	else
	{
		int level = 0;
		int64_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
		for (; level < 3; ++level)
		{
			if (diff_ticks < mask)
				break;

			mask <<= TIME_LEVEL_SHIFT;
		}
		wheel_index = OFFSET(level) + INDEX(expire_ticks, level);
	}

	TimerList& t_list = v_timers_[wheel_index];
	t_list.push_back(timer);

	timer->set_wheel_index(wheel_index);
	TimerListItor iter = t_list.end();
	timer->set_itor(--iter);
}

void TimerWheel::moveTimerList(int32_t wheel_index)
{
	TimerList move_list;
	move_list.splice(move_list.end(), v_timers_[wheel_index]);
	for (TimerPtr& timer : move_list)
		addTimer(timer);
}
