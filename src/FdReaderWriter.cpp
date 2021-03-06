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

#include <unistd.h>

#include "FdReaderWriter.h"
#include "jh_types.h"
#include "logging.h"
#include "File.h"

SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOTICE);

using namespace JetHead;

FdReaderWriter::FdReaderWriter( int fd ) : 
	mFd(fd), mSelector( NULL )
{
}	

FdReaderWriter::~FdReaderWriter()
{
}

void FdReaderWriter::setSelector(SelectorListener *listener, Selector *selector)
{	
	setSelector(listener, selector, POLLIN);
}


void FdReaderWriter::setSelector(SelectorListener *listener, Selector *selector,
								 short subEvents)
{	
	TRACE_BEGIN(LOG_LVL_NOTICE);
	if ( mFd != -1 )
	{
		if ( mSelector != NULL )
			mSelector->removeListener( mFd, listener );
		mSelector = selector;
		if ( mSelector != NULL )
			mSelector->addListener( mFd, subEvents, listener );
	}
}


int FdReaderWriter::read(void *buffer, int length)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	return ::read(mFd, buffer, length);
}


int FdReaderWriter::write(const void *buffer, int length)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	return ::write(mFd, buffer, length);
}


JetHead::ErrCode FdReaderWriter::close()
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	int res = ::close(mFd);
	
	if ( res == -1 )
		return getErrorCode( errno );

	return JetHead::kNoError;	
}
