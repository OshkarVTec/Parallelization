EXECUTION_TIME = 11436.78  # Example execution time in seconds

# Open the file operations_count
file_path = "/home/oskar/Documents/ITC/redes avanzadas/paralelizacion/Parallelization/operations_count.txt"


# Initialize counters for total pixels read and written
total_pixels_read = 0
total_pixels_written = 0

# Read the file and parse the data
with open(file_path, "r") as file:
    for line in file:
        if line.startswith("Total pixels read:"):
            total_pixels_read += int(line.split(":")[1].strip())
        elif line.startswith("Total pixels written:"):
            total_pixels_written += int(line.split(":")[1].strip())

# Calculate the total pixels processed
total_pixels_processed = total_pixels_read + total_pixels_written

# Calculate the total instructions processed
total_instructions_processed = total_pixels_processed * 20

# Calculate millions of instructions per second (MIPS)
mips = (total_instructions_processed / EXECUTION_TIME) / 1_000_000

# Print the results
print(f"Total pixels processed: {total_pixels_processed}")
print(f"Total instructions processed: {total_instructions_processed}")
print(f"Execution time (seconds): {EXECUTION_TIME}")
print(f"Millions of Instructions Per Second (MIPS): {mips}")
