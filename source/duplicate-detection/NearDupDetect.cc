/*
 * NearDupDetect.cpp
 *
 *  Created on: 2008-12-19
 *      Author: jinglei
 */

#include <idmlib/duplicate-detection/NearDupDetect.h>

#include <wiselib/StopWatch.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <vector>
#include <algorithm>
#include <utility>
using namespace std;

namespace sf1v5{

/********************************************************************************
  Description: NearDuplicateClustering clusters NearDuplicateSignature objects
                 in near-optimum way without doing exhaustive pari-wise comparison
                 of all the objects in the pool. The exhaustive approach is
                 prohibitive since it will be nC2 comparisons. Instead, it firsts
                 hashes all the objects into rough clusters where all the objects
                 share at least the same one-word bitstring. In each one-word
                 pool, an exhaustive pair-wise comparison is done to find
                 near-duplicate clusters. Assuming the total bitstring size is M*32,
                 this algorithm finds all the near-duplicates with up to (M-1)
                 differences in bits.
               According to Monika Henzinger paper, M value of 12 is used over 1.6B
                 web pages. So unless one needs to handle more than 2B documents,
                 M value of 12 seems sufficient.
               This class has only two methods. One to add near-duplicate objects
                 and the other is to collect results.
  Comments   :
  History    : Yeogirl Yun                                      1/16/07
                 Initial Revision
********************************************************************************/

/********************************************************************************
  Description: Returns true iff the document pair has already been compared.
  Comments   :
********************************************************************************/
const int DBdegree=16;
const char* indexFile = "dupdb.dat";
//SDB NearDupDetect::sdb(DBdegree,indexFile);

bool NearDupDetect::has_already_compared(int docID1, int docID2)
{
    uint64_t key;
    // construct a unique key given two docIDs.
    if (docID1 > docID2) {
        key = docID1;
        key <<= 32;
        key += docID2;
    } else {
        key = docID2;
        key <<= 32;
        key += docID1;
    }

    if (pairTable.find(key)) {
        return true;
    } else {
        pairTable.insert(key);
        return false;
    }
}


void NearDupDetect::add_near_duplicate_signatures(const std::string& docName, wiselib::CBitArray& bitArray) {
    wiselib::DynamicArray<NearDuplicateSignature> sigSet;
    // generate NearDuplicateSignature objects from this bit pattern
/*
    NearDuplicateSignature::
        generate_near_duplicate_signatures(ndAlgo.num_dimensions(),
                bitArray,
                docName,
                sigSet);
   for (int i = 0; i < sigSet.size(); i++) {
	   //mKeySet.insert(sigSet[i].get_sub_key().key);
      //mMultiMap.insert(make_pair(sigSet[i].get_sub_key().key,make_pair(sigSet[i].get_doc_id(),sigSet[i])));
    }
*/
}


/********************************************************************************
  Description: Scans all the one-word clusters and compute near-duplicate socres
                 and its pairs into results after sorting.
               It uses BPTree class to do an efficient sorting among NearDuplicatePair
                 objects.
  Comments   :
********************************************************************************/
void NearDupDetect::
find_near_duplicate_pairs(wiselib::DynamicArray<NearDuplicatePair>& results)
{
//    typedef MULTI_MAP_T::iterator CIT;
//    typedef pair<CIT,CIT> RANGE;
//    int count=1;
//    for(std::set<ub4>::iterator iterKey=mKeySet.begin();iterKey!=mKeySet.end();iterKey++,count++)
//    {
//    	if(count%512==0) cout<<".";
//    	if(count%5120==0) cout<<endl;
//    	RANGE range=mMultiMap.equal_range(*iterKey);
//     	for(CIT iter1=range.first;iter1!=range.second;iter1++)
//    	{
//    		CIT iter2=iter1;
//    		++iter2;
//    		for(;iter2!=range.second;iter2++)
//    	    {
//    		    if (!has_already_compared(iter1->second.first, iter2->second.first))
//    		    {
//    			    NearDuplicatePair ndPair;
//    		       ndPair.sigFirst = &(iter1->second.second);
//    		       ndPair.sigSecond = &(iter2->second.second);
//    		       //ndPair.neardupScore=0;
//    		       ndPair.neardupScore = ndAlgo.neardup_score(iter1->second.second, iter2->second.second);
//    		       if(ndAlgo.is_neardup(ndPair.neardupScore.key))
//    		    	   results.push_back(ndPair);
//    		    }
//
//    	    }
//    	}
//    }
}



/********************************************************************************
  Description: Tests a near duplicate algorithm by analyzing the documents
                 and shows the near duplicate pairs and clusters to standard out.
  Comments   :
********************************************************************************/
void NearDupDetect::test_near_duplicate_algorithm(const wiselib::DynamicArray<wiselib::YString>& docNames,
							    const wiselib::DynamicArray<wiselib::YString>& documents)
{
    // now generate near duplicate signatures from the documents.
  wiselib::StopWatch sw1;
	cout<<"Generate near duplicate signatures from the documents."<<endl;
    for (int i = 0; i < documents.size(); i++) {
        wiselib::CBitArray bitArray;
        ndAlgo.generate_document_signature(documents[i], bitArray);
        add_near_duplicate_signatures(docNames[i], bitArray);
        if(i%512==0) cout<<".";
        if(i%5120==0) cout<<endl;
     }
    cout<<"sketching time:"<<endl;
    sw1.show();
    wiselib::StopWatch sw2;
    // now find the near duplicate clustering resutls.
    cout<<"Find the near duplicate pairs."<<endl;
    wiselib::DynamicArray<NearDuplicatePair> results;
    find_near_duplicate_pairs(results);
    cout<<"Now, sort the duplicate pairs!"<<endl;
    results.sort();

    ofstream fPairScore("pair_score.res");
    ofstream fClusters("cluster.res");

    // from the max to min neardup scores, print the pairs.
    int cnt=0;
    fPairScore << "NearDuplicateClustering:: showing most near-duplicate pairs..." << endl;
    for (int i = results.high(); i >= 0; i--) {
        fPairScore << setw(10) << cnt++ << "th pair: " << results[i] << endl;
    }
    cout << endl;
    cout<<"FINDING TIME:"<<endl;
    sw2.show();


    // now find connected components. anything above threshold
    // match is considerd to be an edge.
#if(0)
    cout<<"Find the near duplicate clusters."<<endl;
    using namespace boost;
    cout << "NearDuplicateClustering:: showing near-duplicate clusters..." << endl;
    typedef adjacency_list <vecS, vecS, undirectedS> Graph;
    Graph G;

    for (int i = results.high(); i >= 0; i--) {
        const NearDuplicatePair& pair = results[i];
        if (ndAlgo.is_neardup(pair.neardupScore.key)) {
            add_edge(pair.sigFirst->get_doc_id(), pair.sigSecond->get_doc_id(), G);
        }
    }

    std::vector<int> component(num_vertices(G));
    int num = connected_components(G, &component[0]);

    fClusters << "Total number of clusters: " << num << endl;
    for (vector<int>::size_type i = 0; i != component.size(); ++i)
        fClusters << "Document <" << NearDuplicateSignature::get_doc_string(i)
            << "> is in cluster {" << component[i] << "}" << endl;
    fClusters << endl;
#endif
}

}