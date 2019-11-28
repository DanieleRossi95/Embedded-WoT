# coding=utf-8

import click
import functools
import json
import jsonschema as js
import ast
from datetime import datetime


# le funzioni non sono legate alle proprietà anche se ci accedono, 
# per cui possono essere inserite in qualunque parte del codice

class StartWithoutNumberStringParamType(click.ParamType):
    name = 'string'

    def convert(self, value, param, ctx):
        try:
            if(value[0].isdigit() == False):
                return str(value)
            else:
                raise ValueError
        except TypeError:
            self.fail('Expected string, got {a} of type {b}'.format(a=value, b=type(value).__name__), param, ctx)
        except ValueError:
            self.fail('Thing title MUST start with a character, not with a number', param, ctx)


class NotZeroIntParamType(click.ParamType):
    name = 'integer'

    def convert(self, value, param, ctx):
        try:
            if(value != "0"):
                return int(value)
            else:
                raise ValueError
        except TypeError:
            self.fail('Expected string for int() conversion, got {a} of type {b}'.format(a=value, b=type(value).__name__), param, ctx)
        except ValueError:
            self.fail('0 is not an allowed number\n', param, ctx)


class ObjectStringParamType(click.ParamType):
    name = 'dict'

    def convert(self, value, param, ctx):
        try:
            if(('{' in value) or (':' in value)):
                return ast.literal_eval(value)  
            else:
                value = value.replace('"', '')
                value = value.replace("'", '')
                s = "'" + value + "'"
                return ast.literal_eval(s)
        except Exception:
            self.fail('Element format is incorrect\n', param, ctx)


class DateTimeParamType(click.ParamType):
    name = 'datetime'            
    
    def convert(self, value, param, ctx):
        try:
            date = datetime.strptime(value, '%m-%d-%Y %H:%M')
            return str(date)
        except Exception:
            self.fail('Element format is incorrect', param, ctx)     

SWN_STRING = StartWithoutNumberStringParamType()
NZ_INT = NotZeroIntParamType()
OBJ_STRING = ObjectStringParamType()
DATETIME_STRING = DateTimeParamType()

# JSON SCHEMA per la TD
schema = json.load(open('prova_schema.json'))

# document contenente la TD
x = {}


# option in comune ai diversi comandi
def common_options(f):
    options = [
        click.option('--nproperties', type=int, default=0, help='Number of properties of the Thing.'),
        click.option('--thingname', type=str, help='Name of the Thing.'),
    ]
    return functools.reduce(lambda x, opt: opt(x), options, f)
    

# i tre parametri passati alle callback sono obbligatori anche se non vengono utilizzati
# value è il valore assegnato alla proprietà
def compute_properties(ctx, param, value):
    click.echo('Number of properties: %s' % value)
    # se non si ritorna il valore della proprietà, 
    # alla variabile in cli corrispondente alla proprietà non viene assegnato nulla
    # e di conseguenza il valore viene perso
    return value

@click.group(invoke_without_command=True)
@common_options
@click.pass_context
# cli è una callback del gruppo definito precedentemente
def cli(ctx, **kwargs):
    """WoT module for build TDs and executable scripts for embedded systems"""
    click.echo('This module allow you to build custom Thing Descriptions and executable scripts for expose Thing on embedded systems')
    click.echo('Use --help option to see documentation')
    if(ctx.invoked_subcommand is None):
        click.confirm('\nUse the wizard?', default=True, abort=True)
        ctx.invoke(start)
    else:
        pass        
    # appena termina il codice della cli viene eseguito il codice del comando start
    

