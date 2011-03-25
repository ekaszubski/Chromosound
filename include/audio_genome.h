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
namespace AudioGenomDefs
{
	typedef double _FitnessType;
	typedef unsigned int _SizeType;

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

	namespace AudioBitEncodings
	{
		typedef unsigned int _Storage;
		// bool - toggle
		const static _Storage commit = 0;
		const static _Storage key_rotate = 1;
		const static _Storage key_flip = 2;

		// int - increment by value
		const static _Storage duration = 3;
		const static _Storage time = 4;
		const static _Storage pitch = 5;
	}

	struct AudioStateControl
	{
		typedef int _Storage;
		// which aspect of the current waveform state to modify
		// [pitch+/-, duration+/-, pause|, time*, key_flip|, key_rotate+/-, separator|]
		// each bit builds upon the changes made by the previous bit
		_Storage toggle_type;
		// the value for this modification
		_Storage toggle_value;
	};

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

	// required to calculate the fitness function for the AudioGenome
	class AudioGenome : public _GenomeBase
	{
	public:
		std::vector<WaveDescriptor> wave_descriptors;
		typedef typename std::vector<WaveDescriptor>::iterator _WaveDescriptorIterator;

		// we want to use the same WaveFSM object for all audio genes so that the song is continuous.
		static WaveFSM wave_fsm;
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

}

#endif /* AUDIO_GENOME_H_ */
