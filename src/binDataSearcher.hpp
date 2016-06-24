#ifndef BINDATASEARCHER_HPP_
#define BINDATASEARCHER_HPP_

/*
 * Description:	This class should be able to search for all instances of a block of data meeting at least two of three
 * 					criteria: header, footer, and length.  At least two of these criteria must be specified in the
 * 					class constructor.  If the class is given a header/footer, the blocks of data can be found by
 * 					searching for the header and then the footer.  Optionally, a length could be given to limit how far
 *						ahead to search for the footer.  If the class is given a header/length, the blocks of data can be
 * 					found by searching for the header and then retrieving length data following the header.  Similarly,
 * 					a footer and length could be used to find the blocks of data.
 * 
 * Notes:			Additionally, the caller may give a value that indicates the offset within a potential block that 
 * 					describes the total length of the block.  In that case, the length value would be the upper bound
 * 					for values retrieved from length offset.  This would be useful in cases where the records being sought
 * 					contain a value describing the record's total length.  (TODO - This will also require an endian flag
 * 					as the target records may not always come from a system with the same endianness as the one running
 * 					this library.  It will also require a size flag as the endian routines need to know if they are
 * 					switching 16, 32, or 64 bits.)
 * 
 * 					The length offset should be allowed to be negative.  In the case of Windows event log records, the 
 * 					length of the entire record is stored before the header ID.  The start of the block would then have to
 * 					be changed from the header ID location to the length location.....
 * 
 * 					I just realized that the headerID does not always indicate the start of the record.  There could be
 * 					more extreme examples where the headerID is in the middle of the block, but Windows event log records 
 * 					make a good example.  There is a record length DWORD right before the headerID.  headerID should 
 * 					probably be renamed to something like locatorID and it should be accompanied by a lead-in length that
 * 					may be zero where this situation does not come into play and the locatorID does start the record.
 * 
 * 					I almost changed this class to a template class on the identifier and the terminator.  I was thinking
 * 					that I could then read out entire identifier-sized blocks from the stream and compare the entire thing
 * 					to the sought value.  That is a bad plan because I have to shift through the stream one byte at a
 * 					time.  It makes more sense to read a large chunk and compare, one byte at a time until I have a
 * 					complete match or a mismatch, in which case, I move on.  That will be more efficient.
 * 
 * 					I should read relatively large bytes from the binData object to avoid the overhead or repeatedly
 * 					reading pieces out of the file.  Buffer the file reads and search each buffer however is neccessary to
 * 					find the values.
 */
 
#include "binData.hpp"

#include <vector>
using namespace std;

enum endianness_t { little, big };
enum endiansize_t { sixteen, thirtytwo, sixtyfour };

class binDataSearcher {
	public:
		binDataSearcher(	vector<u_int8_t>& vIdentifiers,	//Array of bytes describing an identifier value 
								u_int32_t uiIdentifierLeadIn, 	//Number of bytes prior to the identifier where the record starts
								vector<u_int8_t>& vTerminators,	//Array of bytes describing a terminating value
								u_int32_t uiLength, 					//Maximum length of potential records
								u_int32_t uiLengthValOffset, 		//Offset within the start of the lead-in to the end of the terminator that describes the record length
								endianness_t endianness, 			//Endianness of the record length value
								endiansize_t endiansize				//Size of the record length value for the endian switch
							);  
		~binDataSearcher(); 
		
		int load(binData* pData);
		int getNext(void** ppData);		//Find the next block of data matching criteria given in the constructor, allocate memory, and point *ppData to the location.
		int deallocData(void** ppData);	//Deallocate memory allocated by getNext()
		
	private:
		int search(void* pData, u_int32_t ulSize, vector<u_int8_t>* pSeek);
	
	private:
		vector<u_int8_t> m_vIdentifiers; 
		u_int32_t m_uiIdentifierLeadIn;
		vector<u_int8_t> m_vTerminators;
		u_int32_t m_uiLength;
		u_int32_t m_uiLengthValOffset;
		endianness_t m_endianness;
		endiansize_t m_endiansize;
		binData* m_pBinData;
};

#endif /*BINDATASEARCHER_HPP_*/
