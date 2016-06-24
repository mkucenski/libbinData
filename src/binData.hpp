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
		
		virtual int getData(void* pData, u_int32_t ulSize, u_int32_t* p_ulSizeRead) = 0;								//Read ulSize bytes of data
		virtual int getData(void* pData, u_int32_t ulSize, u_int32_t ulOffset, u_int32_t* p_ulSizeRead) = 0;	//Read ulSize bytes of data from offset ulOffset
};

#endif /*BINDATA_HPP_*/
