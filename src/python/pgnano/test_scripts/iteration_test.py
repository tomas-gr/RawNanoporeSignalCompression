import os
from pgnano.test_scripts.run_converters import run_copy

def iteration_test(n: int, path: str):
    assert n > 0

    sizes = []
    error = False
    sizes.append(os.stat(path).st_size)

    origin_filename = path
    destination_filename = path + str(1)
    run_copy(origin_filename, destination_filename, "--pgnano")
    if os.path.isfile(destination_filename):
        sizes.append(os.stat(destination_filename).st_size)

    for i in range(1,n):
        origin_filename = path + str(i)
        destination_filename = path + str(i + 1)
        run_copy(origin_filename, destination_filename, "--pgnano")
        if os.path.isfile(path + str(i)):
            sizes.append(os.stat(destination_filename).st_size)
            os.remove(origin_filename)
        else:
            error = True
            break

    if os.path.isfile(destination_filename):
        os.remove(path + str(n))

    if error:
        return []
    else:
        assert len(sizes) == n + 1
        return sizes