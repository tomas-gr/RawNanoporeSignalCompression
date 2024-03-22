import pod5
import sys

def sample_smallest_read(in_path: str, out_path: str):
    min_samples = (1 << 64)
    min_read = None
    with pod5.Reader(in_path) as r:
        for read in r.reads():
            this_samples = read.num_samples
            if this_samples > 0 and this_samples < min_samples:
                min_samples = this_samples
                min_read = read
        if not min_read:
            raise Exception("Input file does not contain any read with non null signal!")
        with pod5.Writer(out_path, "sample_smallest_read") as w:
            w.add_read(min_read.to_read())

def _print_usage_help():
    print("Usage: in_file dest_location")

#python 
#src/python/pgnano/data_obtention/sample_one_read.py
#/data/datananoraw/dna/Human_GGiab10_3_ONT/batch2.pod5
#/data/pgnanoraw/pod5_fork/my_test_data/one_read.pod5
sample_smallest_read("/data/datananoraw/dna/Human_GGiab10_3_ONT/batch2.pod5", "/data/pgnanoraw/pod5_fork/my_test_data/one_read.pod5")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Not enough arguments supplied.")
        _print_usage_help()
        sys.exit(-1)
    if len(sys.argv) > 3:
        print("Too many arguments provided")
        _print_usage_help()
        sys.exit(-1)
    
    try:
        in_path = sys.argv[1]
        out_path = sys.argv[2]
        sample_smallest_read(in_path, out_path)
    except Exception as e:
        print(e)