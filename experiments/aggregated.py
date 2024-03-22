import os
import re
import pprint
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

            # Check if the file contains "Files consistent"
            if "Files consistent" not in content:
                print(f"Skipping file {filename} as it does not contain 'Files consistent'")
                continue

            # Parse the file content to get the metrics
            metrics = re.findall(r'([\w\s]+): ([\d\.]+)', content)

            # Add the metrics to the compressor dictionary
            for metric, value in metrics:
                # Remove any irrelevant character like "\n" or "\t" from the metric name
                metric = metric.strip()
                if metric not in results[database][compressor_name]:
                    results[database][compressor_name][metric] = []
                results[database][compressor_name][metric].append(float(value))


        # Calculate real bps
        total_samples = sum(results[database][compressor_name]['Samples'])
        total_bytes = sum(results[database][compressor_name]['Bytes after compression'])
        total_bps = total_bytes * 8 / total_samples
        results[database][compressor_name]['bps compression'] = total_bps

        if compressor_name == 'KD':
            coded_keys_bytes = sum(results[database][compressor_name]['keys coded size'])
            comp_keys_bytes = sum(results[database][compressor_name]['keys comp size'])
            keys_bps = comp_keys_bytes * 8 / coded_keys_bytes
            results[database][compressor_name]['Keys bps'] = keys_bps

            coded_data_bytes = sum(results[database][compressor_name]['data coded size'])
            comp_data_bytes = sum(results[database][compressor_name]['data comp size'])
            data_bps = comp_data_bytes * 8 / coded_data_bytes
            results[database][compressor_name]['data bps'] = data_bps

        if compressor_name == 'LH':
            coded_keys_bytes = sum(results[database][compressor_name]['keys coded size'])
            comp_keys_bytes = sum(results[database][compressor_name]['keys comp size'])
            keys_bps = comp_keys_bytes * 8 / coded_keys_bytes
            results[database][compressor_name]['Keys bps'] = keys_bps

            coded_M_bytes = sum(results[database][compressor_name]['M coded size'])
            comp_M_bytes = sum(results[database][compressor_name]['M comp size'])
            M_bps = comp_M_bytes * 8 / coded_M_bytes
            results[database][compressor_name]['M bps'] = M_bps

            coded_Lhigh_bytes = sum(results[database][compressor_name]['Lhigh coded size'])
            comp_Lhigh_bytes = sum(results[database][compressor_name]['Lhigh comp size'])
            Lhigh_bps = comp_Lhigh_bytes * 8 / coded_Lhigh_bytes
            results[database][compressor_name]['Lhigh bps'] = Lhigh_bps    

        if compressor_name == 'LL_LH':
            coded_keys_bytes = sum(results[database][compressor_name]['keys coded size'])
            comp_keys_bytes = sum(results[database][compressor_name]['keys comp size'])
            keys_bps = comp_keys_bytes * 8 / coded_keys_bytes
            results[database][compressor_name]['Keys bps'] = keys_bps

            coded_M_bytes = sum(results[database][compressor_name]['M coded size'])
            comp_M_bytes = sum(results[database][compressor_name]['M comp size'])
            M_bps = comp_M_bytes * 8 / coded_M_bytes
            results[database][compressor_name]['M bps'] = M_bps

            coded_Llow_bytes = sum(results[database][compressor_name]['Llow coded size'])
            comp_Llow_bytes = sum(results[database][compressor_name]['Llow comp size'])
            Llow_bps = comp_Llow_bytes * 8 / coded_Llow_bytes
            results[database][compressor_name]['Llow bps'] = Llow_bps

            coded_Lhigh_bytes = sum(results[database][compressor_name]['Lhigh coded size'])
            comp_Lhigh_bytes = sum(results[database][compressor_name]['Lhigh comp size'])
            Lhigh_bps = comp_Lhigh_bytes * 8 / coded_Lhigh_bytes
            results[database][compressor_name]['Lhigh bps'] = Lhigh_bps

        if (compressor_name == "N01" or compressor_name == "N02" or compressor_name == "N03"):
            coded_keys_bytes = sum(results[database][compressor_name]['keys coded size'])
            comp_keys_bytes = sum(results[database][compressor_name]['keys comp size'])
            keys_bps = comp_keys_bytes * 8 / coded_keys_bytes
            results[database][compressor_name]['Keys bps'] = keys_bps

            coded_S_bytes = sum(results[database][compressor_name]['S coded size'])
            comp_S_bytes = sum(results[database][compressor_name]['S comp size'])
            S_bps = comp_S_bytes * 8 / coded_S_bytes
            results[database][compressor_name]['S bps'] = S_bps

            coded_M_bytes = sum(results[database][compressor_name]['M coded size'])
            comp_M_bytes = sum(results[database][compressor_name]['M comp size'])
            M_bps = comp_M_bytes * 8 / coded_M_bytes
            results[database][compressor_name]['M bps'] = M_bps

            coded_Llow_bytes = sum(results[database][compressor_name]['Llow coded size'])
            comp_Llow_bytes = sum(results[database][compressor_name]['Llow comp size'])
            Llow_bps = comp_Llow_bytes * 8 / coded_Llow_bytes
            results[database][compressor_name]['Llow bps'] = Llow_bps

            coded_Lhigh_bytes = sum(results[database][compressor_name]['Lhigh coded size'])
            comp_Lhigh_bytes = sum(results[database][compressor_name]['Lhigh comp size'])
            Lhigh_bps = comp_Lhigh_bytes * 8 / coded_Lhigh_bytes
            results[database][compressor_name]['Lhigh bps'] = Lhigh_bps



        # Calculate the relevant data for each metric
        for metric in results[database][compressor_name]:
            if metric == 'bps compression':
                continue
            elif metric == 'Keys bps':
                continue
            elif metric == 'data bps':
                continue
            elif metric == 'M bps':
                continue
            elif metric == 'Lhigh bps':
                continue
            elif metric == 'Llow bps':
                continue
            elif metric == 'S bps':
                continue
            else:
                values = results[database][compressor_name][metric]
                average = sum(values) / len(values)
                results[database][compressor_name][metric] = average

def write_to_csv(results):
    # Open the CSV file in write mode
    with open('results_aggregated.csv', 'w', newline='') as file:
        writer = csv.writer(file, delimiter=';')

        # Write the header row
        writer.writerow(['Database', 'Compressor', 'Metric', 'Average Value'])

        # Write the data rows
        for database in results:
            for compressor in results[database]:
                for metric in results[database][compressor]:
                    average_value = results[database][compressor][metric]
                    writer.writerow([database, compressor, metric, average_value])

write_to_csv(results)