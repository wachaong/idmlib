///
/// @file algorithm.hpp
/// @brief The first version of KPE algorithm
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-08
/// @date Updated 2010-04-08
///

#ifndef _IDM_KPE_ALGORITHM_H_
#define _IDM_KPE_ALGORITHM_H_

// #include "input.hpp"
#include "kpe_output.h"
#include "kpe_scorer.h"
#include "kpe_algorithm_def.h"
#include "../idm_types.h"
#include <boost/type_traits.hpp>
#include "../util/FSUtil.hpp"
#include "../util/idm_term.h"
#include "../util/idm_id_manager.h"
#include <util/izene_log.h>
NS_IDMLIB_KPE_BEGIN



template < class OutputType>
class KPEAlgorithm : public boost::noncopyable
{
typedef uint32_t id_type;
typedef char pos_type;
typedef izenelib::util::UString string_type;


public:
  typedef KPEScorer ScorerType;
  typedef typename OutputType::function_type callback_type;

    KPEAlgorithm(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, callback_type callback )
    : dir_(dir), id_dir_(dir+"/id"), tmp_dir_(dir+"/tmp"), kp_dir_(dir+"/kp"), analyzer_(analyzer)
    , id_manager_(NULL), outside_idmanager_(false)
    , output_(callback), max_phrase_len_(7)
    , pTermListWriter_(NULL), pHashWriter_(NULL), kpWriter_(NULL)
    , cache_size_(0), cache_vec_(cache_size_),scorer_(NULL), outside_scorer_(false)
    , last_doc_id_(0), doc_count_(0), all_term_count_(0)
    {
      init_();
    }

    KPEAlgorithm(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, callback_type callback, idmlib::util::IDMIdManager* id_manager )
    : dir_(dir), id_dir_(dir+"/id"), tmp_dir_(dir+"/tmp"), kp_dir_(dir+"/kp"), analyzer_(analyzer)
    , id_manager_(id_manager), outside_idmanager_(true)
    , output_(callback), max_phrase_len_(7)
    , pTermListWriter_(NULL), pHashWriter_(NULL), kpWriter_(NULL)
    , cache_size_(0), cache_vec_(cache_size_),scorer_(NULL), outside_scorer_(false)
    {
      init_();
    }

    ~KPEAlgorithm()
    {
      release_();

    }

    void try_compute_num(uint32_t num)
    {
      try_compute_num_ = num;
    }

    void set_no_freq_limit()
    {
      no_freq_limit_ = true;
    }

    void add_manmade(const izenelib::util::UString& ustr)
    {
      std::vector<idmlib::util::IDMTerm> term_list;
      analyzer_->GetTgTermList(ustr, term_list );
      std::vector<uint32_t> termid_list(term_list.size());
      for(uint32_t i=0;i<term_list.size();i++)
      {
        termid_list[i] = term_list[i].id;
      }
      manmade_.insert(termid_list,0);
    }

    void set_tracing(const izenelib::util::UString& ustr)
    {
      std::vector<idmlib::util::IDMTerm> term_list;
      analyzer_->GetTgTermList(ustr, term_list );
      tracing_.resize(term_list.size());
      std::cout<<"[tracing] ";
      for(uint32_t i=0;i<term_list.size();i++)
      {
        tracing_[i] = term_list[i].id;
        std::cout<<tracing_[i]<<",";
      }
      std::cout<<std::endl;
    }

    void set_max_phrase_len(uint8_t max_len)
    {
        max_phrase_len_ = max_len;
    }

    void set_cache_size(uint32_t size)
    {
      cache_size_ = size;
      cache_vec_.resize(cache_size_);
      cache_vec_.resize(0);
    }

    bool load(const std::string& res_ath)
    {
      if( scorer_ == NULL )
      {
        scorer_ = new ScorerType(analyzer_);
        bool b = scorer_->load(res_ath);
        if(!b)
        {
          std::cerr<<"[KPE] resource load failed"<<std::endl;
          delete scorer_;
          scorer_ = NULL;
          return false;
        }
        outside_scorer_ = false;
      }
      return true;
    }

    void set_scorer(ScorerType* scorer)
    {
      scorer_ = scorer;
      outside_scorer_ = true;
    }

    void insert( const izenelib::util::UString& article, uint32_t docId = 1)
    {
//       std::cout<<docId<<"##### "<<article.length()<<std::endl;
        if( docId != last_doc_id_ )
        {
          try_compute_();
        }
        std::vector<idmlib::util::IDMTerm> term_list;
        analyzer_->GetTgTermList(article, term_list );
        if( !term_list.empty() )
        {
          uint32_t begin = 0;
          while( addTerms_(docId, term_list, begin) ) {}
        }
        if( docId != last_doc_id_ )
        {
            setDocCount_( getDocCount() + 1 );
        }
        last_doc_id_ = docId;
    }

    void close()
    {

      compute_();
      kp_construct_();
    }



    uint32_t getDocCount()
    {
        return doc_count_;
    }

private:


