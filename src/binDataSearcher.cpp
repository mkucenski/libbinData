// Copyright 2016 Matthew A. Kucenski
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

#include "binDataSearcher.hpp"

#include "misc/debugMsgs.h"

binDataSearcher::binDataSearcher(	vector<u_int8_t>& vIdentifiers, 
												u_int32_t uiIdentifierLeadIn,
												vector<u_int8_t>& vTerminators,
												u_int32_t uiLength,
												u_int32_t uiLengthValOffset,
												endianness_t endianness,
												endiansize_t endiansize) :	m_vIdentifiers(vIdentifiers),
																					m_uiIdentifierLeadIn(uiIdentifierLeadIn),
																					m_vTerminators(vTerminators),
																					m_uiLength(uiLength),
																					m_uiLengthValOffset(uiLengthValOffset),
																					m_endianness(endianness),
																					m_endiansize(endiansize) {
}

binDataSearcher::~binDataSearcher() {
} 

int binDataSearcher::load(binData* pData) {
	int rv = -1;
	
	if (pData) {
		m_pBinData = pData;
		rv = 0;
	}
	
	return rv;
}

int binDataSearcher::getNext(void** ppData) {
	int rv = -1;
	
	if (ppData && *ppData == NULL) {
		//TODO 	Search for the next block of data in m_pBinData that matches the criteria given in the constructor.
		//			Allocate the appropriate amount of memory to *ppData and copy the block to the newly allocated space.
		//			The caller is responsible for deleting the allocated data.
	}
	
	return rv;
}

int binDataSearcher::deallocData(void** ppData) {
	int rv = -1;
	
	if (ppData && *ppData != NULL) {
		//TODO	Deallocate *ppData (previously allocated by getNext()) and set *ppData to NULL.
	}
	
	return rv;
}

int binDataSearcher::search(void* pData, u_int32_t ulSize, vector<u_int8_t>* pSeek) {
	int rv = -1;
	
	/* TODO if (pData && ulSize > 0) {
		for (u_int32_t i=0; i<ulSize; i++) {
			if ((u_int8_t*)pData[i] == pSeek->front()) {
			}
		}
	} else {
		DEBUG_ERROR("binDataSearcher::search() Invalid data pointer or size value.");
	}*/
	
	return rv;
}
