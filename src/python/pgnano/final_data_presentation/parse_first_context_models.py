import pandas as pd

s = """CppModel (whole signal): 7.111706345241177
VbzModel (whole signal): 7.40003364227046
EvenOddTimestepCodingModel (whole signal): 7.146420740655483
EvenOddDerivativeCodingModel (whole signal): 7.146327261955129
StabilitySeparatorModel (whole signal): 7.931595298335969
DerivativeContextModel (whole signal): 7.11559386393826
DerivativeContextModel2-[3] (whole signal): 7.133451492951723
DerivativeContextModel4-[3] (whole signal): 7.153115500843877
DerivativeContextModel2-[5] (whole signal): 7.131040480419787
DerivativeContextModel4-[5] (whole signal): 7.152457955642823
DerivativeContextModel2-[5, 3] (whole signal): 7.153892050303075
DerivativeContextModel4-[5, 3] (whole signal): 7.187839430362135
DerivativeContextModel2-[10, 5, 3] (whole signal): 7.171400604050297
DerivativeContextModel4-[10, 5, 3] (whole signal): 7.223855718775054
CppModel (chunked signal): 7.113610097103676
VbzModel (chunked signal): 7.389829638289934
EvenOddTimestepCodingModel (chunked signal): 7.162066028176183
EvenOddDerivativeCodingModel (chunked signal): 7.161938953154666
StabilitySeparatorModel (chunked signal): 7.946219271679419
DerivativeContextModel (chunked signal): 7.1297964459921745
DerivativeContextModel2-[3] (chunked signal): 7.146509590722495
DerivativeContextModel4-[3] (chunked signal): 7.18458556969159
DerivativeContextModel2-[5] (chunked signal): 7.145307198122496
DerivativeContextModel4-[5] (chunked signal): 7.18584640454793
DerivativeContextModel2-[5, 3] (chunked signal): 7.176211083215135
DerivativeContextModel4-[5, 3] (chunked signal): 7.233664475230493
DerivativeContextModel2-[10, 5, 3] (chunked signal): 7.2051016311596365
DerivativeContextModel4-[10, 5, 3] (chunked signal): 7.287383385578133
"""

lines = s.splitlines()
names, results = zip(*map(lambda x: x.split(":"), lines))
is_chunked = list(map(lambda x: x.find("(chunked signal)") != -1, names))
names = list(
            map(
                lambda x: x.replace("(whole signal)", ""),
                map(
                    lambda x: x.replace("(chunked signal)", ""),
                    names)))
results = map(lambda x: float(x),results)
df = pd.DataFrame({'names': names, 'results': results})
chunked = df[is_chunked]
non_chunked = df[list(map(lambda x: not x, is_chunked))]
joined = pd.merge(chunked, non_chunked, on="names", suffixes=("-chunked", "-whole"))
joined = joined.sort_values(by="results-chunked")
joined["names"] = joined["names"].apply(lambda x: x.replace(',',';'))
def change_names(x):
    x = x.replace('CppModel', 'Base')
    x = x.replace('VbzModel', 'Vbz')
    x = x.replace('StabilitySeparatorModel', 'Estabilidad')
    x = x.replace('EvenOddDerivativeCodingModel', 'DiferenciaParImpar')
    x = x.replace('EvenOddTimestepCodingModel', 'TiempoParImpar')
    if x.find("DerivativeContextModel2") != -1:
        x = x.replace("DerivativeContextModel2", "Thresholds")
    elif x.find("DerivativeContextModel4") != -1:
        x = x.replace("DerivativeContextModel4", "SignoYThresholds")
    else:
        x = x.replace("DerivativeContextModel", "Signo")
    return x
joined["names"] = joined["names"].apply(change_names)
joined.to_csv("/data/pgnanoraw/pod5_fork/final_data/parsed-first-context-models.csv", index=False)
