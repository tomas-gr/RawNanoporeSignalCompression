from pgnano.stats_analysis.primitives import PGPoreType
from pgnano.stats_analysis.coding_analysis_scripts import transform_error_to_code, transform_signal_to_error, estimate_geometric_param
from pgnano.final_data_presentation.shared import OUT_PATH
from pgnano.final_data_presentation.final_data_preparation import final_flattened_sample_data
from pgnano.final_data_presentation.extended_bincount import bincount
from scipy.stats import laplace, geom, norm
from scipy.linalg import lstsq
from pgnano.stats_analysis.jupyter_data_preparation import flatten_sample_data
from itertools import product, repeat
from functools import partial
import numpy as np
import matplotlib.pyplot as plt

def unrice(x):
    if x % 2 == 0:
        return -np.int16(x) // 2
    else:
        return (np.int16(x) // 2) + 1

def estimate_b_laplace(data, mean):
    return sum(abs(data - mean))/len(data)

def calc_estimators(error: np.ndarray[np.int16],
                    code: np.ndarray[np.uint16]):
    mu = np.mean(error)
    b = estimate_b_laplace(error, mu)
    p = estimate_geometric_param(code + 1)
    return mu, b, p

def fit_laplace(mu, b, counts, name, ax):
    ax.plot(counts.xs, counts.ys / len(counts), label="real")
    ax.plot(counts.xs, laplace.pdf(counts.xs, mu, b), label="aproximación")

def fit_geometric(p, counts, name, ax):
    ax.plot(counts.xs, counts.ys / len(counts), label="real")
    ax.plot(counts.xs[1:] - 1, geom.pmf(counts.xs, p)[1:], label="aproximación")

def plot_count(counts, ax, label=None):
    ax.plot(counts.xs, counts.ys / len(counts), label=label)

def fit_high_low(error, code, bit):
    low_mask = np.uint16((1 << bit) - 1)
    high_mask = ~low_mask

    centered_low_error = ((error + (low_mask >> 1)) & low_mask) - (low_mask >> 1) # Signo + valor absoluto

    mu_low, b_low, p_low = calc_estimators(error & low_mask, code & low_mask)
    mu_centered_low, b_centered_low, _ = calc_estimators(centered_low_error, code & low_mask)
    mu_high, b_high, p_high = calc_estimators((error % (1 << (bit + 1)) - (1 << bit)), (code & high_mask) >> bit)
    

    centered_low_error_counts = bincount(centered_low_error)
    low_error_counts = bincount(error & low_mask)
    low_code_counts = bincount(code & low_mask)
    figs, axs = plt.subplots(2,2, figsize=(10*2,10*2))
    
    tmp = np.vectorize(unrice)(code & low_mask)
    counts = bincount(tmp)

    plot_count(counts, axs[1][1], "foo")
    plot_count(low_error_counts, axs[0][0], "error bajo")
    fit_laplace(mu_centered_low, b_centered_low, centered_low_error_counts, f"low-modulo-{bit}-laplace-fit", axs[0][1])
    fit_geometric(p_low, low_code_counts, f"low-{bit}-geometric-fit", axs[1][0])
        
    axs[0][0].legend()
    axs[0][0].set_title(f"primeros {bit} bits del error (rep. nativa máquina en complemento de 2)")
    axs[0][1].legend()
    axs[0][1].set_title(f"ajuste de los primeros {bit} bits del error centrado a una laplaciana discreta")
    axs[1][0].legend()
    axs[1][0].set_title(f"ajuste de los primeros {bit} bits del mapeo de Rice del error a una geométrica")
    axs[1][1].legend()
    axs[1][1].set_title(f"deshacer el mapeo de rice sólo a los primeros {bit} bits del error")

    for (x,y) in product(range(0,2), range(0,2)):
        axs[x][y].set_ylabel('P(n)')
        axs[x][y].set_xlabel('n')

    plt.savefig(OUT_PATH + "/fit_graphs/" + f"summary-{bit}.png")
    plt.clf()
    plt.cla()

def fit_golomb_ready(error, code, prefix):
    mu, b, p = calc_estimators(error, code)
    counts = bincount(error)
    fit_laplace(mu, b, counts, prefix + "-laplace-fit")
    counts = bincount(code)
    fit_geometric(p, counts, prefix + "-geometric-fit")


signal_data, chunk_data = final_flattened_sample_data(PGPoreType.P10_4_1,100)
one_signal = signal_data[10]
error = transform_signal_to_error(one_signal)
code = transform_error_to_code(error)


###########################

mu, b, p = calc_estimators(error, code)
counts = bincount(error)
fit_laplace(mu, b, counts, "total-laplace-fit", plt)
plt.ylabel("P(n)")
plt.xlabel("n")
plt.title("Ajuste del error de predicción por una laplaciana discreta")
plt.legend()
plt.savefig(OUT_PATH + "/fit_graphs/total-error.png")
plt.clf()
plt.cla()
counts = bincount(code)
fit_geometric(p, counts, "total-geometric-fit", plt)
plt.ylabel("P(n)")
plt.xlabel("n")
plt.title("Ajuste del mapeo de rice del error de predicción por una geometrica")
plt.legend()
plt.savefig(OUT_PATH + "/fit_graphs/total-code.png")
plt.clf()
plt.cla()

###########################

fit_high_low(error, code, 4)
fit_high_low(error, code, 5)
fit_high_low(error, code, 8)
fit_high_low(error, code, 11)

###########################
bit = 5
low_mask = np.uint16((1 << bit) - 1)
high_mask = ~low_mask
centered_low_error = ((error + (low_mask >> 1)) & low_mask) - (low_mask >> 1) # Signo + valor absoluto
mu_high, b_high, p_high = calc_estimators((error % (1 << (bit + 1)) - (1 << bit)), (code & high_mask) >> bit)

high_code_counts = bincount((code & high_mask) >> bit)

_ = plt.figure(figsize=(10,10))
fit_geometric(p_high, high_code_counts, f"high-{bit}-geometric-fit", plt)
plt.xlabel('n')
plt.ylabel('P(n)')
plt.legend()
plt.title(f"ajuste de los últimos {16 - bit} bits del mapeo de Rice del error a una geométrica")
plt.show()
plt.savefig(OUT_PATH + f"{bit}-{16-bit}-high-geometric-fit.png")
plt.clf()
plt.cla()
###########################
bit = 8
low_mask = np.uint16((1 << bit) - 1)
high_mask = ~low_mask
centered_low_error = ((error + (low_mask >> 1)) & low_mask) - (low_mask >> 1) # Signo + valor absoluto
mu_high, b_high, p_high = calc_estimators((error % (1 << (bit + 1)) - (1 << bit)), (code & high_mask) >> bit)

high_code_counts = bincount((code & high_mask) >> bit)

_ = plt.figure(figsize=(10,10))
fit_geometric(p_high, high_code_counts, f"high-{bit}-geometric-fit", plt)
plt.xlabel('n')
plt.ylabel('P(n)')
plt.legend()
plt.title(f"ajuste de los últimos {16 - bit} bits del mapeo de Rice del error a una geométrica")
plt.show()
plt.savefig(OUT_PATH + f"{bit}-{16-bit}-high-geometric-fit.png")
plt.clf()
plt.cla()
###########################

np.vectorize(lambda x: 2*x - 1 if x > 0 else (-(2*x)))

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
#FIXME:
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

counts = bincount(error)
plt.xlabel('n')
plt.ylabel('P(n)')
plt.plot(counts.xs, counts.ys / len(counts), label="real")
normal_ys = norm.pdf(counts.xs, np.mean(error), np.std(error))
plt.plot(counts.xs, normal_ys, label="aproximación")
plt.legend()
plt.savefig(OUT_PATH + "ajuste-normal-total")
plt.cla()
plt.clf()