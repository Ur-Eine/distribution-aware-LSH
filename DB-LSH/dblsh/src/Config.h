#pragma once
#include<string>
#include<assert.h>
#include<iostream>

#define DATATYPE float

using namespace std;

class Config
{
public:
	Config() {}
	~Config() {}

	static string trainedWFile;
	static string dataset;
	static float gamma;
private:

};

