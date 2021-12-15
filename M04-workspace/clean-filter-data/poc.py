import sys

path = sys.argv[1]
other_args = sys.argv[2:]
for index, arg in enumerate(other_args):
   arg = arg.split('=')
   arg[0] = arg[0][2:]
   other_args[index] = arg

other_args=dict(other_args)