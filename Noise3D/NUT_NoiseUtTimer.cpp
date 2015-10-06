
/***********************************************************************

							类：NOISE TIMER

	简述：高精度计时器，主要用WINAPI的queryPerformanceCount来实现    

***********************************************************************/
#include "NoiseUtility.h"

NoiseUtTimer::NoiseUtTimer(NOISE_TIMER_TIMEUNIT timeUnit = NOISE_TIMER_TIMEUNIT_MILLISECOND)
{
	//默认用毫秒制
	mTimeUnit				= timeUnit;
	mMilliSecondsPerCount	= 0.0;
	mDeltaTime				= 0.0;
	mTotalTime				= 0.0;
	mIsPaused				= FALSE;

	//每秒可以数多少次
	INT64 countsPerSecond;
	//获取这个计数计时器的频率
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	mMilliSecondsPerCount = (1000.0) /(double)countsPerSecond;//每一count多少毫秒
	QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);
}

void NoiseUtTimer::SelfDestruction()
{
};

void NoiseUtTimer::Next()
{
	if(mIsPaused)
	{
		mDeltaTime = 0.0;
	}
	//如果没有暂停
	else
	{
		//更新count
		mPrevCount = mCurrentCount;
		QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);

		//如果在省电模式下，若切换处理器可能会导致counts也是负的
		mDeltaCount = mCurrentCount - mPrevCount;
		BOOL isDeltaTimePositive = ((mDeltaCount) > 0);
		if(isDeltaTimePositive)
		{
			mDeltaTime =(double)(mDeltaCount * mMilliSecondsPerCount);
		}
		else
		{
			mDeltaTime = 0.0;
		};

		//没暂停就更新总时间 单位：ms
		mTotalTime += mDeltaTime;
	};
};

double NoiseUtTimer::GetTotalTime()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return mTotalTime; 
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return (mTotalTime/1000); 
		break;
	};
	return 0;
};

double NoiseUtTimer::GetInterval()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return mDeltaTime; 
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return (mDeltaTime/1000); 
		break;
	};
	return 0;
};

void NoiseUtTimer::SetTimeUnit(NOISE_TIMER_TIMEUNIT timeUnit)
{
	if (timeUnit ==NOISE_TIMER_TIMEUNIT_SECOND||timeUnit==NOISE_TIMER_TIMEUNIT_MILLISECOND)
	{mTimeUnit = timeUnit;};
};

void NoiseUtTimer::Pause()
{
	mIsPaused = TRUE;
};

void NoiseUtTimer::Continue()
{
	mIsPaused = FALSE;
};

void NoiseUtTimer::ResetAll()
{
	mTotalTime	= 0.0;
	mDeltaTime	= 0.0;
	mIsPaused	= FALSE;
};

void NoiseUtTimer::ResetTotalTime()
{
	mTotalTime = 0;
};
