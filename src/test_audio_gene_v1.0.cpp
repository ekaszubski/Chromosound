/*******************************************************************************
 *
 *      test_audio_gene
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
 *      * Neither the name of "Chromosound" nor the names of its
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
#include <iostream>
#include <typeinfo>
#include <deque>

std::deque<int> state;
typename std::deque<int>::iterator it = state.begin();

#include "../include/genetic_process.h"
#include <time.h>

//up to 1/(2^4) second beat resolution
//so for each note, we can hold it 1, 1/2, 1/4, 1/8, or 1/16 beat
// (ie 5 combinations)
#define duration_resolution 5
// assume we're running 1 measure per second
// 4 beats per measure
// then we have 4*16 potential beat-start combinations
// #define num_beats 4 * 16

ALuint sound_source_;
ALenum shape_ = ALUT_WAVEFORM_SINE;
ALfloat frequency_ = 1000;
ALfloat phase_ = 0.0f;
ALfloat duration_ = 0.5f;

void reportError( void )
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

template<class _KeyDataType>
float getFrequency( _KeyDataType key )
{
	return key <= 0 ? 1.0 : 440 * pow( 2, ( (float) key - 49 ) / 12 );
}

template<class _DataType>
void range( _DataType & value, _DataType low, _DataType high, bool wrap = false )
{
	if ( !wrap )
	{
		value = value < low ? low : value > high ? high : value;
		return;
	}
	while ( value < low )
		value = high + ( low - value );
	while ( value > high )
		value -= ( high - low );
}

typedef unsigned int _SizeType;
typedef int _AudioStateControlStorage;

namespace AudioBitEncodings
{
	// bool - toggle
	const static _SizeType commit = 0;
	const static _SizeType key_rotate = 1;
	const static _SizeType key_flip = 2;

	// int - increment by value
	const static _SizeType duration = 3;
	const static _SizeType time = 4;
	const static _SizeType pitch = 5;
}

// one possible encoding
/*struct AudioNote
 {

 // piano key index
 _SizeType note_index; //[0,88]
 // what proportion of a beat this note will last
 _SizeType duration_index; //[1,duration_resolution]
 // which beat this note will play on; depends on time (num_beats)
 // _SizeType beat_index; //[1, num_beats]
 };*/

// another possible implementation
struct AudioStateControl
{
	// which aspect of the current waveform state to modify
	// [pitch+/-, duration+/-, pause|, time*, key_flip|, key_rotate+/-, separator|]
	// each bit builds upon the changes made by the previous bit
	_AudioStateControlStorage toggle_type;
	// the value for this modification
	_AudioStateControlStorage toggle_value;
};

typedef AudioStateControl _DataType;

// enables printing of custom states
namespace GAUtil
{
	template<>
	struct Str<_DataType>
	{
		static std::string str( _DataType data )
		{
			std::stringstream ss;
			ss << "(" << data.toggle_type << ":" << data.toggle_value << ")";// << ":" << data.beat_index << ")";
			return ss.str();
		}
	};
}

struct WaveDescriptor
{
	int type; //silent/active
	ALenum wave_type;
	float frequency;
	float phase;
	float duration;

	WaveDescriptor( int type_, ALenum wave_type_, float frequency_, float phase_, float duration_ )
	{
		type = type_;
		wave_type = wave_type_;
		frequency = frequency_;
		phase = phase_;
		duration = duration_;
	}
};

struct WaveState
{

	// 0: we had a commit bit; now we're counting the number of cycles until the next event
	// 1: our note timed out
	// 2: we reached another commit bit before the current note's time was up


	int key_index;
	bool key_is_major;
	int duration_index;
	int beat_frequency_index;
	int pitch_index;

	float phase;
	ALenum shape;

	WaveState()
	{
		key_index = 0;
		key_is_major = true;
		duration_index = 0;
		beat_frequency_index = 0;
		pitch_index = 40;

		phase = 0;
		shape = ALUT_WAVEFORM_SINE;
	}

	std::string toString()
	{
		std::stringstream ss;
		ss << "key_index: " << key_index << "\n";
		ss << "key_is_major: " << key_is_major << "\n";
		ss << "duration_index: " << duration_index << "\n";
		ss << "beat_frequency_index: " << beat_frequency_index << "\n";
		ss << "pitch_index: " << pitch_index << "\n";
		return ss.str();
	}
};

// 0:0, 1:0, 0:0, 2:0
// no effect,

struct WaveFSM
{
	// if our note timed out, we can play the previous note and start counting the silence time
	// if we found a commit bit and our note didn't time out, we can play the previous note for the number of cycles specified by the counter
	// if we found a commit bit and our note did time out, we can start playing the silence wave for the number of cycles specified by the counter
	// either way, if we found a commit bit, we can start counting the number of cycles to play the current note

