#ifndef IDMLIB_ProbSimMatch_HPP
#define IDMLIB_ProbSimMatch_HPP

#include "sift.hpp"

#include <am/leveldb/Table.h>
#include <3rdparty/am/drum/drum.hpp>


DRUM_BEGIN_NAMESPACE

template <>
struct BucketIdentifier<idmlib::ise::Sketch>
{
    static std::size_t Calculate(idmlib::ise::Sketch const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key.desc[0] >> (64 - num_bucket_bits));
    }
};

template <>
struct BucketIdentifier<idmlib::ise::SimHash>
{
    static std::size_t Calculate(idmlib::ise::SimHash const& key, std::size_t const& num_bucket_bits)
    {
        return static_cast<std::size_t>(key.desc[0] >> (64 - num_bucket_bits));
    }
};

DRUM_END_NAMESPACE

namespace idmlib{ namespace ise{

class BruteForce
{
public:
    struct Parameter
    {
        static const unsigned p = 24;
        static const unsigned h = 3;
        static const unsigned k = 2324;
    };

    typedef izenelib::drum::Drum<
        Sketch,
        std::set<unsigned>,
        char,
        izenelib::am::leveldb::TwoPartComparator,
        izenelib::am::leveldb::Table
    > SketchToImgIdType;

    BruteForce(const std::string& path);

    ~BruteForce();

    void Init();

    void Insert(unsigned key, const std::vector<Sketch>& sketches);

    void Delete(unsigned key);

    void Search(const std::vector<Sketch>& sketches, std::vector<unsigned>& results) const;

    void Finish();

private:
    void LoadSketches_(std::istream& iar);

    void SaveSketches_(std::ostream& oar);

    void GenTableIds_(const Sketch& sketch, std::vector<unsigned>& table_ids) const;

private:
    std::string sketch_path_;
    std::string drum_path_;

    std::vector<std::vector<Sketch> > sketches_;
    boost::shared_ptr<SketchToImgIdType> sketch_to_imgid_;
    std::vector<unsigned> bit_flip_table_;
};

class ProbSimMatch
{
public:
    struct Parameter
    {
        static const unsigned p = 24;
        static const unsigned h = 2;
        static const unsigned k = 21;
        static const unsigned ki = 6;
    };

    typedef izenelib::drum::Drum<
        SimHash,
        std::set<unsigned>,
        char,
        izenelib::am::leveldb::TwoPartComparator,
        izenelib::am::leveldb::Table
    > SimHashToImgIdType;

    ProbSimMatch(const std::string& path);

    ~ProbSimMatch();

    void Init();

    void Insert(unsigned key, const std::vector<Sift::Feature>& sifts);

    void Delete(unsigned key);

    void Search(const std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results) const;

    void Finish();

private:
    void InitRandVecTable_();

    void LoadSimHashes_(std::istream& iar);

    void SaveSimHashes_(std::ostream& oar);

    void MapFeatureToCharVec_(const std::vector<float>& feature, std::vector<float>& char_vec) const;

    void GenSimHash_(const std::vector<float>& char_vec, SimHash& simhash) const;

    void GenTableIds_(const std::vector<float>& char_vec, SimHash& simhash, std::vector<unsigned>& table_ids) const;

private:
    std::string simhash_path_;
    std::string drum_path_;
    std::string rand_vec_path_;

    std::vector<std::vector<SimHash> > simhashes_;
    boost::shared_ptr<SimHashToImgIdType> simhash_to_imgid_;
    std::vector<SimHash> rand_vec_table_;
    std::vector<std::vector<unsigned> > bit_flip_table_;
};

}}

MAKE_MEMCPY_TYPE(idmlib::ise::Sketch)
MAKE_MEMCPY_TYPE(idmlib::ise::SimHash)
MAKE_FEBIRD_SERIALIZATION(std::set<unsigned>)

#endif
