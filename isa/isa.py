import re
from dataclasses import dataclass
from pprint import pprint
import yaml
from rich.console import Console
from rich.table import Table
import click
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

@dataclass
class Register:
    name: str
    width: int
    lanes: int

class ISA:
    _WIDE_RE = re.compile(r'(\w+)\[(\d+):(\d+)\]')
    def __init__(self, registers, instructions):
        self.registers = registers
        self.instructions = instructions

    @classmethod
    def load(cls, f):
        d = yaml.load(f, Loader=Loader)
        
        register_desc = d['registers']
        registers = []
        for name, props in register_desc.items():
            if m := cls._WIDE_RE.match(name):
                name, start, end = m.groups()
                start, end = int(start), int(end)
                print(name, start, end)
                for i in range(end, start):
                    registers.append(Register(name=f'{name}{i}', width=props['width'], lanes=props.get('lanes', 1)))

        return cls(registers, d['instructions'])

@click.group()
def cli(): ...

@cli.command()
def registers():
    with open('isa.yml') as f: isa = ISA.load(f)
    t = Table('name', 'width', 'lanes')
    scalar_size = 0
    vector_size = 0
    for register in isa.registers:
        if register.lanes == 1:
            scalar_size += register.width
        else:
            vector_size += register.width*register.lanes

        t.add_row(register.name, str(register.width), str(register.lanes))
    
    c = Console()
    c.print(t)
    
    total_size = scalar_size+vector_size
    c.print(f'total size: {total_size} bits ~ {total_size/1024:.2f} kib')
    c.print(f' |- scalar: {scalar_size} bits ~ {scalar_size/1024:.2f} kib')
    c.print(f' |- vector: {vector_size} bits ~ {vector_size/1024:.2f} kib')

if __name__ == '__main__':
    cli()