	unsigned int timer_counter;
	unsigned int timer_max;
	bool timer_enabled;
	bool note_interrupted;
	unsigned int last_note_duration;
	bool counting_silence;
	bool has_previous_state;

	WaveState state, last_state;

	WaveFSM()
	{
		printf( "\n\n This should only print once!!\n\n" );
		timer_counter = 0;
		timer_max = 0;
		timer_enabled = true;
		note_interrupted = false;
		last_note_duration = 0;
		counting_silence = false;
		has_previous_state = false;
	}

	// return 0 to do nothing
	// return 1 to play last_state
	// return 2 to play silence
	int update( _DataType control_bit )
	{
		__DEBUG__NORMAL__ printf( "%u: Updating %s\n", timer_counter, GAUtil::Str<_DataType>::str( control_bit ).c_str() );

		int result = 0;
		bool do_commit = false;
		switch ( control_bit.toggle_type )
		{
		//bool - toggle if( value )
		case AudioBitEncodings::commit:
			do_commit = control_bit.toggle_value;
			if ( do_commit ) __DEBUG__NORMAL__ printf( "Saving current state:\n%s\n", state.toString().c_str() );
			// save our current state for reference; start making changes to our new state
			last_state = state;
			has_previous_state = true;
			break;
		case AudioBitEncodings::key_rotate:
			state.key_index += control_bit.toggle_value ? 1 : -1;
			range( state.key_index, (int) 0, (int) 12, true );
			__DEBUG__NORMAL__ printf( "Updated key_index: %u\n", state.key_index );
			break;
		case AudioBitEncodings::key_flip:
			if ( control_bit.toggle_value ) state.key_is_major = !state.key_is_major;
			__DEBUG__NORMAL__ printf( "Updated key_is_major: %u\n", state.key_is_major );
			break;

			// int - increment by value
		case AudioBitEncodings::duration:
			state.duration_index += control_bit.toggle_value;
			range( state.duration_index, (int) 0, (int) 4 );
			__DEBUG__NORMAL__ printf( "Updated duration_index: %u\n", state.duration_index );
			break;
		case AudioBitEncodings::time:
			state.beat_frequency_index += control_bit.toggle_value;
			range( state.beat_frequency_index, (int) 0, (int) 3 );
			__DEBUG__NORMAL__ printf( "Updated beat_frequency_index: %u\n", state.beat_frequency_index );
			break;
		case AudioBitEncodings::pitch:
			state.pitch_index += control_bit.toggle_value;
			range( state.pitch_index, (int) 21, (int) 68 );
			__DEBUG__NORMAL__ printf( "Updated pitch_index: %u\n", state.pitch_index );
			break;
		}

		if ( timer_enabled ) ++timer_counter;
		if ( timer_counter == timer_max ) timer_enabled = false;

		if ( !has_previous_state ) return 0;

		// commit reached
		if ( do_commit )
		{
			// our counter now contains the number of cycles for which we should play silence
			if ( counting_silence )
			{
				result = 2;
				last_note_duration = timer_counter;
				__DEBUG__NORMAL__ printf( "We should play silence for %u cycles\n", last_note_duration );
			}
			// we interrupted playback of the previous note; our counter shows how many cycles we should play it for
			else
			{
				result = 1;
				last_note_duration = timer_counter;
				__DEBUG__NORMAL__ printf( "Note interrupted. We should play sound for %u cycles\n", last_note_duration );
			}

			// start a timer to count the number of cycles the current note is active
			startTimer( getDuration( state.duration_index ) );
		}
		// we timed out, store how long we were playing the note for and start tracking how long we should play silence
		else if ( !timer_enabled )
		{
			result = 1;
			counting_silence = true;
			last_note_duration = timer_counter;
			__DEBUG__NORMAL__ printf( "Note finished playing normally. We should play sound for %u cycles\n", last_note_duration );
			startTimer();
		}

		return result;
	}
	// 1. commit reached; start countdown
	// 2. countdown timed out; start countup
	// 3. commit reached;
	//       1       2     3
	// X X X C X X X[X]X X[C]

	void startTimer( unsigned int timer_value = 0 )
	{
		__DEBUG__NORMAL__ printf( "Starting timer for %u cycles\n", timer_value );
		timer_enabled = true;
		timer_counter = 0;
		timer_max = timer_value;
	}

	// 0 -> wait 1; 4 -> wait 16
	unsigned int getDuration( unsigned int duration_index_ )
	{
		return pow( 2, duration_index_ );
	}
};

// 0 -> 1/16; 4 -> 1/1
static float getDurationFromIndex( _SizeType duration_index )
{
	return 1 / pow( 2, 4 - duration_index );
}