# le funzioni che vengono eseguite dai comandi non possono essere chiamate al di fuori del comando stesso
@cli.command()
@common_options
# se si passa il contesto ad una callback, allora come parametro le si deve passare ctx obbligatoriamente
@click.pass_context
def start(ctx, thingname, **kwargs):
    """Start wizard"""
    ctx.ensure_object(dict)
    click.echo('\nWizard start...')
    # THING TITLE
    thingTitle = click.prompt('\nThing Title', type=SWN_STRING)
    ctx.obj['title'] = thingTitle     
    # THING CONTEXT
    uri = 'https://www.w3.org/2019/wot/td/v1'
    if(click.confirm('\nUse the default Thing Context?', default=True)):
        ctx.obj['@context'] = uri
    else:
        contextElements = click.prompt('Thing Context number of elements', type=NZ_INT)
        click.echo('Tip: Thing Context elements MUST be URIs or JSON OBJECTs')
        click.echo("Mandatory element: '%s' \n" % uri)
        # se all'interno di un dizionario (ctx.obj) si vuole definire un array a cui aggiungere un
        # elemento alla volta, si possono utilizzare i metodi setdeafult per creare l'array e 
        # append per aggiungere gli elementi
        ctx.obj.setdefault('@context', [])
        for i in range(1, contextElements+1):
            inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
            ctx.obj['@context'].append(inp)        
    # THING TYPE
    if(click.confirm('\nInsert Thing Type?', default=False)):
       typeElements = click.prompt('Thing Type number of elements', type=NZ_INT)
       click.echo('Tip: Thing Type elements MUST be STRINGs\n')
       if(typeElements == 1):
           inp = click.prompt('Insert element', type=SWN_STRING)
           ctx.obj['@type'] = inp
       elif(typeElements > 1):
           ctx.obj.setdefault('@type', [])
           for i in range(1, typeElements+1):
               inp = click.prompt('Insert element %d' % i, type=SWN_STRING)
               ctx.obj['@type'].append(inp)
    # THING ID
    if(click.confirm('\nInsert Thing ID URI?', default=False)):
        inp = click.prompt('Thing ID URI', type=SWN_STRING)
        ctx.obj['id'] = inp
    # THING DESCRIPTION
    if(click.confirm('\nInsert Thing Description?', default=False)):
        inp = click.prompt('Thing Description', type=SWN_STRING)
        ctx.obj['description'] = inp
    # THING VERSION
    if(click.confirm('\nInsert Thing Version?', default=False)):
        inp = click.prompt('Thing Version', type=str)
        ctx.obj['version'] = inp 
    # THING CREATION
    if(click.confirm('\nInsert Thing Creation Date?', default=False)):
        click.echo('Tip: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('\nThing Creation Date', type=DATETIME_STRING)
        ctx.obj['created'] = inp   
    # THING MODIFICATION
    if(click.confirm('\nInsert Thing Modification Date?', default=False)):
        click.echo('Tip: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('\nThing Modification Date', type=DATETIME_STRING)
        ctx.obj['modified'] = inp   
    # THING SUPPORT
    if(click.confirm('\nInsert Thing Support URI?', default=False)):
        inp = click.prompt('Thing Support URI', type=SWN_STRING)
        ctx.obj['support'] = inp
    # THING BASE
    if(click.confirm('\nInsert Thing Base URI?', default=False)):
        inp = click.prompt('Thing Base URI', type=SWN_STRING)
        ctx.obj['base'] = inp   
    # THING LINKS
    if(click.confirm('\nInsert Thing links?', default=False)):
       linksElements = click.prompt('Thing Links number of elements', type=NZ_INT)
       click.echo('Tip: Thing Links elements MUST be JSON OBJECTs\n') 
       ctx.obj.setdefault('links', [])
       for i in range(1, linksElements+1):
           inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
           ctx.obj['links'].append(inp)            
    try:
        js.validate(ctx.obj, schema)
    except Exception as e:
        click.echo(str(e))
    click.echo(ctx.obj)
    


if __name__ == "__main__":
    # la funzione che viene richiamata nel main è la sola ad essere esguita dalla cli,
    # per cui la cli vede solo le proprietà e gli argomenti da lei acceduti 
    # che poi verranno visualizzati nella documentazione (help) 
    # se si inserisce un'opzione prima del comando, allora viene gestita dalla cli, 
    # se viene inserita dopo, viene gestita dal comando stesso
    cli() 
