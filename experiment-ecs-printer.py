import gdb.printing


class idPrinter:
    """Print a foo object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return (str(self.val["id"]) +
                " | " + str(self.val["half"][0]) + "," + str(self.val["half"][1]))


#        return self.val["id"]


#    def children(self):
#        yield "full", self.val["id"]
#        yield "half", self.val["half"]


class archetypePrinter:
    """Print a foo object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return self.val["id"]


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("experiment-ecs")
    pp.add_printer('id', '^Id$', idPrinter)
    #    pp.add_printer('archetype', '^Archetype$', archetypePrinter)
    return pp


gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer())
