/*
 * NameEntityManager.h
 *
 *  Created on: Apr 9, 2010
 *      Author: eric
 */

#ifndef NAMEENTITYMANAGER_H_
#define NAMEENTITYMANAGER_H_

#include "NameEntity.h"
#include <ml/ClassificationManager.h>

namespace idmlib
{

class NameEntityManager
{
public:
	NameEntityManager(std::string& path);

	void loadModels();

	void train(std::vector<NameEntity>& entities);
	void predict(NameEntity& entity);
	void predict(std::vector<NameEntity>& entities);


private:
	static ml::ClassificationManager<NameEntity>* classifier_;
	std::string path_;

};

}
#endif /* NAMEENTITYMANAGER_H_ */