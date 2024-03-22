import re
import pandas as pd
import sys

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise Exception("Incorrect number of parameters passed")
    name = sys.argv[1]
    with open(name, "r") as f:
        lines = f.readlines()
    i = 0
    for line in lines:
        if line.find("Compression Benchmarks") != -1:
            break
        i += 1
    lines = lines[(i+2):]
    #print(list(map(lambda x: x.split(':'), lines))[:3])
    method_name, comp_ratio, speed = zip(*map(lambda x: x.split(':'), lines))
    method_name = list(map(lambda x: x.strip().replace(',', ';'), method_name))
    comp_ratio = list(map(lambda x: x.strip(), comp_ratio))
    speed = list(map(lambda x: x.strip(), speed))
    r = re.compile(r"^((\d)+\.(\d)+).*$")
    comp_ratio = map(lambda x: round(float(r.match(x).group(1)),5), comp_ratio)
    speed = map(lambda x: round(float(r.match(x).group(1)),5), speed)
    df = pd.DataFrame({'Método': method_name, 'Tasa de compresión (bits/símbolo)': comp_ratio, 'Velocidad (MB/s)': speed})
    df.to_csv(name + ".csv", index=False)