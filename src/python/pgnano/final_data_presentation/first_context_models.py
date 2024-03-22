from pgnano.final_data_presentation.final_data_preparation import final_flattened_sample_data
from pgnano.stats_analysis.map_reduce_stats import run_mapper_reducer
from pgnano.stats_analysis.primitives import PGPoreType
from pgnano.final_data_presentation.shared import *
import pgnano.stats_analysis.models as pgnmodels

signal_data, chunked_data = final_flattened_sample_data(PGPoreType.P10_4_1, None)

with open(OUT_PATH + "first-context-models.txt", "a") as f:
    for model in [
        pgnmodels.CppModel(),
        pgnmodels.VbzModel(),
        pgnmodels.EvenOddTimestepCodingModel(),
        pgnmodels.EvenOddDerivativeCodingModel(),
        pgnmodels.StabilitySeparatorModel(),
        pgnmodels.DerivativeContextModel(),
        pgnmodels.DerivativeContextModel2([3]),
        pgnmodels.DerivativeContextModel4([3]),
        pgnmodels.DerivativeContextModel2([5]),
        pgnmodels.DerivativeContextModel4([5]),
        pgnmodels.DerivativeContextModel2([3,5]),
        pgnmodels.DerivativeContextModel4([3,5]),
        pgnmodels.DerivativeContextModel2([3,5,10]),
        pgnmodels.DerivativeContextModel4([3,5,10])
    ]:
        model_name = type(model).__name__
        if hasattr(model, "thresholds"):
            model_name = model_name + "-" + str(model.thresholds)
        report = run_mapper_reducer(model, signal_data)
        f.write(f"{model_name} (whole signal): {report.macro_avg_bits_symbol}\n")
        f.flush()

    for model in [
        pgnmodels.CppModel(),
        pgnmodels.VbzModel(),
        pgnmodels.EvenOddTimestepCodingModel(),
        pgnmodels.EvenOddDerivativeCodingModel(),
        pgnmodels.StabilitySeparatorModel(),
        pgnmodels.DerivativeContextModel(),
        pgnmodels.DerivativeContextModel2([3]),
        pgnmodels.DerivativeContextModel4([3]),
        pgnmodels.DerivativeContextModel2([5]),
        pgnmodels.DerivativeContextModel4([5]),
        pgnmodels.DerivativeContextModel2([3,5]),
        pgnmodels.DerivativeContextModel4([3,5]),
        pgnmodels.DerivativeContextModel2([3,5,10]),
        pgnmodels.DerivativeContextModel4([3,5,10])
    ]:
        model_name = type(model).__name__
        if hasattr(model, "thresholds"):
            model_name = model_name + "-" + str(model.thresholds)
        report = run_mapper_reducer(model, chunked_data)

        f.write(f"{model_name} (chunked signal): {report.macro_avg_bits_symbol}\n")
        f.flush()

#report = run_mapper_reducer(, signal_data)
#print(report)
#report = run_mapper_reducer(, signal_data)
#print(report)1
#report = run_mapper_reducer(, signal_data)
#print(report)
#report = run_mapper_reducer(, signal_data)
#print(report)
#report = run_mapper_reducer(, signal_data)
#print(report)
#report = run_mapper_reducer(, signal_data)
#print(report)
#report = run_mapper_reducer(, signal_data)
#print(report)
#report = run_mapper_reducer(, signal_data)
#print(report)
#res = linear_parameter_search([
#    [1],
#    [2],
#    [3],
#    [5],
#    [10],
#    [3,5,10],
#    [1,2,3],
#    [3,5,10,20],
#    [1,2,3,5,10,20,30]
#], signal_data)
#print(res)