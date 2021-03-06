/*
 * Copyright (c) 2010, JetHead Development, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the JetHead Development nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JH_TIMER_H_
#define JH_TIMER_H_

#include "Event.h"
#include "Thread.h"
#include "Mutex.h"
#include "TimerManager.h"
#include "jh_list.h"

class TimerListener
{
public:
	/**
	 * Called by TimerManager when the timeout expires.  You class implements 
	 *  this and provide an pointer to this interface when a timer is created.
	 */
	virtual void onTimeout( uint32_t private_data ) = 0;

protected:
	//! Virtual destructor because the compiler is whiney
	virtual ~TimerListener() {}
};


class Timer : public RefCount
{
public:
	/**
	 *  @brief Initialize a new Timer
	 *
	 *  Create a new timer and set the requested tick time.   All clients
	 *  serviced by this timer will have a minimum resolution of the tick
	 *  time set at initialization.
	 *
	 *	NOTE:   All Timer objects are started immediately upon creation.
	 *
	 *  @param  tickTimeMs - Tick time in milliseconds for this Timer
	 *  @param  stoppable - Indicates whether timer is stoppable
	 */
	Timer(int tickTimeMs,
		  bool stoppable = true);
	
	/**
	 *  @brief Get tick time in milliseconds for this Timer
	 */
	int getTickTime();
	
	/**
	 *  @brief Start timer
	 *
	 *  This method starts the timer thread which will accumulate ticks
	 *  using the interval specified at construction.   This method will
	 *  do nothing if the Timer is already running.
	 */
	void start();
	
	/**
	 *  @brief Stop timer
	 *
	 *  This method stops a running timer thread.   On exit the thread
	 *  is stopped and the timer is no longer running.   This also results
	 *  in all current timer nodes being destroyed without firing.
	 *  This method will do nothing if the Timer is not stoppable.
	 */
	void stop();
	
	/**
	 * @brief Add event to be dispatched in the future
	 *
	 *  Add an event to be dispatched to the given dispatcher at a time in 
	 *  the future.  This event's ref count will be incremented to ensure
	 *  the event is not destroyed while we wait.  NOTE: use the Selector or
	 *  EventThread to send timed events do no call this directly.
	 *
	 * @param event - the event to send.
	 * @param dispatcher - the dispatcher to dispatch event to
	 * @param msecs - how many milliseconds to wait before dispatching.
	 */
	void sendTimedEvent( Event *event, 
						 IEventDispatcher *dispatcher, 
						 uint32_t msecs );

	/**
	 * @brief Add event to be dispatched periodically
	 *
	 *  Add an event to be dispatched to the given dispatcher with a
	 *  regular period.  This event's ref count will be incremented to
	 *  ensure the event is not destroyed before it is removed.  NOTE: use
	 *  the Selector or EventThread to send timed events do no call
	 *  this directly.
	 *
	 * @param event - the event to send.
	 * @param dispatcher - the dispatcher to dispatch event to
	 * @param period - how many milliseconds to wait before dispatching.
	 */
	void sendPeriodicEvent( Event *event,
							IEventDispatcher *dispatcher,
							uint32_t period );

	/**
	 * Remove all timed events from a given dispatcher with a certian
	 * event id or if id is kInvalidId then remove all events on the given
	 * dispatcher.
	 *
	 * @param event_id - the event id to remove or kInvalidEventId to
	 * remove all.
	 * @param dispatcher - the dispatcher for which we are removing events.
	 */
	void removeTimedEvent( Event::Id event_id, IEventDispatcher *dispatcher );

	/**
	 * Remove the timed event.  In this case we don't care what the
	 * dispatcher was.
	 *
	 * @param ev - the event to remove.
	 */
	void removeTimedEvent( Event *ev );

	/**
	 * Remove any EventAgents from this dispatcher with this destination
	 *
	 * @param reciever - the delivery target for an event agent
	 *
	 * @param dispatcher - the dispatcher to clear those agents for or
	 * NULL for all
	 */
	void removeAgentsByReceiver(void *reciever, IEventDispatcher *dispatcher);

	/**
	 * Add a timer listener for a one time delayed notification
	 *
	 * @param listener - the listener to send notification to
	 * @param msecs - how many milliseconds to wait before notification
	 * @param private_data - private data associated with the notification
	 */
	/**
	 * Add a timer listener.  This listener will be called after msecs.
	 */
	void addTimer( TimerListener *listener, 
				   uint32_t msecs,
				   uint32_t private_data );
	
	/**
	 * Add a timer listener for a periodic delayed notification.
	 *
	 * @param listener - the listener to send notification to
	 * @param period - how many milliseconds to wait before notification
	 * @param private_data - private data associated with the notification
	 */
	/**
	 * Add a timer listener.  This listener will be called after msecs.
	 */
	void addPeriodicTimer( TimerListener *listener,
						   uint32_t msecs,
						   uint32_t private_data );
protected:
	
	//!  Reference counted object, destructor is protected.
	virtual ~Timer();
	
	//! These are the time events we are currently tracking
	struct TimerNode
	{
		//! The event we will pass
		SmartPtr<Event> mEvent;
		
		//! The dispatcher (Selector or EventDispatcher) we will send the event
		IEventDispatcher *mDispatcher;

		//! The listener that will be told by the dispatcher of the event
		TimerListener *mListener;

		//! The opaque data that will be given to the listener
		uint32_t mPrivateData;

		//! Which tick will trigger this event
		uint32_t mTick;

		//! After how many ms does this repeat?  (Or 0 for no repetition)
		uint32_t mRepeatMS;

		//! How many leftover ms have we accumulated if this is repeating?
		uint32_t mRemainingMS;
	};
	
	//! Add a new time event
	void addTimerNode( const TimerNode& node );
	
	//! Our main loop
	void clockHandler();

	//! Handle a clock tick (every kMsPerTick)
	void handleTick();
	
	//! Reset timeout list and mTicks
	void reset();
	
	//! Stop the clock thread and clean up data associated with it.
	void doStop();
	
	//! Our thread
	Runnable<Timer> *mClockThread;

	//! What is the resolution of this Timer in ms.
	int mMsPerTick;
	
	//! Is this Timer "stoppable" or not
	bool mStoppable;
	
	//! Locking for internal state (only really mList and mTicks)
	Mutex mMutex;
	
	//! Everything we are waiting on
	JetHead::list<TimerNode> mList;

	//! Number of ticks we have seen
	uint32_t mTicks;
};

#endif // JH_TIMER_H_
