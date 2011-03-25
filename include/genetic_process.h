/*******************************************************************************
 *
 *      genetic_process
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

#ifndef GENETICPROCESS_H_
#define GENETICPROCESS_H_

#include <vector>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include "global_flags.h"
#include <typeinfo>

/*
 * Gene
 *  ^
 *  |
 * Chromosome
 *  ^
 *  |
 * Genome (top level)
 *
 */

namespace GAUtil
{
	static double drand()
	{
		return (double) std::rand() / RAND_MAX;
	}

	// return a random number between "low" and "high"; if "nonzero" is true then this value will never be zero
	template<class _DataType>
	static _DataType rand( const _DataType & low, const _DataType & high, const bool & nonzero = false )
	{
		_DataType value = low + round( ( high - low ) * drand() );
		if ( nonzero && value == 0 )
		{
			value += ( drand() < 0.5 ) ? -1 : 1;
		}

		return value;
	}

	// return a random number within this radius
	template<class _DataType>
	static _DataType rand( _DataType radius, bool nonzero = false )
	{
		return rand( -radius, radius, nonzero );
	}

	template<class _DataType>
	struct Str
	{
		static std::string str( _DataType data )
		{
			return data;
		}
	};
}

// this class encodes the most basic unit of the genome and, most importantly, describes exactly what a mutation on that unit means
template<class _DataType>
class GeneBase
{
public:
	_DataType data_;

	GeneBase( _DataType data = _DataType() ) :
		data_( data )
	{
		//
	}

	const void create( const _DataType & data )
	{
		data_ = data;
	}

	virtual void mutate()
	{

	}

	virtual void randomize()
	{

	}

	const _DataType data() const
	{
		return data_;
	}
};

// template specialization is necessary here to define how to mutate each unit type
template<class _DataType>
class Gene : public GeneBase<_DataType>
{
public:
	Gene( _DataType data ) :
		GeneBase<_DataType> ( data )
	{
		printf( "Using generic gene...\n" );
	}

	void mutate()
	{
		printf( "Using generic gene...\n" );
	}

	void randomize()
	{
		printf( "Using generic gene...\n" );
	}
};

template<>
class Gene<bool> : public GeneBase<bool>
{
public:
	void mutate()
	{
		data_ = !data_;
	}

	void randomize()
	{
		data_ = GAUtil::drand() < 0.5;
	}
};

template<>
class Gene<int> : public GeneBase<int>
{
public:
	void mutate()
	{
		data_ += GAUtil::rand( 10, true );
	}

	void randomize()
	{
		data_ = GAUtil::rand( -100, 100 );
	}
};

template<>
class Gene<unsigned int> : public GeneBase<unsigned int>
{
public:
	void mutate()
	{
		data_ += GAUtil::rand( 0, 20 );
	}

	void randomize()
	{
		data_ = GAUtil::rand( 0, 200 );
	}
};

template<>
class Gene<float> : public GeneBase<float>
{
public:
	void mutate()
	{
		data_ += GAUtil::rand( 10, true );
	}

	void randomize()
	{
		data_ = GAUtil::rand( -100, 100 );
	}
};

template<>
class Gene<double> : public GeneBase<double>
{
public:
	void mutate()
	{
		data_ += GAUtil::rand( 10, true );
	}

	void randomize()
	{
		data_ = GAUtil::rand( -100, 100 );
	}
};

// a chromosome is a sequence of genes, where each gene is some digit (or "floating digit")
template<class _DataType, class _SizeType = unsigned int>
class Chromosome
{
public:
	typedef Gene<_DataType> _Gene;
	typedef Chromosome<_DataType, _SizeType> _Chromosome;
	typedef _Chromosome * _ChromosomePtr;

	typedef std::vector<_Gene> _GeneVector;
	typedef typename _GeneVector::iterator _GeneIterator;

	struct Descriptor
	{
	public:
		_SizeType size_;

