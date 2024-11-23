import gdb.printing


class idPrinter:
    """Print a TypeId object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return ("ID: " + str(self.val["id"]) + " Flags: " + str(self.val["flags"]))

class TypeIdPairPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        id = idPrinter(self.val["first"])
        return (id.to_string() + " Column: " + "asd")


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("experiment-ecs")
    pp.add_printer('id', '^TypeId$', idPrinter)
    pp.add_printer('id_pair', '^TypeId, int$', TypeIdPairPrinter)

    return pp


gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer())
