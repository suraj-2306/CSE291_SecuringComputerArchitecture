import sys

def count_received_bits_with_noise(file_path):
    bit_1_count = 0
    bit_0_count = 0
    noise_count = 0
    noise2_count = 0
    allhits_count=0
    allmiss_count=0
    equalmiss_count=0

    try:
        with open(file_path, 'r') as file:
            for line in file:
                if "Received bit 1" in line:
                    bit_1_count += 1
                elif "Received bit 0" in line:
                    bit_0_count += 1
                elif "Received bit 2" in line:
                    noise_count += 1
                elif "Received bit 3" in line:
                    noise2_count += 1
                elif "Received bit 5" in line:
                    allhits_count += 1
                elif "Received bit 4" in line:
                    allmiss_count += 1
                elif "Received bit 6" in line:
                    equalmiss_count += 1

        total_count = bit_1_count + bit_0_count
        print(f"Received bit 1: {bit_1_count}")
        print(f"Received bit 0: {bit_0_count}")
        print(f"Total received bits: {total_count}")
        print(f"Noise (Received bit 2): {noise_count}")
        print(f"Noise (Received bit 3): {noise2_count}")
        print(f"Noise (Received bit 4): {allmiss_count}")
        print(f"Noise (Received bit 5): {allhits_count}")      
        print(f"Noise (Received bit 6): {equalmiss_count}")  


    except FileNotFoundError:
        print(f"Error: The file '{file_path}' was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    # Ensure the file path argument is provided
    if len(sys.argv) < 2:
        print("Usage: python count_bits.py <file_path>")
        sys.exit(1)

    # Get the file path from the command-line argument
    file_path = sys.argv[1]
    count_received_bits_with_noise(file_path)