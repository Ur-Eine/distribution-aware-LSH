import h5py
import numpy as np
import time
from sklearn.metrics import pairwise_distances

def print_and_file(message, file=None):
    print(message)  # 打印到终端
    if file:
        print(message, file=file)  # 打印到文件

# 2^12 = 4096
def pivots_selection(database, n_pivots=12, method='random', name=None):
    # database: [N, n_dim]
    # n_pivots: integer
    # func: return a n_pivots vectors from database as pivots

    if method == 'random':
        N, n_dim = database.shape
        selected_pivots = np.random.randint(low=0, high=N, size=n_pivots)
        pivots = database[selected_pivots]
    elif method == 'fft':
        pivot_file = f'./pivot/{name}.pivotfft'
        with open(pivot_file, 'r') as f:
            line = f.readline()
            selected_pivots = np.array(line.split(), dtype='int32')
            pivots = database[selected_pivots]
    else:
        raise NotImplementedError
    
    return pivots

def dist_computation(database, pivots, metric='euclidean', n_jobs=-1):
    # func: we use (defaultly) all processors to compute the distance between database and pivots
    #       the metric is specified by user
    assert metric in ['euclidean', 'cosine', 'l1', 'l2', 'manhattan'], 'selected metric is not implemented!'
    # dists: [n_pivots, N]
    p_table = pairwise_distances(database, pivots, metric=metric, n_jobs=n_jobs).T
    return p_table

def mmap_fvecs(fname, dtype='float32', d=None):
    x = np.memmap(fname, dtype='int32', mode='r')
    if d is None:
        d = x[0]
    return x.view(dtype).reshape(-1, d + 1)[:, 1:]

def norm_glove(arr):
    norms = np.linalg.norm(arr, axis=1)
    norm_arr = arr / norms[:, np.newaxis]
    return norm_arr

def load_data(DB_name):
    '''
    load 数据集
    '''
    if DB_name == "gist1m":
        DB_DIR = './dataset/gist/'
        x_d = mmap_fvecs('{}gist_base.fvecs'.format(DB_DIR)) # data (1000000, 960)
        x_q = mmap_fvecs('{}gist_query.fvecs'.format(DB_DIR)) # query (1000, 960)
        xt = mmap_fvecs('{}gist_learn.fvecs'.format(DB_DIR)) # (500000, 960)
        gt_ids = mmap_fvecs('{}gist_groundtruth.ivecs'.format(DB_DIR), 'int32')
    elif DB_name == "sift":
        DB_DIR = './dataset/sift/'
        x_d = mmap_fvecs('{}sift_base.fvecs'.format(DB_DIR)) # data (1000000, 128)
        x_q = mmap_fvecs('{}sift_query.fvecs'.format(DB_DIR)) # query (10000, 128)
        xt = mmap_fvecs('{}sift_learn.fvecs'.format(DB_DIR)) # (100000, 128)
        # gt_ids = None
        gt_ids = mmap_fvecs('{}sift_groundtruth.ivecs'.format(DB_DIR), 'int32') # (100000, 128)
    elif DB_name == "glove":
        DB_DIR = './dataset/glove-100-angular.hdf5'
        dataset = h5py.File(DB_DIR, 'r')
        xd_ori = np.array(dataset["train"]) # ndarray (1183514, 100)
        xq_ori = np.array(dataset['test']) # (10000, 100)
        gt_ids = np.array(dataset['neighbors']) # (10000, 100) 每个query 的 k=100 NN
        x_d = norm_glove(xd_ori) # np.linalg.norm(x_d[0]) = 1
        x_q = norm_glove(xq_ori)
        xt = None
    elif DB_name == "bigann":
        DB_DIR = './dataset/bigann/'
        x_d = mmap_fvecs('{}bigann_base_50m.bvecs'.format(DB_DIR), dtype=np.uint8)
        x_q = mmap_fvecs('{}bigann_query.bvecs'.format(DB_DIR), dtype=np.uint8) 
        gt_ids = None # 这里我需要确认下gt_ids是否和x_d对应
        xt = None
    elif DB_name == "deep1M":
        DB_DIR = './dataset/deep1M/'
        x_d = mmap_fvecs('{}deep1M_base.fvecs'.format(DB_DIR)) # data (1M, 256)
        x_q = mmap_fvecs('{}deep1M_query.fvecs'.format(DB_DIR)) # query (10000, 256)
        xt = mmap_fvecs('{}deep1M_learn.fvecs'.format(DB_DIR))
        gt_ids = mmap_fvecs('{}deep1M_groundtruth.ivecs'.format(DB_DIR), dtype='int32')
    elif DB_name == 'deep50M':
        DB_DIR = './dataset/deep50M/'
        x_d = mmap_fvecs('{}deep_base_50m.fvecs'.format(DB_DIR)) # data (50M, 96)
        x_q = mmap_fvecs('{}deep_query.fvecs'.format(DB_DIR)) # query (10000, 96)
        gt_ids = None
        xt = None
    elif DB_name == 'text2image50M':
        DB_DIR = './dataset/text2image/'
        x_d = mmap_fvecs('{}text2image_base_50m.fvecs'.format(DB_DIR)) # data (50M, 200)
        x_q = mmap_fvecs('{}text2image_query.fvecs'.format(DB_DIR)) # query (100000, 200)
        gt_ids = None
        xt = None
    elif DB_name == 'deep100M':
        DB_DIR = './dataset/deep100M/'
        x_d = mmap_fvecs('{}deep_base_100m.fvecs'.format(DB_DIR)) # data (50M, 96)
        x_q = mmap_fvecs('{}deep_query.fvecs'.format(DB_DIR)) # query (10000, 96)
        gt_ids = None
        xt = None
    elif DB_name == 'mnist':
        DB_DIR = './dataset/mnist/'
        x_d = mmap_fvecs('{}mnist.data_all'.format(DB_DIR), d=784)
        x_q = mmap_fvecs('{}mnist.data_all_query'.format(DB_DIR), d=784)
        gt_ids = None
        xt = None
    elif DB_name == 'cifar':
        DB_DIR = './dataset/cifar/'
        x_d = mmap_fvecs('{}cifar.data_all'.format(DB_DIR), d=3072)
        x_q = mmap_fvecs('{}cifar.data_all_query'.format(DB_DIR), d=3072)
        gt_ids = None
        xt = None
    elif DB_name == 'trevi':
        DB_DIR = './dataset/trevi/'
        x_d = mmap_fvecs('{}trevi.data_all'.format(DB_DIR), d=4096)
        x_q = mmap_fvecs('{}trevi.data_all_query'.format(DB_DIR), d=4096)
        gt_ids = None
        xt = None
    elif DB_name == "sift10m":
        DB_DIR = './dataset/sift10m/'
        x_d = mmap_fvecs('{}sift10m.data_all'.format(DB_DIR)) # data (10000000, 128)
        x_q = mmap_fvecs('{}sift10m.data_all_query'.format(DB_DIR)) # query (10000, 128)
        xt = None
        gt_ids = None
    else:
        # [TO DO] 加上异常报错
        print("unknown dataset")
    # change for faiss1.5.2
    x_q = np.ascontiguousarray(x_q)
    x_d = np.ascontiguousarray(x_d)
    return x_d, x_q, xt, gt_ids