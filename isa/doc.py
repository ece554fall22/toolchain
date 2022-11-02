from jinja2 import Environment, FileSystemLoader, select_autoescape
from isa import ISA

if __name__ == '__main__':
    env = Environment(
        loader=FileSystemLoader('templates'),
        autoescape=select_autoescape(),
    )

    template = env.get_template('doc.html')

    with open('isa.yml') as f: isa = ISA.load(f)
    with open('doc.html', 'w') as f:
        f.write(template.render(isa=isa))