    void kp_construct_()
    {
      if( kpWriter_->getItemCount() ==0 )
      {
        return;
      }
      kpWriter_->close();
      std::string kpWriterPath = kpWriter_->getPath();
      delete kpWriter_;
      kpWriter_ = NULL;
      {
        typename KPSSFType::SorterType kpsorter;
        kpsorter.sort(kpWriterPath);
      }
      hash_t hash = 0;
      KPItem kp;
      std::vector<KPItem> kp_list;
      hash_t last_hash = 0;

      {
        typename KPSSFType::ReaderType kpReader(kpWriterPath);
        kpReader.open();
        while( kpReader.next(hash, kp) )
        {
          if( hash!=last_hash && !kp_list.empty() )
          {
            for(uint32_t i=1;i<kp_list.size();i++)
            {
              kp_list[0].docitem_list.insert(kp_list[0].docitem_list.end(), kp_list[i].docitem_list.begin(), kp_list[i].docitem_list.end());
              kp_list[0].lc_list.insert(kp_list[0].lc_list.end(), kp_list[i].lc_list.begin(), kp_list[i].lc_list.end());
              kp_list[0].rc_list.insert(kp_list[0].rc_list.end(), kp_list[i].rc_list.begin(), kp_list[i].rc_list.end());
            }
            idmlib::util::accumulateList(kp_list[0].docitem_list);
            idmlib::util::accumulateList(kp_list[0].lc_list);
            idmlib::util::accumulateList(kp_list[0].rc_list);
            outputKP_(kp_list[0]);
            kp_list.resize(0);
          }
          kp_list.push_back(kp);
          last_hash = hash;
        }
        kpReader.close();
      }
      if( !kp_list.empty() )
      {
        for(uint32_t i=1;i<kp_list.size();i++)
        {
          kp_list[0].docitem_list.insert(kp_list[0].docitem_list.end(), kp_list[i].docitem_list.begin(), kp_list[i].docitem_list.end());
          kp_list[0].lc_list.insert(kp_list[0].lc_list.end(), kp_list[i].lc_list.begin(), kp_list[i].lc_list.end());
          kp_list[0].rc_list.insert(kp_list[0].rc_list.end(), kp_list[i].rc_list.begin(), kp_list[i].rc_list.end());
        }
        idmlib::util::accumulateList(kp_list[0].docitem_list);
        idmlib::util::accumulateList(kp_list[0].lc_list);
        idmlib::util::accumulateList(kp_list[0].rc_list);
        outputKP_(kp_list[0]);
        kp_list.resize(0);
      }
      boost::filesystem::remove_all(kpWriterPath);
      if( kpWriter_ == NULL)
      {
        kpWriter_ = new KPSSFType::WriterType(kpWriterPath);
        kpWriter_->open();
      }
    }

    void try_compute_()
    {
      if( try_compute_num_>0 && pHashWriter_->getItemCount() >= try_compute_num_ )
      {
        compute_();
        reinit_();
      }
    }

    void reinit_()
    {
      scorer_->flush();
      last_doc_id_ = 0;
      doc_count_ = 0;
      all_term_count_ = 0;
      boost::filesystem::remove_all(tmp_dir_);
      boost::filesystem::create_directories(tmp_dir_);
      if( pTermListWriter_ == NULL )
      {
        pTermListWriter_ = new TermListWriter(tmp_dir_+"/pInputItemWriter");
        pTermListWriter_->open();
      }
      if( pHashWriter_ == NULL)
      {
        pHashWriter_ = new HashWriter(tmp_dir_+"/pHashItemWriter");
        pHashWriter_->open();
      }
    }

