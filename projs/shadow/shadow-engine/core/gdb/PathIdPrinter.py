# Start off with defining the printer as a Python object.

class SiginfoPrinter:

    # The constructor takes the value and stores it for later.

    def __init__(self, val):
        self.val = val

    # The to_string method returns the value of the
    # si_signo attribute of the directory.

    def to_string(self):
        signo = self.val['path']
        return str(signo)


# Next, define the lookup function that returns the
# printer object when it receives a siginfo_t.

# The function takes the GDB value-type, which, in
# our example is used to look for the siginfo_t.

def my_pp_func(val):
    if str(val.type) == 'SH::Path': return SiginfoPrinter(val)


# Finally, append the pretty-printer as object/ function to
# the list of registered GDB printers.

gdb.pretty_printers.append(my_pp_func)

# Our pretty-printer is now available when we debug
# the inferior program in GDB.
