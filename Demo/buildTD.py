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
                except Exception:
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


def searchName(namesList, interactionTypeS, index):
    nameAlreadyExists = True
    while(nameAlreadyExists):
        name = click.prompt("Insert %s %d Name" % (interactionTypeS, index), type=SWN_STRING)
        if(name.lower() in namesList):
            click.echo('Error: Thing %s already exists\n' % interactionTypeS)
        else:
            nameAlreadyExists = False  
            return name


def addForm(ctx, opType, cType, interactionTypeS, interactionTypeTD, interactionName, index):
    ctx.obj[interactionTypeTD][interactionName].setdefault('forms', [])
    click.echo("Tip: %s Operation Type has only two possible values ('%s', '%s'). You can choose both or one of them" % (interactionTypeS, opType[0], opType[1]))
    click.echo("Tip: %s Operation Content-Type has only two possible values ('%s', '%s'). The default value is the first" % (interactionTypeS, cType[0], cType[1]))
    numOperationType = click.prompt('Press 1 for insert one %s %d Operation Type or 2 for insert both of them' % (interactionTypeS, index), type=click.IntRange(1,2))
    if(numOperationType == 1):
        ot = click.prompt('%s %d Operation Type' % (interactionTypeS, index), type=click.Choice(opType))
        oct = click.prompt('%s %d Operation Content-Type' % (interactionTypeS, index), type=click.Choice(cType), default=cType[0], show_default=True)
        ctx.obj[interactionTypeTD][interactionName]['forms'].append({'href':'', 'contentType': oct, 'op': [ot]})
    elif(numOperationType == 2):
        inp = click.prompt('%s %d Operation Content-Type' % (interactionTypeS, index), type=click.Choice(cType), default=cType[0], show_default=True)
        ctx.obj[interactionTypeTD][interactionName]['forms'].append({'href': '', 'contentType': inp, 'op': opType})


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
    smElements = click.prompt('Press 1 for single element term or 2 for multiple elements term', type=click.IntRange(1,2))  
    elementType = ''
    if(form):
        click.echo('\nTip: elements MUST be STRINGs')
        elementType = SWN_STRING
    else:
        click.echo('\nTip: elements MUST have primitive type or be JSON OBJECTs')
        elementType = OBJ_STRING   
    if(smElements == 1):
        termValue = click.prompt('Element', type=elementType)
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
    elif(smElements == 2):
        numElements = click.prompt('Number of elements', type=NZ_INT)
        if(form):
            if(interactionTypeS == 'Thing'):
                ctx.obj['forms'][0].setdefault(termName, [])
            else:
                ctx.obj[interactionTypeTD][interactionName]['forms'][0].setdefault(termName, [])
        else:
            if(interactionTypeS == 'Thing'):
                ctx.obj.setdefault(termName, [])    
            else:
                ctx.obj[interactionTypeTD][interactionName].setdefault(termName, [])
        for i in range(1, numElements+1):
            inp = click.prompt('Element %d' % i, type=elementType)
            if(form):
                if(interactionTypeS == 'Thing'):
                    ctx.obj['forms'][0][termName].append(inp) 
                else:
                    ctx.obj[interactionTypeTD][interactionName]['forms'][0][termName].append(inp)
            else:
                if(interactionTypeS == 'Thing'):
                    ctx.obj[termName].append(inp)   
                else:
                    ctx.obj[interactionTypeTD][interactionName][termName].append(inp)       


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
                    name = ''
                    nameAlreadyExists = True
                    while(nameAlreadyExists):
                        name = click.prompt('Object Property %d Name' % i, type=SWN_STRING)
                        if(name.lower() in properties):
                            click.echo('Error: Object Property already exists\n')
                        else:
                            nameAlreadyExists = False
                    properties.append(name.lower())            
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