    void compute_()
    {
      releaseCachedHashItem_();
      uint64_t hash_total_count = pHashWriter_->getItemCount();
      if(  hash_total_count==0 )
      {
        return;
      }
      pTermListWriter_->close();
      pHashWriter_->close();
      std::string inputItemPath = pTermListWriter_->getPath();
      std::string hashItemPath = pHashWriter_->getPath();
      delete pTermListWriter_;
      pTermListWriter_ = NULL;
      delete pHashWriter_;
      pHashWriter_ = NULL;

      //set thresholds
      uint32_t docCount = getDocCount_();
      std::cout<<"[KPE] running for "<<docCount<<" docs, hash total count: "<<hash_total_count<<std::endl;
      min_freq_threshold_ = 1;
      min_df_threshold_ = 1;
      if(!no_freq_limit_)
      {
        if( docCount >=10 )
        {
            min_freq_threshold_ = (uint32_t)std::floor( std::log( (double)docCount )/2 );
            if( min_freq_threshold_ < 3 ) min_freq_threshold_ = 3;

            min_df_threshold_ = (uint32_t)std::floor( std::log( (double)docCount )/4 );
            if( min_df_threshold_ < 2 ) min_df_threshold_ = 2;
        }
      }
      uint64_t p = 0;
      typename HashSSFType::SorterType hsorter;
      hsorter.sort(hashItemPath);
      MEMLOG("[KPE1] finished");
      typedef izenelib::am::SSFType<hash_t, uint32_t, uint8_t> HashItemCountSSFType;
      typename HashItemCountSSFType::WriterType hcwriter(tmp_dir_+"/HCWRITER");
      hcwriter.open();
      {
          HashSSFType::ReaderType hreader(hashItemPath);
          hreader.open();
          hash_t hashId;
          hash_t saveId;
          uint32_t count = 0;
          uint32_t icount = 1;
          LOG_BEGIN("Hash", &hreader);
          while( true )
          {
            bool b = true;
            if( cache_size_>0)
            {
              b = hreader.next(hashId, icount);
            }
            else
            {
              b = hreader.nextKey(hashId);
            }
            if( !b ) break;
            if(p==0)
            {
                saveId = hashId;
            }
            else
            {
                if( saveId != hashId )
                {
                    hcwriter.append(saveId, count);
                    saveId = hashId;
                    count = 0;
                }
            }
            count += icount;
            p++;
            LOG_PRINT("Hash", 1000000);
          }
          LOG_END();
          hcwriter.append(saveId, count);
          hreader.close();
          idmlib::util::FSUtil::del(hashItemPath);

      }
      hcwriter.close();
      MEMLOG("[KPE2] finished");
//       if(!tracing_.empty())
//       {
//         typename TermListSSFType::ReaderType fireader(inputItemPath);
//         fireader.open();
//         std::vector<uint32_t> keyList;
//         while( fireader.nextKeyList(keyList) )
//         {
//           if(vec_starts_(keyList, tracing_))
//           {
//             std::cout<<"[tracing] [before-suffix] ";
//             for(uint32_t dd=0;dd<keyList.size();dd++)
//             {
//               std::cout<<keyList[dd]<<",";
//             }
//             std::cout<<std::endl;
//           }
//         }
//         fireader.close();
//       }
      typename TermListSSFType::SorterType fisorter;
      fisorter.sort(inputItemPath);
//       if(!tracing_.empty())
//       {
//         typename TermListSSFType::ReaderType fireader(inputItemPath);
//         fireader.open();
//         std::vector<uint32_t> keyList;
//         while( fireader.nextKeyList(keyList) )
//         {
//           if(vec_starts_(keyList, tracing_))
//           {
//             std::cout<<"[tracing] [after-suffix] ";
//             for(uint32_t dd=0;dd<keyList.size();dd++)
//             {
//               std::cout<<keyList[dd]<<",";
//             }
//             std::cout<<std::endl;
//           }
//         }
//         fireader.close();
//       }
      MEMLOG("[KPE3] finished");
      typedef izenelib::am::SSFType<uint32_t, uint32_t, uint32_t> SortedFragmentItemSSFType;
      typename SortedFragmentItemSSFType::WriterType swriter(tmp_dir_+"/SWRITER");
      swriter.open();
      {
          typename TermListSSFType::ReaderType fireader(inputItemPath);
          fireader.open();
          std::vector<uint32_t> keyList;

          LOG_BEGIN("Suffix", &fireader);
          SortedFragmentItem sItem;
          std::vector<uint32_t> lastTermIdList;
          while( fireader.nextKeyList(keyList) )
          {
            if(!tracing_.empty())
            {
              if(vec_starts_(keyList, tracing_))
              {
                std::cout<<"[tracing] [suffix] ";
                for(uint32_t dd=0;dd<keyList.size();dd++)
                {
                  std::cout<<keyList[dd]<<",";
                }
                std::cout<<std::endl;
              }
            }
              uint32_t zeroPos = 0;
              for(uint32_t i=0;i<keyList.size();i++)
              {
                  if( keyList[i] == 0 )
                  {
                      zeroPos = i;
                      break;
                  }
              }
              std::vector<uint32_t> termIdList(keyList.begin(), keyList.begin()+zeroPos);
              uint32_t docId = 0;
              uint32_t prefixTermId = 0;
              bool hasPrefix = false;
              if( zeroPos == keyList.size()-2 )
              {
                  docId = keyList[keyList.size()-1];

              }
              else if( zeroPos == keyList.size()-3 )
              {
                  docId = keyList[keyList.size()-2];
                  prefixTermId = keyList[keyList.size()-1];
                  hasPrefix = true;
              }
              else
              {
                  //impossible, just a reminder.
              }
              if( p == 0 )
              {
                  SortedFragmentItem tmp(0, termIdList);
                  std::swap(sItem, tmp);
              }
              else
              {
                  uint32_t inc;
                  sItem.getIncrement(termIdList, inc);
                  if( inc != termIdList.size() )//the same termidlist
                  {
                      std::vector<uint32_t> value;
                      sItem.getValue(value);
                      swriter.append( value );

                      std::vector<uint32_t> incTermIdList(sItem.termIdList_.begin(),
                      sItem.termIdList_.begin()+inc);
                      incTermIdList.insert(incTermIdList.end(),
                      termIdList.begin()+inc, termIdList.end());
                      SortedFragmentItem tmp(inc, incTermIdList);
                      std::swap(sItem, tmp);
                  }
              }
              sItem.addDocId(docId, 1);
              if(hasPrefix)
                  sItem.addPrefixTerm(prefixTermId);

              p++;
              LOG_PRINT("Suffix", 1000000);
          }
          LOG_END();
          std::vector<uint32_t> value;
          sItem.getValue(value);
          swriter.append(value);
          fireader.close();
          MEMLOG("Total output: %lu",swriter.getItemCount() );
          idmlib::util::FSUtil::del(inputItemPath);
      }
      swriter.close();
      MEMLOG("[KPE4] finished.");
      typename SortedFragmentItemSSFType::ReaderType reader(swriter.getPath());
      reader.open();
      std::vector<uint32_t> keyList;
      typename CandidateSSFType::WriterType hclWriter(tmp_dir_+"/HCLWRITER");
      hclWriter.open();
      typename Hash2HashSSFType::WriterType h2hWriter(tmp_dir_+"/H2HWRITER");
      h2hWriter.open();
      std::vector<uint32_t> termIdList(0);
      std::vector<uint32_t> docIdList(0);

      LOG_BEGIN("AAA", &reader);

      std::vector<Data > data;
      double aaa_time = 0.0;
      izenelib::util::ClockTimer aaa_clocker;

      std::vector<uint32_t> completeTermIdList;
      while( reader.nextKeyList(keyList) )
      {
          Data this_data;
          this_data.inc = SortedFragmentItem::parseInc(keyList);
          this_data.termid_list = SortedFragmentItem::parseTermIdList(keyList);
          this_data.docitem_list = SortedFragmentItem::parseDocItemList(keyList);
          this_data.freq = SortedFragmentItem::parseFreq(keyList);
          this_data.lc_list = SortedFragmentItem::parsePrefixTermList(keyList);

          completeTermIdList.resize(this_data.inc);
          completeTermIdList.insert( completeTermIdList.end(), this_data.termid_list.begin(), this_data.termid_list.end() );
          this_data.termid_list = completeTermIdList;
          if( this_data.inc==0 )
          {
              aaa_clocker.restart();
              getCandidateLabel2_(data, &hclWriter, &h2hWriter);
              aaa_time += aaa_clocker.elapsed();
              data.resize(0);
          }
          if(!tracing_.empty())
          {
            if(vec_starts_(this_data.termid_list, tracing_))
            {
              std::cout<<"[tracing] [data] ";
              for(uint32_t dd=0;dd<this_data.docitem_list.size();dd++)
              {
                std::cout<<"("<<this_data.docitem_list[dd].first<<"|"<<this_data.docitem_list[dd].second<<"),";
              }
              std::cout<<std::endl;
            }
          }
          data.push_back(this_data);
          p++;
          LOG_PRINT("AAA", 100000);

      }
      aaa_clocker.restart();
      getCandidateLabel2_(data, &hclWriter, &h2hWriter);
      aaa_time += aaa_clocker.elapsed();
      LOG_END();
      hclWriter.close();
      h2hWriter.close();
      MEMLOG("[KPE4] finished.");
      MEMLOG("[KPE4-AAA] %f seconds.", aaa_time);

      {
          typename CandidateSSFType::SorterType sorter;
          sorter.sort(hclWriter.getPath());
      }
      {
          typename Hash2HashSSFType::SorterType sorter;
          sorter.sort(h2hWriter.getPath());
      }
      HashCountMerger hcMerger;
      hcMerger.setObj(hcwriter.getPath(), h2hWriter.getPath() );
      hash_t key = 0;
      std::vector<uint32_t> valueList1;
      std::vector<Hash2HashItem> valueList2;
      typename Hash2CountSSFType::WriterType h2cWriter( tmp_dir_+"/H2CWRITER" );
      h2cWriter.open();
      while( hcMerger.next( key, valueList1, valueList2) )
      {
          if( valueList1.size() == 0 )
          {
              //impossible, just a reminder.
              continue;
          }
          if( valueList1.size() > 1 )
          {
              //impossible, just a reminder.
              continue;
          }
          for(uint32_t i=0;i<valueList2.size();i++)
          {
              h2cWriter.append( valueList2[i].first, Hash2CountItem(valueList1[0],valueList2[i].second) );

          }
      }
      h2cWriter.close();
      {
          typename Hash2CountSSFType::SorterType sorter;
          sorter.sort( h2cWriter.getPath() );
      }
//         sleep(1000);
      typename HashCountListSSFType::WriterType hcltWriter( tmp_dir_+"/HCLTWRITER" );
      hcltWriter.open();
      {
          typename Hash2CountSSFType::ReaderType h2cReader(h2cWriter.getPath());
          h2cReader.open();
          Hash2CountItem hash2CountValue;
          LOG_BEGIN("HashCount", &h2cReader);
          HashCountListItem output(0, 0, 0, 0);
          hash_t key;
          hash_t saveKey;
          while( h2cReader.next(key, hash2CountValue) )
          {
              if(p==0)
              {
                  saveKey = key;
              }
              else
              {
                  if(saveKey != key )
                  {
                      hcltWriter.append(saveKey, output);
                      output = boost::make_tuple(0,0,0,0);
                      saveKey = key;
                  }
              }
              if( hash2CountValue.second == 1 )
              {
                  boost::get<0>(output) = hash2CountValue.first;
              }
              else if( hash2CountValue.second == 2 )
              {
                  boost::get<1>(output) = hash2CountValue.first;
              }
              else if( hash2CountValue.second == 3 )
              {
                  boost::get<2>(output) = hash2CountValue.first;
              }
              else if( hash2CountValue.second == 4 )
              {
                  boost::get<3>(output) = hash2CountValue.first;
              }
              else
              {
                  //impossible, just a reminder.
              }
              p++;
              LOG_PRINT("HashCount", 100000);
          }
          hcltWriter.append(saveKey, output);
          LOG_END();
          h2cReader.close();
          idmlib::util::FSUtil::del(h2cReader.getPath());
      }
      hcltWriter.close();
//             typename HashCandidateLabelItemSSFType::ReaderType tmpReader(hclWriter.getPath());
//             tmpReader.open();
//             while( tmpReader.next() )
//             {
//                 hash_t key;
//                 bool b = tmpReader.getCurrentKey(key);
//                 std::cout<<"TMP :: KEY :: "<<b<<"|||"<<key<<std::endl;
//             }
//             tmpReader.close();
      {
          merger_t finalMerger;
          finalMerger.setObj( hclWriter.getPath(), hcltWriter.getPath() );
          hash_t key = 0;
          std::vector<CandidateItem> valueList1;
          std::vector<HashCountListItem> valueList2;
          uint32_t n = getTermCount_();
          while( finalMerger.next(key, valueList1, valueList2) )
          {
              if( valueList1.size()==0 )
              {
                  //impossible, just a reminder.
                  continue;
              }
              if( valueList1.size()>1 )
              {
                  //we may face to a hash conflict, ignore them
                  continue;
              }

              if( valueList2.size()!=1 )
              {
                  //impossible, just a reminder.
                  continue;
              }
              CandidateItem& hlItem = valueList1[0];
              HashCountListItem& hclItem = valueList2[0];
              std::vector<uint32_t> termIdList = hlItem.termid_list;
              std::vector<id2count_t> docItem = hlItem.docitem_list;
              uint32_t f = hlItem.freq;
              std::vector<id2count_t> leftTermList = hlItem.lc_list;
              std::vector<id2count_t> rightTermList = hlItem.rc_list;
              if( termIdList.size()==1 )
              {
                insertKP_( termIdList, docItem, getScore(f, termIdList.size(),0.0) , leftTermList,rightTermList);
              }
              else if(termIdList.size()==2 )
              {
                  uint32_t f1 = boost::get<0>(hclItem);
                  uint32_t f2 = boost::get<1>(hclItem);
                  if( f1==0 || f2==0 )
                  {
                      //impossible, just a reminder.
                      continue;
                  }
                  double logL = StatisticalScorer::logL(f,f1,f2,n);
                  double mi = StatisticalScorer::mi(f,f1,f2,n);
//                     std::cout<<"LogL: "<<logL<<" , MI: "<<mi<<std::endl;
                  if( logL>=10 && mi>=5 )
                  {
                      insertKP_( termIdList, docItem, getScore(f, termIdList.size(),logL) , leftTermList,rightTermList);
                  }
              }
              else if(termIdList.size()>2)
              {
                  uint32_t f1 = boost::get<0>(hclItem);
                  uint32_t f2 = boost::get<1>(hclItem);
                  uint32_t f3 = boost::get<2>(hclItem);
                  uint32_t f4 = boost::get<3>(hclItem);
                  if( f1==0 || f2==0 || f3==0 || f4==0 )
                  {
                      //impossible, just a reminder.
                      continue;
                  }
                  double logL = StatisticalScorer::logL(f,f1,f2,n);
                  if( logL>=10 && StatisticalScorer::logL(f,f1,f4,n)>=10
                      && StatisticalScorer::logL(f,f3,f2,n)>=10 )
                  {
                      insertKP_( termIdList, docItem, getScore(f, termIdList.size(),logL) , leftTermList,rightTermList);
                  }
              }
          }
      }
    }

