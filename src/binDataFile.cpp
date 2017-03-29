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
	if (rv < 0) {
		DEBUG_ERROR("binDataFile::binDataFile() open(" << filename << ") returned: " << rv);
	}
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

u_int64_t binDataFile::currPos() {
	return m_filestream.tellg();
}

int binDataFile::getData(void* pData, u_int64_t ulSize, u_int64_t* p_ulSizeRead) {
	int rv = -1;

	if (p_ulSizeRead) { 
		*p_ulSizeRead = 0; 
	}
	
	if (ulSize > 0 && pData) {
		u_int64_t pos = currPos();
		m_filestream.read((char*)pData, ulSize);
		u_int64_t ulSizeRead = m_filestream.gcount();

		// TODO	Need to setup return values that mean something so that calls to this function know what happened.
		//
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
		DEBUG_ERROR("binDataFile::getData() Invalid function parameters: ulSize=" << ulSize << ", pData=" << pData);
	}		
	
	return rv;
}

int binDataFile::movePos(int64_t pos, bool fRelative) {
	int rv = -1;

	int64_t posFileCur = currPos();

	if (m_filestream.eof()) {
		DEBUG_INFO("binDataFile::movePos() Previously reached EOF, need to clear first.");
		m_filestream.clear();
	}

	// TODO	Does this work moving forward and reverse?
	//
	if (fRelative) {
		m_filestream.seekg(pos, ios_base::cur);
	} else {
		m_filestream.seekg(pos);
	}

	if (!m_filestream.eof() && !m_filestream.fail()) {
		rv = 0;
	} else {
		DEBUG_ERROR("binDataFile::movePos() Unable to position file pointer. (cur=" << posFileCur << ", pos=" << pos << ", fRelative=" << fRelative << ")");
	}

	return rv;
}

int binDataFile::getData(void* pData, u_int64_t ulSize, u_int64_t ulOffset, u_int64_t* p_ulSizeRead) {
	int rv = -1;
	
	if (ulSize > 0 && pData) {
		if (movePos(ulOffset) >= 0) {
			rv = getData(pData, ulSize, p_ulSizeRead);
		} else {
			DEBUG_ERROR("binDataFile::getData(offset) Unable to position file pointer to offset: " << ulOffset);
		}
	} else {
		DEBUG_ERROR("binDataFile::getData(offset) Invalid function parameters: ulSize=" << ulSize << ", pData=" << pData);
	}		

	return rv;
}

int binDataFile::skipNullBlocks(u_int8_t cByteWidth, u_int64_t* p_posNew) {
	int rv = -1;

	if (cByteWidth == 1 || (cByteWidth % 2) == 0) {

		int cBufSize = 4096;
		u_int8_t rgBuffer[cBufSize];
		u_int64_t cGetCount = 0;
		u_int64_t posFileCur = 0;

		u_int16_t rposChunk = 0;
		bool fNonZeroFound = false;
		while (!fNonZeroFound) {
			posFileCur = currPos();
			memset(&rgBuffer, 0, cBufSize);

			if (getData(&rgBuffer, cBufSize, &cGetCount) >= 0) {
				for (int i=0; i<cGetCount/cByteWidth; i++) { 						// Loop through cByteWidth chunks
					rposChunk = i * cByteWidth; 											// Relative position of the chunk within the larger buffer

					for (int j=0; j<cByteWidth; j++) {									// Loop through individual bytes in the chunk looking for non-zero values
						if (rgBuffer[rposChunk + j] != 0) {
							fNonZeroFound = true;
							break;
						}
					}

					if (fNonZeroFound) {
						if (movePos(posFileCur + rposChunk) >= 0) {
							if (p_posNew) {
								*p_posNew = posFileCur + rposChunk;
							}
							rv = 0;
						} else {
							DEBUG_ERROR("binDataFile::skipNullBlocks() Failure moving to non-null position: " << posFileCur + rposChunk);
						}
						break;
					}
				}
			} else {
				// TODO Need to better clarify what's happening when getData fails.
				//
				DEBUG_WARNING("binDataFile::skipNullBlocks() Unable to read data, eof?");
				break;
			}
		}
	} else {
		DEBUG_ERROR("binDataFile::skipNullBlocks() Invalid cByteWidth!");
	}

	return rv;
}

int binDataFile::getString(string* pString, u_int64_t ulLength) {
	int rv = -1;

	// TODO	This is not very clean/efficient--can I use the ifstream::get function to read null-terminated strings.
	// 		Should this function name be more descriptive in that it is reading null-terminated strings?
	//
	if (pString) {
		pString->clear();

		rv = 0;
		char tmp;
		u_int64_t ulReadLength = 0;
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

int binDataFile::getString(string* pString, u_int64_t ulOffset, u_int64_t ulLength) {
	int rv = -1;

	if (pString) {
		if (movePos(ulOffset) >= 0) {
			rv = getString(pString, ulLength);
		} else {
			DEBUG_ERROR("binDataFile::getString(offset) Error on moving to offset " << ulOffset);
		}
	} else {
		DEBUG_ERROR("binDataFile::getString(offset) Invalid destination pointer.");
	}

	return rv;
}

int binDataFile::getTwoByteCharString(string* pString, u_int64_t ulLength, bool bBigEndian) {
	int rv = -1;
	
	// TODO	This is a bit of a hack to pseudo-support UNICODE (two-byte) strings.
	// 		Is there a better, cleaner, and more robust way to do this?
	//
	if (pString) {
		pString->clear();
		
		rv = 0;
		u_int16_t tmp;
		u_int64_t ulReadLength = 0;
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

int binDataFile::getTwoByteCharString(string* pString, u_int64_t ulOffset, u_int64_t ulLength, bool bBigEndian) {
	int rv = -1;
	if (pString) {
		if (movePos(ulOffset) >= 0) {
				rv = getTwoByteCharString(pString, ulLength, bBigEndian);
		} else {
			DEBUG_ERROR("binDataFile::getTwoByteCharString(offset) Error on moving to offset " << ulOffset);
		}
	} else {
		DEBUG_ERROR("binDataFile::getTwoByteCharString(offset) Invalid destination pointer.");
	}
	return rv;
}