		Descriptor( _SizeType size ) :
			size_( size )
		{
			//
		}
	};

protected:
	Descriptor descriptor_;
	_GeneVector genes_;

public:
	Chromosome( Descriptor descriptor, _GeneVector genes = _GeneVector() ) :
		descriptor_( descriptor )
	{
		if ( genes.size() > 0 ) genes_ = genes;
		else genes_.resize( descriptor_.size_ );
	}

	void mutate( double mutation_rate = 0.001 )
	{
		__DEBUG__VERBOSE__ printf( "Mutating chromosome with mutation rate %f\n", mutation_rate );
		_GeneIterator it = genes_.begin();
		for ( _SizeType i = 0; it != genes_.end(); ++it, ++i )
		{
			if ( GAUtil::drand() <= mutation_rate )
			{
				__DEBUG__VERBOSE__ printf( "--mutating gene %u\n", i );
				it->mutate();
			}
		}
	}

	void randomize()
	{
		_GeneIterator it = genes_.begin();
		for ( ; it != genes_.end(); ++it )
		{
			it->randomize();
		}
	}

	_ChromosomePtr copy()
	{
		_ChromosomePtr new_chromosome = new _Chromosome( descriptor_, _GeneVector( descriptor_.size_ ) );
		std::copy( genes_.begin(), genes_.end(), new_chromosome->genes_.begin() );
		return new_chromosome;
	}

	std::string toString()
	{
		std::stringstream ss;

		_GeneIterator it = genes_.begin();
		ss << GAUtil::Str<_DataType>::str( it->data_ );
		++it;

		for ( ; it != genes_.end(); it++ )
		{
			ss << "\\" << GAUtil::Str<_DataType>::str( it->data_ );
		}

		return ss.str();
	}

	_GeneIterator begin()
	{
		return genes_.begin();
	}

	_GeneIterator end()
	{
		return genes_.end();
	}
};

// a genome is a sequence of chromosomes
template<class _DataType, class _FitnessType = double, class _SizeType = unsigned int>
class Genome
{
	typedef Genome<_DataType, _FitnessType, _SizeType> _Genome;
	typedef _Genome * _GenomePtr;

public:
	typedef _DataType __DataType;
	typedef _FitnessType __FitnessType;
	typedef _SizeType __SizeType;

	typedef Chromosome<_DataType, _SizeType> _Chromosome;
	typedef _Chromosome * _ChromosomePtr;

	typedef typename _Chromosome::_Gene _Gene;

	typedef typename _Chromosome::_GeneVector _GeneVector;
	typedef typename _GeneVector::iterator _GeneIterator;

	typedef std::vector<_ChromosomePtr> _ChromosomeVector;
	typedef typename _ChromosomeVector::iterator _ChromosomeIterator;

	struct Descriptor
	{
	public:
		_SizeType size_;
		typename _Chromosome::Descriptor chromosome_descriptor_;

		Descriptor( _SizeType size, typename _Chromosome::Descriptor chromosome_descriptor ) :
			size_( size ), chromosome_descriptor_( chromosome_descriptor )
		{
			//
		}
	};

protected:
	_FitnessType fitness_;
	Descriptor descriptor_;
	_ChromosomeVector chromosomes_;

public:
	Genome( Descriptor descriptor, _ChromosomeVector chromosomes = _ChromosomeVector() ) :
		fitness_( 0 ), descriptor_( descriptor )
	{
		if ( chromosomes.size() > 0 ) chromosomes_ = chromosomes;
		else chromosomes_.resize( descriptor_.size_ );
	}

	virtual _FitnessType calculateFitness()
	{
		fitness_ = 0;

		/*for ( _ChromosomeIterator chromosome_it = begin(); chromosome_it != end(); ++chromosome_it )
		 {
		 _ChromosomePtr current_chromosome = *chromosome_it;
		 const _Gene & gene1 = *current_chromosome->begin();
		 const _Gene & gene2 = * ( current_chromosome->begin() + 1 );
		 _FitnessType chromosome_fitness = gene1.data() - gene2.data();
		 __DEBUG__VERBOSE__ printf( "chromosome %s fitness: %f\n", current_chromosome->toString().c_str(), chromosome_fitness );
		 fitness_ += chromosome_fitness;
		 for ( _GeneIterator gene_it = current_chromosome->begin(); gene_it != current_chromosome->end(); ++gene_it )
		 {
		 fitness_ += gene_it->data();
		 }
		 }*/
		return fitness_;
	}

public:
	_FitnessType getFitness()
	{
		return fitness_;
	}

