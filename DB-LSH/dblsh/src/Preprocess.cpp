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
#include<algorithm>

#define E 2.718281746
#define PI 3.1415926

#define min(a,b)            (((a) < (b)) ? (a) : (b))

Preprocess::Preprocess(const std::string& path, const std::string& ben_file_)
{
	lsh::timer timer;
	std::cout << "LOADING DATA..." << std::endl;
	timer.restart();
	load_data(path);//用于读取数据文件并写入Preprocess.data中
	std::cout << "LOADING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;

	data_file = path;
	ben_file = ben_file_;
	if (data.N > 500) {//数据量大于500时，创建audio.bench
		ben_create();
	}
}

Preprocess::Preprocess(const std::string& path, const std::string& ben_file_, float beta_)
{

	hasT = true;
	beta = beta_;
	lsh::timer timer;
	std::cout << "LOADING DATA..." << std::endl;
	timer.restart();
	load_data(path);
	std::cout << "LOADING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;

	data_file = path;
	ben_file = ben_file_;//为什么这里又改成1000了？
	if (data.N > 1000) {
		ben_create();
	}
}

void Preprocess::load_data(const std::string& path)
{
	std::string file = path + "_new";
	std::ifstream in(file.c_str(), std::ios::binary);//std::ifstream的构造函数，指定文件路径和读取形式。ios_base::binary是以二进制方式打开文件
	while (!in) {
		printf("Fail to find data file!\n");
		exit(0);
	}

	unsigned int header[3] = {};
	assert(sizeof header == 3 * 4);
	in.read((char*)header, sizeof(header));
	assert(header[0] == sizeof(float));//这三行也确定了数据集该有的格式
	data.N = header[1];//总数据量
	data.dim = header[2];//数据维度

	data.val = new float* [data.N];
	for (int i = 0; i < data.N; ++i) {
		data.val[i] = new float[data.dim];
		//in.seekg(sizeof(float), std::ios::cur);
		in.read((char*)data.val[i], sizeof(float) * header[2]);
	}

	int MaxQueryNum = min(200, (int)data.N - 1);//分200个数据给data.query
	data.query = data.val;
	data.val = &(data.query[MaxQueryNum]);//地址更新从第MaxQueryNum个数据开始，前MaxQueryNum个数据相当与给了data.query
	data.N -= MaxQueryNum;//数据总量也减去MaxQueryNum

	std::cout << "Load from new file: " << file << "\n";
	std::cout << "N=    " << data.N << "\n";
	std::cout << "dim=  " << data.dim << "\n\n";

	in.close();
}

//void Preprocess::load_data(const std::string& path)
//{
//	std::string file = path + "_all";
//	std::ifstream is_new(file.c_str(), std::ios::binary);
//	unsigned header[3] = { 0 };
//	if (!is_new) {
//		std::ifstream is(path.c_str(), std::ios::binary);
//		if (!is) {
//			std::cout << "Fail to open the file:\n" << path << "!\n";
//			exit(-10086);
//		}
//
//		assert(sizeof header == 3 * 4);
//		is.read((char*)header, sizeof(header));
//		assert(header[1] != 0);
//		float* array_ = new float[header[2] * header[1]];
//		is.read((char*)&array_[0], sizeof(float) * header[2] * header[1]);
//		data.N = header[1];
//		//data.N = 100;
//		data.dim = header[2];
//		//Eigen::Map<Eigen::MatrixXf> data_(array_, header[1], header[2]);
//		//data.val = data_;
//		is.close();
//
//		data.val = new float* [data.N];
//		for (int i = 0; i < data.N; ++i) {
//			data.val[i] = new float[data.dim];
//			for (int j = 0; j < data.dim; ++j) {
//				data.val[i][j] = array_[j * data.N + i];
//			}
//		}
//		delete[] array_;
//
//		/*******************Restore the data by other approach******************/
//
//		int MaxQueryNum = std::min(200, (int)data.N - 1);
//		data.query = data.val;
//		data.val = &(data.query[200]);
//		data.N -= MaxQueryNum;
//
//		std::ofstream out(file.c_str(), std::ios::binary);
//		//out.write((char*)header, sizeof(header));
//		for (int i = 0; i < data.N; ++i) {
//			out.write((char*)&i, sizeof(int));
//			out.write((char*)data.val[i], sizeof(float) * header[2]);
//		}
//
//		out.close();
//
//		file.append("_query");
//		std::ofstream outq(file.c_str(), std::ios::binary);
//		//out.write((char*)header, sizeof(header));
//		for (int i = 0; i < MaxQueryNum; ++i) {
//			outq.write((char*)&i, sizeof(int));
//			outq.write((char*)data.query[i], sizeof(float) * header[2]);
//		}
//
//		outq.close();
//
//		exit(10010);
//	}
//	else {
//		file = path + "_new";
//		std::ifstream in(file.c_str(), std::ios::binary);
//		assert(sizeof header == 3 * 4);
//		in.read((char*)header, sizeof(header));
//		assert(header[1] != 0);
//		//float* array_ = new float[header[2] * header[1]];
//		//is.read((char*)&array_[0], sizeof(float) * header[2] * header[1]);
//		data.N = header[1];
//		//data.N = 100;
//		data.dim = header[2];
//		//Eigen::Map<Eigen::MatrixXf> data_(array_, header[1], header[2]);
//		//data.val = data_;
//		//is.close();
//
//		data.val = new float* [data.N];
//		for (int i = 0; i < data.N; ++i) {
//			data.val[i] = new float[data.dim];
//			//in.seekg(sizeof(float), std::ios::cur);
//			in.read((char*)data.val[i], sizeof(float) * header[2]);
//		}
//
//		int MaxQueryNum = std::min(200, (int)data.N - 1);
//		data.query = data.val;
//		data.val = &(data.query[200]);
//		data.N -= MaxQueryNum;
//
//		std::cout << "Load from new file:\n" << file << "****, " << header[2] << "!\n";
//
//		//for (int i = 0; i < data.N; ++i)
//
//		in.close();
//
//	}
//}

