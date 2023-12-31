# Auto-BMC Verification Tool
This is an verification tool that can automatically verify all C functions in the specified directory.

## Features
+ Support file path selection or customise.
+ Built-in function to obtain all function names in C files.
+ Support CBMC and ESBMC model checker.
+ Allow user to set verification parameters and options.
+ Compatible with Windows and Linux systems.
+ Visible verification process and result file writing.

## How to Use it
+ Use `pip install tkinter` to download GUI dependency.
+ Run the python file with `python main.py`.
+ Select BMC and each directory location.
+ Add required verification parameters and options.
+ Click button `Verify` to start verification.
+ Check the verification progress in the bottom text box.
+ The results can be viewed in the output file.
+ Verification for main program needs to be performed manually.  

### The video below illustrates how to use Auto-BMC to verify AUTOSAR standard.  
[![](https://markdown-videos-api.jorgenkh.no/youtube/TtP4cuutl6A)](https://youtu.be/TtP4cuutl6A)

## Improvements
+ Program level verification support.
+ Connect to ChatGPT to autocomplete missing files.
+ Add fuzzing testing.
