# Interactive Parallel File Processing with MPI

This project demonstrates how to process multiple files in parallel using MPI (Message Passing Interface). The program reads files, counts the number of words, and performs various operations based on user input.

## Features

- **Word Count**: Counts the number of words in each file.
- **Specific File Processing**: Counts the number of words in a specific file.
- **Global Word Count**: Counts the total number of words across all files.
- **Word Frequency**: Counts the frequency of specific words across files.

## Prerequisites

- **MPI**: Ensure you have MPI installed on your system.
- **CMake**: Ensure you have CMake installed for building the project.

## Building the Project

1. **Clone the Repository**:
   ```sh
   git clone <repository_url>
   cd parallel_file_processing
   ```

2. **Create a Build Directory**:
   ```sh
   mkdir build
   cd build
   ```

3. **Run CMake**:
   ```sh
   cmake ..
   ```

4. **Build the Project**:
   ```sh
   make
   ```

## Running the Program

1. **Navigate to the Build Directory**:
   ```sh
   cd build
   ```

2. **Run the Program with MPI**:
   ```sh
   mpirun -np 4 ./parallel_file_processing ../test/test_2 ../test/test_1
   ```

## Test Program

The test program includes two test files (`test_1` and `test_2`) located in the `test` directory. You can run the program with the following commands:

```sh
cmake ..
make
mpirun -np 4 ./parallel_file_processing ../test/test_2 ../test/test_1
```

## Usage

1. **Enter Operation Number**:
    - `0`: Exit the program.
    - `1`: Count the number of words in every file.
    - `2`: Count the number of words in one specific file.
    - `3`: Count the total number of words in all files.
    - `4`: Get the number of words in one specific file.

2. **Follow the Prompts**:
    - For operations `2` and `4`, you will be prompted to enter the file number to process.
