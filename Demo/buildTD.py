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
            self.fail('Expected string, got {a} of type {b}\n'.format(a=value, b=type(value).__name__), param, ctx)
        except ValueError:
            self.fail('Thing title MUST start with a character, not with a number\n', param, ctx)


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
                except Exception as e:
                    pass
                if(not(isNumber)):      
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


def handleThingTypes(ctx, inpType, affordanceType, affordanceName, inpName=''):
    # INTEGER/NUMBER
    if(inpType == 'integer' or inpType == 'number'):
        if(click.confirm('\nInsert Minimum Value?', default=False)):
            inp = click.prompt('Minimum Value', type=int)
            if(affordanceType == 'properties'):
                ctx.obj[affordanceType][affordanceName]['minimum'] = inp
            elif(affordanceType == 'actions'):
                ctx.obj[affordanceType][affordanceName]['input'][inpName]['minimum'] = inp    
        if(click.confirm('Insert Maximum Value?', default=False)):
            inp = click.prompt('Maximum Value', type=int)
            if(affordanceType == 'properties'):
                ctx.obj[affordanceType][affordanceName]['maximum'] = inp
            elif(affordanceType == 'actions'):
                ctx.obj[affordanceType][affordanceName]['input'][inpName]['maximum'] = inp     
    # ARRAY
    elif(inpType == 'array'):
        if(click.confirm('\nInsert Array Items?', default=False)):
            arrayElements = click.prompt('Array Items number of elements', type=NN_INT)
            if(arrayElements != 0):
                click.echo('\nTip: Array elements MUST be JSON OBJECTs')
                if(arrayElements == 1):
                    inp = click.prompt('Element', type=OBJ_STRING)
                    if(affordanceType == 'properties'):
                        ctx.obj[affordanceType][affordanceName]['items'] = inp
                    elif(affordanceType == 'actions'):  
                        ctx.obj[affordanceType][affordanceName]['input'][inpName]['items'] = inp  
                elif(arrayElements > 1):
                    if(affordanceType == 'properties'):
                        ctx.obj[affordanceType][affordanceName].setdefault('items', [])
                    elif(affordanceType == 'actions'):
                        ctx.obj[affordanceType][affordanceName]['input'][inpName].setdefault('items', [])    
                    for i in range(1, arrayElements+1):
                        inp = click.prompt('Element %d' % i, type=OBJ_STRING)
                        if(affordanceType == 'properties'):
                            ctx.obj[affordanceType][affordanceName]['items'].append(inp)
                        elif(affordanceType == 'actions'):
                            ctx.obj[affordanceType][affordanceName]['input'][inpName]['items'].append(inp)
        if(click.confirm('Insert Array minIntems?', default=None)):
            inp = click.prompt('Array minIntems', type=NN_INT)
            if(affordanceType == 'properties'):
                ctx.obj[affordanceType][affordanceName]['minItems'] = inp
            elif(affordanceType == 'actions'):
                ctx.obj[affordanceType][affordanceName]['input'][inpName]['minItems'] = inp
        if(click.confirm('Insert Array maxIntems?', default=None)):
            inp = click.prompt('Array maxIntems', type=NN_INT)
            if(affordanceType == 'properties'):
                ctx.obj[affordanceType][affordanceName]['maxItems'] = inp  
            elif(affordanceType == 'actions'):
                ctx.obj[affordanceType][affordanceName]['input'][inpName]['maxItems'] = inp                 
    # OBJECT
    elif(inpType == 'object'):
        if(click.confirm('\nInsert Object Properties?', default=False)):
            propertyNumber = click.prompt('Number of Object Properties', type=NN_INT)
            if(propertyNumber != 0):
                if(affordanceType == 'properties'):
                    ctx.obj[affordanceType][affordanceName].setdefault('properties', {})
                elif(affordanceType == 'actions'):
                    ctx.obj[affordanceType][affordanceName]['input'][inpName].setdefault('properties', {})    
                properties = []
                click.echo('\nTip: Object Properties elements MUST have primitive types or be JSON OBJECTs')
                for i in range(1, propertyNumber+1):
                    name = click.prompt('Object Property %d Name' % i, type=SWN_STRING)
                    properties.append(name)
                    numElements = click.prompt('Object Property %d number of elements' % i, type=NZ_INT)
                    if(numElements == 1):
                        value = click.prompt('Object Property %d element' % i, type=OBJ_STRING)
                        if(affordanceType == 'properties'):
                            ctx.obj[affordanceType][affordanceName]['properties'][name] = value
                        elif(affordanceType == 'actions'):
                            ctx.obj[affordanceType][affordanceName]['input'][inpName]['properties'][name] = value    
                    elif(numElements > 1):
                        if(affordanceType == 'properties'):
                            ctx.obj[affordanceType][affordanceName]['properties'].setdefault(name, [])
                        elif(affordanceType == 'actions'):
                            ctx.obj[affordanceType][affordanceName]['input'][inpName]['properties'].setdefault(name, [])
                        for j in range(1, numElements+1):
                            value = click.prompt('Object Property %d element %d' % (i, j), type=OBJ_STRING)
                            if(affordanceType == 'properties'):
                                ctx.obj[affordanceType][affordanceName]['properties'][name].append(value)
                            elif(affordanceType == 'actions'):
                                ctx.obj[affordanceType][affordanceName]['input'][inpName]['properties'][name].append(value)    
            if(click.confirm('\nInsert which Object Proprerty are required?', default=False)):
                click.echo('\nTip: Insert the indexes divided by one space of the required Object Properties within the previously registered')
                click.echo('Consider index 0 for no required Object Property, 1 as Object Property one, index 2 as Object Property two etc...')
                correctType = False
                while(not(correctType)):
                    inp = click.prompt('Required Object Properties indexes', type=str)
                    inputIndexes = MultipleInputString(inp, properties)
                    if((len(inputIndexes) > 0) and (inputIndexes[0] != 0)):
                        if(affordanceType == 'properties'):
                            ctx.obj[affordanceType][affordanceName].setdefault('required', [])
                            ctx.obj[affordanceType][affordanceName]['required'] = [properties[i-1] for i in inputIndexes]
                        elif(affordanceType == 'actions'):
                            ctx.obj[affordanceType][affordanceName]['input'][inpName].setdefault('required', [])    
                            ctx.obj[affordanceType][affordanceName]['input'][inpName]['required'] = [properties[i-1] for i in inputIndexes]
                        correctType = True
                    elif((len(inputIndexes) > 0) and (inputIndexes[0] == 0)):
                        correctType = True
                    else:    
                        click.echo('Error: Object Properties indexes provided are incorrect\n')  


