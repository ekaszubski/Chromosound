/*******************************************************************************
 *
 *      audio_genome
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

#ifndef AUDIO_GENEOME_H_
#define AUDIO_GENEOME_H_

#include "genetic_process.h"
#include <AL/alut.h>

namespace AudioGenomeDefs
{

	typedef double _FitnessType;
	typedef unsigned int _SizeType;

	template<class _KeyDataType>
	static float getFrequency( const _KeyDataType & key )
	{
		return key <= 0 ? 1.0 : 440 * pow( 2, ( (float) key - 49 ) / 12 );
	}

	static float getDurationFromCycles( const _SizeType & num_cycles, const _SizeType & cycles_per_second = 16 )
	{
		return (float) num_cycles / cycles_per_second;
	}

	// 0 -> 1/16; 4 -> 1/1
	template<class _DataType>
	static float getDurationFromIndex( _DataType duration_index )
	{
		return 1 / pow( 2, 4 - duration_index );
	}

	template<class _DataType>
	static float getDurationFromCycles( _DataType num_cycles )
	{
		return (float) num_cycles / 16;
	}

	struct AudioGeneEncodings
	{
	public:
		typedef unsigned int _Storage;
		// bool - toggle
		const static _Storage commit = 0;
		const static _Storage key_rotate = 1;
		const static _Storage key_flip = 2;

		// int - increment by value
		const static _Storage duration = 3;
		const static _Storage time = 4;
		const static _Storage pitch = 5;
	};

	struct AudioStateControl
	{
		typedef int _Storage;

		AudioStateControl()
		{
			//
		}
		// which aspect of the current waveform state to modify
		// [pitch+/-, duration+/-, pause|, time*, key_flip|, key_rotate+/-, separator|]
		// each bit builds upon the changes made by the previous bit
		_Storage toggle_type;
		// the value for this modification
		_Storage toggle_value;
	};

	typedef AudioStateControl _DataType;

	typedef Genome<_DataType, _FitnessType, _SizeType> _GenomeBase;

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
}

namespace GeneticProcessUtil
{

	template<>
	// static
	std::string geneToString<AudioGenomeDefs::_DataType> ( AudioGenomeDefs::_DataType data )
	{
		std::stringstream ss;
		ss << "(" << data.toggle_type << ":" << data.toggle_value << ")";// << ":" << data.beat_index << ")";
		return ss.str();
	}

}

namespace AudioGenomeDefs
{
	struct WaveFSM
	{
		// if our note timed out, we can play the previous note and start counting the silence time
		// if we found a commit bit and our note didn't time out, we can play the previous note for the number of cycles specified by the counter
		// if we found a commit bit and our note did time out, we can start playing the silence wave for the number of cycles specified by the counter
		// either way, if we found a commit bit, we can start counting the number of cycles to play the current note


		typedef WaveDescriptor _WaveDescriptor;
		typedef WaveState _WaveState;
		typedef AudioGeneEncodings _AudioGeneEncodings;

		unsigned int timer_counter;
		unsigned int timer_max;
		bool timer_enabled;
		bool note_interrupted;
		unsigned int last_note_duration;
		bool counting_silence;
		bool has_previous_state;

		_WaveState state, last_state;

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
		int update( _DataType control_gene )
		{
			//__DEBUG__NORMAL__ printf( "%u: Updating %s\n", timer_counter, GeneticProcessUtil::Str<_DataType>::str( control_gene ).c_str() );

			int result = 0;
			bool do_commit = false;
			switch ( control_gene.toggle_type )
			{
			//bool - toggle if( value )
			case _AudioGeneEncodings::commit:
				do_commit = true; //control_gene.toggle_value;
				//if ( do_commit ) __DEBUG__NORMAL__ printf( "Saving current state:\n%s\n", state.toString().c_str() );
				// save our current state for reference; start making changes to our new state
				last_state = state;
				has_previous_state = true;
				break;
			case _AudioGeneEncodings::key_rotate:
				state.key_index += control_gene.toggle_value ? 1 : -1;
				GeneticProcessUtil::range( state.key_index, (int) 0, (int) 12, true );
				//__DEBUG__NORMAL__ printf( "Updated key_index: %u\n", state.key_index );
				break;
			case _AudioGeneEncodings::key_flip:
				//if ( control_gene.toggle_value ) state.key_is_major = !state.key_is_major;
				//__DEBUG__NORMAL__ printf( "Updated key_is_major: %u\n", state.key_is_major );
				break;

				// int - increment by value
			case _AudioGeneEncodings::duration:
				state.duration_index += control_gene.toggle_value;
				GeneticProcessUtil::range( state.duration_index, (int) 0, (int) 4 );
				//__DEBUG__NORMAL__ printf( "Updated duration_index: %u\n", state.duration_index );
				break;
			case _AudioGeneEncodings::time:
				state.beat_frequency_index += control_gene.toggle_value;
				GeneticProcessUtil::range( state.beat_frequency_index, (int) 0, (int) 3 );
				//__DEBUG__NORMAL__ printf( "Updated beat_frequency_index: %u\n", state.beat_frequency_index );
				break;
			case _AudioGeneEncodings::pitch:
				state.pitch_index += control_gene.toggle_value;
				GeneticProcessUtil::range( state.pitch_index, (int) 21, (int) 68 );
				//__DEBUG__NORMAL__ printf( "Updated pitch_index: %u\n", state.pitch_index );
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
					//__DEBUG__NORMAL__ printf( "We should play silence for %u cycles\n", last_note_duration );
				}
				// we interrupted playback of the previous note; our counter shows how many cycles we should play it for
				else
				{
					result = 1;
					last_note_duration = timer_counter;
					//__DEBUG__NORMAL__ printf( "Note interrupted. We should play sound for %u cycles\n", last_note_duration );
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
				//__DEBUG__NORMAL__ printf( "Note finished playing normally. We should play sound for %u cycles\n", last_note_duration );
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
			//__DEBUG__NORMAL__ printf( "Starting timer for %u cycles\n", timer_value );
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
}

// required for randomization and mutation of a gene based on the AudioNote data type
template<>
class Gene<AudioGenomeDefs::_DataType> : public GeneBase<AudioGenomeDefs::_DataType>
{
public:
	typedef AudioGenomeDefs::AudioGeneEncodings _AudioGeneEncodings;
	void mutate()
	{
		data_.toggle_type += GeneticProcessUtil::rand( 1 );
		// AudioGenomeDefs::range( data_.toggle_type, 0, 5, true );
		switch ( data_.toggle_type )
		{
		//bool - toggle if( value )
		case _AudioGeneEncodings::commit:
			data_.toggle_value = !data_.toggle_value;
			break;
		case _AudioGeneEncodings::key_rotate:
			data_.toggle_value = !data_.toggle_value;
			break;
		case _AudioGeneEncodings::key_flip:
			data_.toggle_value = !data_.toggle_value;
			break;

			// int - increment by value
		case _AudioGeneEncodings::duration:
			data_.toggle_value = GeneticProcessUtil::rand( 2, true );
			//range( data_.toggle_value, (_AudioStateControlStorage) 0, (_AudioStateControlStorage) 4 );
			break;
		case _AudioGeneEncodings::time:
			data_.toggle_value = GeneticProcessUtil::rand( 2, true );
			//range( data_.toggle_value, (_AudioStateControlStorage) 0, (_AudioStateControlStorage) 3 );
			break;
		case _AudioGeneEncodings::pitch:
			data_.toggle_value = GeneticProcessUtil::rand( 20, true );
			//range( data_.toggle_value, (_AudioStateControlStorage) 21, (_AudioStateControlStorage) 68 );
			break;
		}
	}

	void randomize()
	{
		data_.toggle_type = GeneticProcessUtil::rand( 0, 5 );
		switch ( data_.toggle_type )
		{
		//bool - toggle if( value )
		case _AudioGeneEncodings::key_rotate:
			data_.toggle_value = GeneticProcessUtil::rand( 0, 1 );
			break;
		case _AudioGeneEncodings::commit:
			data_.toggle_value = GeneticProcessUtil::rand( 0, 1 );
			break;
		case _AudioGeneEncodings::key_flip:
			data_.toggle_value = GeneticProcessUtil::rand( 0, 1 );
			break;

			// int - increment by value
		case _AudioGeneEncodings::duration:
			data_.toggle_value = GeneticProcessUtil::rand( 2 );
			break;
		case _AudioGeneEncodings::time:
			data_.toggle_value = GeneticProcessUtil::rand( 2 );
			break;
		case _AudioGeneEncodings::pitch:
			data_.toggle_value = GeneticProcessUtil::rand( 20 );
			break;
		}
	}
};

// required to calculate the fitness function for the AudioGenome
class AudioGenome : public AudioGenomeDefs::_GenomeBase
{
public:
	typedef __SizeType _SizeType;
	typedef __FitnessType _FitnessType;
	typedef __DataType _DataType;
	typedef AudioGenomeDefs::_GenomeBase _GenomeBase;
	typedef AudioGenomeDefs::WaveDescriptor _WaveDescriptor;
	typedef AudioGenomeDefs::WaveState _WaveState;
	typedef AudioGenomeDefs::AudioGeneEncodings _AudioGeneEncodings;

	std::vector<_WaveDescriptor> wave_descriptors;
	typedef typename std::vector<_WaveDescriptor>::iterator _WaveDescriptorIterator;

	// we want to use the same WaveFSM object for all audio genes so that the song is continuous.
	static AudioGenomeDefs::WaveFSM wave_fsm;
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
		fitness_ = 1;

		for ( _ChromosomeIterator chromosome_it = begin(); chromosome_it != end(); ++chromosome_it )
		{
			_ChromosomePtr current_chromosome = *chromosome_it;
			// every gene advances our "clock" 1/16 of a beat
			// the wave state is updated first
			// if we've reached a commit bit, the buffer for the wave is generated and queued
			// if our note duration has expired, we go silent (queue silent wave buffer)
			for ( _GeneIterator gene_it = current_chromosome->begin(); gene_it != current_chromosome->end(); ++gene_it )
			{
				_GenePtr current_gene = *gene_it;
				int type = wave_fsm.update( current_gene->data_ );

				if ( type != 0 )
				{
					const float duration = AudioGenomeDefs::getDurationFromCycles( wave_fsm.last_note_duration );
					const float frequency = type == 1 ? AudioGenomeDefs::getFrequency( wave_fsm.last_state.pitch_index ) : type == 2 ? 8 : 0;

					wave_descriptors.push_back( _WaveDescriptor( type, wave_fsm.last_state.shape, frequency, wave_fsm.last_state.phase, duration ) );

					// we want as many beats (and therefore as little silence) as possible
					// so give a positive reward for non-silent notes
					// and a negative reward for silent notes
					if ( type == 1 ) fitness_ += wave_fsm.last_note_duration;
					if ( type == 2 ) fitness_ -= wave_fsm.last_note_duration;
				}
			}
		}

		// printf( "wave descriptors new size: %zu\n", wave_descriptors.size() );

		return fitness_;
	}

	AudioGenome * copy( _SizeType start = 0, _SizeType copy_length = 0 )
	{
		// make a full copy of this genome's chromosomes using the base class's copy function
		AudioGenome * new_genome = new AudioGenome( descriptor_, _GenomeBase::copy( start, copy_length )->chromosomes() );
		return new_genome;
	}
};

#endif /* AUDIO_GENOME_H_ */