    void init_()
    {
      try_compute_num_ = 400000000;
      test_num_ = 0;
      no_freq_limit_ = false;
      last_doc_id_ = 0;
      doc_count_ = 0;
      all_term_count_ = 0;
      boost::filesystem::create_directories(dir_);
      boost::filesystem::remove_all(tmp_dir_);
      boost::filesystem::create_directories(tmp_dir_);
      boost::filesystem::create_directories(id_dir_);
      boost::filesystem::create_directories(kp_dir_);
      if( pTermListWriter_ == NULL )
      {
        pTermListWriter_ = new TermListWriter(tmp_dir_+"/pInputItemWriter");
        pTermListWriter_->open();
      }
      if( pHashWriter_ == NULL)
      {
        pHashWriter_ = new HashWriter(tmp_dir_+"/pHashItemWriter");
        pHashWriter_->open();
      }
      if( kpWriter_ == NULL)
      {
        kpWriter_ = new KPSSFType::WriterType(kp_dir_+"/kpWriter");
        kpWriter_->open();
      }
      //init idmanager
      if( id_manager_ == NULL )
      {
        id_manager_ = new idmlib::util::IDMIdManager(id_dir_);
      }
      kp_construct_();
    }


    void release_()
    {
        if( pTermListWriter_!= NULL)
        {
            delete pTermListWriter_;
            pTermListWriter_ = NULL;
        }
        if( pHashWriter_!= NULL)
        {
            delete pHashWriter_;
            pHashWriter_ = NULL;
        }
        if( kpWriter_!= NULL)
        {
          delete kpWriter_;
          kpWriter_ = NULL;
        }
        if( id_manager_ != NULL && !outside_idmanager_)
        {
          id_manager_->Flush();
          delete id_manager_;
          id_manager_ = NULL;
        }
        if( !outside_scorer_ && scorer_ != NULL)
        {
            delete scorer_;
            scorer_ = NULL;
        }
        boost::filesystem::remove_all(tmp_dir_);
        boost::filesystem::remove_all(kp_dir_);
    }



