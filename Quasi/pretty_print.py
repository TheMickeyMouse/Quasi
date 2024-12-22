import gdb.printing
import itertools

# TUTORIAL FOR ADDING PRETTY PRINTERS:
# goto the home folder (on windows it's C:/users/$username$
# create a .gdbinit file with contents
# ```
# python
# import sys
# sys.path.append('path/to/a/directory/with_the_pretty_print_file/')
#
# from quasi import register_quasi_printer
# register_quasi_printer()
# ```
# choose the path to put quasi.py (the pretty print file)
# copy this current file to it and rename it to quasi.py
# add an empty __init__.py file
# you're (probably) done!


class BufferPrinterIterator:
    def __init__(self, buf, count):
        self.buf = buf
        self.count = count
        self.i = 0

    def __iter__(self):
        return self

    def __next__(self):
        if self.i == self.count:
            raise StopIteration
        self.i += 1
        return f'[{self.i - 1}]', self.buf[self.i - 1]


class QuasiVecPrinter:
    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return f'{self.typename} of len {self.val["size"]}, cap {self.val["capacity"]}'

    def children(self):
        return BufferPrinterIterator(self.val['data'], self.val['size'])

    def display_hint(self):
        return 'array'


class QuasiSpanPrinter:
    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return f'{self.typename} of len {self.val["size"]}'

    def children(self):
        return BufferPrinterIterator(self.val['data'], self.val['size'])

    def display_hint(self):
        return 'array'


class QuasiArrayPrinter:
    def __init__(self, typename, val):
        self.typename = typename
        self.val = val
        self.n = val.type.template_argument(1)

    def to_string(self):
        return self.typename

    def children(self):
        if self.n == 0:
            return iter(())
        else:
            return BufferPrinterIterator(self.val['array'], self.n)

    def display_hint(self):
        return 'array'


class QuasiOptionPrinter:
    def __init__(self, typename, val):
        self.typename = typename
        self.val = val
        self.some = val.type.template_argument(0)

    def to_string(self):
        return f'Some({self.some})' if self.val['isSome'] else 'None()'

    def children(self):
        if self.val['isSome']:
            yield 'Some', self.val['value']
        return


class QuasiRefPrinter:
    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return self.typename

    def children(self):
        yield
        yield 'Ref', self.val['obj'].dereference()


class QuasiOptRefPrinter:
    def __init__(self, typename, val):
        self.typename = typename
        self.val = val
        self.some = val.type.template_argument(0)

    def to_string(self):
        return f'Some({self.some}&)' if self.val['obj'] else 'None()'

    def children(self):
        if self.val['obj']:
            yield 'Some', self.val['obj']
        return


def quasi_lookup(val):
    lookup: str = val.type.tag
    if lookup is None:
        return None
    if not lookup.startswith('Quasi::'):
        return None
    lookup = lookup[7:]
    last = lookup.find('<')
    last = len(lookup) if last == -1 else last
    match lookup[:last]:
        case 'Vec':    return QuasiVecPrinter    (lookup, val)
        case 'Span':   return QuasiSpanPrinter   (lookup, val)
        case 'Array':  return QuasiArrayPrinter  (lookup, val)
        case 'Option': return QuasiOptionPrinter (lookup, val)
        case 'Ref':    return QuasiRefPrinter    (lookup, val)
        case 'OptRef': return QuasiOptRefPrinter (lookup, val)
    return None


def register_quasi_printer():
    obj = gdb.current_objfile()
    if obj is None:
        obj = gdb
    obj.pretty_printers.append(quasi_lookup)
