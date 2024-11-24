// Main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include"Real_Result.h"
#include"PM_Tree.h"
#include"MyFunc.h"
#include"Pivot.h"
#include"Config.h"
#include "pmLsh.h"
#include<set>
#include <string>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

void load_data();

Real_Result real_result;
DataMetric lowData;	
DataMetric lowQueryData;
DataMetric highData;
DataMetric highQueryData;
DataMetric lowData2;
DataMetric lowQueryData2;

using namespace std;
int main(int argc, char const* argv[])
{
	std::cout << BOLDRED << "aaaaaaaaaaa\n" << RESET;
	// return 0;
	Config::dataSetName = "audio";
	Config::lowDim = 15;//会被覆盖，降维后的维度
	Config::highDim = 192;//会被覆盖，降维前的维度
	Config::KNN = 100;//可能是要找的最近邻的数量
	Config::N = 54187;//会被覆盖，数据总数
	//pivot
	Config::pivotNum = 5;
	Config::pivotRandomCount = 5;

	//pm-lsh
	Config::c_appro = 1.5;//会被覆盖，近似率
	Config::T = 0.1;//会被覆盖，更新为beta * n
	Config::beta = 0.1;
	Config::alpha1 = 0.01;//卡方分布的右侧概率
	Config::search_Radius = 3.0;//会被覆盖，初始半径

	//ours
	Config::updateDist = false; // 是否用更新的方式计算不降维的数据的距离
	
	if (argc < 8) {
		std::cout << "Usage: pmlsh datasetName n dim m c beta R_min \n\n";
		exit(-1);
	}

	//mnist 59800 784 15 1.5 0.2 20.0
	//audio 54187 192 15 1.5 0.2 1.0
	//0.alg 1.dataset 2.n 3.dim 4.m 5.c 6.T 7.R_min
	if (argc > 1) {
		Config::dataSetName = argv[1];
		if (argc > 2) {
			Config::N = std::atoi(argv[2]);
			Config::highDim = std::atoi(argv[3]);
			Config::lowDim = std::atoi(argv[4]);
		}

		if (argc > 5) {
			Config::c_appro = std::atof(argv[5]);
			Config::T = std::atof(argv[6]);
			Config::beta = std::atof(argv[6]);
			Config::search_Radius = std::atof(argv[7]);
		}
		if (argc > 8){
			Config::KNN = std::atoi(argv[8]);
		}
		if (argc > 9){
			Config::trainedWFile = argv[9];
		}
		if (argc > 10){
			Config::updateDist = std::atoi(argv[10]);
		}
		if (argc > 11){
			Config::alpha1 = std::atof(argv[11]);
		}
		if (argc > 12){
			Config::sampledParaFile = argv[12];
		}
		if (argc > 13){
			Config::autoT = std::atoi(argv[13]);
		}
	}
	else {
		std::cout << "Using the default configuration!\n\n";
	}
	
	// 参数修正
	if(Config::trainedWFile=="None")Config::updateDist=false;
	Config::t = MyFunc::Ccal_thresh(Config::lowDim, Config::alpha1);
	if(Config::sampledParaFile!="None"){
		cout<<"Loading para from "<<Config::sampledParaFile<<endl;
		std::string file = Config::sampledParaFile;
		std::ifstream in(file.c_str(), std::ios::binary);
		while (!in) {
			printf("Fail to find para file!\n");
			exit(0);
		}
		float *temp=new float[4];
		in.read((char*)temp, sizeof(float)*4);
		Config::c_appro=temp[0];
		Config::alpha1=temp[1];
		Config::t=temp[2];
		if(Config::autoT){
			Config::T = min(temp[3], (float)1);
			Config::beta = min(temp[3], (float)1);
		}
		// Config::t =1; //////////////////////////////////////////////////////
		cout << temp[0] << ' '<< temp[1] << ' ' << temp[2] << ' ' << temp[3] <<endl;///////////////////////////////////////
		cout << Config::c_appro << ' '<< Config::alpha1 << ' ' << Config::t << ' ' << Config::T <<endl;//////////////////////////////////////
	}

	//unix和__unix__是Linux系统自带的宏定义，值为1
	#if defined(unix) || defined(__unix__)
		Config::highDataFilePath = "./../dataset/" + Config::dataSetName + ".data_all";
		Config::highQueryFilePath = "./../dataset/" + Config::dataSetName + ".data_all_query";
		Config::knnResultFilePath = "./../dataset/ANN/" + Config::dataSetName + "_result_pmlsh.txt";
	#else
		Config::highDataFilePath = "E:\\Dataset_for_c\\" + Config::dataSetName + ".data_all";
		Config::highQueryFilePath = "E:\\Dataset_for_c\\" + Config::dataSetName + ".data_all_query";
		Config::knnResultFilePath = "E:\\Dataset_for_c\\wlg\\" + Config::dataSetName + "_result_pmlsh.txt";
	#endif

	std::cout << "Using PM-LSH for " << Config::dataSetName << " ..." << std::endl;
	std::cout << "N=        " << Config::N << std::endl;
	std::cout << "dim=      " << Config::highDim << std::endl;
	std::cout << "c=        " << Config::c_appro << std::endl;
	std::cout << "k=        " << Config::KNN << std::endl;
	std::cout << "m=        " << Config::lowDim << std::endl;
	std::cout << "t=        " << Config::t << std::endl;

	if (Config::N > 9000000) {
		Config::MLeaf = 64;
		Config::M_NUM = 64;
	}

	if (Config::N > 50000000) {
		Config::MLeaf = 128;//这是啥？m树的叶子结点数吗？
		Config::M_NUM = 128;//这是啥？
	}
	load_data();
	pmLsh myPmlsh;
	myPmlsh.constructIndex(lowData);

	lowQueryData.resize(100);//元素个数调为100
	highQueryData.resize(100);
	myPmlsh.improvedSearchWithKth(highData, highQueryData, lowQueryData, lowData2, lowQueryData2, real_result);

	cout << Config::c_appro << ' '<< Config::alpha1 << ' ' << Config::t << ' ' << Config::T <<endl;/////////////////////////////////////
}