    bool addTerms_(uint32_t docId, const std::vector<idmlib::util::IDMTerm>& termList, uint32_t& iBegin)
    {
//         std::cout<<docId<<"#### "<<termList.size()<<std::endl;
        if(iBegin >= termList.size()) return false;
        std::vector<std::pair<bool,uint32_t> > splitVec;
        uint32_t i=iBegin;
        uint32_t _begin = iBegin;
        uint32_t insertTermId;
        for ( ;i<termList.size();i++ )
        {
            bool bSplit = scorer_->isSplitTerm(termList[i].text, termList[i].tag, termList[i].id, insertTermId);

            splitVec.push_back(std::make_pair(bSplit, insertTermId));
            if( i == iBegin ) //first term
            {
                continue;
            }
            else
            {
                if(bSplit)
                {
                    break;
                }
                if( termList[i].position != termList[i-1].position+1 )
                {
                    splitVec.erase( splitVec.end()-1 );
                    break;
                }
            }

        }
        iBegin = i;
        if( splitVec.size() == 0 ) return false;
        bool bFirstTerm = true;
        bool bLastTerm = true;
        if( splitVec.size() == 1 )
        {
            if( splitVec[0].first == true )
            {
                return true;
            }
        }
        else
        {
            bFirstTerm = !splitVec.front().first;
            bLastTerm = !splitVec.back().first;

        }
        std::vector<uint32_t> terms( splitVec.size() );
        for(uint32_t p=_begin;p<_begin+splitVec.size();p++)
        {
            uint32_t _index = p-_begin;
            terms[_index] = splitVec[_index].second;
            id_manager_->Put(terms[_index], termList[p].text);
//             if( !splitVec[_index].first )
//             {
//
//             }
        }
        addTerms_(docId, terms, bFirstTerm, bLastTerm);
        return true;


    }


