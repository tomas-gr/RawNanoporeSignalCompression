import pod5
import numpy as np
from pgnano.data_obtention.dataset_retrieval import get_datasets_with_sequencing

output_path = "/data/pinanoraw/rtorrado/Git_Pinanoraw/pod5_nanoraw/benchmark_file.bin" 
signal_count = 5      # Cantidad de Señales / Reads a considerar #

if __name__ == "__main__":
    datasets = get_datasets_with_sequencing()       # TODOS LOS DATASETS #
    
    sample_files = {}
    for dataset in datasets:
        if dataset.dataset not in sample_files.keys():
            sample_files[dataset.dataset] = dataset.path
    
    with open(output_path, "wb") as destination:
        for (_, value) in sample_files.items():
            with pod5.Reader(value) as r:
                remaining_signals = signal_count
                reads = r.reads()
                while remaining_signals > 0:
                    read = next(reads)
                    number_of_chunks = len(read.signal_rows)
                    for i in range(number_of_chunks):
                        chunk = read.signal_for_chunk(i)
                        chunk_lenght = len(chunk)
                        written_bytes = destination.write(np.uint32(chunk_lenght))
                        assert(written_bytes == 4)
                        written_bytes = destination.write(chunk)
                        assert(written_bytes == chunk_lenght * 2)
                    remaining_signals -= 1