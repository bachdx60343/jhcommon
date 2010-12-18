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

#ifndef JH_SELECTOR_H_
#define JH_SELECTOR_H_

#include "jh_list.h"
#include "EventThread.h"
#include "Mutex.h"

#include <sys/poll.h>

/** This interface is must be implemented by any class that want to revieve 
 *   information about file events.  
 */
class SelectorListener
{
public:
	/** 
	 * This method is called to inform a listener that file event(s)
	 * have occured on a specific file descriptor.  There could be one
	 * or more events.  Also you could be informed about events that
	 * you didn't register for.  Specifically you will always get
	 * POLLNVAL and POLLHUP if they occur.
	 *
	 * @param fd the file descriptor that the event(s) occured on.
	 * @param events the set of events that occured.
	 * @param private_data the private data param that was given to
	 * addListener.
	 */
	virtual void processFileEvents( int fd, short events, 
									jh_ptr_int_t private_data ) = 0;
protected:
	//! Virtual destructor, does nothing, just for compile warning
	virtual ~SelectorListener() {}
};

/**
 * This class is used to build a thread that waits for file events.
 * This class can be used to replace code that would have
 * traditionally used the select or poll system calls.  Apon
 * contruction this class will start a thread.  When descructed this
 * thread will be shut down.  Therefore the lifetime of a selector
 * should be managed to ensure that undo overhead is not generated by
 * the creation/descruction of the thread.  This class also inherites
 * from the EventDispatcher class and therefor is capable of handling
 * recieving of JHCommon event on the thread managed by the selector.
 * All the methods in the Selector class deal with handling file
 * events only.  See EventDispatcher for information about
 * handling JHCommon events.
 */
class Selector : public EventDispatcher
{
public:
	/** 
	 * Will contruct a selector class and start it's thread running.
	 *
	 * @param name the name of the selector.  This name will be used
	 * as the thread name.  This is usefull for debugging.  If a NULL
	 * name is provided or this optional param is omited the default
	 * name of "Selector" will be used as the thread name.
	 */
	Selector( const char *name = NULL );

	/** 
	 * Desctroy the selector and shutdown the thread.  This function
	 * will send and event to the selector to shutdown and will wait
	 * for the thread to exit before returning.
	 */
	virtual ~Selector();

	/** 
	 * Called to shutdown the thread for the selector and wait for it
	 * to end.  If this is not called the descrutor will call it, but
	 * sometimes it's not convenient to wait for the destructor.
	 */
	void shutdown();
	
	/** 
	 * Add a listener for a set of poll events.  See the poll man page
	 * for the possible options.  This call is ussally not called by
	 * the users.  Since most classes will implement a wrapper for
	 * listening to events.
	 *
	 * @param fd the file descriptor to listener for events on.
	 * @param events the bit field of event to listen for.  When these events 
	 *  occur the listener will be called.  These events are defined by the
	 *  systems poll function.  See "man 2 poll" for a full description.
	 * @param listener the interface to call when an event occurs.
	 * @param private_data this data will be passed to the listener when ever
	 *  the listener is informed of an event.
	 */
	void addListener( int fd, short events, 
					  SelectorListener *listener, jh_ptr_int_t private_data = 0 );

	/** 
	 * Remove a listener(s) previously added.  We remove any matches
	 * to the fd and the listener interface.
	 *
	 * @param fd the file descriptor that was previously added.
	 * @param listener the interface previously added.
	 */
	void removeListener( int fd, SelectorListener *listener );
	
private:
	struct ListenerNode
	{
		//! Default constructor, does nothing
		ListenerNode() {}

		//! Things on this fd should be handled by this listener
		ListenerNode( int fd, SelectorListener *listener ) : mFd( fd ), 
			mListener( listener ) {}

		//! Is this Listener the same as the other?
		bool operator==( const ListenerNode &other )
		{
			// Must match FDs, and if listeners are non-NULL, they
			// must match too.
			if ( mFd == other.mFd and
				( ( ( mListener == NULL) or ( other.mListener == NULL ) ) or
					( mListener == other.mListener ) ) )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		
		//! The FD
		int mFd;

		//! The events this listener is interested in
		short mEvents;

		//! The listener to call 
		SelectorListener *mListener;

		//! Some opaque private data that is passed back to the listener
		jh_ptr_int_t mPrivateData;
	};
	
	enum {
		//! mPipe[PIPE_READER] is the read end
		PIPE_READER = 0,

		//! mPipe[PIPE_WRITER] is the write end
		PIPE_WRITER = 1
	};
	
	//! How many FDs can one selector be polling at once?
	static const int kMaxPollFds = 64;
	
	//! Trigger a call to fillPollFds when it is safe to do so
	void updateListeners();	

	//! Call everyone that is listening for events on this fd
	bool callListeners( int fd, uint32_t events );

	//! Fill up the pollfds we will be calling poll on.
	void fillPollFds( struct pollfd *fds, int &numFds );
	
	// Event Dispatcher overrides

	/**
	 * This is the main loop, the innermost portion of the Selector
	 * event loop.  The thread is here most of the time, unless we are
	 * processing an event.
	 */  
	void threadMain(); 
	
	//! Send a null event to the selector to wake it up 
	void wakeThread();

	const Thread *getDispatcherThread();
	
	/**
	 * Find the ListenerNode corresponding to this fd (and optionally
	 * this listener).
	 */
	ListenerNode *findListener( int fd, SelectorListener *listener = NULL );
	
	//! The list of ListenerNodes
	JetHead::list<ListenerNode*> mList;

	//! The lock on my internal state
	Mutex			mLock;

	/**
	 * In order to have non-fd related events working with the
	 * poll-based eventing of the Selector, we have this pipe.  Events
	 * are packaged up and sent on the writable end of the pipe, the
	 * readable end of the pipe triggers a poll event.
	 */
	int				mPipe[ 2 ];

	//! The thread object for the selector's thread
	Runnable<Selector> 	mThread;

	//! Should I be shutting down?
	bool			mShutdown;

	//! Am I running?
	bool			mRunning;

	//! Should I update the pollfds that I am polling on?
	bool			mUpdateFds;

	/**
	 * Used to make calls in the public interface blocking until they
	 * have been properly handled
	 */
	Condition		mCondition;
};

#endif // JH_SELECTOR_H_
