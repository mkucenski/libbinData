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

// #define _DEBUG_ 1

#include "binDataFile.h"

#include "misc/endianSwitch.h"
#include "misc/debugMsgs.h"

binDataFile::binDataFile() {
}

binDataFile::binDataFile(string filename) {
	int rv = open(filename);
	DEBUG_INFO("binDataFile::binDataFile() open(" << filename << ") returned: " << rv);
}

binDataFile::~binDataFile() {
	close();
}

int binDataFile::close() {
	int rv = -1;
	
	if (m_filestream.is_open() == true) {
		m_filestream.close();
		if (!m_filestream.fail()) {
			rv = 0; 
		} else {
			DEBUG_ERROR("binDataFile::close() Failure closing file.");
		}
	} else {
		DEBUG_WARNING("binDataFile::close() File not open.");
	}
	
	return rv;
}
		
int binDataFile::open(string filename) {
	int rv = -1;
	
	if (m_filestream.is_open() == false) {
		m_filestream.open(filename.c_str(), ifstream::binary);
		if (!m_filestream.fail()) {
			rv = 0;
		} else {
			DEBUG_ERROR("binDataFile::open() Failure opening file " << filename << ".");
		}
	} else {
		DEBUG_ERROR("binDataFile::open() File " << filename << " already open.");
	}

	return rv;
}

bool binDataFile::isOpen() {
	bool rv = false;
	
	if (m_filestream.is_open() == true) {
		rv = true;
	}
	
	return rv;
}

bool binDataFile::isEOF() {
	return m_filestream.eof();
}

int binDataFile::seek(u_int32_t ulOffset) {
	int rv = -1;
	
	// DEBUG_INFO("binDataFile::seek() Moving to offset " << ulOffset);
	// m_filestream.seekg(ulOffset, ios_base::beg);
	if (isEOF()) {
		DEBUG_INFO("binDataFile::seek() Previously reached EOF, need to clear in order to allow seek function.");
		m_filestream.clear();
	}
	m_filestream.seekg(ulOffset);
	if (!m_filestream.eof() && !m_filestream.fail()) {
		rv = 0;
	} else {
		DEBUG_ERROR("binDataFile::seek() Unable to position file pointer to offset: " << ulOffset);
	}
	
	return rv;
}

u_int32_t binDataFile::offset() {
	return m_filestream.tellg();
}

int binDataFile::getData(void* pData, u_int32_t ulSize, u_int32_t* p_ulSizeRead) {
	int rv = -1;

	if (p_ulSizeRead) { 
		*p_ulSizeRead = 0; 
	}
	
	if (ulSize > 0) {
		if (pData) {
			u_int32_t pos = offset();
			if (pos == 34502832) DEBUG_INFO("binDataFile::getData() Reading " << ulSize << " bytes from offset " << pos);

			m_filestream.read((char*)pData, ulSize);
			u_int32_t ulSizeRead = m_filestream.gcount();
			if (m_filestream.fail() && !m_filestream.eof()) { // Failure reading and NOT end-of-file -> something went wrong.
				DEBUG_ERROR("binDataFile::getData() Unable to read data from offset " << pos);
			} else { // Otherwise, either not a fail or a fail AND eof
				if (ulSizeRead > 0) { // If we read any data, still ok.
					if (ulSizeRead != ulSize) {
						DEBUG_WARNING("binDataFile::getData() Only read " << ulSizeRead << " out of " << ulSize << " requested from offset " << pos << " (error state = " << m_filestream.rdstate() << ", eof=" << m_filestream.eof() << ", fail=" << m_filestream.fail() << ")");
					}
					if (p_ulSizeRead) {
						*p_ulSizeRead = ulSizeRead;
					}
					rv = 0;
				} else { // If we read zero data, not ok
					DEBUG_ERROR("binDataFile::getData() Read zero bytes.");
				}
			}
		} else {
			DEBUG_INFO("binDataFile::getDat() No destination pointer given, moving ahead " << ulSize << " bytes");
			m_filestream.seekg(ulSize, ios_base::cur);
			if (!m_filestream.eof() && !m_filestream.fail()) {
				rv = 0;
			} else {
				DEBUG_ERROR("binDataFile::getData() Unable to move pointer ahead " << ulSize << " bytes from offset " << offset());
			}
		}
	} else {
		DEBUG_ERROR("binDataFile::getData() Requested data size <= 0");
	}		
	
	return rv;
}

