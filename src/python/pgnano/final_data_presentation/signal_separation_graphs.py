from pgnano.stats_analysis.primitives import PGPoreType
from pgnano.stats_analysis.coding_analysis_scripts import transform_error_to_code, transform_signal_to_error, estimate_geometric_param
from pgnano.final_data_presentation.shared import OUT_PATH
from pgnano.final_data_presentation.final_data_preparation import final_flattened_sample_data
from pgnano.final_data_presentation.extended_bincount import bincount
from scipy.stats import laplace, geom, norm
from scipy.linalg import lstsq
from pgnano.stats_analysis.jupyter_data_preparation import flatten_sample_data
from itertools import repeat
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

#def estimate_triangular_params(data):


def calc_estimators(error: np.ndarray[np.int16],
                    code: np.ndarray[np.uint16]):
    mu = np.mean(error)
    b = estimate_b_laplace(error, mu)
    p = estimate_geometric_param(code + 1)
    return mu, b, p

def fit_laplace(mu, b, counts, name, ax):
    ax.plot(counts.xs, counts.ys, label="real")
    #x = np.linspace(laplace.ppf(0.001,mu,b),laplace.ppf(0.999,mu,b),100)
    #ax.plot(x,laplace.pdf(x,mu,b)*len(counts), label="aproximación")
    ax.plot(counts.xs, laplace.pdf(counts.xs, mu, b) * len(counts), label="aproximación")

def fit_geometric(p, counts, name, ax):
    ax.plot(counts.xs, counts.ys, label="real")
    #ax.plot(geom.pmf(
    #np.arange(
    #        geom.ppf(0.01, p),
    #        geom.ppf(0.99, p))
    #    ,p) * len(counts),
    #    label="aproximación"
    #)
    ax.plot(counts.xs[1:] - 1, geom.pmf(counts.xs, p)[1:] * len(counts), label="aproximación")

def plot_count(counts, ax, label=None):
    ax.plot(counts.xs, counts.ys, label=label)

def fit_high_low(error, code, bit):
    low_mask = np.uint16((1 << bit) - 1)
    high_mask = ~low_mask

    centered_low_error = ((error + (low_mask >> 1)) & low_mask) - (low_mask >> 1) # Signo + valor absoluto

    mu_low, b_low, p_low = calc_estimators(error & low_mask, code & low_mask)
    mu_centered_low, b_centered_low, _ = calc_estimators(centered_low_error, code & low_mask)
    mu_high, b_high, p_high = calc_estimators((error % (1 << (bit + 1)) - (1 << bit)), (code & high_mask) >> bit)
    
    print(f"bit: {bit}")
    print(f"centered low => mu: {mu_centered_low}; b: {b_centered_low}")
    print(f"high => mu: {mu_high}; b: {b_high}")
    print("===================")

    centered_low_error_counts = bincount(centered_low_error)
    low_error_counts = bincount(error & low_mask)
    high_error_counts = bincount((error % (1 << (bit + 1)) - (1 << bit)))
    low_code_counts = bincount(code & low_mask)
    high_code_counts = bincount((code & high_mask) >> bit)
    figs, axs = plt.subplots(1,10, figsize=(10*10,10))
    
    tmp = np.vectorize(unrice)(code & low_mask)
    counts = bincount(tmp)
    plot_count(counts, axs[9], "foo")

    plot_count(low_error_counts, axs[0], "error bajo")
    plot_count(low_code_counts, axs[1], "code bajo")
    plot_count(centered_low_error_counts, axs[2], "error modulo")
    fit_laplace(mu_centered_low, b_centered_low, centered_low_error_counts, f"low-modulo-{bit}-laplace-fit", axs[3])
    fit_geometric(p_low, low_code_counts, f"low-{bit}-geometric-fit", axs[4])
    
    plot_count(high_error_counts, axs[5], "error alto")
    plot_count(high_code_counts, axs[6], "code alto")
    fit_laplace(mu_high, b_high, high_error_counts, f"high-{bit}-laplace-fit", axs[7])
    fit_geometric(p_high, high_code_counts, f"high-{bit}-geometric-fit", axs[8])
    
    axs[0].legend()
    axs[0].set_title(f"primeros {bit} bits del error (rep. nativa máquina en complemento de 2)")
    axs[1].legend()
    axs[1].set_title(f"primeros {bit} bits del mapeo de Rice del error")
    axs[2].legend()
    axs[2].set_title(f"primeros {bit} del error + {((1 << bit) - 1) >> 1} (centrar módulo {2**bit}) (Rep. signo + valor absoluto????)")
    axs[3].legend()
    axs[3].set_title(f"ajuste de los primeros {bit} bits del error centrado a una laplaciana discreta")
    axs[4].legend()
    axs[4].set_title(f"ajuste de los primeros {bit} bits del mapeo de Rice del error a una geométrica")
    axs[5].legend()
    axs[6].legend()
    axs[6].set_title(f"ultimos {bit} bits del mapeo de Rice del error")
    axs[7].legend()
    axs[8].legend()
    axs[8].set_title(f"ajuste de los últimos {16 - bit} bits del mapeo de Rice del error a una geométrica")
    axs[9].legend()
    axs[9].set_title(f"deshacer el mapeo de rice sólo a los primeros {bit} bits del error") # Rice + mascara primeros bit bits - rice
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
plt.savefig(OUT_PATH + "/fit_graphs/total-error.png")
plt.clf()
plt.cla()
counts = bincount(code)
fit_geometric(p, counts, "total-geometric-fit", plt)
plt.savefig(OUT_PATH + "/fit_graphs/total-code.png")
plt.clf()
plt.cla()

