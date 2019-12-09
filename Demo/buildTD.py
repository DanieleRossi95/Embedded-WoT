# coding=utf-8

import click
from jinja2 import Template, FileSystemLoader, Environment
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
            if('{' in value):
                raise Exception
            elif(value[0].isdigit() == True):
                raise ValueError
            else:
                return str(value)        
        except TypeError:
            self.fail('Expected string, got {a} of type {b}\n'.format(a=value, b=type(value).__name__), param, ctx)
        except ValueError:
            self.fail('This Element MUST start with a character, not with a number\n', param, ctx)
        except Exception:
            self.fail('OBJECT Type is not allowed\n', param, ctx)    


class ObjectStringParamType(click.ParamType):
    name = 'dict'

    def convert(self, value, param, ctx):
        try:
            if(('{' in value) or (':' in value)):
                return ast.literal_eval(value)  
            else:
                value = value.replace('"', '')
                value = value.replace("'", '')
                isNumber = False
                try:
                    if('.' in value):
                        n = float(value)
                    else:
                        n = int(value)
                    isNumber = True        
                    return n
                except Exception:
                    pass
                if(not(isNumber)):      
                    s = "'" + value + "'"
                    return ast.literal_eval(s)
        except Exception:
            self.fail('Element format is incorrect\n', param, ctx)            


class NotZeroIntParamType(click.ParamType):
    name = 'integer'

    def convert(self, value, param, ctx):
        try:
            try:
                n = int(value)
            except Exception:
                self.fail("Expected string for int() conversion, got '{a}' of type {b}\n".format(a=value, b=type(value).__name__), param, ctx)
            if(n > 0):
                return n
            else:
                raise ValueError
        except ValueError:
            self.fail('0 is not an allowed number\n', param, ctx)


class NonNegativeIntParamType(click.ParamType):
    name = 'integer'

    def convert(self, value, param, ctx):
        try:
            try:
                n = int(value)
            except Exception:
                self.fail("Expected string for int() conversion, got '{a}' of type {b}\n".format(a=value, b=type(value).__name__), param, ctx)
            if(n >= 0):
                return n
            else:
                raise ValueError
        except ValueError:
            self.fail('Negative numbers are not allowed\n', param, ctx)           


class DateTimeParamType(click.ParamType):
    name = 'datetime'            
    
    def convert(self, value, param, ctx):
        try:
            date = datetime.strptime(value, '%m-%d-%Y %H:%M')
            return str(date)
        except Exception:
            self.fail('Element format is incorrect\n', param, ctx)     


def MultipleInputString(inputList, ValidateInputList):
    try:
        if((len(inputList) == 1) and (int(inputList) == 0)):
            return [0]
        else:    
            inputIndexes = inputList.split(' ')
            inputIndexes = [int(i) for i in inputIndexes]
            validateInputIndexes = [i for i in range(1, len(ValidateInputList)+1)]
            if(all(x in max(validateInputIndexes, inputIndexes, key=len) for x in min(inputIndexes, validateInputIndexes, key=len))):
                return inputIndexes
            else:
                return []    
    except Exception:
        return []  


def searchName(namesList, interactionTypeS, index, opType=[]):
    name = ''
    nameAlreadyExists = True
    while(nameAlreadyExists):
        if(interactionTypeS == 'Thing'):
            name = click.prompt("Insert %s Operation Type %d" % (interactionTypeS, index), type=click.Choice(opType))
        else:    
            name = click.prompt("Insert %s %d Name" % (interactionTypeS, index), type=SWN_STRING)
        if(name.lower() in namesList):
            if(interactionTypeS == 'Thing'):
                click.echo('Error: Operation Type already exists\n')
            else:    
                click.echo('Error: Thing %s already exists\n' % interactionTypeS)
        else:
            nameAlreadyExists = False  
            return name


