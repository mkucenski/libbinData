#ifndef BINDATAFILE_H_
#define BINDATAFILE_H_

#include "binData.hpp"
#include "misc/stringType.h"

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
		
		int seek(u_int32_t ulOffset);																											//Move the pointer to a file offset
		u_int32_t offset();																														//Return the current offset
		
		int getData(void* pData, u_int32_t ulSize, u_int32_t* p_ulSizeRead);														//Read ulSize bytes of data
		int getData(void* pData, u_int32_t ulSize, u_int32_t ulOffset, u_int32_t* p_ulSizeRead);							//Read ulSize bytes of data from offset ulOffset
		
		//int getData(vector<char>* pData, u_int32_t ulSize, u_int32_t* p_ulReadSize = NULL);								//Read up to ulSize bytes of data and report actual bytes read into p_ulReadSize
		//int getData(vector<char>* pData, u_int32_t ulSize, u_int32_t ulOffset, u_int32_t* p_ulReadSize = NULL);	//Read up to ulSize bytes of data from offset ulOffset and report actual bytes read into p_ulReadSize
		
		int getString(string* pString, u_int32_t ulLength);																			//Read ulLength characters into pString
		int getString(string* pString, u_int32_t ulOffset, u_int32_t ulLength);													//Read ulLength characters from offset ulOffset into pString

		int getTwoByteCharString(string_t* pString, u_int32_t ulLength, bool bBigEndian);
		int getTwoByteCharString(string_t* pString, u_int32_t ulOffset, u_int32_t ulLength, bool bBigEndian);
				
	private:
		ifstream m_filestream;
};

#endif /*BINDATAFILE_H_*/
