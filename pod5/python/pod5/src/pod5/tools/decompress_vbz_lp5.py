import lib_pod5 as lp5
import pod5 as p5
import sys
import os


# def _prepare_add_reads_args(self, reads: Sequence[BaseRead]) -> List[Any]:
#    """
#    Converts the List of reads into the list of ctypes arrays of data to be supplied
#    to the c api.
#    """
#    read_id = np.array(
#        [np.frombuffer(read.read_id.bytes, dtype=np.uint8) for read in reads]
#    )
#    read_number = np.array([read.read_number for read in reads], dtype=np.uint32)
#    start_sample = np.array([read.start_sample for read in reads], dtype=np.uint64)
#    channel = np.array([read.pore.channel for read in reads], dtype=np.uint16)
#    well = np.array([read.pore.well for read in reads], dtype=np.uint8)
#    pore_type = np.array(
#        [self.add(PoreType(read.pore.pore_type)) for read in reads], dtype=np.int16
#    )
#    calib_offset = np.array(
#        [read.calibration.offset for read in reads], dtype=np.float32
#    )
#    calib_scale = np.array([read.calibration.scale for read in reads], dtype=np.float32)
#    median_before = np.array([read.median_before for read in reads], dtype=np.float32)
#    end_reason = np.array([self.add(read.end_reason) for read in reads], dtype=np.int16)
#    end_reason_forced = np.array(
#        [read.end_reason.forced for read in reads], dtype=np.bool_
#    )
#    run_info = np.array([self.add(read.run_info) for read in reads], dtype=np.int16)
#    num_minknow_events = np.array(
#        [read.num_minknow_events for read in reads], dtype=np.uint64
#    )
#    tracked_scaling_scale = np.array(
#        [read.tracked_scaling.scale for read in reads], dtype=np.float32
#    )
#    tracked_scaling_shift = np.array(
#        [read.tracked_scaling.shift for read in reads], dtype=np.float32
#    )
#    predicted_scaling_scale = np.array(
#        [read.predicted_scaling.scale for read in reads], dtype=np.float32
#    )
#    predicted_scaling_shift = np.array(
#        [read.predicted_scaling.shift for read in reads], dtype=np.float32
#    )
#    num_reads_since_mux_change = np.array(
#        [read.num_reads_since_mux_change for read in reads], dtype=np.uint32
#    )
#    time_since_mux_change = np.array(
#        [read.time_since_mux_change for read in reads], dtype=np.float32
#    )
#
#    return [
#        read_id.shape[0],
#        read_id,
#        read_number,
#        start_sample,
#        channel,
#        well,
#        pore_type,
#        calib_offset,
#        calib_scale,
#        median_before,
#        end_reason,
#        end_reason_forced,
#        run_info,
#        num_minknow_events,
#        tracked_scaling_scale,
#        tracked_scaling_shift,
#        predicted_scaling_scale,
#        predicted_scaling_shift,
#        num_reads_since_mux_change,
#        time_since_mux_change,
#    ]


overwrite: bool = True

if __name__ == "__main__":
    in_filename = sys.argv[1]
    out_filename = sys.argv[2]
    if overwrite and os.path.exists(out_filename):
        os.remove(out_filename)

    writer_options: lp5.FileWriterOptions
    writer_options = lp5.FileWriterOptions()
    writer_options.signal_compression_type = 2  # Uncompressed signals
    writer = lp5.create_file(out_filename, "canonizer", writer_options)

    # reader = lp5.open_file(in_filename)
    # p5.pack_read_ids
    # reader.plan_traversal()
    # reader.batch_get_signal()
    # reader.batch_get_signal_batches()
    # reader.close()

    with p5.Reader(in_filename) as reader:
        if reader.is_vbz_compressed:
            print("signal is compressed")
        else:
            print("signal is not compressed, exiting...")
            sys.exit()

        reads = [x for x in reader.reads()]
        writer.add_reads(reads)

        # for read_record in reader.reads():
        #    read_record.signal_rows
        #    read = read_record.to_read()
        #    decompressed_signal=p5.vbz_decompress_signal(compressed_signal=read.signal,sample_count=read.sample_count)
        #    writer.add_read(read)
