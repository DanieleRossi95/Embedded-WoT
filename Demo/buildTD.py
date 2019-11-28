# coding=utf-8

import click
import functools
import json
import jsonschema as js
import ast


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
            self.fail('0 is not an allowed number', param, ctx)


class ObjectStringParamType(click.ParamType):
    name = 'dict'

    def convert(self, value, param, ctx):
        try:
            return ast.literal_eval(value)
        except Exception:
            self.fail('Element format is incorrect\n', param, ctx)


SWN_STRING = StartWithoutNumberStringParamType()
NZ_INT = NotZeroIntParamType()
OBJ_STRING = ObjectStringParamType()

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
    if(click.confirm('\nUse the default context?', default=True)):
        ctx.obj['@context'] = uri
    else:
        contextElement = click.prompt('\nContext elements number', type=NZ_INT)
        click.echo('Warning: String context element MUST be enclosed by single or double quotes')
        click.echo('Mandatory element: %s \n' % uri)
        # se all'interno di un dizionario (ctx.obj) si vuole definire un array a cui aggiungere un
        # elemento alla volta, si possono utilizzare i metodi setdeafult per creare l'array e 
        # append per aggiungere gli elementi
        ctx.obj.setdefault('@context', [])
        for i in range(1, contextElement+1):
            inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
            ctx.obj['@context'].append(inp)
    # THING TYPE
    if(click.confirm('\nInsert Thing type?', default=True)):
        inp = click.prompt('Scelta', type=click.Choice([1,2]), show_choices=True)
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
