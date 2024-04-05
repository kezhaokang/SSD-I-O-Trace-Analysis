import glob
import csv

# Function to process a single CSV file
def process_csv(file_path):
    w_count = r_count = w_size = r_size = 0

    with open(file_path, 'rt', newline='') as csvfile:
        csvreader = csv.reader(csvfile)
        for row in csvreader:
            if row[3].lower() == 'read':
                r_count += 1
                r_size += int(row[5])
            elif row[3].lower() == 'write':
                w_count += 1
                w_size += int(row[5])

    # Convert counts to millions and sizes to GB, then round to three decimal places
    r_count_millions = round(r_count / 1e6, 3)
    w_count_millions = round(w_count / 1e6, 3)
    r_size_gb = round(r_size / (2**30), 3)
    w_size_gb = round(w_size / (2**30), 3)

    return (w_count_millions, r_count_millions, w_size_gb, r_size_gb)

# Directory path where the CSV files are stored
directory_path = 'C:\\Users\\86159\\Desktop\\trash\\Trace\\MSR'

# Use glob to get all the csv files in the directory
csv_files = glob.glob(f'{directory_path}/*.csv')

# Process all files and store the results
results = {}
for file in csv_files:
    w_count_m, r_count_m, w_size_gb, r_size_gb = process_csv(file)
    results[file] = {
        'Write Count (Millions)': w_count_m,
        'Read Count (Millions)': r_count_m,
        'Write Size (GB)': w_size_gb,
        'Read Size (GB)': r_size_gb
    }

# Output the results
for file, result in results.items():
    print(f"File: {file}")
    for key, value in result.items():
        print(f"{key}: {value}")
    print()