	// performs mutation and returns the mutated genome
	void mutate( double mutation_rate = 0.001 )
	{
		__DEBUG__VERBOSE__ printf( "Mutating genome with mutation rate %f\n", mutation_rate );
		_ChromosomeIterator it = chromosomes_.begin();
		for ( ; it != chromosomes_.end(); ++it )
		{
			_ChromosomePtr current_chromosome = *it;
			current_chromosome->mutate( mutation_rate );
		}
	}

	void randomize()
	{
		_ChromosomeIterator it = chromosomes_.begin();
		for ( ; it != chromosomes_.end(); ++it )
		{
			if ( *it ) delete *it;
			_ChromosomePtr new_chromosome = new _Chromosome( descriptor_.chromosome_descriptor_ );
			new_chromosome->randomize();
			*it = new_chromosome;
		}
	}

	const _FitnessType & fitness() const
	{
		return fitness_;
	}

	virtual _GenomePtr copy( _SizeType start = 0, _SizeType copy_length = 0 )
	{
		if ( copy_length == 0 ) copy_length = chromosomes_.size();

		__DEBUG__VERBOSE__ printf( "--genome copy from chr%u to chr%u\n", start, start + copy_length );

		_GenomePtr new_genome = new _Genome( descriptor_ );
		// new_genome->chromosomes_.reserve( copy_length );

		_ChromosomeIterator old_it = begin() + start;
		_ChromosomeIterator new_it = new_genome->begin();
		for ( _SizeType i = 0; old_it != end() && i < copy_length; ++old_it, ++new_it, ++i )
		{
			_ChromosomePtr current_chromosome = *old_it;
			*new_it = current_chromosome->copy();
		}

		return new_genome;
	}

	std::string toString()
	{
		std::stringstream ss;

		_ChromosomeIterator it = chromosomes_.begin();
		_ChromosomePtr current_chromosome = *it;
		ss << "[" << fitness_ << "] ";
		ss << ( current_chromosome ? current_chromosome->toString() : "NULL_CHROMOSOME" );
		++it;
		for ( ; it != chromosomes_.end(); ++it )
		{
			current_chromosome = *it;
			ss << "|" << ( current_chromosome ? current_chromosome->toString() : "NULL_CHROMOSOME" );
		}
		return ss.str();
	}

	_ChromosomeIterator begin()
	{
		return chromosomes_.begin();
	}

	_ChromosomeIterator end()
	{
		return chromosomes_.end();
	}

	static bool compare( const _GenomePtr genome1, const _GenomePtr genome2 )
	{
		return genome1->fitness_ > genome2->fitness_;
	}
};

// contains a vector of genomes, where each genome encodes for an individual in the population
template<class _GenomeType>
class GeneticProcess
{
public:
	typedef _GenomeType _Genome;
	typedef _Genome * _GenomePtr;

	typedef typename _Genome::_GeneVector _GeneVector;
	typedef typename _Genome::_GeneIterator _GeneIterator;

	typedef typename _Genome::__DataType _DataType;
	typedef typename _Genome::__FitnessType _FitnessType;
	typedef typename _Genome::__SizeType _SizeType;

	typedef typename _Genome::_Chromosome _Chromosome;
	typedef typename _Genome::_ChromosomePtr _ChromosomePtr;
	typedef typename _Genome::_ChromosomeVector _ChromosomeVector;
	typedef typename _Genome::_ChromosomeIterator _ChromosomeIterator;

	// the population is a vector of genomes
	typedef std::vector<_GenomePtr> _PopulationVector;
	typedef typename _PopulationVector::iterator _PopulationIterator;

	typedef std::pair<_GenomePtr, _GenomePtr> _GeneticPair;

