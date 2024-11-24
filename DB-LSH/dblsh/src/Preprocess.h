#pragma once
#include "StructType.h"
#include <cmath>
#include <assert.h>
#include <unordered_map>

class Preprocess
{
public:
	Data data;
	float* SquareLen = NULL;
	float** Dists = NULL;
	Ben benchmark;
	std::string data_file;
	std::string ben_file;
	bool hasT = false;
	float beta = 0.1f;
	
public:
	Preprocess(const std::string& path, const std::string& ben_file_);
	Preprocess(const std::string& path, const std::string& ben_file_, float beta_);
	void load_data(const std::string& path);
	void ben_make();
	void ben_save();
	void ben_load();
	void ben_create();
	~Preprocess();
};

struct Dist_id
{
	unsigned id = 0;
	float dist = 0;
	bool operator < (const Dist_id& rhs) {
		return dist < rhs.dist;
	}
};

class Parameter //N,dim,S, L, K, M, W;
{
public:
	unsigned N = 0;//数据个数
	unsigned dim = 0;//数据向量维数
	// Number of hash functions
	unsigned S = 0;
	//#L Tables; 
	unsigned L = 0;
	// Dimension of the hash table
	unsigned K = 0;

	int MaxSize = 0;//恒为5

	float R_min = 1.0f;

	Parameter(Preprocess& prep, unsigned L_, unsigned K_, float rmin_);
	~Parameter();
};