int binDataFile::getData(void* pData, u_int32_t ulSize, u_int32_t ulOffset, u_int32_t* p_ulSizeRead) {
	int rv = -1;
	
	if (ulSize > 0) {
		if (seek(ulOffset) >= 0) {
			rv = getData(pData, ulSize, p_ulSizeRead);
		} else {
			DEBUG_ERROR("binDataFile::getData(offset) Unable to position file pointer to offset: " << ulOffset);
		}
	} else {
		DEBUG_ERROR("binDataFile::getData(offset) Requested data size <= 0.");
	}		

	return rv;
}

int binDataFile::skipNullBlocks(u_int8_t cByteWidth, u_int32_t* p_posNew) {
	int rv = -1;

	if (cByteWidth == 1 || (cByteWidth % 2) == 0) {

		int cBufSize = 4096;
		u_int8_t rgBuffer[cBufSize];
		u_int32_t cGetCount = 0;
		u_int32_t posFileCur = 0;

		u_int16_t rposChunk = 0;
		bool fFoundData = false;
		while (!fFoundData) {
			posFileCur = offset();
			DEBUG_INFO("binDataFile::skipNullBlocks() Reading chunk from position:" << posFileCur);
			if (getData(&rgBuffer, cBufSize, &cGetCount) >= 0) {
				if (posFileCur == 34502832) DEBUG_INFO("binDataFile::skipNullBlocks() getData returned:" << cGetCount << " bytes.");
				// for (int k=0; k<cBufSize; k++) {
				// 	DEBUG_INFO("binDataFile::skipNullBlocks() rgBuffer[" << k << "]=" << (char)rgBuffer[k]);
				// }
				for (int i=0; i<cGetCount/cByteWidth; i++) {
					if (posFileCur == 34502832) DEBUG_INFO("binDataFile::skipNullBlocks() Working with segment i=" << i);
					u_int64_t vBlock = 0;
					rposChunk = i * cByteWidth;
		
					for (int j=0; j<cByteWidth; j++) {
						if (posFileCur == 34502832) DEBUG_INFO("binDataFile::skipNullBlocks() Working with byte j=" << j << " (posFileCur=" << posFileCur << ", rposChunk=" << rposChunk << ", byte=" << (u_int8_t)rgBuffer[rposChunk+j] << ")");
						vBlock += (u_int64_t)rgBuffer[rposChunk + j] << j * 8;
						if (posFileCur == 34502832) DEBUG_INFO("binDataFile::skipNullBlocks() vBlock=" << vBlock);
					}

					if (vBlock != 0) {
						if (posFileCur == 34502832) DEBUG_INFO("binDataFile::skipNullBlocks() Found non-null @ position=" << posFileCur + rposChunk);
						fFoundData = true;
						if (seek(posFileCur + rposChunk) >= 0) {
							rv = 0;
						}
						if (p_posNew) {
							*p_posNew = posFileCur + rposChunk;
						}
						break;
					}
				}
			} else {
				DEBUG_INFO("binDataFile::skipNullBlocks() Unable to read data, eof?");
				break;
			}
		}
	} else {
		DEBUG_INFO("binDataFile::skipNullBlocks() Invalid cByteWidth!");
	}

	return rv;
}

