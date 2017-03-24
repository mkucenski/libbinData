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

#ifndef BINDATAFILE_H_
#define BINDATAFILE_H_

#include "binData.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <sys/types.h>

using namespace std;

class binDataFile : public binData {
	public:
		binDataFile();
		binDataFile(string filename);
		~binDataFile();
		
		int open(string filename);
		int close();
		bool isOpen();
		bool isEOF();
		
		int seek(u_int32_t ulOffset);																					//Move the pointer to a file offset
		u_int32_t offset();																								//Return the current offset
		
		int getData(void* pData, u_int32_t ulSize, u_int32_t* p_ulSizeRead);								//Read ulSize bytes of data
		int getData(void* pData, u_int32_t ulSize, u_int32_t ulOffset, u_int32_t* p_ulSizeRead);	//Read ulSize bytes of data from offset ulOffset
		
		int skipNullBlocks(u_int8_t cByteWidth, u_int32_t* p_posNew);										// From the current position in the file, skip over
																																// cByteWidth blocks of NULL data. (Useful for sparse
																																// files.)

		//int getData(vector<char>* pData, u_int32_t ulSize, u_int32_t* p_ulReadSize = NULL);		//Read up to ulSize bytes of data and report actual bytes read into p_ulReadSize
		//int getData(vector<char>* pData, u_int32_t ulSize, u_int32_t ulOffset, u_int32_t* p_ulReadSize = NULL);	//Read up to ulSize bytes of data from offset ulOffset and report actual bytes read into p_ulReadSize
		
		int getString(string* pString, u_int32_t ulLength);													//Read ulLength characters into pString
		int getString(string* pString, u_int32_t ulOffset, u_int32_t ulLength);							//Read ulLength characters from offset ulOffset into pString

		int getTwoByteCharString(string* pString, u_int32_t ulLength, bool bBigEndian=false);
		int getTwoByteCharString(string* pString, u_int32_t ulOffset, u_int32_t ulLength, bool bBigEndian=false);
				
	private:
		ifstream m_filestream;
};

#endif /*BINDATAFILE_H_*/
