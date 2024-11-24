#include "Query.h"
#include "basis.h"
#include <iostream>
#include <set>
#include <queue>
#include <algorithm>

#define pi 3.141592653
#define CANDIDATES 10
#define MINFLOAT -3.40282e+038


Query::Query(unsigned id, float c_, unsigned k_, Hash& hash, Preprocess& prep, float beta_)
{
	flag = id;
	c = c_;
	k = k_;
	beta = beta_;

	init_w = hash.R_min * 4.0f * c_ * c_;

	mydata = prep.data.val;
	dim = prep.data.dim;
	query_point = prep.data.query[flag];
	lsh::timer timer;

	timer.restart();
	cal_hash(hash, prep);
	time_hash = timer.elapsed();

	timer.restart();
	sift(hash, prep);
	time_sift = timer.elapsed();

	time_total = time_hash + time_sift;


}

void Query::cal_hash(Hash& hash, Preprocess& prep)
{
	hashval = new float[hash.S];
	for (int i = 0; i < hash.S; ++i) {
		
		hashval[i] = cal_inner_product(query_point, hash.hashpar.rndAs1[i], dim);
		//q_val[i] = hashval[i];
	}
}

void Query::sift(Hash& hash, Preprocess& prep)
{
	float t = 1.0f;
	if (hash.N < 70000) {
		t = 200.0;
	}
	else if (hash.N >= 70000 && hash.N < 500000) {
		//200-1001
		t = 200 + 7 * ((float)hash.N) / 10000;
		t = 1000;
	}
	else if (hash.N >= 500000 && hash.N < 2000000) {
		//501-1000
		t = 501.0+(1000.0-501.0)/150.0* ((float)hash.N) / 10000;
		t = 2000;
	}
	else if (hash.N >= 2000000 && hash.N < 2000000000) {
		//1000-20000
		t = 1000.0 + 19000.0 / 200000.0 * ((float)hash.N) / 10000;
		t = 20000;
	}
	else if (hash.N >= 2000000000) {
		//1000-20000
		t = 20000.0;
	}
	t *= 2;
	int T = (int)t * 2 * hash.L + k;

	if (prep.hasT) {
		T = beta * hash.N + k;
	}
	T = beta * hash.N + k;

 	Visitor* visits = new Visitor(hash.N, hash.K, hash.dim, k, T, mydata, query_point, hash.R_min * c);

	lsh::timer timer;
	lsh::timer timer2;
	lsh::timer timer3;

	while (! visits->termination && rounds<=30) {
		rounds++;
		res.clear();
		
		for (int i = 0; i < hash.L; ++i) {
			int sta = i * hash.K;
			visits->q_mbr = new float[2 * visits->low_dim];
			for (int j = 0; j < visits->low_dim; ++j) {
				visits->q_mbr[2 * j] = hashval[j+sta] - init_w / 2;
				visits->q_mbr[2 * j + 1] = hashval[j+sta] + init_w / 2;
			}
			timer2.restart();
			hash.myIndexes[i]->windows_query(visits);
			time_non_leaf += timer2.elapsed();
		}
		if (visits->count >= visits->MAXCOST){
			visits->termination = true;
		}
		if (init_w >= 2 * visits->kth_dist){
			visits->termination = true;
		}

		//////////////// new code ////////////////
		timer3.restart();
		int data_id;
		for (int k = visits->last_count;k < visits->count; ++k){
			data_id = visits->res[k].id;
			visits->res[k].dist = sqrt(calL2Sqr_fast(visits->original_data[data_id], visits->q_point, visits->data_dim));
			if (k == visits->k){
				std::sort(visits->res, visits->res + visits->k);
				visits->kth_dist = visits->res[visits->k - 1].dist;
			}else if (k > visits->k){
				visits->kth_dist = visits->kth_dist < visits->res[visits->count - 1].dist ? visits->kth_dist : visits->res[visits->count - 1].dist;
			}
		}
		time_dist += timer3.elapsed();
		visits->last_count = visits->count;
		//////////////////////////////////////////

		init_w *= this->c;
	}

	visits->count = min(visits->count, visits->MAXCOST);

	time_leaf = visits->time_leaf;
	time_non_leaf -= time_leaf;

	time_verify = timer.elapsed();
	timer.restart();

	cost = visits->count;
	num_access = visits->num_leaf_access + visits->num_nonleaf_access;
	num_access = visits->num_nonleaf_access;

	// cout<<visits->time1<<' '<<visits->time2<<' '<<visits->time3<<' '<<visits->time4<<endl;

	//std::sort(visits->res, visits->res + visits->count);
	std::partial_sort(visits->res, visits->res + visits->k, visits->res + visits->count);
	res.assign(visits->res, visits->res + visits->k);
	time_sift = timer.elapsed();
	delete visits;

}


Query::~Query()
{
	//
}

void Performance::update(Query &query, Preprocess &prep)
{
	num++;
	cost += query.cost;
	time_sift += query.time_sift;
	time_verify += query.time_verify;
	time_total += query.time_total;
	time_hash += query.time_hash;
	time_leaf += query.time_leaf;
	time_non_leaf += query.time_non_leaf;
	time_dist += query.time_dist;
	rounds += query.rounds;
	num_access_in_RTree += query.num_access;
	unsigned num0 = query.res.size();
	if (num0 > query.k)
		num0 = query.k;
	res_num += num0;

	std::set<unsigned> set1, set2;
	std::vector<unsigned> set_intersection;
	set_intersection.clear();
	set1.clear();
	set2.clear();

	for (unsigned j = 0; j < num0; j++)
	{
		float rate = query.res[j].dist / prep.benchmark.dist[query.flag][j];
		if (prep.benchmark.dist[query.flag][j] == 0) {
			rate = 1.0f;
		}
		if (rate <0.9)
		{
			std::cerr << "An abnormol ratio appears in:" << query.flag << ',' << j  <<
				std::endl;
			system("pause");
		}
		ratio += rate;

		set1.insert(query.res[j].id);
		set2.insert((unsigned)prep.benchmark.indice[query.flag][j]);
	}
	std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
		std::back_inserter(set_intersection));

	NN_num += set_intersection.size();
}

Performance::~Performance()
{

}

