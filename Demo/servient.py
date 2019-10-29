from jinja2 import Environment, FileSystemLoader, Template

# se il template viene passato direttamente in input  
# template = Template('Hello {{ name }}!')

# se il template si trova all'interno di un file .txt:
# inizialmente, si deve specificare il percorso dove si trova il file di template e lo si passa a FileSystemLoader,
# poi si dichiara un ambiente a partire dal loader definito precedentemente e infine si carica il file di template 
file_loader = FileSystemLoader('templates')
env = Environment(loader=file_loader)
template = env.get_template('hello_world.txt')

output = template.render(name='John Doe')
print(output)