    void addTerms_(uint32_t docId, const std::vector<uint32_t>& termList, bool bFirstTerm = true, bool bLastTerm = true)
    {
//         std::cout<<docId<<"### "<<termList.size()<<" "<<(int)bFirstTerm<<" "<<(int)bLastTerm<<std::endl;
        if( termList.size() == 0 ) return;
        if( termList.size() == 1 )
        {
            if( bFirstTerm && bLastTerm )
            {
                std::vector<uint32_t> inputItem(termList);
                inputItem.push_back(0);
                inputItem.push_back(docId);
                if( scorer_->prefixTest(termList) != KPStatus::RETURN)
                {
                  appendTermList_(inputItem);
                }

                appendHashItem_(hash_(termList));
                incTermCount_(1);
            }
            return;
        }
        if( termList.size() == 2 && !bFirstTerm && !bLastTerm )
        {
            return;
        }
        uint32_t start = 0;
        uint32_t end = termList.size();
        uint32_t increase = termList.size();
        if(!bFirstTerm)
        {
            increase--;
            start = 1;
        }
        if(!bLastTerm)
        {
            increase--;
            end -= 1;
        }

        for(uint32_t i=start; i<end; i++)
        {
            uint32_t len = std::min(end-i,(uint32_t)(max_phrase_len_+1));
            std::vector<uint32_t> frag( termList.begin()+i, termList.begin()+i+len );
            bool valid_frag =true;
            if( scorer_->prefixTest(frag) != KPStatus::RETURN)
            {
              frag.push_back(0);
              frag.push_back(docId);
              if( i!= 0 )
              {
                  frag.push_back(termList[i-1]);
              }
              appendTermList_(frag);
//               if(docId==3)
//               {
//                 std::cout<<"{{3}}";
//                 for(uint32_t dd=0;dd<frag.size();dd++)
//                 {
//                   std::cout<<frag[dd]<<",";
//                 }
//                 std::cout<<std::endl;
//               }
            }
            else
            {
              valid_frag = false;
            }
            if(!tracing_.empty())
            {
              if(vec_starts_(frag, tracing_))
              {
                std::cout<<"[tracing] ["<<(int)valid_frag<<"] ";
                for(uint32_t dd=0;dd<frag.size();dd++)
                {
                  std::cout<<frag[dd]<<",";
                }
                std::cout<<std::endl;
              }
            }
            for(uint32_t j= i+1; j<= end; j++)
            {
                if( j-i >= max_phrase_len_ ) continue;
                std::vector<uint32_t> ifrag( termList.begin()+i, termList.begin()+j );
                appendHashItem_(hash_(ifrag));
            }

        }


        incTermCount_(increase);



    }

    inline void appendTermList_(const std::vector<uint32_t>& inputItem)
    {
      pTermListWriter_->append(inputItem);
    }

    void appendHashItem_(hash_t hash_value)
    {
      if( cache_size_ > 0 )
      {
        if( cache_vec_.size() >= cache_size_ )
        {
//           std::cout<<"[FULL]"<<std::endl;
          //output
          for(uint32_t i=0;i<cache_vec_.size();i++)
          {
            pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
          }
          //clean
          cache_vec_.resize(0);
          cache_map_.clear();
        }
        uint32_t* index=  cache_map_.find(hash_value);
        if( index == NULL)
        {
          cache_vec_.push_back( std::make_pair(hash_value, 1) );
          cache_map_.insert( hash_value, (uint32_t)(cache_vec_.size())-1 );
        }
        else
        {
          cache_vec_[*index].second += 1;
        }
      }
      else
      {
        pHashWriter_->append(hash_value);
      }

    }

    void releaseCachedHashItem_()
    {
      if( cache_size_ > 0 )
      {
        for(uint32_t i=0;i<cache_vec_.size();i++)
        {
          pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
        }
        //clean
        cache_vec_.resize(0);
        cache_map_.clear();
      }
    }

    void setDocCount_(uint32_t count)
    {
        doc_count_ = count;
    }

    uint32_t getDocCount_()
    {
        return doc_count_;
    }



    void incTermCount_(uint32_t inc = 1)
    {
       all_term_count_ += inc;
    }

    uint32_t getTermCount_()
    {
        return all_term_count_;
    }

    inline hash_t hash_(const std::vector<uint32_t>& termIdList)
    {
        return izenelib::util::HashFunction<std::vector<uint32_t> >::generateHash64BySer(termIdList);
    }

    inline const bool AstartWithB(const std::vector<uint32_t>& termIdList1, const std::vector<uint32_t>& termIdList2)
    {
        if(termIdList1.size()<termIdList2.size()) return false;
        for(uint32_t i=0;i<termIdList2.size();i++)
        {
            if( termIdList1[i] != termIdList2[i] ) return false;
        }
        return true;
    }

    bool makeKPStr_(const std::vector<uint32_t>& termIdList,
        std::vector<izenelib::util::UString>& strList,
        izenelib::util::UString& result)
    {
        bool b = makeKPStr_(termIdList, strList);
        if(!b) return false;
        b = makeKPStr_(strList, result);
        if(!b) return false;
        return true;
    }