static float getDurationFromCycles( _SizeType num_cycles )
{
	return (float) num_cycles / 16;
}

// required for randomization and mutation of a gene based on the AudioNote data type
template<>
class Gene<_DataType> : public GeneBase<_DataType>
{
public:
	void mutate()
	{
		data_.toggle_type += GAUtil::rand( 1 );
		range( data_.toggle_type, 0, 5, true );
		switch ( data_.toggle_type )
		{
		//bool - toggle if( value )
		case AudioBitEncodings::commit:
			data_.toggle_value = !data_.toggle_value;
			break;
		case AudioBitEncodings::key_rotate:
			data_.toggle_value = !data_.toggle_value;
			break;
		case AudioBitEncodings::key_flip:
			data_.toggle_value = !data_.toggle_value;
			break;

			// int - increment by value
		case AudioBitEncodings::duration:
			data_.toggle_value = GAUtil::rand( 2, true );
			//range( data_.toggle_value, (_AudioStateControlStorage) 0, (_AudioStateControlStorage) 4 );
			break;
		case AudioBitEncodings::time:
			data_.toggle_value = GAUtil::rand( 2, true );
			//range( data_.toggle_value, (_AudioStateControlStorage) 0, (_AudioStateControlStorage) 3 );
			break;
		case AudioBitEncodings::pitch:
			data_.toggle_value = GAUtil::rand( 20, true );
			//range( data_.toggle_value, (_AudioStateControlStorage) 21, (_AudioStateControlStorage) 68 );
			break;
		}
	}

	void randomize()
	{
		data_.toggle_type = GAUtil::rand( 0, 5 );
		switch ( data_.toggle_type )
		{
		//bool - toggle if( value )
		case AudioBitEncodings::key_rotate:
			data_.toggle_value = GAUtil::rand( 0, 1 );
			break;
		case AudioBitEncodings::commit:
			data_.toggle_value = GAUtil::rand( 0, 1 );
			break;
		case AudioBitEncodings::key_flip:
			data_.toggle_value = GAUtil::rand( 0, 1 );
			break;

			// int - increment by value
		case AudioBitEncodings::duration:
			data_.toggle_value = GAUtil::rand( 2 );
			break;
		case AudioBitEncodings::time:
			data_.toggle_value = GAUtil::rand( 2 );
			break;
		case AudioBitEncodings::pitch:
			data_.toggle_value = GAUtil::rand( 20 );
			break;
		}
	}
};

typedef double _FitnessType;

typedef Genome<_DataType, _FitnessType, _SizeType> _GenomeBase;

typedef typename _GenomeBase::_Chromosome _Chromosome;
typedef typename _GenomeBase::_ChromosomePtr _ChromosomePtr;

WaveFSM wave_fsm = WaveFSM();

// required to calculate the fitness function for the AudioGenome
class AudioGenome : public _GenomeBase
{
public:
	std::vector<WaveDescriptor> wave_descriptors;
	typedef typename std::vector<WaveDescriptor>::iterator _WaveDescriptorIterator;
	// we want to use the same WaveFSM object for all audio genes so that the song is continuous.
	AudioGenome( Descriptor descriptor, _ChromosomeVector chromosomes = _ChromosomeVector() ) :
		_GenomeBase( descriptor, chromosomes )
	{
		//
	}

	_FitnessType calculateFitness()
	{
		// printf( "calculateFitness in AudioGenome\n" );
		wave_descriptors.clear();// = std::vector<WaveDescriptor>();
		// printf( "wave descriptors size: %zu\n", wave_descriptors.size() );
		fitness_ = 0;

		for ( _ChromosomeIterator chromosome_it = begin(); chromosome_it != end(); ++chromosome_it )
		{
			_ChromosomePtr current_chromosome = *chromosome_it;
			// every gene advances our "clock" 1/16 of a beat
			// the wave state is updated first
			// if we've reached a commit bit, the buffer for the wave is generated and queued
			// if our note duration has expired, we go silent (queue silent wave buffer)
			for ( _GeneIterator gene_it = current_chromosome->begin(); gene_it != current_chromosome->end(); ++gene_it )
			{
				int type = wave_fsm.update( gene_it->data_ );

				if ( type != 0 )
				{
					const float duration = getDurationFromCycles( wave_fsm.last_note_duration );
					const float frequency = type == 1 ? getFrequency( wave_fsm.last_state.pitch_index ) : type == 2 ? 10 : 0;

					wave_descriptors.push_back( WaveDescriptor( type, wave_fsm.last_state.shape, frequency, wave_fsm.last_state.phase, duration ) );

					// we want as many beats (and therefore as little silence) as possible
					// so give a positive reward for non-silent notes
					// and a negative reward for silent notes
					if ( type == 1 ) ++fitness_;
					if ( type == 2 ) --fitness_;
				}
			}
		}

		// printf( "wave descriptors new size: %zu\n", wave_descriptors.size() );

		return fitness_;
	}

