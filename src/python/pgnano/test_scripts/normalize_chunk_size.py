import os
from pgnano.test_scripts.run_converters import run_copy

def normalize_chunk_size(path) -> bool:
    tmp_file_path = f"{path}.normalized"
    copy_exit_code = run_copy(path, tmp_file_path, "--VBZ", True).exitcode
    if copy_exit_code != 0:
        try:  
            os.remove(tmp_file_path)
        except:
            pass
        return False
    try:
        os.remove(path)
    except:
        pass
    try:
        os.rename(tmp_file_path, path)
    except:
        pass
    return True