    bool makeKPStr_(const std::vector<uint32_t>& termIdList, std::vector<izenelib::util::UString>& strList)
    {
        if ( termIdList.empty() ) return false;//if empty
        strList.resize(termIdList.size());
        izenelib::util::UString ustr;
        bool bb = true;
        bb = id_manager_->GetStringById(termIdList[0], strList[0]);
        if(!bb)
        {
            std::cout<<"!!!!can not find term id "<<termIdList[0]<<std::endl;
            return false;
        }
        for(std::size_t i=1;i<termIdList.size();i++)
        {
            bb = id_manager_->GetStringById(termIdList[i], strList[i]);
            if(!bb)
            {
                return false;
            }
        }
        return true;
    }

    bool makeKPStr_(const std::vector<izenelib::util::UString>& strList, izenelib::util::UString& result)
    {
        result.clear();
        if(strList.size()>0)
        {
            if( strList[0].length() == 0 ) return false;
            result.append(strList[0]);
            for(std::size_t i=1;i<strList.size();i++)
            {
                if( strList[i].length() == 0 ) return false;
                if( result.isChineseChar( result.length()-1 )
                    && strList[i].isChineseChar(0) )
                {
                }
                else
                {
                    result.push_back(' ');
                }
                result.append(strList[i]);
            }
        }
        return true;
    }

    template <class T>
    bool vec_equal_(const std::vector<T>& v1, const std::vector<T>& v2)
    {
      if(v1.size()!=v2.size()) return false;
      for(uint32_t i=0;i<v1.size();i++)
      {
        if(v1[i]!=v2[i]) return false;
      }
      return true;
    }

    template <class T>
    bool vec_starts_(const std::vector<T>& v1, const std::vector<T>& v2)
    {
      if(v1.size()<v2.size()) return false;
      for(uint32_t i=0;i<v2.size();i++)
      {
        if(v1[i]!=v2[i]) return false;
      }
      return true;
    }

    uint8_t getScore(uint32_t f, uint32_t termCount, double logL)
    {
        double score=sqrt(f> 200 ? 100 : f/2)*(termCount > 4 ? 4: termCount);
        if(logL>=10.0)
        {
            score *= sqrt(logL> 100 ? 100 : logL);
        }
        else
        {
            score *= 5;
        }
        return (uint8_t)(ceil(score)+1);
    }

    void insertKP_(const std::vector<uint32_t>& terms, const std::vector<id2count_t>& docItem
              , uint8_t score ,const std::vector<id2count_t>& leftTermList
              ,const std::vector<id2count_t>& rightTermList)
    {
        std::vector<izenelib::util::UString> strList;
        izenelib::util::UString KPStr;
        bool validKP = makeKPStr_(terms, strList, KPStr);
        if(validKP)
        {
            hash_t hash = hash_(terms);
            kpWriter_->append(hash, KPItem( KPStr, docItem, score, leftTermList, rightTermList));

        }
    }

    void outputKP_(const KPItem& kpItem)
    {
      output_.output(kpItem.text, kpItem.docitem_list, kpItem.docitem_list.size(), kpItem.score , kpItem.lc_list, kpItem.rc_list);
    }



