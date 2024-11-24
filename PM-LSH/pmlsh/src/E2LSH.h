#pragma once
#include<vector>
#include<assert.h>
#include<map>
#include <random>
#include <iostream>
#include <functional>
#include <ctime>
#include "Config.h"
using namespace std;

class E2LSH
{
public:
	E2LSH() {}
	~E2LSH(){}
	void constructTable(Config & config);
	vector<DATATYPE> getHashVal(const vector<DATATYPE>& domin, Config& config) const;
	vector<DATATYPE> getHashVal2(const vector<DATATYPE>& domin, Config& config) const;
private:
	void constructParam(Config& config);
	void loadParam(Config& config);
	vector<vector<DATATYPE>>hash_a;  //a*o
	vector<vector<DATATYPE>>hash_b;  //Hash function for the remaining dimensions
};

