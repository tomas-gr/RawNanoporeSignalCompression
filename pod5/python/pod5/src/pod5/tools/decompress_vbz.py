import pod5 as p5
import sys
import os
import re

overwrite: bool = True

if __name__ == "__main__":
    in_filename = sys.argv[1]
    out_filename = sys.argv[2]
    log_filename = (
        re.sub(".pod5", "", in_filename)
        + "_to_"
        + re.sub(".pod5", "", out_filename)
        + ".log.csv"
    )
    if overwrite and os.path.exists(out_filename):
        os.remove(out_filename)
    with p5.Reader(in_filename) as reader, p5.Writer(out_filename) as writer, open(
        log_filename, "w"
    ) as log_file:
        if reader.is_vbz_compressed:
            print("signal is compressed")
        else:
            print("signal is not compressed, exiting...")
            sys.exit()

        log_file.write("read_id, read_number\n")

        for read_record in reader.reads():
            log_msg = f"{read_record.read_id}, {read_record.read_number}\n"
            log_file.write(log_msg)
            read = read_record.to_read()
            writer.add_read(read)
