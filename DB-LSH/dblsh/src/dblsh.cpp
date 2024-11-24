#include "dblsh.h"
#include "Preprocess.h"
#include "basis.h"
#include <fstream>
#include <assert.h>
#include <random>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <sstream>
#include <numeric>
#include "Config.h"

Hash::Hash(Preprocess& prep_, Parameter& param_,
	const std::string& file)
{
	N = param_.N;//数据数
	dim = param_.dim;//数据维数
	L = param_.L;//表数
	K = param_.K;//哈希函数个数
	S = L * K;
	R_min = param_.R_min;


	MaxSize = param_.MaxSize;
	std::cout << std::endl << "START HASHING..." << std::endl << std::endl;
	lsh::timer timer;

	std::cout << "SETTING HASH PARAMETER..." << std::endl;
	timer.restart();
	SetHash();//将哈希函数用标准正态分布赋值
	std::cout << "SETTING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;
	//在此处可以再加一个函数，使用训练的参数代替标准正态分布初始化
	if (!Config::trainedWFile.empty()){
		timer.restart();
		LoadHash(Config::trainedWFile);
		std::cout << "LOADING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;
	}
	std::cout << "COMPUTING HASH..." << std::endl;
	timer.restart();
	GetHash(prep_);
	std::cout << "COMPUTING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;

	std::cout << "BUILDING INDEX..." << std::endl;
	std::cout << "THERE ARE " << L << " " << K << "-D HASH TABLES." << std::endl;
	timer.restart();
	GetTables(prep_);
	std::cout << "BUILDING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;


}

bool Hash::IsBuilt(const std::string& file)
{
	return 0;
}

void Hash::SetHash()//将哈希函数用标准正态分布赋值
{
	hashpar.rndAs1 = new float* [S];
	hashpar.rndAs2 = new float* [S];


	for (unsigned i = 0; i < S; i++) {
		hashpar.rndAs1[i] = new float[dim];
		hashpar.rndAs2[i] = new float[1];
	}

	std::mt19937 rng(unsigned(0));//这是干嘛？好像是正态分布生成器的种子。
	//std::mt19937 rng(unsigned(std::time(0)));
	std::normal_distribution<float> nd;//nd(rng)返回一个标准正态分布采样值
	for (unsigned j = 0; j < S; j++)
	{
		for (unsigned i = 0; i < dim; i++)
		{
			hashpar.rndAs1[j][i] = (nd(rng));
		}
		for (unsigned i = 0; i < 1; i++)
		{
			hashpar.rndAs2[j][i] = (nd(rng));
		}
	}
}

void Hash::LoadHash(const std::string& path){
	std::string file;
	for(unsigned k = 0; k < L; k++){
		file = path + std::to_string(k) + ".bin";
		std::cout<<"Load from new file: "<<file<<std::endl;
		std::ifstream in(file.c_str(), std::ios::binary);
		while (!in) {
			printf("Fail to find data file!\n");
			exit(0);
		}
		for(unsigned j = 0; j < K; j++){
			in.read((char*)hashpar.rndAs1[k * K + j], sizeof(float) * dim);
			for(unsigned i = 0; i < dim; i++){
				hashpar.rndAs1[k * K + j][i] *= Config::gamma;
			}
		}
		in.close();
	}
}

void Hash::GetHash(Preprocess& prep)//和RS Tree的构造有关
{
	showMemoryInfo();//这是一个空的函数
	hs.resize(L);//std::vector::resize(n)，将该向量的元素个数变为n
	for (int i = 0; i < L; ++i) {
		hs[i] = new (TreeDataP<float>*[N]);//和RS tree类，具体没细看
	}
	showMemoryInfo();
	for (int i = 0; i < L; ++i) {//第i章表
		for (int j = 0; j < N; ++j) {//第j个数据
			hs[i][j] = new TreeDataP<float>(K);
			hs[i][j]->id = j;
			for (int k = 0; k < K; ++k) {//第k个哈希函数
				//映射的数据直接用于构造RS树，因此不好直接将通过训练的方式得到的投影后的数据在此处复制。好像也可以，这里只是复制没有构造树的结构。
				hs[i][j]->data[k] = cal_inner_product(prep.data.val[j], hashpar.rndAs1[i * K + k], dim);//内积
			}
		}
	}
	showMemoryInfo();
}

void Hash::GetTables(Preprocess& prep)
{

	//the memory policy of page file
	int page_len = 4096;
	if (N < 70000) {
		page_len = 4096;
	}
	else if (N >= 70000 && N < 500000) {
		//200-501
		page_len = 8192;
	}
	else if (N >= 500000 && N < 2000000) {
		//501-1000
		page_len = 8192 * 1;
	}
	else if (N >= 2000000 && N < 2000000000) {
		//1000-20000
		page_len = 8192 * 2;
	}
	else if (N >= 2000000000) {
		//1000-20000
		page_len = 8192 * 2;
	}


	PageFile::c_policy pols[] = { PageFile::C_FULLMEM, PageFile::C_LRU, PageFile::C_MRU, PageFile::C_NO_CACHE };
	int pagefile_cache_size = 0; //we use full memory
	bool force_new = true; //if file exists, we will force overwrite it

	myIndexes = new RStarTree<TreeDataP, float>*[L];

	
	if (N > 1) {
		lsh::timer timer;
//#pragma omp parallel for
		for (int i = 0; i < L; ++i) {
			timer.restart();
			//n1[0] = i + '0';
			std::string file = "RStar_index_file//" + std::to_string(i) + "_rstar.rt";
			myIndexes[i] = new RStarTree<TreeDataP, float>(file.c_str(), K, page_len, ".",
				pols[0], pagefile_cache_size, force_new);

			//construct tree using STR bulkloading
			myIndexes[i]->bulkload_str(hs[i], N, 0.7);
			

			for (int j = 0; j < N; ++j) {
				delete[] hs[i][j]->data;
				hs[i][j]->data = NULL;
				delete hs[i][j];
			}
			//clear_2d_array(hs[i], N);
			delete[] hs[i];
			hs[i] = NULL;
			printf("The %d-th R*-Tree has been built. Elapsed time: %.3fs\n", i, timer.elapsed());

			//showMemoryInfo();
		}
	}
	else {
		lsh::progress_display pd(L * N);
		for (int i = 0; i < L; ++i) {
			//n1[0] = i + '0';
			std::string file = "RStar_index_file//" + std::to_string(i) + "_rstar.rt";
			myIndexes[i] = new RStarTree<TreeDataP, float>(file.c_str(), K, page_len, ".",
				pols[0], pagefile_cache_size, force_new);

			//insert one by one
			for (int j = 0; j < N; ++j) {
				myIndexes[i]->insert(hs[i][j]);
				++pd;
			}
			for (int j = 0; j < N; ++j) {
				delete[] hs[i][j]->data;
				hs[i][j]->data = NULL;
				delete hs[i][j];
			}
			//clear_2d_array(hs[i], N);
			delete[] hs[i];
			hs[i] = NULL;
		}
	}
}

Hash::~Hash()
{
	clear_2d_array(hashpar.rndAs1, S);
	clear_2d_array(hashpar.rndAs2, S);
	for (int i = 0; i < L; ++i) {
		delete myIndexes[i];
	}
	delete myIndexes;
}