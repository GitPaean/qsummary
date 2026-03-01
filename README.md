# Qsummary 

qsummary is a lightweight application for plotting summary vectors from reservoir simulations. The main building blocks for this program is the Ecl/IO routines from opm-common and Qt5.

The applications Qsummary don't have a well design graphical user interface with command buttons and menus. If this is what you are looking for, please consider the ResInsight 

https://github.com/OPM/ResInsight

With Qsummary you can create a number of charts from the command line as shown in the example below

```
qsummary NORNE_ATW2013.ESMRY NORNE_ATW2013_SENS1.SMSPEC -v WOPT:B-* -z
```

This command will create 7 charts. 


- Qsummary supports both SMSPEC and ESMRY summary files. 
- Option -z ignors all summary vectors which only holds zero values.
- You can easily export all charts to a PDF file 
   * \<ctrl\> + p and use the file save dialog
   * :pdf <file name> on the application command line 


Use option -h on the command line to get help one command line options, commands and key controls.

## run-flow

`run-flow` is a GUI application for editing and running OPM-flow simulation DATA files. It provides:

- A text editor for viewing and editing DATA files
- Controls to specify the path to the `flow` executable
- A "Run" button to launch the simulation, with real-time progress output
- Automatic loading and plotting of key summary vectors (FPR, FOPR, TCPU) when the simulation completes

Usage:
```
run-flow
```

The application opens a window where you can:
1. Open a DATA file using "File > Open DATA File" or the "Open..." button
2. Edit the file content in the built-in text editor
3. Set the path to the `flow` executable
4. Click "Run" to start the simulation and monitor its progress
5. When the simulation finishes, summary plots are automatically displayed

## Building

Clone opm-common and qsummary and make sure that these two repos a are located next to each other. Start with building opm-common before building qsummary.