# dictionary contenente la TD
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
    # THING FORM
    ctx.obj.setdefault('forms', [])
    opType = ['readallproperties', 'writeallproperties', 'readmultipleproperties', 'writemultipleproperties']
    click.echo("\nTip: Thing Operation Type has four possible values ('%s', '%s', '%s', '%s'). You can choose a subset or all of them" % (opType[0], opType[1], opType[2], opType[3]))
    click.echo("Tip: Thing Operation Content-Type has only two possible values ('%s', '%s'). The default value is the first" % (cType[0], cType[1]))
    numOperationType = click.prompt('Press 1 for insert a subset of Thing Operation Types or 2 for insert all of them', type=click.IntRange(1,2))
    if(numOperationType == 1):
        numberOT = click.prompt('Number of Thing Operation Types', type=NZ_INT)
        thingOT = []
        for i in range(1, numberOT+1):
            ot = click.prompt('Thing Operation Type %d' % i, type=click.Choice(opType))
            thingOT.append(ot)
        oct = click.prompt('\nThing Operation Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)
        ctx.obj['forms'].append({'href':'', 'contentType': oct, 'op': thingOT})
    elif(numOperationType == 2):
        inp = click.prompt('Thing Operation Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)
        ctx.obj['forms'].append({'href': '', 'contentType': inp, 'op': opType})                
    # THING FORM RESPONSE
    if(click.confirm('\nInsert Thing Operation Response?', default=False)):
            inp = click.prompt('Insert Thing Operation Response Content-Type', type=click.Choice(cType), default=cType[0], show_default=True)
            ctx.obj['forms'][0]['response'] = inp     
    # THING FORM ADDITIONAL TERMS 
    addTerm(ctx, True, 'Thing')      
    # THING META-TYPE
    addMetaType(ctx, 'Thing')
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
            if(click.confirm('\nInsert Property %d Operation Response?' % p, default=False)):
                inp = click.prompt('Insert Property %d Operation Response Content-Type' % p, type=click.Choice(cType), default=cType[0], show_default=True)
                ctx.obj['properties'][propertyName]['forms'][0]['response'] = inp      
            # PROPERTY FORM ADDITIONAL TERMS 
            addTerm(ctx, True, 'Property', 'properties', propertyName) 
            # PROPERTY TYPE
            if(click.confirm('\nInsert Property %d Type?' % p, default=False)):
                inpType = click.prompt('Property %d Type' % p, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                ctx.obj['properties'][propertyName]['type'] = inpType
                handleThingTypes(ctx, inpType, 'properties', propertyName)       
            # PROPERTY FORMAT
            if(click.confirm('\nInsert Property %d Format?' % p, default=False)):
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
            if(click.confirm('\nInsert Property %d Title?' % p, default=False)):
                inp = click.prompt('Property Title', type=SWN_STRING) 
                ctx.obj['properties'][propertyName]['title'] = inp
            # PROPERTY DESCRIPTION 
            if(click.confirm('\nInsert Property %d Description?' % p, default=False)):
                inp = click.prompt('Property %d Description' % p, type=SWN_STRING) 
                ctx.obj['properties'][propertyName]['description'] = inp 
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
            if(click.confirm('\nInsert Action %d Operation Response?' % a, default=False)):
                inp = click.prompt('Insert Action %d Operation Response Content-Type' % a, type=click.Choice(cType), default=cType[0], show_default=True)
                ctx.obj['actions'][actionName]['forms'][0]['response'] = inp    
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
                        handleThingTypes(ctx, inpType, 'actions', actionName, inpName)  
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
                handleThingTypes(ctx, outType, 'actions', actionName, outName)
            else:
                actionFunctions[a-1]['isOutput'] = False
            # ACTION BODY 
            click.echo('\nTip: The Body of the function corresponding to the Thing Action MUST be written in Embedded-C directly executable in Embedded-Systems')
            click.echo('You have to provide only the code enclosed by braces on one line, neither Function name, inputs, outputs (return)')
            click.echo('This elements are retrived from the information you gave before')
            actionFunctions[a-1].setdefault('body', {})
            actionFunctions[a-1]['body'] = click.prompt('Function Body', type=str) 
            body = actionFunctions[a-1]['body']
            click.echo('\n{}\n'.format(actionFunctions[a-1]))
            click.echo(body)
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
            if(click.confirm('\nInsert Action %d Title?' % a, default=False)):
                inp = click.prompt('Action Title', type=SWN_STRING) 
                ctx.obj['actions'][actionName]['title'] = inp
            # ACTION DESCRIPTION 
            if(click.confirm('\nInsert Action %d Description?' % a, default=False)):
                inp = click.prompt('Action Description', type=SWN_STRING) 
                ctx.obj['actions'][actionName]['description'] = inp    
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
            if(click.confirm('\nInsert Event %d Operation Response?' % e, default=False)):
                inp = click.prompt('Insert Thing %d Operation Response Content-Type' % e, type=click.Choice(cType), default=cType[0], show_default=True)
                ctx.obj['events'][eventName]['forms'][0]['response'] = inp  
            # EVENT FORM ADDITIONAL TERMS    
            addTerm(ctx, True, 'Event', 'events', eventName) 
            # EVENT CONDITION
            click.echo('\nTip: The Event Condition that, when it is True, will trigger the asynchronous data pushing to Consumers, can include every relational and logic operator like standard conditions in programming languages')
            eventConditions[e-1] = click.prompt('Event %d Condition' % e, type=str)
            # EVENT SUBSCRIPTION
            if(click.confirm('\nInsert Event %d Subscription Schema?' % e, default=True)):
                ctx.obj['events'][eventName].setdefault('subscription', {})
                subscriptionTerms = click.prompt('Event %d Subscription Schema number of Terms' % e, type=NN_INT)
                if(subscriptionTerms != 0):
                    subscriptions = []
                    for i in range(1, subscriptionTerms+1):
                        termName = ''
                        termAlreadyExists = True
                        while(termAlreadyExists):
                            termName = click.prompt('Insert Term %d Name' % i, type=SWN_STRING)
                            if(termName.lower() in subscriptions):
                                click.echo('Error: Subscription Term already exists\n')
                            else:
                                termAlreadyExists = False
                        subscriptions.append(termName.lower()) 
                        elementsNumber = click.prompt('Insert Term %d number of elements' % i, type=NZ_INT)       
                        click.echo('\nTip: Event Subscription Term elements MUST have primitive types or be a JSON OBJECTs')
                        if(elementsNumber == 1):
                            termValue = click.prompt('Insert Term %d element' % i, type=OBJ_STRING)
                            ctx.obj['events'][eventName]['subscription'][termName] = termValue
                        elif(elementsNumber > 1):
                            ctx.obj['events'][eventName]['subscription'].detdefault(termName, [])
                        for j in range(1, elementsNumber+1):
                            inp = click.prompt('Insert Term %d element %d' % (i, j), type=OBJ_STRING)
                            ctx.obj['events'][eventName]['subscription'][termName].append(inp)
            # EVENT DATA
            if(click.confirm('\nInsert Event %d Data Schema?' % e, default=True)):
                ctx.obj['events'][eventName].setdefault('data', {})
                dataTerms = click.prompt('Event %d Data Schema number of Terms' % e, type=NN_INT)
                if(dataTerms != 0):
                    data = []
                    for i in range(1, dataTerms+1):
                        termName = ''
                        termAlreadyExists = True
                        while(termAlreadyExists):
                            termName = click.prompt('Insert Term %d Name' % i, type=SWN_STRING)
                            if(termName.lower() in data):
                                click.echo('Error: Data Term already exists\n')
                            else:
                                termAlreadyExists = False
                        data.append(termName.lower()) 
                        elementsNumber = click.prompt('Insert Term %d number of elements' % i, type=NZ_INT)       
                        click.echo('\nTip: Event Data Term elements MUST have primitive types or be a JSON OBJECTs')
                        if(elementsNumber == 1):
                            termValue = click.prompt('Insert Term %d element' % i, type=OBJ_STRING)
                            ctx.obj['events'][eventName]['data'][termName] = termValue
                        elif(elementsNumber > 1):
                            ctx.obj['events'][eventName]['data'].detdefault(termName, [])
                        for j in range(1, elementsNumber+1):
                            inp = click.prompt('Insert Term %d element %d' % (i, j), type=OBJ_STRING)
                            ctx.obj['events'][eventName]['data'][termName].append(inp)  
            # EVENT CANCELLATION
            if(click.confirm('\nInsert Event %d Cancellation Schema?' % e, default=True)):
                ctx.obj['events'][eventName].setdefault('cancellation', {})
                cancellationTerms = click.prompt('Event %d Cancellation Schema number of Terms' % e, type=NN_INT)
                if(cancellationTerms != 0):
                    cancellations = []
                    for i in range(1, cancellationTerms+1):
                        termName = ''
                        termAlreadyExists = True
                        while(termAlreadyExists):
                            termName = click.prompt('Insert Term %d Name' % i, type=SWN_STRING)
                            if(termName.lower() in cancellations):
                                click.echo('Error: Cancellation Term already exists\n')
                            else:
                                termAlreadyExists = False
                        cancellations.append(termName.lower()) 
                        elementsNumber = click.prompt('Insert Term %d number of elements' % i, type=NZ_INT)       
                        click.echo('\nTip: Event Cancellation Term elements MUST have primitive types or be a JSON OBJECTs')
                        if(elementsNumber == 1):
                            termValue = click.prompt('Insert Term %d element' % i, type=OBJ_STRING)
                            ctx.obj['events'][eventName]['cancellation'][termName] = termValue
                        elif(elementsNumber > 1):
                            ctx.obj['events'][eventName]['cancellation'].detdefault(termName, [])
                        for j in range(1, elementsNumber+1):
                            inp = click.prompt('Insert Term %d element %d' % (i, j), type=OBJ_STRING)
                            ctx.obj['events'][eventName]['cancellation'][termName].append(inp)
            # EVENT META-TYPE    
            addMetaType(ctx, 'Event', 'events', eventName)
            # EVENT TITLE
            if(click.confirm('\nInsert Event %d Title?' % e, default=False)):
                inp = click.prompt('Event %d Title' % e, type=SWN_STRING) 
                ctx.obj['events'][eventName]['title'] = inp
            # EVENT DESCRIPTION 
            if(click.confirm('\nInsert Event %d Description?' % e, default=False)):
                inp = click.prompt('Event %d Description' % e, type=SWN_STRING) 
                ctx.obj['events'][eventName]['description'] = inp    
            # EVENT ADDITIONAL TERMS
            addTerm(ctx, False, 'Event', 'events', eventName)         

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