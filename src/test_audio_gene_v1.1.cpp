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

#include "../include/genetic_process.h"
#include "../include/audio_genome.h"
#include "../include/alut_util.h"
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

// 0:0, 1:0, 0:0, 2:0
// no effect,
typedef AudioGenome _AudioGenome;
typedef _AudioGenome _Genome;
typedef GeneticProcess<_Genome> _GeneticProcess;

typedef typename _GeneticProcess::_GenomePtr _GenomePtr;

typedef typename _GeneticProcess::_PopulationVector _PopulationVector;
typedef typename _GeneticProcess::_PopulationIterator _PopulationIterator;

typedef typename _GeneticProcess::_ChromosomeIterator _ChromosomeIterator;

typedef typename _GeneticProcess::_GeneIterator _GeneIterator;
typedef typename _GeneticProcess::_GeneticPair _GeneticPair;

typedef AudioGenomeDefs::_SizeType _SizeType;
typedef AudioGenomeDefs::_GenomeBase _GenomeBase;
typedef AudioGenomeDefs::WaveDescriptor _WaveDescriptor;

typedef typename _GenomeBase::_Chromosome _Chromosome;
typedef typename _GenomeBase::_ChromosomePtr _ChromosomePtr;
typedef typename _AudioGenome::_WaveDescriptorIterator _WaveDescriptorIterator;

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
			_WaveDescriptor current_descriptor = *wave_descriptor_it;
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
			alutCheckAndQueueBuffer( sound_source_, alutCreateBufferWaveform( current_descriptor.wave_type, current_descriptor.frequency, current_descriptor.phase, current_descriptor.duration ) );

		}
	}
	return num_buffers;
}

// enables printing of custom states
/*namespace GeneticProcessUtil
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
}*/

//WaveFSM AudioGenome::wave_fsm = WaveFSM();

int main( int argc, char **argv )
{
	const _SizeType population_size = 10, genome_size = 4 * 16, chromosome_size = 1;
	const double mutation_rate = 0.05;
	const long rand_seed = time( NULL );

	_GeneticProcess::Descriptor descriptor( population_size, mutation_rate, rand_seed, _Genome::Descriptor( genome_size, _Chromosome::Descriptor( chromosome_size ) ) );

	_GeneticProcess process( descriptor );

	process.initializePopulation();
	process.printPopulation();

	if ( !alutInit( &argc, argv ) ) alutReportError();

	alGenSources( 1, &sound_source_ );

	loadPopulationIntoBuffer( process.population() );

	process.step();
	process.printPopulation();

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
				loadPopulationIntoBuffer( process.step() );
				//process.printPopulation();
			}
		}

		alutSleep( 1 );
	}
	while ( num_buffers_queued > 0 && generation_counter < 20 );

	process.evaluatePopulation();

	process.printPopulation();

	process.evaluatePopulation();

	if ( !alutExit() )
	{
		alutReportError();
	}
	return EXIT_SUCCESS;
}