    //with complete termidlist version.
    void getCandidateLabel2_(const std::vector<Data>& data, CandidateSSFType::WriterType* htlWriter, typename Hash2HashSSFType::WriterType* h2hWriter)
    {
        if( data.size()==0 ) return;

        //sorting with postorder
        std::vector<uint32_t> post_order(data.size());
        {
          std::stack<uint32_t> index_stack;
          std::stack<uint32_t> depth_stack;
          uint32_t i=0;
          for( uint32_t m=0;m<data.size();m++)
          {
            uint32_t depth = data[m].inc;
            if(m>0)
            {
              while( !depth_stack.empty() && depth <= depth_stack.top() )
              {
                post_order[i] = index_stack.top();
                ++i;

                index_stack.pop();
                depth_stack.pop();
              }
            }
            index_stack.push(m);
            depth_stack.push(depth);
          }
          while( !index_stack.empty() )
          {
            post_order[i] = index_stack.top();
            ++i;

            index_stack.pop();
            depth_stack.pop();
          }
        }

        std::stack<uint32_t> depth_stack;
        std::stack<uint32_t> freq_stack;
        std::stack<std::vector<id2count_t> > doc_item_stack;
        std::stack<std::vector<id2count_t> > prefix_term_stack;
        std::stack<id2count_t > suffix_term_stack;
        if(!tracing_.empty())
        {
          test_num_++;
        }
        for( uint32_t m=0;m<data.size();m++)
        {
          uint32_t depth = data[post_order[m]].inc;
          std::vector<uint32_t> termIdList = data[post_order[m]].termid_list;
          std::vector<id2count_t> docItemList = data[post_order[m]].docitem_list;
          uint32_t freq = data[post_order[m]].freq;
          std::vector<id2count_t > prefixTermList = data[post_order[m]].lc_list;
          std::vector<id2count_t > suffixTermList;
          if(!tracing_.empty())
          {
            if(vec_starts_(termIdList, tracing_))
            {
              std::cout<<"[tracing] {data} "<<test_num_<<","<<m<<","<<post_order[m]<<","<<depth;
              for(uint32_t dd=0;dd<docItemList.size();dd++)
              {
                std::cout<<"("<<docItemList[dd].first<<"|"<<docItemList[dd].second<<"),";
              }
              std::cout<<std::endl;
            }
          }
          while( !depth_stack.empty() && depth < depth_stack.top() )
          {
            freq+=freq_stack.top();
            docItemList.insert( docItemList.end(), doc_item_stack.top().begin(), doc_item_stack.top().end() );
            prefixTermList.insert( prefixTermList.end(), prefix_term_stack.top().begin(), prefix_term_stack.top().end() );
            //suffix howto?
            //suffixTermList, suffix_term_stack
//             suffixTermList.insert( suffixTermList.end(), suffix_term_stack.top().begin(), suffix_term_stack.top().end() );
            suffixTermList.push_back(suffix_term_stack.top() );
            //pop stack
            depth_stack.pop();
            freq_stack.pop();
            doc_item_stack.pop();
            prefix_term_stack.pop();
            suffix_term_stack.pop();
          }
          depth_stack.push( depth);
          freq_stack.push(freq);
          doc_item_stack.push( docItemList);
          prefix_term_stack.push(prefixTermList);
          //get the suffix term
          uint32_t suffix_termid = termIdList[depth];
          suffix_term_stack.push(std::make_pair(suffix_termid, freq) );

          if( termIdList.size() > max_phrase_len_ ) continue;
//           std::vector<uint32_t> docIdList;
//           std::vector<uint32_t> tfInDocList;
//           idmlib::util::getIdAndCountList(docItemList, docIdList, tfInDocList);
//
//           std::vector<uint32_t> leftTermIdList;
//           std::vector<uint32_t> leftTermCountList;
//           idmlib::util::getIdAndCountList(prefixTermList, leftTermIdList, leftTermCountList);
//
//           std::vector<uint32_t> rightTermIdList;
//           std::vector<uint32_t> rightTermCountList;
//           idmlib::util::getIdAndCountList(suffixTermList, rightTermIdList, rightTermCountList);

          idmlib::util::accumulateList(docItemList);
          idmlib::util::accumulateList(prefixTermList);
          idmlib::util::accumulateList(suffixTermList);
          SI lri(termIdList, freq);
          SCI leftLC( prefixTermList);
          SCI rightLC( suffixTermList);
          if(!tracing_.empty())
          {
            if(vec_equal_(termIdList, tracing_))
            {
              std::cout<<"[tracing] freq: "<<freq<<std::endl;
              for(uint32_t dd=0;dd<docItemList.size();dd++)
              {
                std::cout<<"[tracing] doc item: "<<docItemList[dd].first<<","<<docItemList[dd].second<<"]"<<std::endl;
              }
            }
          }
          if(manmade_.find(termIdList)!=NULL)
          {
            insertKP_( termIdList, docItemList, freq, prefixTermList, suffixTermList );
            continue;
          }
          int status = KPStatus::CANDIDATE;
          if( freq<min_freq_threshold_ ) status = KPStatus::NON_KP;
          else if( docItemList.size() < min_df_threshold_ ) status = KPStatus::NON_KP;
          else
          {
              status = scorer_->prefixTest(termIdList);
          }

          if( status == KPStatus::NON_KP || status == KPStatus::RETURN )
          {
              continue;
          }
          if( status == KPStatus::KP )
          {
              insertKP_( termIdList, docItemList, freq, prefixTermList, suffixTermList );
              continue;
          }
//           std::pair<bool, double> lcdResult = scorer_->test(lri, leftLC);
//           if( !lcdResult.first ) continue;
//           std::pair<bool, double> rcdResult = scorer_->test(lri, rightLC);
//           if( !rcdResult.first ) continue;

          //debug output
//           std::cout<<lri.ToString(id_manager_)<<std::endl;
//           std::cout<<leftLC.ToString(id_manager_)<<std::endl;
//           std::cout<<rightLC.ToString(id_manager_)<<std::endl;
          std::pair<bool, double> scorer_result = scorer_->test(lri, leftLC, rightLC);
          if( !scorer_result.first ) continue;
          CandidateItem htList(termIdList, docItemList, freq, prefixTermList,suffixTermList );
          hash_t hashId = hash_(termIdList);
          htlWriter->append(hashId , htList );
          if( termIdList.size() >= 2 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.end()-1);
                  Hash2HashItem item(hashId, 1);
                  h2hWriter->append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.begin()+1, termIdList.end());
                  Hash2HashItem item(hashId, 2);
                  h2hWriter->append( hash_(vec), item);
              }
          }
          if( termIdList.size() >= 3 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.begin()+1);
                  Hash2HashItem item(hashId, 3);
                  h2hWriter->append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.end()-1, termIdList.end());
                  Hash2HashItem item(hashId, 4);
                  h2hWriter->append( hash_(vec), item);
              }
          }

        }



    }


private:
  const std::string dir_;
  const std::string id_dir_;
  const std::string tmp_dir_;
  const std::string kp_dir_;
  idmlib::util::IDMAnalyzer* analyzer_;
  idmlib::util::IDMIdManager* id_manager_;
  bool outside_idmanager_;
  OutputType output_;
  uint8_t max_phrase_len_;

  TermListWriter* pTermListWriter_;
  HashWriter* pHashWriter_;
  KPSSFType::WriterType* kpWriter_;
  uint32_t cache_size_;
  izenelib::am::rde_hash<hash_t, uint32_t> cache_map_;
  std::vector< std::pair<hash_t, uint32_t> > cache_vec_;

  ScorerType* scorer_;
  bool outside_scorer_;

  uint32_t last_doc_id_;
  uint32_t doc_count_;
  uint32_t all_term_count_;

  uint32_t min_freq_threshold_;
  uint32_t min_df_threshold_;
  uint32_t try_compute_num_;
  bool no_freq_limit_;
  izenelib::am::rde_hash<std::vector<uint32_t>, int> manmade_;
  std::vector<uint32_t> tracing_;
  uint32_t test_num_;


};

NS_IDMLIB_KPE_END

#endif