	AudioGenome * copy( _SizeType start = 0, _SizeType copy_length = 0 )
	{
		_GenomeBase * old_genome = _GenomeBase::copy( start, copy_length );
		AudioGenome * new_genome = new AudioGenome( descriptor_, chromosomes_ );
		delete old_genome;
		return new_genome;
	}
};

typedef AudioGenome _Genome;
typedef AudioGenome::_WaveDescriptorIterator _WaveDescriptorIterator;

typedef GeneticProcess<_Genome> _GeneticProcess;

typedef typename _GeneticProcess::_GenomePtr _GenomePtr;

typedef typename _GeneticProcess::_PopulationVector _PopulationVector;
typedef typename _GeneticProcess::_PopulationIterator _PopulationIterator;

typedef typename _GeneticProcess::_ChromosomeIterator _ChromosomeIterator;

typedef typename _GeneticProcess::_GeneIterator _GeneIterator;
typedef typename _GeneticProcess::_GeneticPair _GeneticPair;

unsigned int loadPopulationIntoBuffer( _PopulationVector population )
{
	unsigned int num_buffers = 0;
	// queue up (starting at the front) our rotating queue of file buffers
	for ( _PopulationIterator population_it = population.begin(); population_it != population.end(); ++population_it )
	{
		_GenomePtr current_genome = *population_it;
		num_buffers += current_genome->wave_descriptors.size();
		// printf( "number of wave descriptors: %zu\n", current_genome->wave_descriptors.size() );
		for ( _WaveDescriptorIterator wave_descriptor_it = current_genome->wave_descriptors.begin(); wave_descriptor_it != current_genome->wave_descriptors.end(); ++wave_descriptor_it )
		{
			WaveDescriptor current_descriptor = *wave_descriptor_it;
			// every gene advances our "clock" 1/16 of a beat
			// the wave state is updated first
			// if we've reached a commit bit, the buffer for the wave is generated and queued
			// if our note duration has expired, we go silent (queue silent wave buffer)

			if ( current_descriptor.type == 1 )
			{
				printf( "Creating sound clip: %f %f\n", current_descriptor.frequency, current_descriptor.duration );
			}
			else
			{
				printf( "Creating silent clip: %f %f\n", current_descriptor.frequency, current_descriptor.duration );
			}
			checkAndQueueBuffer( alutCreateBufferWaveform( current_descriptor.wave_type, current_descriptor.frequency, current_descriptor.phase, current_descriptor.duration ) );

		}
	}
	return num_buffers;
}

//WaveFSM AudioGenome::wave_fsm = WaveFSM();

int main( int argc, char **argv )
{
	const _SizeType population_size = 10, genome_size = 4*16, chromosome_size = 1;
	const double mutation_rate = 0.05;
	const long rand_seed = time( NULL );

	_GeneticProcess::Descriptor descriptor( population_size, mutation_rate, rand_seed, _Genome::Descriptor( genome_size, _Chromosome::Descriptor( chromosome_size ) ) );

	_GeneticProcess process( descriptor );

	process.initializePopulation();
	process.evaluatePopulation();
	process.printPopulation();

	if ( !alutInit( &argc, argv ) ) reportError();

	alGenSources( 1, &sound_source_ );

	loadPopulationIntoBuffer( process.population() );

	_SizeType generation_counter = 0;

	alSourcePlay( sound_source_ );
	int num_buffers_processed, num_buffers_queued;
	do
	{
		alGetSourcei( sound_source_, AL_BUFFERS_PROCESSED, &num_buffers_processed );
		alGetSourcei( sound_source_, AL_BUFFERS_QUEUED, &num_buffers_queued );

		printf( "%i/%i:%u\n", num_buffers_processed, num_buffers_queued, generation_counter );

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
			// checkAndQueueBuffer( createBufferFromFile( filenames_.getFrontAdvance() ) );

			if ( num_buffers_queued <= 50 )
			{
				++generation_counter;
				process.step();
				process.evaluatePopulation();
				loadPopulationIntoBuffer( process.population() );
				process.printPopulation();
			}
		}

		alutSleep( 1 );
	}
	while ( num_buffers_queued > 0 && generation_counter < 15 );

	process.evaluatePopulation();

	process.printPopulation();

	process.evaluatePopulation();

	if ( !alutExit() )
	{
		reportError();
	}
	return EXIT_SUCCESS;
}
