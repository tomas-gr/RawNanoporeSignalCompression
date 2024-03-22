from pgnano.stats_analysis.primitives import PGPoreType
from pgnano.stats_analysis.coding_analysis_scripts import transform_error_to_code, transform_signal_to_error, estimate_geometric_param
from pgnano.final_data_presentation.shared import OUT_PATH
from pgnano.final_data_presentation.final_data_preparation import final_flattened_sample_data
from pgnano.final_data_presentation.extended_bincount import bincount
from scipy.stats import laplace, geom
from scipy.linalg import lstsq
from pgnano.stats_analysis.jupyter_data_preparation import flatten_sample_data
from itertools import repeat
import numpy as np
import matplotlib.pyplot as plt

def estimate_b_laplace(data, mean):
    return sum(abs(data - mean))/len(data)


signal_data, chunk_data = final_flattened_sample_data(PGPoreType.P10_4_1,100)
error = transform_signal_to_error(signal_data[10])
code = transform_error_to_code(error)

############################

mu = np.mean(error)
b = estimate_b_laplace(error, mu)
p = estimate_geometric_param(code + 1)

counts = bincount(error)

plt.plot(counts.xs, counts.ys, label="real")
x = np.linspace(laplace.ppf(0.001,mu,b),laplace.ppf(0.999,mu,b),100)
plt.plot(x,laplace.pdf(x,mu,b)*len(error), label="aproximación")
plt.savefig(OUT_PATH + "total-laplace-fit.png")
plt.clf()

counts = bincount(code)
plt.plot(counts.xs, counts.ys, label="real")
plt.plot(geom.pmf(
    np.arange(
        geom.ppf(0.01, p),
        geom.ppf(0.999999999, p))
    ,p) * len(code),
    label="aproximación"
)
plt.savefig(OUT_PATH + "total-geometric-fit.png")
plt.clf()

###########################

counts = bincount(error // 256)

mu = np.mean(error // 256)
b = estimate_b_laplace(error // 256, mu)
p = estimate_geometric_param(((code & 0xFF00) >> 8) + 1)

plt.plot(counts.xs, counts.ys, label="real")
x = np.linspace(laplace.ppf(0.001,mu,b),laplace.ppf(0.999,mu,b),100)
plt.plot(x,laplace.pdf(x,mu,b)*len(error), label="aproximación")
plt.savefig(OUT_PATH + "8-8-laplace-fit.png")
plt.clf()

counts = bincount((code & 0xFFE0) >> 5)
plt.plot(counts.xs, counts.ys, label="real")
plt.plot(geom.pmf(
    np.arange(
        geom.ppf(0.01, p),
        geom.ppf(0.999999999, p))
    ,p) * len(code),
    label="aproximación"
)
plt.savefig(OUT_PATH + "8-8-geometric-fit.png")
plt.clf()


###########################

counts = bincount(error // 32)

mu = np.mean(error // 32)
b = estimate_b_laplace(error // 32, mu)
p = estimate_geometric_param(((code & 0xFFE0) >> 5) + 1)

plt.plot(counts.xs, counts.ys, label="real")
x = np.linspace(laplace.ppf(0.001,mu,b),laplace.ppf(0.999,mu,b),100)
plt.plot(x,laplace.pdf(x,mu,b)*len(error), label="aproximación")
plt.savefig(OUT_PATH + "5-11-laplace-fit.png")
plt.clf()

counts = bincount((code & 0xFFE0) >> 5)
plt.plot(counts.xs, counts.ys, label="real")
plt.plot(geom.pmf(
    np.arange(
        geom.ppf(0.01, p),
        geom.ppf(0.999999999, p))
    ,p) * len(code),
    label="aproximación"
)
plt.savefig(OUT_PATH + "5-11-geometric-fit.png")
plt.clf()


#########################

counts = bincount(error // 16)

mu = np.mean(error // 16)
b = estimate_b_laplace(error // 16, mu)
p = estimate_geometric_param(((code & 0xFFF0) >> 4) + 1)

plt.plot(counts.xs, counts.ys, label="real")
x = np.linspace(laplace.ppf(0.001,mu,b),laplace.ppf(0.999,mu,b),100)
plt.plot(x,laplace.pdf(x,mu,b)*len(error), label="aproximación")
plt.savefig(OUT_PATH + "4-12-laplace-fit.png")
plt.clf()

counts = bincount((code & 0xFFF0) >> 4)
plt.plot(counts.xs, counts.ys, label="real")
plt.plot(geom.pmf(
    np.arange(
        geom.ppf(0.01, p),
        geom.ppf(0.9999999, p))
    ,p) * len(code),
    label="aproximación"
)
plt.savefig(OUT_PATH + "4-12-geometric-fit.png")
plt.clf()

########################
np.vectorize(lambda x: 2*x - 1 if x > 0 else (-(2*x)))
def unrice(x):
    if x % 2 == 0:
        return -np.int16(x) // 2
    else:
        return (np.int16(x) + 1) // 2


tmp = np.vectorize(unrice)(code & 0x1F)
counts = bincount(tmp)

linear_pos = None
linear_neg = None

linear_pos = lstsq(
                    np.column_stack((
                        list(repeat(1,17)),
                        range(17))
                    ),
                    counts.ys[counts.xs >= 0]
                )[0]

linear_neg = lstsq(
                    np.column_stack((
                        list(repeat(1,15)),
                        range(-15,0))
                    ),
                    counts.ys[counts.xs < 0]
                )[0]
print(len(counts.ys[counts.xs >= 0]))
print(counts.xs)
n_pos = linear_pos[0]
m_pos = linear_pos[1]
n_neg = linear_neg[0]
m_neg = linear_neg[1]

figs, axs = plt.subplots(1,2, figsize=(12,5))
axs[0].plot(counts.xs, counts.ys)
axs[1].plot(counts.xs, counts.ys)
axs[1].plot(counts.xs[counts.xs >= 0], counts.xs[counts.xs >= 0] * np.array(list(repeat(m_pos, len(counts.xs[counts.xs >=  0])))) + np.array(list(repeat(n_pos, len(counts.xs[counts.xs >= 0])))))
axs[1].plot(counts.xs[counts.xs < 0], counts.xs[counts.xs < 0] * np.array(list(repeat(m_neg, len(counts.xs[counts.xs <  0])))) + np.array(list(repeat(n_neg, len(counts.xs[counts.xs < 0])))))
plt.savefig(OUT_PATH + "ajuste-triangular")
plt.clf()
plt.cla()