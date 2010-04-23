#ifndef CharikarAlgorithm_h
#define CharikarAlgorithm_h 1
/**
 * @file
 * @brief impliments Charikar's algorith
 *
 * CharikarAlgorithm implements random projection based near-duplicate
 * algorithm found in Charikar's paper, "Similarity Estimation Techniques
 * from Rounding Algorithms". This algorithm implementation is based
 * on Henzinger in "Finding Near-Duplicate Web Pages: A Large-Scale
 * Evaluation of Algorithms"
 * Given a document, represented as a sequence of token strings,
 * a document signature is nBit vector where all of the sum
 * of random projections of document tokens are later cast to etiher
 * 1 (when positive) and 0 (when negative).
 * Given the two document signature objects, the number of
 * agreed-on bits represents the cosine similarity between the two
 * vectors and any number of bit matches higher than a threshold
 * is construted as a near-duplicate.
 *
 * @author Yeogirl Yun
 * @date 1/22/07
 */

#include "RandomProjection.h"
#include "RandomProjectionEngine.h"
#include "NearDuplicateAlgorithm.h"

#include <wiselib/CBitArray.h>
#include <boost/thread/mutex.hpp>
#include <vector>

namespace sf1v5{
/**
 * @brief Charikar alogrithm
 */
class CharikarAlgorithm : public NearDuplicateAlgorithm {
private:
    /**
     * @brief
     */
    RandomProjectionEngine rpEngine;
    /**
     * @brief dimensions number
     */
    int nDimensions;
    /**
     * @brief similarity threshold
     */
    double threshold;
    boost::mutex mutex_;
public:
    /**
     * @brief default dimensions number
     */
    /**
     * @brief default threshold value
     */
    static const double DEFAULT_THRESHOLD_VALUE = 0.97;
    static const int DEFAULT_NUM_DIMENSIONS = 384;
public:
    /**
     * @brief constructor of CharikarAlgorithm, initial members
     *
     * @param nDim dimensions number
     * @param tvalue threshold value
     */
    inline CharikarAlgorithm(int nDim=CharikarAlgorithm::DEFAULT_NUM_DIMENSIONS, double tvalue=CharikarAlgorithm::DEFAULT_THRESHOLD_VALUE) :
        rpEngine(nDim), nDimensions(nDim), threshold(tvalue) { }
    inline CharikarAlgorithm(double tvalue):
        rpEngine(DEFAULT_NUM_DIMENSIONS), nDimensions(DEFAULT_NUM_DIMENSIONS), threshold(tvalue) { }
    /**
     * @brief disconstructor
     */
    inline ~CharikarAlgorithm() {}
public:
    /**
     * @brief get dimensions number
     *
     * @return dimensions number
     */
    inline int num_dimensions() { return nDimensions; }
    /**
     * @brief get score of two documents
     *
     * @param sig1 first signature
     * @param sig2 second signature
     *
     * @return the duplicate score
     */
    int neardup_score(NearDuplicateSignature& sig1, NearDuplicateSignature& sig2);
    /**
     * @brief get score of 2 documents
     *
     * @param sig1 first bit array
     * @param sig2 second bit array
     *
     * @return the duplicate score
     */
    int neardup_score(const wiselib::CBitArray& sig1, const wiselib::CBitArray& sig2);
    /**
     * @brief Generates a Charikar document signature from a document tokens.
     *
     * @param[in] docTokens doc tokens array
     * @param[out] bitArray bit array to set
     */
    void generate_document_signature(const wiselib::DynamicArray<wiselib::ub8>& docTokens, wiselib::CBitArray& bitArray);

    void generate_document_signature(const wiselib::YString& document, wiselib::CBitArray& bitArray);

    /**
     * @brief generate signature from a vector
     *
     * @param[in] docTokens input source, a term id array
     * @param[out] bitArray signature
     */
    void generate_document_signature(const std::vector<unsigned int>& docTokens, wiselib::CBitArray& bitArray);

 /**
     * @brief generate signature from a vector
     *
     * @param[in] docTokens input source, a term id array
     * @param[out] bitArray signature
     */
    void generate_document_signature(const std::vector<std::string>& docTokens, wiselib::CBitArray& bitArray);
    /**
     * @brief check if is near duplicate
     *
     * @param neardupScore duplicate score to check
     *
     * @return true, if is near duplicate, false else.
     */
    bool is_neardup(float neardupScore) {
        return (double)neardupScore/(double)nDimensions > threshold;
    }

    /**
     * @brief set threshold
     *
     * @param th target threshold to set
     */
    void set_threshold(float th= DEFAULT_THRESHOLD_VALUE) {
        threshold = th;
    }
};
}

#endif