# CUSTOM TYPES
SWN_STRING = StartWithoutNumberStringParamType()
NZ_INT = NotZeroIntParamType()
NN_INT = NonNegativeIntParamType() 
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
    click.echo('\nWizard start...\n')
    click.echo('THING')
    cType = ['application/json', 'text/html']
    # THING TITLE
    thingTitle = click.prompt('Thing Title', type=SWN_STRING)
    ctx.obj['title'] = thingTitle     
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
                   
    # THING META-TYPE
    if(click.confirm('\nInsert Thing Meta-Type?', default=False)):
       typeElements = click.prompt('Thing Meta-Type number of elements', type=NN_INT)
       click.echo('\nTip: Thing Meta-Type elements MUST be STRINGs')
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
        click.echo('\nTip: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('Thing Creation Date', type=click.DateTime('%m-%d-%Y %H:%M'))
        ctx.obj['created'] = inp   
    # THING MODIFICATION
    if(click.confirm('\nInsert Thing Modification Date?', default=False)):
        click.echo('\nTip: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('Thing Modification Date', type=click.DateTime('%m-%d-%Y %H:%M'))
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
    while(click.confirm('\nAdd additional Term', default=False)):
        termName = click.prompt('Term name', type=SWN_STRING)
        smElements = click.prompt('Press 1 for single element term or 2 for multiple elements term', type=click.IntRange(1,2))  
        click.echo('\nTip: elements MUST have primitive type or be JSON OBJECTs')
        if(smElements == 1):
            termValue = click.prompt('Insert element', type=OBJ_STRING)
            ctx.obj[termName] = termValue
        elif(smElements == 2):
            ctx.obj.setdefault(termName, [])
            numElements = click.prompt('Number of elements', type=NZ_INT)
            for i in range(1, numElements+1):
                inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
                ctx.obj[termName].append(inp)              
    cType = ['application/json', 'text/html']
    # THING PROPERTIES
    click.echo('\n\nTHING PROPERTIES')
    if(click.confirm('Insert Thing Properties?', default=True)):
        ctx.obj.setdefault('properties', {})
        numProperties = click.prompt('Number of Properties', type=NN_INT)
        for p in range(1, numProperties+1):
            # PROPERTY NAME
            click.echo()
            propertyName = click.prompt("Insert Property %d Name" % p, type=SWN_STRING)
            click.echo('\n%s' % propertyName.upper())
            ctx.obj['properties'].setdefault(propertyName, {})
            # PROPERTY FORM
            ctx.obj['properties'][propertyName].setdefault('forms', [])
            opType = ['readproperty', 'writeproperty']
            click.echo("Tip: Property Operation Type has only two possible values ('%s', '%s'). You can choose both or one of them" % (opType[0], opType[1]))
            click.echo("Tip: Property Operation Content-Type has only two possible values ('%s', '%s'). The default value is the first" % (cType[0], cType[1]))
            numOperationType = click.prompt('Press 1 for insert one Property Operation Type or 2 for insert both of them', type=click.IntRange(1,2))
            if(numOperationType == 1):
                ot = click.prompt('Property Operation Type', type=click.Choice(opType))
                oct = click.prompt('Property Operation Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)
                ctx.obj['properties'][propertyName]['forms'].append({'href':'', 'contentType':oct, 'op': [ot]})
            elif(numOperationType == 2):
                inp = click.prompt('Property Operation Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)
                ctx.obj['properties'][propertyName]['forms'].append({'href': '', 'contentType': inp, 'op': opType})
            # PROPERTY FORM RESPONSE
            if(click.confirm('\nInsert Property Operation Response?', default=False)):
                inp = click.prompt('Insert Property Operation Response Content-Type', type=click.Choice(cType), show_default=True)
                ctx.obj['properties'][propertyName]['forms'][0]['response'] = inp      
            # PROPERTY FORM ADDITIONAL TERMS 
            while(click.confirm('\nAdd additional Form Term?', default=False)):
                termName = click.prompt('Term name', type=SWN_STRING)
                smElements = click.prompt('Press 1 for single element term or 2 for multiple elements term', type=click.IntRange(1,2))  
                click.echo('\nTip: elements MUST be STRINGs')
                if(smElements == 1):
                    termValue = click.prompt('Element', type=SWN_STRING)
                    ctx.obj['properties'][propertyName]['forms'][0][termName] = termValue
                elif(smElements == 2):
                    ctx.obj['properties'][propertyName]['forms'][0].setdefault(termName, [])
                    numElements = click.prompt('Number of elements', type=NZ_INT)
                    for i in range(1, numElements+1):
                        inp = click.prompt('Element %d' % i, type=SWN_STRING)
                        ctx.obj['properties'][propertyName]['forms'][0][termName].append(inp)  
            # PROPERTY TYPE
            if(click.confirm('\nInsert Property Type?', default=False)):
                inpType = click.prompt('Property Type', type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                ctx.obj['properties'][propertyName]['type'] = inpType
                handleThingTypes(ctx, inpType, 'properties', propertyName)       
            # PROPERTY FORMAT
            if(click.confirm('\nInsert Property Format?', default=False)):
                inp = click.prompt('Property Format', type=SWN_STRING) 
                ctx.obj['properties'][propertyName]['format'] = inp
            # PROPERTY META-TYPE    
            if(click.confirm('\nInsert Property Meta-Type?', default=False)):
                typeElements = click.prompt('Property Meta-Type number of elements', type=NN_INT)
                if(typeElements != 0):
                    click.echo('\nTip: Property Meta-Type elements MUST be STRINGs')
                    if(typeElements == 1):
                        inp = click.prompt('Insert element', type=SWN_STRING)
                        ctx.obj['properties'][propertyName]['@type'] = inp
                    elif(typeElements > 1):
                        ctx.obj['properties'][propertyName].setdefault('@type', [])
                        for i in range(1, typeElements+1):
                            inp = click.prompt('Insert element %d' % i, type=SWN_STRING)
                            ctx.obj['properties'][propertyName]['@type'].append(inp)
            # PROPERTY READONLY/WRITEONLY
            op = ctx.obj['properties'][propertyName]['forms'][p-1]['op']
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
            if(click.confirm('\nInsert Property Title?', default=False)):
                inp = click.prompt('Property Title', type=SWN_STRING) 
                ctx.obj['properties'][propertyName]['title'] = inp
            # PROPERTY DESCRIPTION 
            if(click.confirm('\nInsert Property Description?', default=False)):
                inp = click.prompt('Property Description', type=SWN_STRING) 
                ctx.obj['properties'][propertyName]['description'] = inp 
            # PROPERTY ADDITIONAL TERMS
            while(click.confirm('\nAdd additional Term', default=False)):
                termName = click.prompt('Term name', type=SWN_STRING)
                smElements = click.prompt('Press 1 for single element term or 2 for multiple elements term', type=click.IntRange(1,2))  
                click.echo('\nTip: elements MUST have primitive type or be JSON OBJECTs')
                if(smElements == 1):
                    termValue = click.prompt('Insert element', type=OBJ_STRING)
                    ctx.obj['properties'][propertyName][termName] = termValue
                elif(smElements == 2):
                    ctx.obj['properties'][propertyName].setdefault(termName, [])
                    numElements = click.prompt('Number of elements', type=NZ_INT)
                    for i in range(1, numElements+1):
                        inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
                        ctx.obj['properties'][propertyName][termName].append(inp)             
    # THING ACTION
    click.echo('\n\nTHING ACTIONS')
    if(click.confirm('Insert Thing Actions?', default=True)):
        ctx.obj.setdefault('actions', {})
        numActions = click.prompt('Number of Actions', type=NN_INT)
        for a in range(1, numActions+1):    
            # ACTION NAME
            click.echo()
            actionName = click.prompt("Insert Action %d Name" % a, type=SWN_STRING)
            click.echo('\n%s' % actionName.upper())
            ctx.obj['actions'].setdefault(actionName, {})  
            # ACTION FORM          
            ctx.obj['actions'][actionName].setdefault('forms', [])
            inp = click.prompt('Action Operation Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)
            ctx.obj['actions'][actionName]['forms'].append({'href': '', 'contentType': inp, 'op': 'invokeaction'})
            # ACTION FORM RESPONSE
            if(click.confirm('\nInsert Action Operation Response?', default=False)):
                inp = click.prompt('Insert Action Operation Response Content-Type', type=click.Choice(cType), show_default=True)
                ctx.obj['actions'][actionName]['forms'][0]['response'] = inp    
            # ACTION FORM ADDITIONAL TERMS 
            while(click.confirm('\nAdd additional Form Term?', default=False)):
                termName = click.prompt('Term name', type=SWN_STRING)
                smElements = click.prompt('Press 1 for single element term or 2 for multiple elements term', type=click.IntRange(1,2))  
                click.echo('\nTip: elements MUST be STRINGs')
                if(smElements == 1):
                    termValue = click.prompt('Element', type=SWN_STRING)
                    ctx.obj['actions'][actionName]['forms'][0][termName] = termValue
                elif(smElements == 2):
                    ctx.obj['actions'][actionName]['forms'][0].setdefault(termName, [])
                    numElements = click.prompt('Number of elements', type=NZ_INT)
                    for i in range(1, numElements+1):
                        inp = click.prompt('Element %d' % i, type=SWN_STRING)
                        ctx.obj['actions'][actionName]['forms'][0][termName].append(inp)
            # ACTION INPUT
            if(click.confirm('\nThis Action has Inputs?', default=False)):
                inputNumber = click.prompt('Number of Action Inputs', type=NN_INT)
                if(inputNumber != 0):
                    ctx.obj['actions'][actionName].setdefault('input', {})
                    for i in range(1, inputNumber+1):
                        inpName = click.prompt('\nAction Input %s Name' % i, type=SWN_STRING)
                        inpType = click.prompt('Action Input %s Type' % i, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                        ctx.obj['actions'][actionName]['input'].setdefault(inpName, {})
                        ctx.obj['actions'][actionName]['input'][inpName]['type'] = inpType
                        handleThingTypes(ctx, inpType, 'actions', actionName, inpName)  
            # ACTION OUTPUT
            if(click.confirm('\nThis Action has Output?', default=False)):    
                ctx.obj['actions'][actionName].setdefault('output', {})
                outName = click.prompt('\nAction Output Name', type=SWN_STRING)
                outType = click.prompt('Action Output Type', type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                ctx.obj['actions'][actionName]['output'].setdefault(outName, {})
                ctx.obj['actions'][actionName]['output'][outName]['type'] = outType
                handleThingTypes(ctx, outType, 'actions', actionName, outName)    
            # ACTION SAFETY
            if(click.confirm('\nThis Action is safe?', default=False)):
                ctx.obj['actions'][actionName]['safe'] = True
            else:
                ctx.obj['actions'][actionName]['safe'] = False
            # ACTION IDEMPOTENCY
            if(click.confirm('\nThis Action is idempotent?', default=False)):
                ctx.obj['actions'][actionName]['idempotent'] = True
            else:
                ctx.obj['actions'][actionName]['idempotent'] = False            
            # ACTION META-TYPE    
            if(click.confirm('\nInsert Action Meta-Type?', default=False)):
                typeElements = click.prompt('Action Meta-Type number of elements', type=NN_INT)
                if(typeElements != 0):
                    click.echo('\nTip: Action Meta-Type elements MUST be STRINGs')
                    if(typeElements == 1):
                        inp = click.prompt('Insert element', type=SWN_STRING)
                        ctx.obj['actions'][actionName]['@type'] = inp
                    elif(typeElements > 1):
                        ctx.obj['actions'][actionName].setdefault('@type', [])
                        for i in range(1, typeElements+1):
                            inp = click.prompt('Insert element %d' % i, type=SWN_STRING)
                            ctx.obj['actions'][actionName]['@type'].append(inp)  
            # ACTION TITLE
            if(click.confirm('\nInsert Action Title?', default=False)):
                inp = click.prompt('Action Title', type=SWN_STRING) 
                ctx.obj['actions'][actionName]['title'] = inp
            # ACTION DESCRIPTION 
            if(click.confirm('\nInsert Action Description?', default=False)):
                inp = click.prompt('Action Description', type=SWN_STRING) 
                ctx.obj['actions'][actionName]['description'] = inp    
            # ACTION ADDITIONAL TERMS
            while(click.confirm('\nAdd additional Term', default=False)):
                termName = click.prompt('Term name', type=SWN_STRING)
                smElements = click.prompt('Press 1 for single element term or 2 for multiple elements term', type=click.IntRange(1,2))  
                click.echo('\nTip: elements MUST have primitive type or be JSON OBJECTs')
                if(smElements == 1):
                    termValue = click.prompt('Insert element', type=OBJ_STRING)
                    ctx.obj['actions'][actionName][termName] = termValue
                elif(smElements == 2):
                    ctx.obj['actions'][actionName].setdefault(termName, [])
                    numElements = click.prompt('Number of elements', type=NZ_INT)
                    for i in range(1, numElements+1):
                        inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
                        ctx.obj['actions'][actionName][termName].append(inp)                             
    try:
        js.validate(ctx.obj, schema)
    except Exception as e:
        click.echo(str(e))
    click.echo('\n{}'.format(json.dumps(ctx.obj, indent=4)))
    


if __name__ == "__main__":
    # la funzione che viene richiamata nel main è la sola ad essere esguita dalla cli,
    # per cui la cli vede solo le proprietà e gli argomenti da lei acceduti 
    # che poi verranno visualizzati nella documentazione (help) 
    # se si inserisce un'opzione prima del comando, allora viene gestita dalla cli, 
    # se viene inserita dopo, viene gestita dal comando stesso
    cli() 