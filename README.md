# poll-monitoring-system

A program to monitor the poll status using fork, pipe, exec, etc. in C++.

A user would require a csv file to input the initial data for poll participants.

In order for the bulk-vote option to be activated, the user needs to input a text file containing a group of RINs.

## how to compile & run

make

./output -m size -f csv_file(.csv)
