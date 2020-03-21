import jinja2
import json

# se il template viene passato direttamente in input  
# template = Template('Hello {{ name }}!')

# se il template si trova all'interno di un file .txt:
# inizialmente, si deve specificare il percorso dove si trova il file di template e lo si passa a FileSystemLoader,
# poi si dichiara un ambiente a partire dal loader definito precedentemente e infine si carica il file di template 

d = json.load(open('prova-td.json'))

#d['forms'][0]['href'] = '" + urlServer + "/properties/" + property1_name + "'

file_loader = jinja2.FileSystemLoader('Templates')
env = jinja2.Environment(loader=file_loader, extensions=['jinja2.ext.do'])

env.trim_blocks = True
env.lstrip_blocks = True
env.rstrip_blocks = True

template = env.get_template('thing-template.txt')

output = template.render(obj=d)
print(output)