	struct Family
	{
		_GeneticPair parents_;
		_GeneticPair children_;

		Family()
		{
			//
		}

		Family( _GeneticPair parents, _GeneticPair children ) :
			parents_( parents ), children_( children )
		{
			//
		}

		Family( _GenomePtr parent1, _GenomePtr parent2, _GenomePtr child1, _GenomePtr child2 ) :
			parents_( _GeneticPair( parent1, parent2 ) ), children_( _GeneticPair( child1, child2 ) )
		{
			//
		}
	};

	struct Descriptor
	{
	public:
		_SizeType population_size_;
		double mutation_rate_;
		long random_seed_;
		typename _Genome::Descriptor genome_descriptor_;

		Descriptor( _SizeType population_size, double mutation_rate, long random_seed, typename _Genome::Descriptor genome_descriptor ) :
			population_size_( population_size ), mutation_rate_( mutation_rate ), random_seed_( random_seed ), genome_descriptor_( genome_descriptor )
		{
			std::srand( random_seed_ );
		}
	};

	typedef Family _Family;

	struct PopulationStatistics
	{
		_FitnessType total_fitness_;
		_FitnessType total_fitness_proportion_;
		_FitnessType min_fitness_;
		_FitnessType max_fitness_;
		_FitnessType avg_fitness_;

		PopulationStatistics()
		{
			//
		}
	};

	struct Flags
	{
		bool population_evaluated_;

		Flags() :
			population_evaluated_( false )
		{
			//
		}
	};

protected:
	_PopulationVector population_;
	Descriptor descriptor_;
	PopulationStatistics population_stats_;
	Flags flags_;

public:
	GeneticProcess( Descriptor descriptor, _PopulationVector population = _PopulationVector() ) :
		descriptor_( descriptor )
	{
		if ( population.size() > 0 ) population_ = population;
		else population_.resize( descriptor_.population_size_ );

	}

	virtual ~GeneticProcess()
	{
		//
	}

	_PopulationVector & population()
	{
		return population_;
	}

	void initializePopulation()
	{
		_PopulationIterator it = population_.begin();
		for ( ; it != population_.end(); ++it )
		{
			_GenomePtr new_genome = new _Genome( descriptor_.genome_descriptor_ );
			new_genome->randomize();
			*it = new_genome;
		}
	}

	void printPopulation()
	{
		_PopulationIterator it = population_.begin();
		for ( _SizeType i = 0; it != population_.end(); ++it, ++i )
		{
			_GenomePtr current_genome = *it;
			__DEBUG__QUIET__ printf( "%u: %p\n%s\n", i, current_genome, current_genome ? current_genome->toString().c_str() : "NULL_GENOME" );
		}
	}

	_FitnessType evaluateIndividual( _GenomePtr individual )
	{

		// filter input vector through individual
		// get output vector from individual;
		// calculate performance for individual
		return individual->calculateFitness();
	}

	void evaluatePopulation( bool unconditional_evaluation = false )
	{
		if ( !flags_.population_evaluated_ || unconditional_evaluation )
		{
			population_stats_.total_fitness_ = 0;
			_PopulationIterator it = population_.begin();

			_FitnessType current_fitness;

			for ( ; it != population_.end(); ++it )
			{
				current_fitness = evaluateIndividual( *it );
				if ( it == population_.begin() )
				{
					population_stats_.min_fitness_ = current_fitness;
					population_stats_.max_fitness_ = current_fitness;
				}
				if ( current_fitness < population_stats_.min_fitness_ ) population_stats_.min_fitness_ = current_fitness;
				if ( current_fitness > population_stats_.max_fitness_ ) population_stats_.max_fitness_ = current_fitness;
				population_stats_.total_fitness_ += current_fitness;
			}
			population_stats_.avg_fitness_ = population_stats_.total_fitness_ / (_FitnessType) population_.size();

			if( population_stats_.min_fitness_ == population_stats_.max_fitness_ == 0 ) population_stats_.min_fitness_ = population_stats_.max_fitness_ = 1;

			if ( population_stats_.min_fitness_ != population_stats_.max_fitness_ )
			{
				population_stats_.total_fitness_ += -1 * (_FitnessType) population_.size() * population_stats_.min_fitness_;
			}
			population_stats_.total_fitness_proportion_ = (_FitnessType) RAND_MAX / population_stats_.total_fitness_;

			flags_.population_evaluated_ = true;
		}
		else
		{
			__DEBUG__NORMAL__ printf( "--statistics already gathered:\n" );
		}
		__DEBUG__QUIET__ printf( "--population stats:\nmin: %f\nmax: %f\navg: %f\n\n", population_stats_.min_fitness_, population_stats_.max_fitness_, population_stats_.avg_fitness_ );
	}