def addForm(ctx, opType, cType, interactionTypeS, interactionTypeTD='', interactionName='', index=0):
    numOperationType = 0
    if(interactionTypeS == 'Thing'):
        ctx.obj.setdefault('forms', [])
        click.echo("\nTip: Thing Operation Type has four possible values ('%s', '%s', '%s', '%s'). You can choose a subset or all of them" % (opType[0], opType[1], opType[2], opType[3]))
        click.echo("Tip: Thing Operation Content-Type has only two possible values ('%s', '%s'). The default value is the first" % (cType[0], cType[1]))
        numOperationType = click.prompt('Press 1 for insert a subset of Thing Operation Types or 2 for insert all of them', type=click.IntRange(1,2))
    else:
        ctx.obj[interactionTypeTD][interactionName].setdefault('forms', [])
        click.echo("Tip: %s Operation Type has only two possible values ('%s', '%s'). You can choose both or one of them" % (interactionTypeS, opType[0], opType[1]))
        click.echo("Tip: %s Operation Content-Type has only two possible values ('%s', '%s'). The default value is the first" % (interactionTypeS, cType[0], cType[1]))
        numOperationType = click.prompt('Press 1 for insert one %s %d Operation Type or 2 for insert both of them' % (interactionTypeS, index), type=click.IntRange(1,2))
    if(numOperationType == 1):
        ot = []
        if(interactionTypeS == 'Thing'):
            numberOT = click.prompt('Number of Thing Operation Types', type=NZ_INT)
            for i in range(1, numberOT+1):
                inp = searchName(ot, 'Thing', i, opType)
                ot.append(inp)
            ct = click.prompt('\nThing Operation Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)    
            ctx.obj['forms'].append({'href': '', 'contentType': ct, 'op': ot}) 
        else:    
            inp = click.prompt('%s %d Operation Type' % (interactionTypeS, index), type=click.Choice(opType))
            ot.append(inp)
            ct = click.prompt('%s %d Operation Content-Type' % (interactionTypeS, index), type=click.Choice(cType), default=cType[0], show_default=True)
            ctx.obj[interactionTypeTD][interactionName]['forms'].append({'href':'', 'contentType': ct, 'op': ot})
    elif(numOperationType == 2):
        if(interactionTypeS == 'Thing'):
            ct = click.prompt('Thing Operation Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)
            ctx.obj['forms'].append({'href': '', 'contentType': ct, 'op': opType})  
        else:                       
            ct = click.prompt('%s %d Operation Content-Type' % (interactionTypeS, index), type=click.Choice(cType), default=cType[0], show_default=True)
            ctx.obj[interactionTypeTD][interactionName]['forms'].append({'href': '', 'contentType': ct, 'op': opType})


def addTerm(ctx, form, interactionTypeS, interactionTypeTD='', interactionName=''):
    terms = []
    question = ''
    if(form):
        question = '\nAdd additional Form Term?'
    else:
        question = '\nAdd additional %s Term?' % interactionTypeS
    while(click.confirm(question, default=False)):
        termName = ''
        termAlreadyExists = True
        while(termAlreadyExists):
            termName = click.prompt('Term name', type=SWN_STRING)
            if(termName.lower() in terms):
                click.echo('Error: Term already exists\n')
            else:
                termAlreadyExists = False    
        terms.append(termName.lower()) 
        elementType = ''
        if(form):
            click.echo('\nTip: Term elements MUST be STRINGs')
            elementType = SWN_STRING
        else:
            click.echo('\nTip: Term elements MUST have primitive type or be JSON OBJECTs')
            elementType = OBJ_STRING   
        termValue = click.prompt('Term Element', type=elementType)
        if(form):
            if(interactionTypeS == 'Thing'):
                ctx.obj['forms'][0][termName] = termValue
            else:
                ctx.obj[interactionTypeTD][interactionName]['forms'][0][termName] = termValue
        else:
            if(interactionTypeS == 'Thing'):
                ctx.obj[termName] = termValue     
            else:
                ctx.obj[interactionTypeTD][interactionName][termName] = termValue                     


def addFormResponse(ctx, cType, interactionTypeS, interactionTypeTD='', interactionName='', index=0):
    if(click.confirm('\nInsert %s Operation Response?' % interactionTypeS, default=False)):
        inp = click.prompt('Insert %s %d Operation Response Content-Type' % (interactionTypeS, index), type=click.Choice(cType), default=cType[0], show_default=True)
        if(interactionTypeS == 'Thing'):
            ctx.obj['forms'][0]['response'] = inp  
        else:    
            ctx.obj[interactionTypeTD][interactionName]['forms'][0]['response'] = inp      


def addMetaType(ctx, interactionTypeS, interactionTypeTD='', interactionName=''):
    if(click.confirm('\nInsert %s Meta-Type?' % interactionTypeS, default=False)):
        typeElements = click.prompt('%s Meta-Type number of elements' % interactionTypeS, type=NN_INT)
        click.echo('\nTip: %s Meta-Type elements MUST be STRINGs' % interactionTypeS)
        if(typeElements == 1):
            inp = click.prompt('Insert element', type=SWN_STRING)
            if(interactionTypeS == 'Thing'):
                ctx.obj['@type'] = inp
            else:
                ctx.obj[interactionTypeTD][interactionName]['@type'] = inp
        elif(typeElements > 1):
            if(interactionTypeS == 'Thing'):
                ctx.obj.setdefault('@type', [])
            else:
                ctx.obj[interactionTypeTD][interactionName].setdefault('@type', [])     
            for i in range(1, typeElements+1):
                inp = click.prompt('Insert element %d' % i, type=SWN_STRING)
                if(interactionTypeS == 'Thing'):
                    ctx.obj['@type'].append(inp)
                else:
                    ctx.obj[interactionTypeTD][interactionName]['@type'].append(inp)


def addTitle(ctx, interactionTypeS, interactionTypeTD='', interactionName='', index=0):
    if(interactionTypeS == 'Thing'):
        thingTitle = click.prompt('Thing Title', type=SWN_STRING)
        ctx.obj['title'] = thingTitle   
    else:
        if(click.confirm('\nInsert %s Title?' % interactionTypeS, default=False)):
            inp = click.prompt('%s %d Title' % (interactionTypeS, index), type=SWN_STRING) 
            ctx.obj[interactionTypeTD][interactionName]['title'] = inp


def addDescription(ctx, interactionTypeS, interactionTypeTD='', interactionName='', index=0):
    if(click.confirm('\nInsert %s Description?' % interactionTypeS, default=False)):
        if(interactionTypeS == 'Thing'):
            inp = click.prompt('Thing Description', type=SWN_STRING)
            ctx.obj['description'] = inp
        else:
            inp = click.prompt('%s %d Description' % (interactionTypeS, index), type=SWN_STRING) 
            ctx.obj[interactionTypeTD][interactionName]['description'] = inp   


def handleThingTypes(ctx, inpType, interactionTypeTD, affordanceName, dataType='', termName=''):
    # INTEGER/NUMBER
    if(inpType == 'integer' or inpType == 'number'):
        if(click.confirm('\nInsert Minimum Value?', default=False)):   
            inp = click.prompt('Minimum Value', type=int)
            if(interactionTypeTD == 'properties'):
                ctx.obj[interactionTypeTD][affordanceName]['minimum'] = inp
            else:
                ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['minimum'] = inp     
        if(click.confirm('Insert Maximum Value?', default=False)):
            inp = click.prompt('Maximum Value', type=int)
            if(interactionTypeTD == 'properties'):
                ctx.obj[interactionTypeTD][affordanceName]['maximum'] = inp   
            else:
                ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['maximum'] = inp     
    # ARRAY
    elif(inpType == 'array'):
        if(click.confirm('\nInsert Array Items?', default=False)):
            arrayItems = click.prompt('Number of Items', type=NN_INT)
            if(arrayItems != 0):
                if(interactionTypeTD == 'properties'):
                    ctx.obj[interactionTypeTD][affordanceName].setdefault('items', [])
                else:
                    ctx.obj[interactionTypeTD][affordanceName][dataType][termName].setdefault('items', [])
                for i in range(1, arrayItems+1):
                    obj = {}
                    itemName = click.prompt('Item %d Name' % i, type=SWN_STRING)
                    obj.setdefault(itemName, {})
                    itemNumTerm = click.prompt('Item %d Number of Terms' % i, type=NZ_INT)
                    for j in range(1, itemNumTerm+1):
                        tName = click.prompt('Item %d Term %d Name' % (i,j), type=SWN_STRING)
                        numElements = click.prompt('Item %d Term %d number of elements' % (i,j), type=NZ_INT)
                        click.echo('\nTip: Term elements MUST have primitive type or be JSON OBJECTs')
                        if(numElements == 1):    
                            tElement = click.prompt('Item %d Term %d element' % (i,j), type=OBJ_STRING)
                            obj[itemName][tName] = tElement
                            if(interactionTypeTD == 'properties'):
                                ctx.obj[interactionTypeTD][affordanceName]['items'].append(obj)
                            else:
                                ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['items'].append(obj)  
                        elif(numElements > 1):
                            obj[itemName].setdefault(tName, [])
                            for z in range(1, numElements+1):
                                value = click.prompt('Item %d Term %d element %d' % (i, j, z), type=OBJ_STRING)
                                obj[itemName][tName].append(value)
                                if(interactionTypeTD == 'properties'):
                                    ctx.obj[interactionTypeTD][affordanceName]['items'].append(obj)
                                else:
                                    ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['items'].append(obj)
        if(click.confirm('\nInsert Array minIntems?', default=None)):
            inp = click.prompt('Array minIntems', type=NN_INT)
            if(interactionTypeTD == 'properties'):
                ctx.obj[interactionTypeTD][affordanceName]['minItems'] = inp
            else:
                ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['minItems'] = inp
        if(click.confirm('Insert Array maxIntems?', default=None)):
            inp = click.prompt('Array maxIntems', type=NN_INT)
            if(interactionTypeTD == 'properties'):
                ctx.obj[interactionTypeTD][affordanceName]['maxItems'] = inp  
            else:
                ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['maxItems'] = inp                 
    # OBJECT
    elif(inpType == 'object'):
        if(click.confirm('\nInsert Object Properties?', default=False)):
            propertyNumber = click.prompt('Number of Object Properties', type=NN_INT)
            if(propertyNumber != 0):
                if(interactionTypeTD == 'properties'):
                    ctx.obj[interactionTypeTD][affordanceName].setdefault('properties', {})
                else:
                    ctx.obj[interactionTypeTD][affordanceName][dataType][termName].setdefault('properties', {})    
                properties = []
                for i in range(1, propertyNumber+1):
                    name = ''
                    nameAlreadyExists = True
                    while(nameAlreadyExists):
                        name = click.prompt('Object Property %d Name' % i, type=SWN_STRING)
                        if(name.lower() in properties):
                            click.echo('Error: Object Property already exists\n')
                        else:
                            nameAlreadyExists = False
                    properties.append(name.lower())     
                    if(interactionTypeTD == 'properties'):
                        ctx.obj[interactionTypeTD][affordanceName]['properties'].setdefault(name, {})    
                    else:
                        ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['properties'].setdefault(name, {})           
                    numTerms = click.prompt('Object Property %d number of Terms' % i, type=NZ_INT)
                    for j in range(1, numTerms+1):
                        tName = click.prompt('Object Property %d Term %d name' % (i,j), type=SWN_STRING)
                        numElements = click.prompt('Object Property %d Term %d number of elements' % (i,j), type=NZ_INT)
                        click.echo('\nTip: Term elements MUST have primitive type or be JSON OBJECTs')
                        if(numElements == 1):    
                            tElement = click.prompt('Object Property %d Term %d element' % (i,j), type=OBJ_STRING)
                            if(interactionTypeTD == 'properties'):
                                ctx.obj[interactionTypeTD][affordanceName]['properties'][name][tName] = tElement
                            else:
                                ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['properties'][name][tName] = tElement    
                        elif(numElements > 1):
                            if(interactionTypeTD == 'properties'):
                                ctx.obj[interactionTypeTD][affordanceName]['properties'][name].setdefault(tName, [])
                            else:
                                ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['properties'][name].setdefault(tName, [])
                            for z in range(1, numElements+1):
                                value = click.prompt('Object Property %d Term %d element %d' % (i, j, z), type=OBJ_STRING)
                                if(interactionTypeTD == 'properties'):
                                    ctx.obj[interactionTypeTD][affordanceName]['properties'][name][tName].append(value)
                                else:
                                    ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['properties'][name][tName].append(value)    
            if(click.confirm('\nInsert which Object Proprerty are required?', default=False)):
                click.echo('\nTip: Insert the indexes divided by one space of the required Object Properties within the previously registered')
                click.echo('Consider index 0 for no required Object Property, 1 as Object Property one, index 2 as Object Property two etc...')
                correctType = False
                while(not(correctType)):
                    inp = click.prompt('Required Object Properties indexes', type=str)
                    inputIndexes = MultipleInputString(inp, properties)
                    if((len(inputIndexes) > 0) and (inputIndexes[0] != 0)):
                        if(interactionTypeTD == 'properties'):
                            ctx.obj[interactionTypeTD][affordanceName].setdefault('required', [])
                            ctx.obj[interactionTypeTD][affordanceName]['required'] = [properties[i-1] for i in inputIndexes]
                        else:
                            ctx.obj[interactionTypeTD][affordanceName][dataType][termName].setdefault('required', [])    
                            ctx.obj[interactionTypeTD][affordanceName][dataType][termName]['required'] = [properties[i-1] for i in inputIndexes]
                        correctType = True
                    elif((len(inputIndexes) > 0) and (inputIndexes[0] == 0)):
                        correctType = True
                    else:    
                        click.echo('Error: Object Properties indexes provided are incorrect\n')  


def handleEventData(ctx, dataTypeS, eventName, index):
    dataTypeTD = dataTypeS.lower()
    if(click.confirm('\nInsert Event %s Schema?' % dataTypeS, default=True)):
        termsNumber = click.prompt('Event %d %s Schema number of Terms' % (index, dataTypeS), type=NN_INT)
        if(termsNumber != 0):
            ctx.obj['events'][eventName].setdefault(dataTypeTD, {})
            termNames = []
            for i in range(1, termsNumber+1):
                termName = ''
                termAlreadyExists = True
                while(termAlreadyExists):
                    termName = click.prompt('Insert Term %d Name' % i, type=SWN_STRING)
                    if(termName.lower() in termNames):
                        click.echo('Error: Subscription Term already exists\n')
                    else:
                        termAlreadyExists = False
                termNames.append(termName.lower())
                ctx.obj['events'][eventName][dataTypeTD].setdefault(termName, {}) 
                if(click.confirm('Insert Term %d Type?' % i, default=True)):
                    inpType = click.prompt('Term %d Type' % i, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                    ctx.obj['events'][eventName][dataTypeTD][termName]['type'] = inpType
                    handleThingTypes(ctx, inpType, 'events', eventName, dataTypeTD, termName)
                while(click.confirm('\nAdd additional Term %d element?' % i, default=False)):
                    termElementName = click.prompt('Insert Element name', type=SWN_STRING)
                    valuesNumber = click.prompt('Insert Element number of values', type=NZ_INT)
                    click.echo('\nTip: Element values MUST have primitive types or be a JSON OBJECTs')
                    if(valuesNumber == 1):
                        termElementValue = click.prompt('Insert Element value', type=OBJ_STRING)
                        ctx.obj['events'][eventName][dataTypeTD][termName][termElementName] = termElementValue
                    elif(valuesNumber > 1): 
                        ctx.obj['events'][eventName][dataTypeTD][termName].setdefault(termElementName, [])
                        for j in range(1, valuesNumber+1):
                            termElementValue = click.prompt('Insert Element value %d' % j, type=OBJ_STRING)
                            ctx.obj['events'][eventName][dataTypeTD][termName][termElementName].append(termElementValue)     


# CUSTOM TYPES
SWN_STRING = StartWithoutNumberStringParamType()
OBJ_STRING = ObjectStringParamType()
NZ_INT = NotZeroIntParamType()
NN_INT = NonNegativeIntParamType() 
DATETIME_STRING = DateTimeParamType()

# JSON SCHEMA per la TD
schema = json.load(open('thing-schema.json'))

# JINJA2 Template
file_loader = FileSystemLoader('Templates')
env = Environment(loader=file_loader, extensions=['jinja2.ext.do'])
env.trim_blocks = True
env.lstrip_blocks = True
env.rstrip_blocks = True

template = env.get_template('thing-template.txt')

# dictionary contenente la TD
td = {}

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
    click.echo('\nWizard start...\n')
    click.echo('THING')
    cType = ['application/json', 'text/html']
    # THING TITLE
    addTitle(ctx, 'Thing')  
    # THING CONTEXT
    uri = 'https://www.w3.org/2019/wot/td/v1'
    if(click.confirm('\nUse the default Thing Context?', default=True)):
        ctx.obj['@context'] = uri
    else:
        contextElements = click.prompt('Thing Context number of elements', type=NZ_INT)
        click.echo('\nTip: Thing Context elements MUST be URIs or JSON OBJECTs')
        click.echo("Mandatory element: '%s'" % uri)
        # se all'interno di un dizionario (ctx.obj) si vuole definire un array a cui aggiungere un
        # elemento alla volta, si possono utilizzare i metodi setdeafult per creare l'array e 
        # append per aggiungere gli elementi
        ctx.obj.setdefault('@context', [])
        for i in range(1, contextElements+1):
            inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
            ctx.obj['@context'].append(inp)  
    # THING FORM
    opType = ['readallproperties', 'writeallproperties', 'readmultipleproperties', 'writemultipleproperties']
    addForm(ctx, opType, cType, 'Thing')          
    # THING FORM RESPONSE
    addFormResponse(ctx, cType, 'Thing')
    # THING FORM ADDITIONAL TERMS 
    addTerm(ctx, True, 'Thing')      
    # THING META-TYPE
    addMetaType(ctx, 'Thing')
    # THING ID
    if(click.confirm('\nInsert Thing ID URI?', default=False)):
        inp = click.prompt('Thing ID URI', type=SWN_STRING)
        ctx.obj['id'] = inp
    # THING DESCRIPTION
    addDescription(ctx, 'Thing')
    # THING VERSION
    if(click.confirm('\nInsert Thing Version?', default=False)):
        inp = click.prompt('Thing Version', type=str)
        ctx.obj['version'] = inp 
    # THING CREATION
    if(click.confirm('\nInsert Thing Creation Date?', default=False)):
        click.echo('\nTip: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('Thing Creation Date', type=click.DateTime(formats=['%m-%d-%Y %H:%M']), default=datetime.now().strftime('%m-%d-%Y %H:%M:%S'))
        inp = str(inp).replace(' ', 'T') 
        ctx.obj['created'] = inp  
    # THING MODIFICATION
    if(click.confirm('\nInsert Thing Modification Date?', default=False)):
        click.echo('\nTip: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('Thing Modification Date', type=click.DateTime(formats=['%m-%d-%Y %H:%M']), default=datetime.now().strftime('%m-%d-%Y %H:%M:%S'))
        inp = str(inp).replace(' ', 'T') 
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
    if(click.confirm('\nInsert Thing Links?', default=False)):
        linksElements = click.prompt('Thing Links number of elements', type=NN_INT)
        if(linksElements != 0):
            click.echo('\nTip: Thing Links elements MUST be JSON OBJECTs') 
            ctx.obj.setdefault('links', [])
            for i in range(1, linksElements+1):
                inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
                ctx.obj['links'].append(inp)
    # THING ADDITIONAL TERMS
    addTerm(ctx, False, 'Thing')       
    # THING PROPERTIES
    click.echo('\n\nTHING PROPERTIES')
    if(click.confirm('Insert Thing Properties?', default=True)):
        ctx.obj.setdefault('properties', {})
        numProperties = click.prompt('Number of Properties', type=NN_INT)
        thingProperties = []
        for p in range(1, numProperties+1):
            # PROPERTY NAME
            click.echo()
            propertyName = searchName(thingProperties, 'Property', p)  
            thingProperties.append(propertyName.lower())
            click.echo('\n%s' % propertyName.upper())
            ctx.obj['properties'].setdefault(propertyName, {})
            # PROPERTY FORM
            opType = ['readproperty', 'writeproperty']
            addForm(ctx, opType, cType, 'Property', 'properties', propertyName, p)
            # PROPERTY FORM RESPONSE
            addFormResponse(ctx, cType, 'Property', 'properties', propertyName, p)
            # PROPERTY FORM ADDITIONAL TERMS 
            addTerm(ctx, True, 'Property', 'properties', propertyName) 
            # PROPERTY TYPE
            if(click.confirm('\nInsert Property Type?', default=True)):
                inpType = click.prompt('Property %d Type' % p, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                ctx.obj['properties'][propertyName]['type'] = inpType
                handleThingTypes(ctx, inpType, 'properties', propertyName)       
            # PROPERTY FORMAT
            if(click.confirm('\nInsert Property Format?', default=False)):
                inp = click.prompt('Property Format', type=SWN_STRING) 
                ctx.obj['properties'][propertyName]['format'] = inp
            # PROPERTY META-TYPE    
            addMetaType(ctx, 'Property', 'properties', propertyName)
            # PROPERTY READONLY/WRITEONLY
            ctx.obj['properties'][propertyName]['observable'] = False  
            op = ctx.obj['properties'][propertyName]['forms'][0]['op']
            if((len(op) == 2) and (op[0] == 'readproperty' and op[1] == 'writeproperty')):
                ctx.obj['properties'][propertyName]['readOnly'] = True  
                ctx.obj['properties'][propertyName]['writeOnly'] = True
            elif((len(op) == 1) and (op[0] == 'readproperty')):
                ctx.obj['properties'][propertyName]['readOnly'] = True  
                ctx.obj['properties'][propertyName]['writeOnly'] = False
            elif((len(op) == 1) and (op[0] == 'writeproperty')):
                ctx.obj['properties'][propertyName]['readOnly'] = False  
                ctx.obj['properties'][propertyName]['writeOnly'] = True
            # PROPERTY TITLE
            addTitle(ctx, 'Property', 'properties', propertyName, p)
            # PROPERTY DESCRIPTION 
            addDescription(ctx, 'Property', 'properties', propertyName, p)
            # PROPERTY ADDITIONAL TERMS
            addTerm(ctx, False, 'Property', 'properties', propertyName)          
    # THING ACTION
    click.echo('\n\nTHING ACTIONS')
    if(click.confirm('Insert Thing Actions?', default=True)):
        ctx.obj.setdefault('actions', {})
        numActions = click.prompt('Number of Actions', type=NN_INT)
        thingActions = []
        actionFunctions = []
        for a in range(1, numActions+1):    
            # ACTION NAME
            click.echo()
            actionName = searchName(thingActions, 'Action', a)    
            thingActions.append(actionName.lower())
            actionFunctions.append({'name':actionName})        
            click.echo('\n%s' % actionName.upper())
            ctx.obj['actions'].setdefault(actionName, {})
            # ACTION FORM          
            ctx.obj['actions'][actionName].setdefault('forms', [])
            inp = click.prompt('Action %d Operation Content-Type' % a, type=click.Choice(cType), default=cType[0], show_default=True)
            ctx.obj['actions'][actionName]['forms'].append({'href': '', 'contentType': inp, 'op': 'invokeaction'})
            # ACTION FORM RESPONSE
            addFormResponse(ctx, cType, 'Action', 'actions', actionName, a)
            # ACTION FORM ADDITIONAL TERMS 
            addTerm(ctx, True, 'Action', 'actions', actionName) 
            # ACTION INPUT
            if(click.confirm('\nAction %d has Inputs?' % a, default=True)):
                inputNumber = click.prompt('Number of Action Inputs', type=NN_INT)
                if(inputNumber != 0):
                    ctx.obj['actions'][actionName].setdefault('input', {})
                    actionFunctions[a-1]['isInput'] = True
                    actionFunctions[a-1]['inputNumber'] = inputNumber 
                    actionFunctions[a-1].setdefault('input', [])
                    for i in range(1, inputNumber+1):
                        inpName = click.prompt('\nAction Input %s Name' % i, type=SWN_STRING)
                        inpType = click.prompt('Action Input %s Type' % i, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                        ctx.obj['actions'][actionName]['input'].setdefault(inpName, {})
                        ctx.obj['actions'][actionName]['input'][inpName]['type'] = inpType
                        actionFunctions[a-1]['input'].append({'name':inpName})
                        actionFunctions[a-1]['input'][i-1]['type'] = inpType
                        handleThingTypes(ctx, inpType, 'actions', actionName, 'input', inpName)  
                else:
                    actionFunctions[a-1]['isInput'] = False
            else:
                actionFunctions[a-1]['isInput'] = False                
            # ACTION OUTPUT
            if(click.confirm('\nAction %d has Output?' % a, default=True)):    
                ctx.obj['actions'][actionName].setdefault('output', {})
                actionFunctions[a-1]['isOutput'] = True
                actionFunctions[a-1].setdefault('output', {})
                outName = click.prompt('\nAction Output Name', type=SWN_STRING)
                outType = click.prompt('Action Output Type', type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                ctx.obj['actions'][actionName]['output'].setdefault(outName, {})
                ctx.obj['actions'][actionName]['output'][outName]['type'] = outType
                actionFunctions[a-1]['output']['name'] = outName
                actionFunctions[a-1]['output']['type'] = outType
                handleThingTypes(ctx, outType, 'actions', actionName, 'output', outName)
            else:
                actionFunctions[a-1]['isOutput'] = False
            # ACTION BODY 
            click.echo('\nTip: The Body of the function corresponding to the Thing Action MUST be written in Embedded-C directly executable in Embedded-Systems')
            click.echo('You have to provide only the code enclosed by braces on one line, neither Function name, inputs, outputs (return)')
            click.echo('This elements are retrived from the information you gave before')
            actionFunctions[a-1].setdefault('body', {})
            actionFunctions[a-1]['body'] = click.prompt('Function Body', type=str) 
            body = actionFunctions[a-1]['body']
            # ACTION SAFETY
            if(click.confirm('\nAction %d is safe?' % a, default=False)):
                ctx.obj['actions'][actionName]['safe'] = True
            else:
                ctx.obj['actions'][actionName]['safe'] = False
            # ACTION IDEMPOTENCY
            if(click.confirm('\nAction %d is idempotent?' % a, default=False)):
                ctx.obj['actions'][actionName]['idempotent'] = True
            else:
                ctx.obj['actions'][actionName]['idempotent'] = False            
            # ACTION META-TYPE    
            addMetaType(ctx, 'Action', 'actions', actionName)
            # ACTION TITLE
            addTitle(ctx, 'Action', 'actions', actionName, a)
            # ACTION DESCRIPTION 
            addDescription(ctx, 'Action', 'actions', actionName, a)
            # ACTION ADDITIONAL TERMS
            addTerm(ctx, False, 'Action', 'actions', actionName)   
    # THING EVENT
    click.echo('\n\nTHING EVENTS')
    if(click.confirm('Insert Thing Events?', default=True)):
        ctx.obj.setdefault('events', {})
        numEvents = click.prompt('Number of Events', type=NN_INT)
        thingEvents = []
        eventConditions = []
        for e in range(1, numEvents+1):    
            # EVENT NAME
            click.echo()
            eventName = searchName(thingEvents, 'Event', e)  
            thingEvents.append(eventName.lower())   
            click.echo('\n%s' % eventName.upper())
            ctx.obj['events'].setdefault(eventName, {})  
            # EVENT FORM
            opType = ["subscribeevent", "unsubscribeevent"]
            addForm(ctx, opType, cType, 'Event', 'events', eventName, e)
            # THING FORM RESPONSE
            addFormResponse(ctx, cType, 'Event', 'events', eventName, e)
            # EVENT FORM ADDITIONAL TERMS    
            addTerm(ctx, True, 'Event', 'events', eventName) 
            # EVENT CONDITION
            tip = ('\nTip: The Event Condition that, when it is True, will trigger the asynchronous data pushing to Consumers,'
                    '\ncan include every relational and logic operator like standard conditions in programming languages but no brackets or punctuation marks')
            click.echo(tip)
            click.echo('Example: if (property1 <= 0)')
            condition = click.prompt('Event %d Condition' % e, type=str)
            eventConditions.append(condition)
            # EVENT SUBSCRIPTION
            handleEventData(ctx, 'Subscription', eventName, e)
            # EVENT DATA
            handleEventData(ctx, 'Data', eventName, e) 
            # EVENT CANCELLATION
            handleEventData(ctx, 'Cancellation', eventName, e)
            # EVENT META-TYPE    
            addMetaType(ctx, 'Event', 'events', eventName)
            # EVENT TITLE
            addTitle(ctx, 'Event', 'events', eventName, e)
            # EVENT DESCRIPTION 
            addDescription(ctx, 'Event', 'events', eventName, e)
            # EVENT ADDITIONAL TERMS
            addTerm(ctx, False, 'Event', 'events', eventName)         
    '''try:
        js.validate(ctx.obj, schema)
    except Exception as e:
        click.echo(str(e))'''
    output = template.render(obj=ctx.obj)    
    click.echo('\n{}'.format(output))
    #click.echo('\n{}'.format(json.dumps(ctx.obj, indent=4)))
    

if __name__ == "__main__":
    # la funzione che viene richiamata nel main è la sola ad essere esguita dalla cli,
    # per cui la cli vede solo le proprietà e gli argomenti da lei acceduti 
    # che poi verranno visualizzati nella documentazione (help) 
    # se si inserisce un'opzione prima del comando, allora viene gestita dalla cli, 
    # se viene inserita dopo, viene gestita dal comando stesso
    cli() 