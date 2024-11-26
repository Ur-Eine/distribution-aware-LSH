# Towards Accurate Distance Estimation for Distribution-Aware $c$-ANN Search

This paper is submitted to ICDE2025, which is underreviewing currently.

## Abstract

Locality sensitive hashing (LSH) is a representative approach for nearest neighbor (NN) search in high-dimensional spaces, which is able to answer $c$-approximate NN ($c$-ANN) queries in sublinear time with constant probability. 
Existing advanced LSH methods leverage a plurality of novel techniques such as query-aware dynamic bucketing, virtual rehashing, and efficient indexing to achieve state-of-the-art performance. However, they rely on similar random LSH functions, which provides distance estimations that are irrelevant to the given data distribution. Therefore, the quality of the searched candidates is suboptimal. In this study, we reformulate the $c$-ANN query from the perspective of data distribution. Specifically, an accurate distance estimator based on the data distribution in Euclidean space is introduced to enable the generation of high-quality candidates. Furthermore, we provide a theoretical analysis to demonstrate that our algorithm can answer distribution-aware $c$-ANN queries successfully with at least a constant probability. Extensive experiments are conducted on real datasets to empirically demonstrate the superiority of the proposed algorithm over existing state-of-the-art LSH methods on both efficiency and effectiveness. 

## Repo Description

An accurately data-aware distance estimator is as follows:

$$\mathbb{E}[\|o_1 - o_2\|_2^2] = \frac{\sum_{k=1}^d \lambda_k}{\sum_{k=1}^m \lambda_k} \mathbb{E}[\|{W_m}^T o_1 - {W_m}^T o_2\|_2^2]$$

Where $o_1, o_2$ are two points in the orginal space, $W_m \in \mathbb{R}^{d\times m}$ is obtained through the principal components analysis (PCA), $\lambda_k$ is the $k$-th largest eigenvalue. 

This estimator is extremely effective than previous ones that are based on random projection. 

It has two obvious advantages:

* It is simple. For different point pairs, it has a uniform factor that multiplies the distance in the projected space. Therefore, we can directly construct an efficient index structure in the projected space to search the approximate $k$ nearest neighbors.

* The accurate distance estimator enables fewer number of candidates to be accessed for a given accuracy compared with previous LSH approaches.

## Deep1M reproduction
The tested datasets are available at https://www.cse.cuhk.edu.hk/systems/hash/gqr/datasets.html.

1. Download and preprocess the datasets, in which the data-aware orthogonal transformation is obtained though PCA(principal components analysis) according to the estimator described above.

```
python PCA/pca.py
```

2. Pass the obtained $W$ file into PM-LSH+ or DB-LSH+, which can be excuted as follows.

```
./pmlsh deep1m 1000000 256 15 1.5 0.01 0.2 100 ../dataset/TrainedW/linear_pca_deep1m.bin

or 

./dblsh deep1m 1.2 100 1 10 0.08 0.03 ./../dataset/TrainedW/linear_10_pca_deep1M_10000/
```

Where the excutable files (e.g., pmlsh or dblsh) can be obtained according to the README.md in the corresponding directory (e.g., distribution-aware-LSH/PM-LSH/README.md), and the parameter (i.e., ../dataset/TrainedW/linear_pca_deep1m.bin) is the path that indicates the trained $W$ by PCA. Moreover, the meaning of the other paramters can also be found in the corresponding README.md. 

## Acknowledgement
This repo is mainly based on the following repos:

* **DB-LSH** (ICDE): https://github.com/Jacyhust/DB-LSH
* **PM-LSH** (VLDB): https://github.com/Jacyhust/PM-LSH
* **DADE** (VLDB): https://github.com/Ur-Eine/DADE

If you think this repo is useful, please consider to cite their papers.
