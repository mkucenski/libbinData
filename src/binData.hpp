// Copyright 2007 Matthew A. Kucenski
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BINDATA_HPP_
#define BINDATA_HPP_

/*	
 * This class is defined as the base class of binDataFile so that things like binDataStream can later be created
 * and classes like binDataSearcher can work with them all.
 */

#include <sys/types.h>

class binData {
	public:
		binData() {};
		virtual ~binData() {};
		
		virtual int getData(void* pData, u_int64_t ulSize, u_int64_t* p_ulSizeRead) = 0;								//Read ulSize bytes of data
		virtual int getData(void* pData, u_int64_t ulSize, u_int64_t ulOffset, u_int64_t* p_ulSizeRead) = 0;	//Read ulSize bytes of data from offset ulOffset

		virtual int skipNullBlocks(u_int8_t cByteWidth, u_int64_t* p_posNew) = 0;										// From the current position in the file, skip over
																																				// cByteWidth blocks of NULL data. (Useful for sparse
																																				// files.)
};

#endif /*BINDATA_HPP_*/
