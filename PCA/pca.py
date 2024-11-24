from transformers import HfArgumentParser
import numpy as np
import os
from utils import load_data
from dataclasses import dataclass

def pca(x, d):
    """
    主成分分析
    :param x: 待降维样本, shape=(特征数,样本数)
    :param d: 降到d维
    :return: 降维后样本, shape=(d,样本数)
    """
    x = np.mat(x)
    x -= x.mean(axis=1)  # 中心化
    lmd, w = np.linalg.eig(x * x.T)  # 求特征值和特征向量
    w_ = w[:, :d]  # 投影矩阵
    x = w_.T * x  # 投影
    return x, w_


@dataclass
class Config:
    dataset: str = 'sift10m' # sift, deep1M, gist1m, mnist, cifar, trevi, sift10m
    project: str = 'PMLSH' # PMLSH, DBLSH
    metric: str = 'euclidean'

    N_s: int = 10000
    max_dim: int = 10 # pca 降维后取前 max_dim 个维数用于后续随机采样
    T = 1 # Hash Table数
    full_dim: bool = True # 是否得到正交方阵
    low_dim: int = 15

    object_dir: str = './pca_projection/'


if __name__ == '__main__':
    parser = HfArgumentParser(Config)
    config = parser.parse_args_into_dataclasses()[0]
    
    if config.project == 'DBLSH':
        save_dir = config.object_dir + f'linear_{config.max_dim}_{config.T}_{config.low_dim}_pca_{config.dataset}_{config.N_s}/'
        if not os.path.exists(save_dir):
            os.makedirs(save_dir, exist_ok=True)

        x_data, x_query, x_train, gt_idx = load_data(config.dataset)
        N, dim = x_data.shape
        np.random.seed(2024)
        sample_index = np.random.randint(0, N ,config.N_s)
        x_sample = x_data[sample_index]
        x_lowdim, W = pca(x_sample.T, config.max_dim)
        W = W.T
        for k in range(config.T):
            with open(save_dir + f'{k}.bin', 'wb') as f:
                if config.T == 1 and config.max_dim == config.low_dim:
                    index = range(config.low_dim)
                else:
                    index = np.random.randint(0, config.max_dim ,config.low_dim)
                for i in index:
                    for j in range(dim):
                        f.write(W[i, j].tobytes())

    elif config.project == 'PMLSH':
        save_dir = config.object_dir
        if not os.path.exists(save_dir):
            os.makedirs(save_dir, exist_ok=True)

        x_data, x_query, x_train, gt_idx = load_data(config.dataset)
        N, dim = x_data.shape
        np.random.seed(2024)
        # sample_index = np.random.randint(0, N ,config.N_s)
        # x_sample = x_data[sample_index]
        if config.full_dim:
            x_lowdim, W = pca(x_data.T, dim)
            W = W.T
            with open(save_dir + f'linear_pca_{config.dataset}.bin', 'wb') as f:
                for i in range(dim):
                    for j in range(dim):
                        f.write(W[i, j].tobytes())
        else:
            x_lowdim, W = pca(x_data.T, config.low_dim)
            W = W.T
            with open(save_dir + f'linear_{config.low_dim}_pca_{config.dataset}.bin', 'wb') as f:
                for i in range(config.low_dim):
                    for j in range(dim):
                        f.write(W[i, j].tobytes())

    import pdb;pdb.set_trace()
