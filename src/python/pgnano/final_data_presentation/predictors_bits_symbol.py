from pgnano.final_data_presentation.final_data_preparation import final_flattened_sample_data
from pgnano.stats_analysis.map_reduce_stats import run_mapper_reducer
from pgnano.stats_analysis.primitives import PGPoreType
from sklearn.linear_model import LinearRegression
from sklearn.neural_network import MLPRegressor
import pgnano.stats_analysis.models as pgnmodels

if __name__ == "__main__":
    signal_data, chunked_data = final_flattened_sample_data(PGPoreType.P10_4_1, 100)
    with open("final_predictors.txt", "a") as f:
        #report_vbz = run_mapper_reducer(pgnmodels.VbzModel(), signal_data)
        #f.write(f"vbz: {report_vbz.macro_avg_bits_symbol}\n")
        #report_vbz_chunked = run_mapper_reducer(pgnmodels.VbzModel(), chunked_data)
        #f.write(f"vbz chunked: {report_vbz_chunked.macro_avg_bits_symbol}\n")
#
        #report_cpp = run_mapper_reducer(pgnmodels.CppModel(), signal_data)
        #f.write(f"cpp: {report_cpp.macro_avg_bits_symbol}\n")
        #report_cpp_chunked = run_mapper_reducer(pgnmodels.CppModel(), chunked_data)
        #f.write(f"cpp chunked: {report_cpp_chunked.macro_avg_bits_symbol}\n")
#
        #f.flush()
#
        #report_linear = run_mapper_reducer(pgnmodels.SklearnModel(LinearRegression()), signal_data)
        #f.write(f"linear: {report_linear.macro_avg_bits_symbol}\n")
        #f.flush()
        #report_linear_chunked = run_mapper_reducer(pgnmodels.SklearnModel(LinearRegression()), chunked_data)
        #f.write(f"linear chunked: {report_linear_chunked.macro_avg_bits_symbol}\n")
        #f.flush()

        report_deriv_neg = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=False,use_deriv=True), signal_data)
        f.write(f"deriv_neg: {report_deriv_neg.macro_avg_bits_symbol}\n")
        report_deriv_neg_chunked = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=False,use_deriv=True), chunked_data)
        f.write(f"deriv_neg chunked: {report_deriv_neg_chunked.macro_avg_bits_symbol}\n")

        report_deriv_pos = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=True,use_deriv=True), signal_data)
        f.write(f"deriv_pos: {report_deriv_pos.macro_avg_bits_symbol}\n")
        report_deriv_pos_chunked = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=True,use_deriv=True), chunked_data)
        f.write(f"deriv_pos chunked: {report_deriv_pos_chunked.macro_avg_bits_symbol}\n")

        f.flush()


        report_1_neg = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=False,use_deriv=False), signal_data)
        f.write(f"1_neg: {report_1_neg.macro_avg_bits_symbol}\n")
        report_1_neg_chunked = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=False,use_deriv=False), chunked_data)
        f.write(f"1_neg chunked: {report_1_neg_chunked.macro_avg_bits_symbol}\n")

        report_1_pos = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=True,use_deriv=False), signal_data)
        f.write(f"1_pos: {report_1_pos.macro_avg_bits_symbol}\n")
        report_1_pos_chunked = run_mapper_reducer(pgnmodels.DerivativeContextModel5(add=True,use_deriv=False), chunked_data)
        f.write(f"1_pos chunked: {report_1_pos_chunked.macro_avg_bits_symbol}\n")

        f.flush()

        report_neural = run_mapper_reducer(pgnmodels.SklearnModel(MLPRegressor(hidden_layer_sizes=[3,3,3], activation='relu', max_iter=2000)), signal_data)
        f.write(f"neural: {report_neural.macro_avg_bits_symbol}\n")
        f.flush()
        report_neural_chunked = run_mapper_reducer(pgnmodels.SklearnModel(MLPRegressor(hidden_layer_sizes=[3,3,3], activation='relu', max_iter=2000)), chunked_data)
        f.write(f"neural chunked: {report_neural_chunked.macro_avg_bits_symbol}\n")
        f.flush()