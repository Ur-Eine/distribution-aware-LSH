#include "E2LSH.h"
#include <fstream>

void E2LSH::constructTable(Config& config)
{
	if(config.trainedWFile == "None")constructParam(config);
	else loadParam(config);
}

vector<DATATYPE> E2LSH::getHashVal(const vector<DATATYPE>& domin, Config& config) const
{
	vector<DATATYPE> sum;
	vector<DATATYPE> hashVal;
	sum.resize(config.lowDim);
	for (unsigned i = 0; i < config.lowDim; ++i){
		for (unsigned j = 0; j < domin.size(); ++j){
			sum[i] += domin[j] * hash_a[i][j];
		}
		hashVal.emplace_back(sum[i]);
	}
	return hashVal;
}

vector<DATATYPE> E2LSH::getHashVal2(const vector<DATATYPE>& domin, Config& config) const{
	vector<DATATYPE> sum2;
	vector<DATATYPE> hashVal2;
	sum2.resize(config.highDim - config.lowDim);
	for (unsigned i = 0; i < (config.highDim - config.lowDim); ++i){
		for (unsigned j = 0; j < domin.size(); ++j){
			sum2[i] += domin[j] * hash_b[i][j];
		}
		hashVal2.emplace_back(sum2[i]);
	}
	return hashVal2;
}

void E2LSH::constructParam(Config& config)
{
	mt19937 rng(unsigned(time(0)));
	hash_a.resize(config.lowDim);

	std::normal_distribution<float> nd;
	int dim = config.highDim;

	for (vector<vector<DATATYPE> >::iterator iter = hash_a.begin(); iter != hash_a.end(); ++iter){
		for (unsigned i = 0; i < dim; ++i){
			iter->emplace_back(nd(rng));
		}
	}	
}

void E2LSH::loadParam(Config& config)
{
	std::string file = config.trainedWFile;
	std::ifstream in(file.c_str(), std::ios::binary);
	while (!in) {
		printf("Fail to find data file!\n");
		exit(0);
	}
	hash_a.resize(config.lowDim);
	int dim = config.highDim;

	float** rndAs1;
	float** rndAs2;  
	rndAs1 = new float* [config.lowDim];
	for (unsigned i = 0; i < config.lowDim; i++) {
		rndAs1[i] = new float[dim];
		in.read((char*)rndAs1[i], sizeof(float) * dim);
	}
	if(config.updateDist){
		rndAs2 = new float* [config.highDim - config.lowDim];
		for (unsigned i = 0; i < (config.highDim - config.lowDim); i++) {
			rndAs2[i] = new float[dim];
			in.read((char*)rndAs2[i], sizeof(float) * dim);
		}
	}
	in.close();

	for (unsigned i = 0; i < config.lowDim; i++){
		for (unsigned j = 0; j < dim; j++){
			hash_a[i].push_back(rndAs1[i][j]);
		}
	}
	if(config.updateDist){
		hash_b.resize(config.highDim - config.lowDim);
		for (unsigned i = 0; i < (config.highDim - config.lowDim); i++){
			for (unsigned j = 0; j < dim; j++){
				hash_b[i].push_back(rndAs2[i][j]);
			}
		}
	}
}