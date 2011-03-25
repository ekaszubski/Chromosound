/*******************************************************************************
 *
 *      test_genetic_process
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
 *      * Neither the name of "GeneticAlgorithm" nor the names of its
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

#include "../include/genetic_process.h"
#include <time.h>

#define duration_resolution 16
#define num_beats 4 * duration_resolution

typedef unsigned int _SizeType;

struct AudioNote
{

	// piano key index
	_SizeType note_index; //[0,88]
	// what proportion of a beat this note will last
	_SizeType duration_index; //[1,duration_resolution]
	// which beat this note will play on; depends on time (num_beats)
	_SizeType beat_index; //[1, num_beats]
};

// enables printing of custom states
namespace GAUtil
{
	template<>
	struct Str<AudioNote>
	{
		static std::string str( AudioNote data )
		{
			std::stringstream ss;
			ss << "(" << data.note_index << ":" << data.duration_index << ":" << data.beat_index << ")";
			return ss.str();
		}
	};
}

// required for randomization and mutation of a gene based on the AudioNote data type
template<>
class Gene<AudioNote> : public GeneBase<AudioNote>
{
public:
	void mutate()
	{
		data_.note_index = GAUtil::rand( 1, 88 );
		data_.duration_index = GAUtil::rand( 1, duration_resolution );
		data_.beat_index = GAUtil::rand( 1, num_beats );
	}

	void randomize()
	{
		mutate();
	}
};

typedef AudioNote _DataType;
typedef double _FitnessType;

typedef Genome<_DataType, _FitnessType, _SizeType> _GenomeBase;

typedef typename _GenomeBase::_Chromosome _Chromosome;
typedef typename _GenomeBase::_ChromosomePtr _ChromosomePtr;


// required to calculate the fitness function for the AudioGenome
class AudioGenome : public _GenomeBase
{
public:
	AudioGenome( Descriptor descriptor, _ChromosomeVector chromosomes = _ChromosomeVector() ) :
		_GenomeBase( descriptor, chromosomes )
	{
		//
	}

	_FitnessType calculateFitness()
	{
		fitness_ = 1;
		printf("GOOD");
		return fitness_;
	}

	AudioGenome * copy( _SizeType start = 0, _SizeType copy_length = 0 )
	{
		return static_cast<AudioGenome *>( _GenomeBase::copy( start, copy_length) );
	}
};

typedef AudioGenome _Genome;

int main( int argc, char ** argv )
{

	const static _SizeType population_size = 4, genome_size = 10, chromosome_size = 2;
	double mutation_rate = 0.2;
	long rand_seed = time( NULL );

	typedef GeneticProcess<_Genome> _GeneticProcess;

	typedef typename _GeneticProcess::_GenomePtr _GenomePtr;

	typedef typename _GeneticProcess::_Family _Family;

	_GeneticProcess::Descriptor descriptor( population_size, mutation_rate, rand_seed, _Genome::Descriptor( genome_size, _Chromosome::Descriptor( chromosome_size ) ) );

	_GeneticProcess process( descriptor );

	process.printPopulation();

	process.initializePopulation();

	process.printPopulation();

	process.step( 1 );

	process.evaluatePopulation();

	process.printPopulation();

	process.evaluatePopulation();

	return 0;
}
