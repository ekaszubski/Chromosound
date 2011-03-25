/*******************************************************************************
 *
 *      global_flags
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

#ifndef GLOBAL_FLAGS_H_
#define GLOBAL_FLAGS_H_

namespace flags
{
	namespace io
	{
		namespace debug
		{
			const static int SILENT = 0;
			const static int QUIET = 1;
			const static int NORMAL = 2;
			const static int VERBOSE = 3;

			const static int LEVEL = QUIET;
		}
	}
}

#ifndef __DEBUG__LEVELS__
#define __DEBUG__LEVELS__

#define __DEBUG__SILENT__  if( flags::io::debug::LEVEL >= flags::io::debug::SILENT )
#define __DEBUG__QUIET__  if( flags::io::debug::LEVEL >= flags::io::debug::QUIET )
#define __DEBUG__NORMAL__  if( flags::io::debug::LEVEL >= flags::io::debug::NORMAL )
#define __DEBUG__VERBOSE__  if( flags::io::debug::LEVEL >= flags::io::debug::VERBOSE )

#endif

#endif /* GLOBAL_FLAGS_H_ */
