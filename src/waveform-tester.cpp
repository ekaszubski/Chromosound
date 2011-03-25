/*******************************************************************************
 *
 *      waveform-tester
 * 
 *      Copyright (c) 2011, edward
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *      
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of "ALUT" nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *      
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************/

#include <stdlib.h>
#include <AL/alut.h>
#include <stdio.h>
#include <deque>
#include <string>
#include <sstream>
#include <time.h>
#include <math.h>

ALuint sound_source_;
ALenum shape_ = ALUT_WAVEFORM_SINE;
ALfloat frequency_ = 1000;
ALfloat phase_ = 0.0f;
ALfloat duration_ = 0.5f;

static void reportError( void )
{
	fprintf( stderr, "ALUT error: %s\n", alutGetErrorString( alutGetError() ) );
	exit( EXIT_FAILURE );
}

void checkAndQueueBuffer( const ALuint & buffer )
{
	if ( !buffer ) reportError();
	else
	{
		alSourceQueueBuffers( sound_source_, 1, &buffer );
	}
}

static float frand()
{
	return (float) rand() / RAND_MAX;
}

static int irand( int min, int max )
{
	return min + (max - min) * frand();
}

static float keyToFreq( unsigned int key )
{
	return 440 * pow( 2, ( (float)key - 49 ) / 12 );
}

int main( int argc, char **argv )
{
	long rand_seed = time( NULL );
	printf( "seed: %li\n", rand_seed );
	srand( rand_seed );

	if ( !alutInit( &argc, argv ) ) reportError();

	// printf( "%s\n", alutGetMIMETypes( ALUT_LOADER_BUFFER ) );

	alGenSources( 1, &sound_source_ );

	// queue up (starting at the front) our rotating queue of file buffers
	for ( int i = 0; i < 20; i++ )
	{
		int key = irand( 1, 88 );
		float frequency = keyToFreq( key );
		printf( "%i %f\n", key, frequency );
		checkAndQueueBuffer( alutCreateBufferWaveform( shape_, frequency, phase_, 0.0625 ) );
	}

	alSourcePlay( sound_source_ );
	int num_buffers_processed, num_buffers_queued, num_songs_played = 0;
	do
	{
		alGetSourcei( sound_source_, AL_BUFFERS_PROCESSED, &num_buffers_processed );
		alGetSourcei( sound_source_, AL_BUFFERS_QUEUED, &num_buffers_queued );

		printf( "%i %i %i\n", num_buffers_processed, num_buffers_queued, num_songs_played );

		// each time we process a buffer, unqueue the most recently processed buffer
		// and queue up the buffer for the next item in the filenames list
		if ( num_buffers_processed > 0 )
		{
			// unload the front buffer
			ALuint buffer[num_buffers_processed];
			alSourceUnqueueBuffers( sound_source_, num_buffers_processed, &buffer[0] );

			// delete this buffer since we can't reuse them nicely
			alDeleteBuffers( num_buffers_processed, &buffer[0] );

			// add the next file to the queue
			//checkAndQueueBuffer( createBufferFromFile( filenames_.getFrontAdvance() ) );

			num_songs_played++;
		}

		alutSleep( 1 );
	}
	while ( num_buffers_queued > 0 && num_songs_played < 15 );

	if ( !alutExit() )
	{
		reportError();
	}
	return EXIT_SUCCESS;
}