struct Tuple
{
	unsigned id;
	float dist;
};

bool comp(const Tuple& a, const Tuple& b)
{
	return a.dist < b.dist;
}

void Preprocess::ben_make()
{
	int MaxQueryNum = min(200, (int)data.N - 201);//这里减或者加多少都没用，信息已经损失了
	benchmark.N = MaxQueryNum, benchmark.num = 100;
	benchmark.indice = new int* [benchmark.N];
	benchmark.dist = new float* [benchmark.N];
	for (unsigned j = 0; j < benchmark.N; j++) {
		benchmark.indice[j] = new int[benchmark.num];
		benchmark.dist[j] = new float[benchmark.num];
	}

	lsh::progress_display pd(benchmark.N);//好像是用于展示过程。差不多，是用于打印进度条的表头。
	for (unsigned j = 0; j < benchmark.N; j++)
	{
		std::vector<Tuple> dists(data.N);

		for (unsigned i = 0; i < data.N; i++)
		{
			dists[i].id = i;
			dists[i].dist = cal_dist(data.val[i], data.query[j], data.dim);//计算原始空间中数据的欧氏距离，所有数据和划分出去的查询的距离
		}

		sort(dists.begin(), dists.end(), comp);//升序排序
		for (unsigned i = 0; i < benchmark.num; i++)
		{
			benchmark.indice[j][i] = (int)dists[i].id;//所有查询的最近的benchmark.num(100)个点的id。
			benchmark.dist[j][i] = dists[i].dist;//所有查询的最近的benchmark.num(100)个点的距离。
		}
		++pd;
	}

	//clear_2d_array(Coe_Mat, data.N);
}



void Preprocess::ben_save()//由此可知bench的结构：benchmark.N-->benchmark.num-->所有查询最邻近点的下标-->所有查询最邻近点的欧氏距离
{
	std::ofstream out(ben_file.c_str(), std::ios::binary);
	out.write((char*)&benchmark.N, sizeof(unsigned));
	out.write((char*)&benchmark.num, sizeof(unsigned));

	for (unsigned j = 0; j < benchmark.N; j++) {
		out.write((char*)&benchmark.indice[j][0], sizeof(int) * benchmark.num);
	}

	for (unsigned j = 0; j < benchmark.N; j++) {
		out.write((char*)&benchmark.dist[j][0], sizeof(float) * benchmark.num);
	}

	out.close();
}

void Preprocess::ben_load()//Preprocess::ben_save()的逆过程
{
	std::ifstream in(ben_file.c_str(), std::ios::binary);
	in.read((char*)&benchmark.N, sizeof(unsigned));
	in.read((char*)&benchmark.num, sizeof(unsigned));

	benchmark.indice = new int* [benchmark.N];
	benchmark.dist = new float* [benchmark.N];
	for (unsigned j = 0; j < benchmark.N; j++) {
		benchmark.indice[j] = new int[benchmark.num];
		in.read((char*)&benchmark.indice[j][0], sizeof(int) * benchmark.num);
	}

	for (unsigned j = 0; j < benchmark.N; j++) {
		benchmark.dist[j] = new float[benchmark.num];
		in.read((char*)&benchmark.dist[j][0], sizeof(float) * benchmark.num);
	}
	in.close();
}

void Preprocess::ben_create()
{
	unsigned a_test = data.N + 1;
	lsh::timer timer;
	std::ifstream in(ben_file.c_str(), std::ios::binary);
	in.read((char*)&a_test, sizeof(unsigned));//那定义时赋值有啥意义呢？答：测试a_test的值发现，当audio.bench不存在时，此处in.read()不改变a_test的值，a_test = data.N + 1，则后续代码执行else，创建audio.bench；当audio.bench存在时，此处in.read()改变a_test的值，改变的值符合后续if条件执行if，读取audio.bench。
	in.close();
	if (a_test > 0 && a_test < data.N)//�ж��Ƿ��ܸ�дa_test
	{
		std::cout << "LOADING BENMARK..." << std::endl;
		timer.restart();
		ben_load();
		std::cout << "LOADING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;

	}
	else
	{
		std::cout << "MAKING BENMARK..." << std::endl;
		timer.restart();
		ben_make();
		std::cout << "MAKING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;

		std::cout << "SAVING BENMARK..." << std::endl;
		timer.restart();
		ben_save();
		std::cout << "SAVING TIME: " << timer.elapsed() << "s." << std::endl << std::endl;

	}
}

Preprocess::~Preprocess()
{
	int MaxQueryNum = min(200, (int)data.N - 201);

	clear_2d_array(data.query, data.N + MaxQueryNum);
	//clear_2d_array(Dists, MaxQueryNum);
	clear_2d_array(benchmark.indice, benchmark.N);
	clear_2d_array(benchmark.dist, benchmark.N);
	delete[] SquareLen;
}


Parameter::Parameter(Preprocess& prep, unsigned L_, unsigned K_, float rmin_)
{
	N = prep.data.N;
	dim = prep.data.dim;
	L = L_;
	K = K_;
	MaxSize = 5;
	R_min = rmin_;
}

Parameter::~Parameter()
{
}