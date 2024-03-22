import numpy as np

class ExtendedBincount:
    def __init__(self, xs, ys):
        self.xs = xs
        self.ys = ys

    def __len__(self):
        return sum(self.ys)


def bincount(input: np.ndarray) -> ExtendedBincount:
    min_value = np.min(input)
    max_value = np.max(input)
    xs = np.asarray(range(min_value, max_value + 1))
    if min_value < 0:
        transformed_input = input - min_value
    else:
        transformed_input = input
    ys = np.bincount(transformed_input)
    ys = ys[np.nonzero(ys)[0][0]:]
    return ExtendedBincount(xs, ys)