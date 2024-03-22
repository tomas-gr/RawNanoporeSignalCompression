import os
import re
import csv

# Initialize the results dictionary
results = {}

# Iterate over all directories in the "results" directory
for database in os.listdir('results'):
    # Initialize the database dictionary
    results[database] = {}

    # Iterate over all directories in the database directory
    for compressor_name in os.listdir(f'results/{database}'):
        # Initialize the compressor dictionary
        results[database][compressor_name] = {}

        # Iterate over all files in the compressor directory
        for filename in os.listdir(f'results/{database}/{compressor_name}'):
            # Open the output file and read its contents
            with open(f'results/{database}/{compressor_name}/{filename}', 'r') as file:
                content = file.read()

            # Check if the file contains required information
            if "Bytes after compression" not in content or "Compression Time (s)" not in content or "Decompression Time (s)" not in content:
                print(f"Skipping file {filename} as it does not contain required information")
                continue

            # Parse the file content to get the required values
            bytes_after_compression = re.search(r'Bytes after compression: (\d+)', content)
            bytes_after_coding = re.search(r'Bytes after coding: (\d+)', content)
            compression_time = re.search(r'Compression Time \(s\): ([\d\.]+)', content)
            decompression_time = re.search(r'Decompression Time \(s\): ([\d\.]+)', content)
            bps_compression = re.search(r'bps compression: ([\d\.]+)', content)
            max_resident_size = re.search(r'Maximum resident set size \(kbytes\): (\d+)', content)

            if bytes_after_compression and bytes_after_coding and compression_time and decompression_time and bps_compression and max_resident_size:
                bytes_value_d = int(bytes_after_compression.group(1))
                bytes_value_c = int(bytes_after_coding.group(1))
                compression_time_value = float(compression_time.group(1))
                decompression_time_value = float(decompression_time.group(1))
                bps_compression_value = float(bps_compression.group(1))
                max_resident_size_value = round(int(max_resident_size.group(1)) / 1000)  # Dividir por 1000 y redondear al entero más cercano

                # Calculate MB/s for Compression Time
                if compression_time_value != 0:
                    compression_mb_per_sec = (bytes_value_c / 1000000) / compression_time_value  # Convert bytes to MB
                else:
                    compression_mb_per_sec = 0

                # Calculate MB/s for Decompression Time
                if decompression_time_value != 0:
                    decompression_mb_per_sec = (bytes_value_d / 1000000) / decompression_time_value  # Convert bytes to MB
                else:
                    decompression_mb_per_sec = 0

                # Add the values to the compressor dictionary
                results[database][compressor_name][filename] = {
                    'Compression MB/s': compression_mb_per_sec,
                    'Decompression MB/s': decompression_mb_per_sec,
                    'bps compression': bps_compression_value,
                    'Maximum resident set size (kbytes)': max_resident_size_value
                }

def write_to_csv(results):
    # Open the CSV file in write mode
    with open('results.csv', 'w', newline='') as file:
        writer = csv.writer(file, delimiter=';')

        # Write the header row
        writer.writerow(['Database', 'Compressor', 'Filename', 'bps compression', 'Compression MB/s', 'Decompression MB/s', 'Maximum resident set size (kbytes)'])

        # Write the data rows
        for database in results:
            for compressor in results[database]:
                for filename, values in results[database][compressor].items():
                    # Write a row for each file
                    writer.writerow([
                        database,
                        compressor,
                        filename,
                        "{:.3f}".format(values['bps compression']),
                        "{:.1f}".format(values['Compression MB/s']),
                        "{:.1f}".format(values['Decompression MB/s']),
                        values['Maximum resident set size (kbytes)']
                    ])

                # After each compressor, write an empty row (for space)
                writer.writerow([])

write_to_csv(results)
