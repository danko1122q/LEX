/*
 * File Bundler Program
 * This program bundles multiple files into a single C header file by converting
 * each file's contents into a byte array. Useful for embedding resources in C programs.
 */

#include <stdio.h>

/*
 * ARGS_SHIFT Macro
 * Purpose: Decrements argc and increments argv pointer to move to the next argument
 * The do-while(0) pattern ensures the macro behaves like a single statement,
 * allowing it to be used safely in if-statements without braces
 * 
 * Example: If argv = ["prog", "out.h", "file1.txt"]
 *          After ARGS_SHIFT(): argv = ["out.h", "file1.txt"], argc decreases by 1
 */
#define ARGS_SHIFT()                                                                               \
  do                                                                                               \
  {                                                                                                \
    argc--;                                                                                        \
    argv++;                                                                                        \
  }                                                                                                \
  while (0)

int main(int argc, char *argv[])
{
  /*
   * Argument Validation
   * Requires at least 3 arguments: program name, output file, and at least one input file
   * argc < 3 means: program name + output file + 0 input files (insufficient)
   */
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s <output file> files...\n", argv[0]);
    return 1;  // Return non-zero to indicate error
  }

  /*
   * Skip Program Name
   * After this shift: argv[0] becomes the output filename
   * Original argv[0] (program name) is no longer needed
   */
  ARGS_SHIFT();

  /*
   * Open Output File
   * Opens the output header file in write mode ("w")
   * This will overwrite existing file or create new one
   */
  FILE *out = fopen(argv[0], "w");
  if (!out)
  {
    /*
     * Error Handling Bug Fix
     * Original code used argv[1] which is incorrect after ARGS_SHIFT()
     * Should use argv[0] which now points to the output filename
     */
    fprintf(stderr, "Failed to open %s to write.\n", argv[0]);  // Fixed: was argv[1]
    return 1;
  }

  /*
   * Skip Output Filename
   * After this shift: argv[0] points to first input file
   * argc now represents the number of input files to process
   */
  ARGS_SHIFT();

  /*
   * Write Header Guard - Opening
   * Prevents multiple inclusion of the header file
   * Standard practice in C header files to avoid redefinition errors
   */
  fprintf(out, "#ifndef BUNDLE_H\n");
  fprintf(out, "#define BUNDLE_H\n\n");

  /*
   * Main Processing Loop
   * Iterates through all input files and converts each to a byte array
   */
  for (int i = 0; i < argc; i++)
  {
    /*
     * Open Input File
     * Opens in read mode ("r") to read file contents byte by byte
     */
    FILE *fp = fopen(argv[i], "r");
    if (!fp)
    {
      fprintf(stderr, "Failed to open %s to read.\n", argv[i]);
      fclose(out);  // Good practice: close output file before exiting
      return 1;
    }

    /*
     * Write Array Declaration
     * Creates a const char array named bundle0, bundle1, bundle2, etc.
     * Each array will contain the hexadecimal bytes of the corresponding file
     */
    fprintf(out, "const char bundle%d[] = {", i);

    int index = 0;  // Tracks byte position for formatting (10 bytes per line)

    /*
     * File Reading Loop
     * Reads file byte-by-byte and writes as hexadecimal values
     */
    int should_read = 1;
    while (should_read)
    {
      /*
       * Read Single Byte
       * fgetc() returns int (not char) to accommodate EOF value (-1)
       * EOF is returned when end of file is reached
       */
      int byte = fgetc(fp);
      if (byte == EOF)
      {
        /*
         * Null Terminator
         * Adds '\0' at the end to make the array null-terminated
         * This allows the bundled data to be used as a C string if needed
         */
        byte        = 0;
        should_read = 0;  // Exit loop after writing null terminator
      }

      /*
       * Formatting: Line Breaks
       * Adds newline and indentation every 10 bytes for readability
       * Modulo operator (%) gives remainder: 0, 10, 20, 30... trigger new lines
       */
      if (index % 10 == 0)
      {
        fprintf(out, "\n    ");  // 4-space indentation
      }
      
      /*
       * Write Byte as Hexadecimal
       * Format: 0x%02X means hexadecimal with 2 digits, zero-padded
       * Examples: 65 -> 0x41, 10 -> 0x0A, 255 -> 0xFF
       */
      fprintf(out, "0x%02X, ", byte);
      index++;
    }

    /*
     * Close Array Declaration
     * Completes the array syntax and adds blank line for separation
     */
    fprintf(out, "\n};\n\n");
    fclose(fp);  // Close input file after processing
  }

  /*
   * Create Array of Pointers
   * Creates a main array that holds pointers to all bundle arrays
   * This allows easy iteration: bundle[0], bundle[1], bundle[2], etc.
   * Useful for accessing bundled files programmatically
   */
  fprintf(out, "const char* bundle[] = {\n");
  for (int i = 0; i < argc; i++)
  {
    fprintf(out, "    bundle%d,\n", i);
  }
  fprintf(out, "};\n\n");

  /*
   * Write Header Guard - Closing
   * Closes the #ifndef preprocessor directive
   */
  fprintf(out, "#endif\n");
  
  /*
   * Cleanup and Exit
   * Close output file and return 0 to indicate success
   */
  fclose(out);
  return 0;
}