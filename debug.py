#!/data/pgnanoraw/.conda/envs/pgnano/bin/python3.11

import sys
import os
import functools

interpreter = "/data/pinanoraw/rtorrado/.conda/envs/pod5_nanoraw/bin/python3.11"
batch_script = "/data/pinanoraw/rtorrado/Git_Pinanoraw/pod5_nanoraw/src/python/pgnano/test_scripts/batch_test.py"
c_program = "/data/pgnanoraw/pod5_fork/build/src/c++/copy"
c_params = "/data/datananoraw/BAM_TEST/batch15.pod5 /data/pgnanoraw/pod5_fork/my_test_data/out.pod5 --pgnano /data/datananoraw/BAM_TEST/calls2ref.bam"
valgrind = "valgrind"
valgrind_params = "--track-origins=yes --leak-check=full --exit-on-first-error=yes --read-var-info=yes"
c_valgrind_params = c_params
#FIXME: use subprocess instead of os.system
if __name__ == "__main__":
    argc = len(sys.argv)
    if argc >= 2:
        selection = sys.argv[1]
        pass_arguments = sys.argv[2:]
        pass_arguments = functools.reduce(lambda x, y: x + " " + y, pass_arguments, "")
        if selection == "reg":
            print(f"{batch_script} {pass_arguments}")
            os.system(f"{interpreter} {batch_script} {pass_arguments}")
        if selection == "c":
            os.system(f"{c_program} {c_params}")
        if selection == "valgrind":
            os.system(f"{valgrind} {valgrind_params} {c_program} {c_params}")