	// assumes evaluatePopulation() has been run and the relevant statistics have been gathered
	_GenomePtr rouletteSelect()
	{
		__DEBUG__NORMAL__ printf( "--starting roulette\n" );
		// total area = RAND_MAX
		// ball position = rand()
		// we have N individuals, each with a fitness F
		// our total required space is SUM( i_N->F ) = T
		// so the area for each individual is ( RAND_MAX / T ) * i_N->F


		_FitnessType selection = (_FitnessType) std::rand();
		_FitnessType total = 0;

		_PopulationIterator it = population_.begin();
		for ( ; it != population_.end(); ++it )
		{
			_GenomePtr current_genome = *it;
			_FitnessType lower_bound = ( population_stats_.min_fitness_ == population_stats_.max_fitness_ ) ? 0 : -population_stats_.min_fitness_;
			_FitnessType slice = ( lower_bound + current_genome->fitness() ) * population_stats_.total_fitness_proportion_;
			__DEBUG__NORMAL__ printf( "total: %f\nslice: %f\n", total, slice );
			if ( total <= selection && selection < total + slice ) return current_genome;
			total += slice;
		}

		__DEBUG__QUIET__ printf( "roulette select failed!\n" );

		return NULL;
	}

	const _PopulationVector & step( _SizeType num_generations = 1 )
	{
		__DEBUG__QUIET__ printf( "\n--stepping for %u generations--\n", num_generations );
		for ( _SizeType i = 0; i < num_generations; ++i )
		{
			__DEBUG__QUIET__ printf( "--currently on generation %u--\n", i );
			std::vector<_GeneticPair> best_parents = selectBestParents( true );

			__DEBUG__NORMAL__
			{
				printf( "--selected best parents:\n" );
				for ( typename std::vector<_GeneticPair>::iterator it = best_parents.begin(); it != best_parents.end(); ++it )
				{
					printf( "{%s}\n{%s}\n-----\n", it->first->toString().c_str(), it->second->toString().c_str() );
				}
			}

			createNewGeneration( best_parents );
		}
		__DEBUG__QUIET__ printf( "--stepping complete\n\n" );
		return population_;
	}

	// 1) evaluate the population
	// 2) roulette-style selection of parents (remove parent from list of possibilities after selection)
	// 3) pair n = { parent 2n, parent 2n + 1 }
	std::vector<_GeneticPair> selectBestParents( bool pre_evaluate = false )
	{
		if ( pre_evaluate ) evaluatePopulation();

		// std::sort_heap( population_.front(), population_.back(), _Genome::compare );

		// we can only ever have an even number of parents (from which we produce an even number of children)
		// if our max population size is odd, we must create one more child than we need to fill our population requirements
		std::vector<_GeneticPair> parent_pairs;

		_SizeType num_pairs = ceil( descriptor_.population_size_ / 2 );
		parent_pairs.reserve( num_pairs );

		for ( _SizeType i = 0; i < num_pairs; ++i )
		{
			// we want parents to breed more than once but we don't want a parent to breed with itself
			_GenomePtr parent1 = rouletteSelect();
			_GenomePtr parent2 = NULL;
			do
			{
				parent2 = rouletteSelect();
			}
			while ( parent1 == parent2 );

			parent_pairs.push_back( _GeneticPair( parent1, parent2 ) );
		}

		return parent_pairs;
	}

