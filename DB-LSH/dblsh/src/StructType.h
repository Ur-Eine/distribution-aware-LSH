#pragma once
struct Data
{
	// Dimension of data
	unsigned dim = 0;
	// Number of data
	unsigned N = 0;
	// Data matrix
	float** val = nullptr;
	float** query=nullptr; // NO MORE THAN 200 POINTS
};

struct Ben
{
	unsigned N = 0;//查询数
	unsigned num = 0;//查询记录前num个最邻近点
	int** indice = nullptr;//每个查询前num最邻近点的id
	float** dist = nullptr;//每个查询前num最邻近点的欧氏距离
};

struct HashParam
{
	// the value of a in S hash functions
	float** rndAs1 = nullptr;//列数和数据一样
	// the value of a in S hash functions
	float** rndAs2 = nullptr;//只有一列，好像没有用到这个参数

};



