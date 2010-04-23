/**
   @file NearDuplicateSignature.h
   @author Jinglei
   @date 2009.11.24
 */
#ifndef NearDuplicateSignature_h
#define NearDuplicateSignature_h 1

#include "TypePair.h"
#include "RandomProjection.h"
#include "Trie.h"
#include "TypeKey.h"
#include "CharikarAlgorithm.h"

#include <wiselib/basics.h>
#include <wiselib/IntTypes.h>
#include <wiselib/CBitArray.h>
#include <wiselib/DArray.h>

#include <set>
namespace sf1v5{
/**
 * @brief NearDuplicateSignature
 */
class NearDuplicateSignature {
private:
    /**
     * @brief key of this NearDuplicateSignature
     */
    wiselib::TypeKey<wiselib::ub4> key;
    wiselib::CBitArray bitArray;
    /**
     * document id of this signature
     */
    wiselib::TypeKey<int> docID;

    /**
     * @brief bits length of every word
     */
    static const int NUM_BITS_PER_WORD = sizeof(wiselib::ub4)*8;
public:
    /**
     * string to docID mapping.
     */
    static wiselib::Trie<wiselib::TypePair<std::string, int> > docIDTable;
    /**
     * docID to string mapping
     */
    static wiselib::DynamicArray<string> docNameTable;
public:
    /**
     * null constructor
     */
    inline NearDuplicateSignature() : key(0), docID(0) {}
    /**
     * @brief constructor
     *
     * @param k key
     * @param ba bit array
     */
    inline NearDuplicateSignature(wiselib::ub4 k, const wiselib::CBitArray& ba)
        : key(k), bitArray(ba) {}
    /**
     * @brief disconstructor
     */
    inline ~NearDuplicateSignature() {}
public:
    /**
     * @brief get bitarray
     *
     * @return reference of bitArray
     */
    inline wiselib::CBitArray& get_bitarray() { return bitArray; }

    /**
     * @brief get sub key
     *
     * multiset/hashtable interface
     *
     * @return key
     */
    inline const wiselib::TypeKey<wiselib::ub4>& get_sub_key() const { return key; }
    /**
     * @brief get id
     *
     * @return docId
     */
    inline const wiselib::TypeKey<int>& get_key() const { return docID; }
    /**
     * @brief get doc id
     *
     * @return int value of docID
     */
    inline int get_doc_id() const { return docID.key; }
    /**
     * @brief get document string by doc id
     *
     * @param docid doc id
     *
     * @return string of the document
     */
    inline static const string& get_doc_string(int docid) { return docNameTable[docid]; }

    /**
     * @brief display this object
     *
     * @param stream out stream
     */
    inline void display(ostream& stream) const {
        stream << "[" << docNameTable[docID.key] << ": " << key << ", " << bitArray << "]";
    }

    /**
     * @brief static function to generate NearDuplicateSignature
     *
     * @param nDimensions dimension count
     * @param bitArray bit array
     * @param docName document name
     * @param ndSignatureArray NearDuplicateSignature array to build
     */
    static void  generate_near_duplicate_signatures(int nDimensions,
            const wiselib::CBitArray& bitArray,
            const std::string& docName,
            std::set<NearDuplicateSignature>& ndSignatureSet);

    static void  generate_near_duplicate_signatures(int nDimensions,
           const wiselib::CBitArray& bitArray,
           unsigned int docID,
           std::set<NearDuplicateSignature>& ndSignatureSet);

    /**
     * Enable it to be used in STL set , map, etc..
     */
    inline friend bool operator<(const NearDuplicateSignature& sigCompare1,const NearDuplicateSignature& sigCompare2)
    {
    	return sigCompare1.get_sub_key()<sigCompare2.get_sub_key();
    }

    /**
     * Enable it to use boost serialization.
     */
    template<class Archive> void serialize(Archive & ar,
                				const unsigned int version) {
            	//ar & key;
            	ar & bitArray;
            	ar & docID;
              }
};
//define ostream<<NearDuplicateSignature, look <basics.h>
DEF_DISPLAY(NearDuplicateSignature);
}
#endif