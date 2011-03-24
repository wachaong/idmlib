#include <idmlib/semantic_space/semantic_space_builder.h>

using namespace idmlib::ssp;

bool SemanticSpaceBuilder::Build()
{
	std::vector<std::string> scdFileList;
	if (!getScdFileList(scdPath_, scdFileList)) {
		return false;
	}

	// doc_terms_map docTermsMap;
	term_vector termVec;
	docid_t docid = 0;
	docid_t last_docid = 0;
	docid_t doc_count = 0;

	// process all SCD files
	for (size_t i = 0; i < scdFileList.size(); i++)
	{
		//std::cout << scdFileList[i] << std::endl;
		std::string scdFile = scdFileList[i];
		izenelib::util::ScdParser scdParser(encoding_);
		if(!scdParser.load(scdFile) )
		{
		  DLOG(WARNING) << "Load scd file failed: " << scdFile << std::endl;
		  return false;
		}

		// parse SCD
		izenelib::util::ScdParser::iterator iter = scdParser.begin();
		for ( ; iter != scdParser.end(); iter ++)
		{
			if ( (doc_count++) >= maxDoc_ )
				break;

			izenelib::util::SCDDocPtr pDoc = *iter;

			doc_properties_iterator proIter;
			for (proIter = pDoc->begin(); proIter != pDoc->end(); proIter ++)
			{
				izenelib::util::UString propertyName = proIter->first;
				const izenelib::util::UString & propertyValue = proIter->second;
				propertyName.toLowerString();
				if ( propertyName == izenelib::util::UString("docid", encoding_) ) {
					// collection should be indexed (using index data)
					last_docid = docid;
					bool ret = pIdManager_->getDocIdByDocName(propertyValue, docid, false);
					if (!ret) {
						DLOG(WARNING) << "Document (" << propertyValue << ") does not existed: !" << std::endl;
						return false;
					}
					//std::cout << endl << "docid: " << la::to_utf8(propertyValue) << " => " << docid << endl;
				}
				else if ( propertyName == izenelib::util::UString("title", encoding_) ) {
					//std::cout << la::to_utf8(proIter->second) << std::endl;
				}
				else if ( propertyName == izenelib::util::UString("content", encoding_)) {
					// process raw content
					termVec.clear();
					getDocTerms(propertyValue, termVec);
				}
			}

			// check docid
			if (docid == last_docid) {
				DLOG(WARNING) << "Duplicated docid: " << docid << "(Check SCD properties!)" << std::endl;
				return false;
			}

			// docTermsMap.clear();
			//docTermsMap[docid] = termVec;
			//docTermsMap.insert(make_pair(docid, termVec));
			pSSpace_->processDocument(docid, termVec);
		}

		if ( doc_count >= maxDoc_ )
			break;
	}

	std::cout << "process Space..." << std::endl;
	time_t time1, time2;
	time1 = time (NULL);
	pSSpace_->processSpace();
	time2 = time (NULL);
	std::cout << "time elapsed: " << (time2-time1) << std::endl;

	return true;
}

/// Private ////////////////////////////////////////////////////////////////////

bool SemanticSpaceBuilder::getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec)
{
	//termIdList_.clear();
	//pLA_->process(pIdManager_.get(), ustrDoc, termIdList_);

	termList_.clear();
	//pLA_->process(ustrDoc, termList_);
	pIdmAnalyzer_->GetTermList(ustrDoc, termList_, false);

	termid_t termid;
	for ( la::TermList::iterator iter = termList_.begin(); iter != termList_.end(); iter++ )
	{
		//if ( filter(iter->text_) )
		//	continue;

		boost::shared_ptr<sTermUnit> pTerm(new sTermUnit());
		pIdManager_->getTermIdByTermString(iter->text_, termid);
		pTerm->termid = termid;
		termVec.push_back(pTerm);

		//cout << iter->textString()  << "(" << termid << ") "; //
	}

	return true;
}

bool SemanticSpaceBuilder::getScdFileList(const std::string& scdDir, std::vector<std::string>& fileList)
{
	if ( exists(scdDir) )
	{
		if ( !is_directory(scdDir) ) {
			std::cout << "It's not a directory: " << scdDir << std::endl;
			return false;
		}

		directory_iterator iterEnd;
		for (directory_iterator iter(scdDir); iter != iterEnd; iter ++)
		{
			std::string file_name = iter->path().filename();
			//std::cout << file_name << endl;

			if (izenelib::util::ScdParser::checkSCDFormat(file_name) )
			{
				izenelib::util::SCD_TYPE scd_type = izenelib::util::ScdParser::checkSCDType(file_name);
				if( scd_type == izenelib::util::INSERT_SCD ||scd_type == izenelib::util::UPDATE_SCD )
				{
					fileList.push_back( iter->path().string() );
				}
			}
		}

		if (fileList.size() > 0) {
			return true;
		}
		else {
			std::cout << "There is no scd file in: " << scdDir << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "File path dose not existed: " << scdDir << std::endl;
		return false;
	}
}