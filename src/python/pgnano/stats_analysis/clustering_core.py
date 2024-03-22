import numpy as np
import numpy.typing as npt
import matplotlib.pyplot as plt
import os
from itertools import repeat
from sklearn.cluster import KMeans, DBSCAN

def scatter_plot_signal(signal: npt.NDArray[np.int16]) -> None:
    counts = np.zeros(1 << 16)
    xs = []
    ys = []
    for x in signal:
        idx = x + (1 << 15)
        ys.append(counts[idx])
        xs.append(x)
        counts[idx] += 1
    plt.scatter(xs,ys,s=1)

def one_axis_scatter_plot(xs) -> None:
    plt.scatter(x=xs, y=list(repeat(0,len(xs))))

def kmeans_cluster(signal: npt.NDArray[np.int16], k: int):
    clusterer = KMeans(n_clusters=k)
    clusterer.fit(signal.reshape(-1,1))
    return clusterer.cluster_centers_

def dbscan_cluster(signal: npt.NDArray[np.int16], eps=10):
    clusterer = DBSCAN(eps=eps)
    res = clusterer.fit_predict(signal.reshape(-1,1))
    return res