#for bit in range(4,5):
#    fit_high_low(error, code, bit)

fit_high_low(error, code, 4)
fit_high_low(error, code, 5)
fit_high_low(error, code, 8)
fit_high_low(error, code, 11)
#########################
#
#mu, b, p = calc_estimators(error // 256, (code & 0xFF00) >> 8)
#counts = bincount(error // 256)
#fit_laplace(mu, b, counts, "8-8-laplace-fit")
#counts = bincount((code & 0xFFE0) >> 5)
#fit_geometric(p, counts, "8-8-geometric-fit")
#
#########################

#mu = np.mean(error // 32)
#b = estimate_b_laplace(error // 32, mu)
#p = estimate_geometric_param(((code & 0xFFE0) >> 5) + 1)
#counts = bincount(error // 32)
#fit_laplace(mu, b, counts, "5-11-laplace-fit")
#counts = bincount((code & 0xFFE0) >> 5)
#fit_geometric(p, counts, "5-11-geometric-fit")

#########################


#mu = np.mean(error // 16)
#b = estimate_b_laplace(error // 16, mu)
#p = estimate_geometric_param(((code & 0xFFF0) >> 4) + 1)
#counts = bincount(error // 16)
#fit_laplace(mu,b,counts,"4-12-laplace-fit")
#counts = bincount((code & 0xFFF0) >> 4)
#fit_geometric(p, counts, "4-12-geometric-fit")

########################
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
plt.plot(counts.xs, counts.ys, label="real")
normal_ys = norm.pdf(counts.xs, np.mean(error), np.std(error)) * len(counts)
plt.plot(counts.xs, normal_ys, label="aproximación")
plt.legend()
plt.savefig(OUT_PATH + "ajuste-normal-total")
plt.cla()
plt.clf()

circular_error = (error + 127) & 0xFF
counts = bincount(circular_error)
plt.plot(counts.xs, counts.ys, label="real")
normal_ys = norm.pdf(counts.xs, np.mean(circular_error), np.std(circular_error)) * len(counts)
plt.plot(counts.xs, normal_ys, label="aproximación")
plt.legend()
plt.savefig(OUT_PATH + "ajuste-normal-8")
plt.cla()
plt.clf()