/*int binDataFile::getData(vector<char>* pData, u_int32_t ulSize, u_int32_t* p_ulReadSize) {
	int rv = -1;
	
	if (ulSize > 0) {
		if (pData) {
			pData->clear();
	
			rv = 0;
			char tmp;
			u_int32_t ulReadSize = 0;
			while (ulReadSize < ulSize) {
				if (getData(&tmp, 1) >= 0) {
					ulReadSize += 1;
					pData->push_back(tmp);
				} else {
					DEBUG_WARNING("binDataFile::getData(<vector>) Failure reading value. Read " << ulReadSize << " out of " << ulSize << " requested bytes.");
					break;
				}
			}
			if (p_ulReadSize) {
				*p_ulReadSize = ulReadSize;
			}
		} else {
			DEBUG_ERROR("binDataFile::getData(<vector>) Invalid destination pointer.");
		}
	} else {
		DEBUG_ERROR("binDataFile::getData(<vector>) Requested data size <= 0");
	}
		
	return rv;
}

int binDataFile::getData(vector<char>* pData, u_int32_t ulSize, u_int32_t ulOffset, u_int32_t* p_ulReadSize) {
	int rv = -1;
	
	if (ulSize > 0) {
		if (pData) {
			if (seek(ulOffset) >= 0) {
				rv = getData(pData, ulSize, p_ulReadSize);
			} else {
				DEBUG_ERROR("binDataFile::getString(<vector> offset) Error on seek to offset " << ulOffset);
			}
		} else {
			DEBUG_ERROR("binDataFile::getString(<vector> offset) Invalid destination pointer.");
		}
	} else {
		DEBUG_ERROR("binDataFile::getData(<vector> offset) Requested data size <= 0.");
	}		

	return rv;
}*/

int binDataFile::getString(string* pString, u_int32_t ulLength) {
	int rv = -1;
	
	if (pString) {
		pString->clear();

		rv = 0;
		char tmp;
		u_int32_t ulReadLength = 0;
		while (ulLength == 0 || ulReadLength < ulLength) {
			if (getData(&tmp, 1, NULL) >= 0) {
				if (ulLength == 0 && tmp == '\0') {
					break;
				}
				ulReadLength += 1;
				pString->push_back(tmp);
			} else {
				DEBUG_ERROR("binDataFile::getString() Failure reading character.");
				rv = -1;
				break;
			}
		}
	} else {
		DEBUG_ERROR("binDataFile::getString() Invalid destination pointer.");
	}
	
	return rv;
}

int binDataFile::getString(string* pString, u_int32_t ulOffset, u_int32_t ulLength) {
	int rv = -1;

	if (pString) {
		if (seek(ulOffset) >= 0) {
			rv = getString(pString, ulLength);
		} else {
			DEBUG_ERROR("binDataFile::getString(offset) Error on seek to offset " << ulOffset);
		}
	} else {
		DEBUG_ERROR("binDataFile::getString(offset) Invalid destination pointer.");
	}

	return rv;
}

int binDataFile::getTwoByteCharString(string* pString, u_int32_t ulLength, bool bBigEndian) {
	int rv = -1;
	
	if (pString) {
		pString->clear();
		
		rv = 0;
		u_int16_t tmp;
		u_int32_t ulReadLength = 0;
		while (ulLength == 0 || ulReadLength < ulLength) {
			if (getData(&tmp, 2, NULL) >= 0) {
				(bBigEndian ? BIGTOHOST16(tmp) : LITTLETOHOST16(tmp));
				if (ulLength == 0 && tmp == L'\0') {
					break;
				}
				ulReadLength += 1;
				pString->push_back(tmp);
			} else {
				DEBUG_ERROR("binDataFile::getTwoByteCharString() Failure reading character.");
				rv = -1;
				break;
			}
		}
	} else {
		DEBUG_ERROR("binDataFile::getTwoByteCharString() Invalid destination pointer.");
	}
	
	return rv;
}

int binDataFile::getTwoByteCharString(string* pString, u_int32_t ulOffset, u_int32_t ulLength, bool bBigEndian) {
	int rv = -1;
	if (pString) {
		if (seek(ulOffset) >= 0) {
				rv = getTwoByteCharString(pString, ulLength, bBigEndian);
		} else {
			DEBUG_ERROR("binDataFile::getTwoByteCharString(offset) Error on seek to offset " << ulOffset);
		}
	} else {
		DEBUG_ERROR("binDataFile::getTwoByteCharString(offset) Invalid destination pointer.");
	}
	return rv;
}
