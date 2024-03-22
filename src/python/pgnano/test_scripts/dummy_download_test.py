import pod5
import subprocess
import os
import random
from pgnano.constants.constants import venv_interpreter_path, observational_equivalence_test_file

# Test that a given file can be correctly read and written
# after download, to discard data corruption
def dummy_download_test(path) -> bool:
    cwd = os.getcwd()
    rnd = random.SystemRandom()
    tmp_path = cwd if cwd.endswith(os.sep) else cwd + os.sep
    tmp_path = cwd + str(rnd.randint(0, 1 << 15))
    error = False

    try:
        with pod5.Reader(path) as r, pod5.Writer(tmp_path) as w:
            for x in r.reads():
                w.add_read(x.to_read())
    except Exception:
        error = True
    
    if not os.path.isfile(tmp_path):
        error = True

    if error:
        if os.path.isfile(tmp_path):
            os.remove(tmp_path)
        return False
    
    comparison_exit_code = subprocess.run([venv_interpreter_path, observational_equivalence_test_file, path, tmp_path]).returncode
    os.remove(tmp_path)
    return comparison_exit_code == 0