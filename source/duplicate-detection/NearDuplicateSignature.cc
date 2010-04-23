#include <idmlib/duplicate-detection/NearDuplicateSignature.h>

using namespace sf1v5;
using namespace wiselib;

/********************************************************************************
  Description: NearDuplicateSignature is data structure used in
                 NearDuplicateClustering. It is a 4-byte key, bitstring, and
                 docID. Only those of bitstrings that has the same key will be
                 compared exhaustively to determine near-duplicate clustering.
               See NearDuplicateClustering and NearDuplicateAlgorithm derived classes
                 for alogirthms to generate bitstring.
  Comments   :
  History    : Yeogirl Yun                                           1/16/07
                 Initial Revision
********************************************************************************/

// string to docID mapping.
Trie<TypePair<string, int> > NearDuplicateSignature::docIDTable;
// docID to string mapping
DynamicArray<string> NearDuplicateSignature::docNameTable;
/********************************************************************************
  Description:  Construct nWords number of NearDuplicateSignature from a bitArray.
  Comments   :
********************************************************************************/


void NearDuplicateSignature::generate_near_duplicate_signatures(int nDimensions,
				   const CBitArray& bitArray,
				   const string& docName,
				   std::set<NearDuplicateSignature>& ndSignatureSet)
{
    int nWords = bitArray.GetLength()/sizeof(ub4);
    const ub1* bytestream = bitArray.GetBuffer();
    ub4 key = 0;
    const ub4* p = (ub4*)bytestream;
    for (int i =0; i < nWords; i++) {
    	if(p[i]!=0){
        key = p[i];
        NearDuplicateSignature nds;
        nds.key = key;
        nds.bitArray = bitArray;
        const string& docstring = docName;
        const TypePair<string, int>* si = docIDTable.find(docstring);
        if (si)
            nds.docID = si->second;
        else {
            TypePair<string, int> newSI(docstring, docNameTable.size());
            nds.docID = docNameTable.size();
            docNameTable[docNameTable.size()] = docstring;
            docIDTable.insert(newSI);
        }
        ndSignatureSet.insert(nds);
    	}
    }
}

void NearDuplicateSignature::generate_near_duplicate_signatures(int nDimensions,
				   const CBitArray& bitArray,
				   unsigned int docID,
				   std::set<NearDuplicateSignature>& ndSignatureSet)
{
    int nWords = bitArray.GetLength()/sizeof(ub4);
    const ub1* bytestream = bitArray.GetBuffer();
    ub4 key = 0;
    const ub4* p = (ub4*)bytestream;
    for (int i =0; i < nWords; i++) {
    	if(p[i]!=0){
        key = p[i];
        NearDuplicateSignature nds;
        nds.key = key;
        nds.bitArray = bitArray;
        nds.docID.key = docID;
        ndSignatureSet.insert(nds);
    	}
    }
}