	// performs all steps necessary to generate a new population including deleting the old one
	void createNewGeneration( std::vector<_GeneticPair> parent_pairs )
	{
		__DEBUG__VERBOSE__ printf( "--creating new generation from %zu parent pairs--\n", parent_pairs.size() );
		// for each pair, make a pair of new children via:
		// crossover
		// mutation

		// using the parent pairs, we will make all the new families we need for the new population
		std::vector<_Family> new_families;
		new_families.reserve( parent_pairs.size() );

		// make the new children
		typename std::vector<_GeneticPair>::iterator it = parent_pairs.begin();
		for ( ; it != parent_pairs.end(); ++it )
		{
			_GeneticPair current_parent_pair = *it;
			_Family current_family = crossover( current_parent_pair );
			current_family.children_.first->mutate( descriptor_.mutation_rate_ );
			current_family.children_.second->mutate( descriptor_.mutation_rate_ );

			new_families.push_back( current_family );
		}

		// delete the parents and put the children into the population

		_SizeType population_counter = 0;
		typename std::vector<_Family>::iterator new_families_it = new_families.begin();
		_PopulationIterator population_it = population_.begin();
		for ( ; new_families_it != new_families.end(); ++new_families_it )
		{
			_Family current_family = *new_families_it;

			for ( _SizeType i = 0; i < 2 && population_counter < descriptor_.population_size_; ++i, ++population_it, ++population_counter )
			{
				_GenomePtr current_genome = *population_it;

				delete current_genome;
				current_genome = ( i == 0 ) ? current_family.children_.first : current_family.children_.second;

				*population_it = current_genome;
			}
		}

		flags_.population_evaluated_ = false;
	}

	// performs crossover and returns the entire family
	// note: performs crossover only on whole chromosomes so no chromosomes are ever split
	_Family crossover( const _GeneticPair & parents )
	{
		__DEBUG__VERBOSE__ printf( "--crossing over parents:\n{%s}\n{%s}\n", parents.first->toString().c_str(), parents.second->toString().c_str() );
		_SizeType crossover_point = GAUtil::rand( (_SizeType) 0, descriptor_.genome_descriptor_.size_ - 1 );

		__DEBUG__VERBOSE__ printf( "--selected crossover point %u\n", crossover_point );

		// copy everything before the crossover point from the parents into the children
		_Family result( parents, _GeneticPair( parents.first->copy( 0, crossover_point ), parents.second->copy( 0, crossover_point ) ) );

		__DEBUG__VERBOSE__ printf( "--child1's first data chunk: %s\n", result.children_.first->toString().c_str() );
		__DEBUG__VERBOSE__ printf( "--child2's first data chunk: %s\n", result.children_.second->toString().c_str() );

		__DEBUG__VERBOSE__ printf( "--copied parents genetic data into children; beginning crossover\n" );

		std::pair<_ChromosomeIterator, _ChromosomeIterator> child_chromosome_it( result.children_.first->begin() + crossover_point, result.children_.second->begin() + crossover_point );
		std::pair<_ChromosomeIterator, _ChromosomeIterator> parent_chromosome_it( result.parents_.first->begin() + crossover_point, result.parents_.second->begin() + crossover_point );
		// std::pair<_ChromosomePtr, _ChromosomePtr> current_chromosome;

		// iterate through every remaining chromosome and swap
		for ( ; child_chromosome_it.first != result.children_.first->end(); ++child_chromosome_it.first, ++child_chromosome_it.second, ++parent_chromosome_it.first, ++parent_chromosome_it.second )
		{
			* ( child_chromosome_it.first ) = ( *parent_chromosome_it.second )->copy();
			* ( child_chromosome_it.second ) = ( *parent_chromosome_it.first )->copy();
		}

		__DEBUG__VERBOSE__ printf( "--child1's full data: %s\n", result.children_.first->toString().c_str() );
		__DEBUG__VERBOSE__ printf( "--child2's full data: %s\n", result.children_.second->toString().c_str() );

		return result;
	}

};

#endif /* GENETICPROCESS_H_ */