void load_data()
{
	cout << "LOADING DATA...\n";
	Timer timer;
	highData.loadData(Config::highDataFilePath, Config::highDim);
	highQueryData.loadData(Config::highQueryFilePath, Config::highDim);
	cout << "FINISH LOADING WITH TIME: " << timer.elapsed() << " s.\n\n";

	cout << "HASHING...\n";
	timer.restart();
	E2LSH* e2lsh = new E2LSH;
	Config config;
	e2lsh->constructTable(config);
	lowData.lowerDimFromAnother(highData, e2lsh, config);
	lowQueryData.lowerDimFromAnother(highQueryData, e2lsh, config);
	cout << "FINISH HASHING WITH TIME: " << timer.elapsed() << " s.\n\n";

	if(Config::updateDist){
		cout << "HASHING PART 2...\n";
		timer.restart();
		lowData2.lowerDimFromAnother2(highData, e2lsh, config);
		lowQueryData2.lowerDimFromAnother2(highQueryData, e2lsh, config);
		cout << "FINISH HASHING PART 2 WITH TIME: " << timer.elapsed() << " s.\n\n";
	}

	//cout << "Loading knn result\n";
	real_result.cal_knn_result(Config::knnResultFilePath, highData, highQueryData, Config::KNN, Config::highDim);//计算knn	
	real_result.load(Config::knnResultFilePath);
	if (highData.size() != lowData.size()) {
		cout << "highData size "<< highData.size()<<" is not same as lowData size "<< lowData.size() << endl;
		exit(-1);
	}		
	Config::T = Config::T * highData.size();// 从 beta 变为 n*beta 
	delete e2lsh;

	
}




