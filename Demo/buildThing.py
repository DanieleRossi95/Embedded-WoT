# coding=utf-8
import os
import click
from jinja2 import Template, FileSystemLoader, Environment
import functools
import json
import jsonschema as js
from datetime import datetime


# le funzioni non sono legate alle proprietà anche se ci accedono, 
# per cui possono essere inserite in qualunque parte del codice

class StartWithoutNumberStringParamType(click.ParamType):
    name = 'string'

    def convert(self, value, param, ctx):
        try:
            if(value[0].isdigit() == True):
                raise ValueError
            else:
                return str(value)        
        except TypeError:
            self.fail('Expected string, got {a} of type {b}\n'.format(a=value, b=type(value).__name__), param, ctx)
        except ValueError:
            self.fail('This Element MUST start with a character, not with a number\n', param, ctx)  


class ObjectStringParamType(click.ParamType):
    name = 'dict'

    def convert(self, value, param, ctx):
        try:
            if(('{' in value) or ('}' in value) or ('[' in value) or (']' in value) or (':' in value)):
                return json.loads(value)  
            else:
                num = value.replace('"', '')
                num = value.replace("'", '')
                isNumber = False
                try:
                    if('.' in num):
                        n = float(num)
                    else:
                        n = int(num)
                    isNumber = True        
                    return n
                except Exception:
                    pass
                if(not(isNumber)):      
                    return value
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


class ArduinoLibraryParamType(click.ParamType):
    name = "string"

    def convert(self, value, param, ctx):
        if(value[-2:] == '.h'):
            return value
        else:
            self.fail('Arduino Library .h extension is missing\n', param, ctx)


def MultipleInputString(inputList, validateInputList):
    try:
        if((len(inputList) == 1) and (int(inputList) == 0)):
            return [0]
        else:    
            inputIndexes = inputList.split(' ')
            inputIndexes = [int(i) for i in inputIndexes]
            if(len(inputIndexes) <= len(validateInputList)):
                validateInputIndexes = [i for i in range(1, len(validateInputList)+1)]
                if(all(x in max(validateInputIndexes, inputIndexes, key=len) for x in min(inputIndexes, validateInputIndexes, key=len))):
                    return inputIndexes
                else:
                    raise Exception
            else:
                raise Exception      
    except Exception:
        return []  


def searchNameTD(namesList, interactionTypeS, index, opType=[]):
    name = ''
    nameAlreadyExists = True
    listLower = [name.lower() for name in namesList]
    while(nameAlreadyExists):
        if(interactionTypeS == 'Thing'):
            name = click.prompt("Insert %s Operation Type %d" % (interactionTypeS, index), type=click.Choice(opType), show_choices=False)
        else:    
            name = click.prompt("Insert %s %d Name" % (interactionTypeS, index), type=SWN_STRING)
        if(name.lower() in listLower):
            if(interactionTypeS == 'Thing'):
                click.echo('Error: Operation Type already exists\n')
            else:    
                click.echo('Error: Thing %s already exists\n' % interactionTypeS)
        else:
            nameAlreadyExists = False  
            return name


def searchName(nameList, name):
    nameAlreadyExists = False
    if(name in nameList and len(nameList) > 0):
        nameAlreadyExists = True
    return nameAlreadyExists    


def addForm(ctx, opType, interactionTypeS, interactionTypeTD='', interactionName='', index=0):
    numOperationType = 0
    if(interactionTypeS == 'Thing'):
        ctx.obj['td'].setdefault('forms', [])
        click.echo("\nHint: Thing Operation Type has four possible values ('%s', '%s', '%s', '%s'). You can choose a subset or all of them" % (opType[0], opType[1], opType[2], opType[3]))
        numOperationType = click.prompt('Press 1 for insert a subset of Thing Operation Types or 2 for insert all of them', type=click.IntRange(1,2))
    else:
        ctx.obj['td'][interactionTypeTD][interactionName].setdefault('forms', [])
        click.echo("Hint: %s Operation Type has only two possible values ('%s', '%s'). You can choose both or one of them" % (interactionTypeS, opType[0], opType[1]))
        numOperationType = click.prompt('Press 1 for insert one %s %d Operation Type or 2 for insert both of them' % (interactionTypeS, index), type=click.IntRange(1,2))
    if(numOperationType == 1):
        ot = []
        if(interactionTypeS == 'Thing'):
            numberOT = click.prompt('Number of Thing Operation Types', type=NZ_INT)
            for i in range(1, numberOT+1):
                inp = searchNameTD(ot, 'Thing', i, opType)
                ot.append(inp)
            ctx.obj['td']['forms'].append({'href': '', 'contentType': 'application/json', 'op': ot}) 
        else:    
            inp = click.prompt('%s %d Operation Type' % (interactionTypeS, index), type=click.Choice(opType))
            ot.append(inp)
            ctx.obj['td'][interactionTypeTD][interactionName]['forms'].append({'href':'', 'contentType': 'application/json', 'op': ot})
    elif(numOperationType == 2):
        if(interactionTypeS == 'Thing'):
            ctx.obj['td']['forms'].append({'href': '', 'contentType': 'application/json', 'op': opType})  
        else:                       
            ctx.obj['td'][interactionTypeTD][interactionName]['forms'].append({'href': '', 'contentType': 'application/json', 'op': opType})


def addTerm(ctx, form, interactionTypeS, interactionTypeTD='', interactionName=''):
    terms = []
    question = ''
    if(form):
        if(interactionTypeS == 'Action'):
            question = 'Add additional Form Term?'
        else:
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
            click.echo('\nHint: Term elements MUST be STRINGs')
            elementType = SWN_STRING
        else:
            click.echo('\nHint: Term elements MUST have primitive type or be JSON OBJECTs\nFor JSON OBJECTs it MUST use double quotes instead of single ones')
            elementType = OBJ_STRING   
        termValue = click.prompt('Term Element', type=elementType)
        if(form):
            if(interactionTypeS == 'Thing'):
                ctx.obj['td']['forms'][0][termName] = termValue
            else:
                ctx.obj['td'][interactionTypeTD][interactionName]['forms'][0][termName] = termValue
        else:
            if(interactionTypeS == 'Thing'):
                ctx.obj['td'][termName] = termValue     
            else:
                ctx.obj['td'][interactionTypeTD][interactionName][termName] = termValue                       


def addMetaType(ctx, interactionTypeS, interactionTypeTD='', interactionName=''):
    if(click.confirm('\nInsert %s Meta-Type?' % interactionTypeS, default=False)):
        typeElements = click.prompt('%s Meta-Type number of elements' % interactionTypeS, type=NN_INT)
        click.echo('\nHint: %s Meta-Type elements MUST be STRINGs' % interactionTypeS)
        if(typeElements == 1):
            inp = click.prompt('Insert element', type=SWN_STRING)
            if(interactionTypeS == 'Thing'):
                ctx.obj['td']['@type'] = inp
            else:
                ctx.obj['td'][interactionTypeTD][interactionName]['@type'] = inp
        elif(typeElements > 1):
            if(interactionTypeS == 'Thing'):
                ctx.obj['td'].setdefault('@type', [])
            else:
                ctx.obj['td'][interactionTypeTD][interactionName].setdefault('@type', [])     
            for i in range(1, typeElements+1):
                inp = click.prompt('Insert element %d' % i, type=SWN_STRING)
                if(interactionTypeS == 'Thing'):
                    ctx.obj['td']['@type'].append(inp)
                else:
                    ctx.obj['td'][interactionTypeTD][interactionName]['@type'].append(inp)


def addTitle(ctx, interactionTypeS, interactionTypeTD='', interactionName='', index=0):
    if(interactionTypeS == 'Thing'):
        thingTitle = click.prompt('Thing Title', type=SWN_STRING)
        ctx.obj['td']['title'] = thingTitle   
    else:
        if(click.confirm('\nInsert %s Title?' % interactionTypeS, default=False)):
            inp = click.prompt('%s %d Title' % (interactionTypeS, index), type=SWN_STRING) 
            ctx.obj['td'][interactionTypeTD][interactionName]['title'] = inp


def addDescription(ctx, interactionTypeS, interactionTypeTD='', interactionName='', index=0):
    if(click.confirm('\nInsert %s Description?' % interactionTypeS, default=False)):
        if(interactionTypeS == 'Thing'):
            inp = click.prompt('Thing Description', type=SWN_STRING)
            ctx.obj['td']['description'] = inp
        else:
            inp = click.prompt('%s %d Description' % (interactionTypeS, index), type=SWN_STRING) 
            ctx.obj['td'][interactionTypeTD][interactionName]['description'] = inp   


def handleThingTypes(ctx, inpType, interactionTypeTD, interactionName, dataType='', termName='', actionIndex=(-1, -1), array=False, obj=False, pName=''):
    # INTEGER/NUMBER
    if(inpType == 'integer' or inpType == 'number'):
        if(click.confirm('\nInsert Minimum Value?', default=False)):
            inp = 0
            if(inpType == 'integer'): 
                inp = click.prompt('Minimum Value', type=int)
            elif(inpType == 'number'):
                inp = click.prompt('Minimum Value', type=float)
            if(interactionTypeTD == 'properties'):
                if(array and obj):
                    ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName]['items']['minimum'] = inp
                elif(array):
                    ctx.obj['td'][interactionTypeTD][interactionName]['items']['minimum'] = inp 
                elif(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName]['minimum'] = inp
                else:
                    ctx.obj['td'][interactionTypeTD][interactionName]['minimum'] = inp    
            else:
                if(array and obj):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName]['items']['minimum'] = inp 
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName]['items']['minimum'] = inp 
                elif(array):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items']['minimum'] = inp 
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items']['minimum'] = inp 
                elif(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName]['minimum'] = inp 
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName]['minimum'] = inp
                else:
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['minimum'] = inp  
                    if(interactionTypeTD == 'actions'):
                       actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['minimum'] = inp   
        if(click.confirm('Insert Maximum Value?', default=False)):
            inp = 0
            if(inpType == 'integer'): 
                inp = click.prompt('Maximum Value', type=int)
            elif(inpType == 'number'):
                inp = click.prompt('Maximum Value', type=float)
            if(interactionTypeTD == 'properties'):
                if(array and obj):
                    ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName]['items']['maximum'] = inp
                elif(array):
                    ctx.obj['td'][interactionTypeTD][interactionName]['items']['maximum'] = inp 
                elif(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName]['maximum'] = inp
                else:
                    ctx.obj['td'][interactionTypeTD][interactionName]['maximum'] = inp    
            else:
                if(array and obj):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName]['items']['maximum'] = inp 
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName]['items']['maximum'] = inp 
                elif(array):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items']['maximum'] = inp 
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items']['maximum'] = inp 
                elif(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName]['maximum'] = inp 
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName]['maximum'] = inp
                else:
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['maximum'] = inp  
                    if(interactionTypeTD == 'actions'):
                       actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['maximum'] = inp   
    # ARRAY
    elif(inpType == 'array'):
        itemsType = click.prompt('\nArray Items Type', type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'null']), show_default=True)
        if(interactionTypeTD == 'properties'):
            if(obj):
                ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName].setdefault('items', {})
                ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName]['items']['type'] = itemsType
                handleThingTypes(ctx, itemsType, interactionTypeTD, interactionName, array=True, obj=True, pName=pName)
            else:    
                ctx.obj['td'][interactionTypeTD][interactionName].setdefault('items', {})   
                ctx.obj['td'][interactionTypeTD][interactionName]['items']['type'] = itemsType
                handleThingTypes(ctx, itemsType, interactionTypeTD, interactionName, array=True)
        else:
            if(obj):
                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName].setdefault('items', {})
                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName]['items'] = itemsType
                if(interactionTypeTD == 'actions'):
                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName].setdefault('items', {})
                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName]['items'] = itemsType
                handleThingTypes(ctx, itemsType, interactionTypeTD, interactionName, dataType, termName, actionIndex, True, True, pName)
            else:    
                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName].setdefault('items', {})
                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items']['type'] = itemsType
                if(interactionTypeTD == 'actions'):
                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]].setdefault('items', {})
                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items']['type'] = itemsType
                handleThingTypes(ctx, itemsType, interactionTypeTD, interactionName, dataType, termName, actionIndex, array=True)         
        if(click.confirm('\nInsert Array minIntems?', default=None)):
            inp = click.prompt('Array minIntems', type=NN_INT)
            if(interactionTypeTD == 'properties'):
                if(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName]['minItems'] = inp
                else:    
                    ctx.obj['td'][interactionTypeTD][interactionName]['minItems'] = inp
            else:
                if(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName]['minItems'] = inp
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName]['minItems'] = inp
                else:        
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['minItems'] = inp
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['minItems'] = inp 
        if(click.confirm('Insert Array maxItems?', default=None)):
            inp = click.prompt('Array maxItems', type=NN_INT)
            if(interactionTypeTD == 'properties'):
                if(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName]['properties'][pName]['maxItems'] = inp
                else:    
                    ctx.obj['td'][interactionTypeTD][interactionName]['maxItems'] = inp
            else:
                if(obj):
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][pName]['maxItems'] = inp
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][pName]['maxItems'] = inp
                else:        
                    ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['maxItems'] = inp
                    if(interactionTypeTD == 'actions'):
                        actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['maxItems'] = inp               
    # OBJECT
    elif(inpType == 'object'):
        if(click.confirm('\nInsert Object Properties?', default=False)):
            propertyNumber = click.prompt('Number of Object Properties', type=NN_INT)
            if(propertyNumber != 0):
                if(interactionTypeTD == 'properties'):
                    if(array):
                        ctx.obj['td'][interactionTypeTD][interactionName]['items'].setdefault('properties', {})
                    else:
                        ctx.obj['td'][interactionTypeTD][interactionName].setdefault('properties', {})
                else:
                    if(array):
                        ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items'].setdefault('properties', {})
                        if(interactionTypeTD == 'actions'):
                            actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items'].setdefault('properties', [])
                    else:
                        ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName].setdefault('properties', {}) 
                        if(interactionTypeTD == 'actions'):
                            actionFunctions[actionIndex[0]][dataType][actionIndex[1]].setdefault('properties', [])
                properties = []
                for i in range(1, propertyNumber+1):
                    propertyName = ''
                    nameAlreadyExists = True
                    while(nameAlreadyExists):
                        propertyName = click.prompt('\nObject Property %d Name' % i, type=SWN_STRING)
                        if(propertyName.lower() in properties):
                            click.echo('Error: Object Property already exists\n')
                        else:
                            nameAlreadyExists = False
                    properties.append(propertyName.lower())   
                    objProperty = {}  
                    if(interactionTypeTD == 'properties'):
                        if(array):
                            ctx.obj['td'][interactionTypeTD][interactionName]['items']['properties'].setdefault(propertyName, {})  
                        else:
                            ctx.obj['td'][interactionTypeTD][interactionName]['properties'].setdefault(propertyName, {})         
                    else:
                        objProperty.setdefault(propertyName, {})  
                        if(array):
                            ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items']['properties'].setdefault(propertyName, {})
                            if(interactionTypeTD == 'actions'):
                                actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items']['properties'].setdefault(propertyName, {})  
                        else:
                            ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'].setdefault(propertyName, {})  
                            if(interactionTypeTD == 'actions'):
                                actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'].setdefault(propertyName, {}) 
                    propertyType = click.prompt('Object Property %d Type' % i, type=click.Choice(['boolean', 'integer', 'number', 'string', 'array', 'null']))
                    if(interactionTypeTD == 'properties'):
                        if(array):
                            ctx.obj['td'][interactionTypeTD][interactionName]['items']['properties'][propertyName]['type'] = propertyType   
                            handleThingTypes(ctx, propertyType, interactionTypeTD, interactionName, array=True, obj=True, pName=propertyName) 
                        else:
                            ctx.obj['td'][interactionTypeTD][interactionName]['properties'][propertyName]['type'] = propertyType
                            handleThingTypes(ctx, propertyType, interactionTypeTD, interactionName, obj=True, pName=propertyName)        
                    else:
                        objProperty[propertyName]['type'] = propertyType 
                        if(array):
                            ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items']['properties'][propertyName]['type'] = propertyType
                            if(interactionTypeTD == 'actions'):
                                actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items']['properties'][propertyName]['type'] = propertyType
                            handleThingTypes(ctx, propertyType, interactionTypeTD, interactionName, dataType, termName, actionIndex, True, True, propertyName)
                        else:
                            ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][propertyName]['type'] = propertyType
                            if(interactionTypeTD == 'actions'):
                                actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['properties'][propertyName]['type'] = propertyType
                            handleThingTypes(ctx, propertyType, interactionTypeTD, interactionName, dataType, termName, actionIndex, False, True, propertyName)
            if(click.confirm('\nInsert which Object Proprerty are required?', default=False)):
                click.echo('\nHint: Insert the indexes divided by one space of the required Object Properties within the previously registered')
                click.echo('Consider index 0 for no required Object Property, 1 as Object Property one, index 2 as Object Property two etc...')
                correctType = False
                while(not(correctType)):
                    inp = click.prompt('Required Object Properties indexes', type=str)
                    inputIndexes = MultipleInputString(inp, properties)
                    if((len(inputIndexes) > 0) and (inputIndexes[0] != 0)):
                        if(interactionTypeTD == 'properties'):
                            if(array):
                                ctx.obj['td'][interactionTypeTD][interactionName]['items'].setdefault('required', [])
                                ctx.obj['td'][interactionTypeTD][interactionName]['items']['required'] = [properties[i-1] for i in inputIndexes]
                            else: 
                                ctx.obj['td'][interactionTypeTD][interactionName].setdefault('required', [])
                                ctx.obj['td'][interactionTypeTD][interactionName]['required'] = [properties[i-1] for i in inputIndexes]   
                        else:
                            if(array):
                                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items'].setdefault('required', [])    
                                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items']['required'] = [properties[i-1] for i in inputIndexes]
                                if(interactionTypeTD == 'actions'):
                                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items'].setdefault('required', [])
                                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['items']['required'] = [properties[i-1] for i in inputIndexes]
                            else:
                                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName].setdefault('required', [])    
                                ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['required'] = [properties[i-1] for i in inputIndexes]
                                if(interactionTypeTD == 'actions'):
                                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]].setdefault('required', [])
                                    actionFunctions[actionIndex[0]][dataType][actionIndex[1]]['required'] = [properties[i-1] for i in inputIndexes]
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
            ctx.obj['td']['events'][eventName].setdefault(dataTypeTD, {})
            termNames = []
            for i in range(1, termsNumber+1):
                termName = ''
                termAlreadyExists = True
                while(termAlreadyExists):
                    termName = click.prompt('Insert Term %d Name' % i, type=SWN_STRING)
                    if(termName.lower() in termNames):
                        click.echo('Error: %s Term already exists\n' % dataTypeS)
                    else:
                        termAlreadyExists = False
                termNames.append(termName.lower())
                ctx.obj['td']['events'][eventName][dataTypeTD].setdefault(termName, {}) 
                inpType = click.prompt('Term %d Type' % i, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                ctx.obj['td']['events'][eventName][dataTypeTD][termName]['type'] = inpType
                handleThingTypes(ctx, inpType, 'events', eventName, dataTypeTD, termName)
                click.echo('\nHint: Term value is which that users have to assign to the Term itself to make sure that their messages are compatible with the Schema and consequently be accepted')
                if(inpType == 'boolean'):
                    inpValue = click.prompt('Term %d value' % i, type=bool)
                    inpValue = inpValue.lower()
                elif(inpType == 'integer'):
                    inpValue = click.prompt('Term %d value' % i, type=int) 
                elif(inpType == 'number'):
                    inpValue = click.prompt('Term %d value' % i, type=float)     
                elif(inpType == 'string' or inpType == 'null'):
                    inpValue = click.prompt('Term %d value' % i, type=str)     
                elif(inpType == 'object' or inpType == 'array'):
                    inpValue = click.prompt('Term %d value' % i, type=OBJ_STRING)            
                ctx.obj['td']['events'][eventName][dataTypeTD][termName]['value'] = inpValue


def handleTemplateTypes(ctx, termName, interactionTypeTD, interactionName='', dataType=''):
    t = {}
    t['name'] = termName
    termType = ''
    if(interactionTypeTD == 'properties'):
        termType = ctx.obj['td'][interactionTypeTD][termName]['type']
    else:
        termType = ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['type']      
    t['type'] = termType
    if(termType == 'integer' or termType == 'number'):
        if(interactionTypeTD == 'properties'):
            if('minimum' in ctx.obj['td'][interactionTypeTD][termName]):
                t['minimum'] = ctx.obj['td'][interactionTypeTD][termName]['minimum']
            if('maximum' in ctx.obj['td'][interactionTypeTD][termName]):
                t['maximum'] = ctx.obj['td'][interactionTypeTD][termName]['maximum']
        else:
            if('minimum' in ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]):
                t['minimum'] = ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['minimum']
            if('maximum' in ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]):
                t['maximum'] = ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['maximum']
    elif(termType == 'array'):
        if(interactionTypeTD == 'properties'):
            t['items'] = ctx.obj['td'][interactionTypeTD][termName]['items']
            if('minItems' in ctx.obj['td'][interactionTypeTD][termName]):
                t['minItems'] = ctx.obj['td'][interactionTypeTD][termName]['minItems']
            if('maxItems' in ctx.obj['td'][interactionTypeTD][termName]):
                t['maxItems'] = ctx.obj['td'][interactionTypeTD][termName]['maxItems'] 
        else:
            t['items'] = ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['items']
            if('minItems' in ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]):
                t['minItems'] = ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['minItems']
            if('maxItems' in ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]):
                t['maxItems'] = ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['maxItems']  
    elif(termType == 'object'):
        if(interactionTypeTD == 'properties'):
            if('properties' in ctx.obj['td'][interactionTypeTD][termName]):
                objProp = list(ctx.obj['td'][interactionTypeTD][termName]['properties'].keys())
                t.setdefault('properties', [])
                for propName in objProp:
                    p = {}
                    p = dict(ctx.obj['td'][interactionTypeTD][termName]['properties'][propName])
                    p['name'] = propName
                    t['properties'].append(p)
            if('required' in ctx.obj['td'][interactionTypeTD][termName]):
                t['required'] = ctx.obj['td'][interactionTypeTD][termName]['required']
        else:
            if('properties' in ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]):
                objProp = list(ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'].keys())
                t.setdefault('properties', [])
                for propName in objProp:
                    p = {}
                    p = dict(ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['properties'][propName])
                    p['name'] = propName
                    t['properties'].append(p)
            if('required' in ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]):
                t['required'] = ctx.obj['td'][interactionTypeTD][interactionName][dataType][termName]['required']        
    return t    


def writeFile(filePath, fileContent):
    directory = os.path.dirname(filePath)
    if not os.path.exists(directory):
        os.makedirs(directory)
    of = open(filePath, 'w')
    of.write(fileContent)
    of.close()


# CUSTOM TYPES
SWN_STRING = StartWithoutNumberStringParamType()
OBJ_STRING = ObjectStringParamType()
NZ_INT = NotZeroIntParamType()
NN_INT = NonNegativeIntParamType() 
DATETIME_STRING = DateTimeParamType()
ALIB_STRING = ArduinoLibraryParamType()

# JSON SCHEMA per la TD
schema = json.load(open('thing-schema.json'))

# JINJA2 Template
file_loader = FileSystemLoader('Templates')
env = Environment(loader=file_loader, extensions=['jinja2.ext.do'])
env.trim_blocks = True
env.lstrip_blocks = True
env.rstrip_blocks = True

template = env.get_template('thing-template.txt')

# THING Informations
thingProperties = []
thingActions = []
actionFunctions = []
thingEvents = []
eventConditions = []

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
@click.pass_context
# cli è una callback del gruppo definito precedentemente
def cli(ctx, **kwargs):
    """WoT module for build TDs and executable scripts for embedded systems"""
    click.echo('This module allow you to build custom Thing Descriptions and executable scripts for expose Thing on embedded systems')
    click.echo('Use --help option to see documentation\n')
    if(ctx.invoked_subcommand is None):
        click.confirm('Use the wizard?', default=True, abort=True)
        ctx.invoke(start)
    else:
        pass        
    # appena termina il codice della cli viene eseguito il codice del comando start
    

# le funzioni che vengono eseguite dai comandi non possono essere chiamate al di fuori del comando stesso
@cli.command()
# se si passa il contesto ad una callback, allora come parametro le si deve passare ctx obbligatoriamente
@click.pass_context
def start(ctx, **kwargs):
    """Start wizard"""
    ctx.ensure_object(dict)
    ctx.obj.setdefault('td', {})
    click.echo('Wizard start...\n')
    click.echo('THING')
    # THING TITLE
    addTitle(ctx, 'Thing')  
    # THING CONTEXT
    uri = 'https://www.w3.org/2019/wot/td/v1'
    if(click.confirm('\nUse the default Thing Context?', default=True)):
        ctx.obj['td']['@context'] = uri
    else:
        contextElements = click.prompt('Thing Context number of elements', type=NZ_INT)
        click.echo('\nHint: Thing Context elements MUST be URIs or JSON OBJECTs\nFor JSON OBJECTs it MUST use double quotes instead of single ones')
        # se all'interno di un dizionario (ctx.obj['td']) si vuole definire un array a cui aggiungere un
        # elemento alla volta, si possono utilizzare i metodi setdeafult per creare l'array e 
        # append per aggiungere gli elementi
        ctx.obj['td'].setdefault('@context', [])
        for i in range(1, contextElements+1):
            inp = click.prompt('Insert element %d' % i, type=OBJ_STRING)
            ctx.obj['td']['@context'].append(inp)
        ctx.obj['td']['@context'].append(uri)
    # THING FORM
    opType = ['readallproperties', 'writeallproperties', 'readmultipleproperties', 'writemultipleproperties']
    addForm(ctx, opType, 'Thing')
    # THING FORM ADDITIONAL TERMS 
    addTerm(ctx, True, 'Thing')      
    # THING META-TYPE
    addMetaType(ctx, 'Thing')
    # THING ID
    if(click.confirm('\nInsert Thing ID URI?', default=False)):
        inp = click.prompt('Thing ID URI', type=SWN_STRING)
        ctx.obj['td']['id'] = inp
    # THING DESCRIPTION
    addDescription(ctx, 'Thing')
    # THING VERSION
    if(click.confirm('\nInsert Thing Version?', default=False)):
        inp = click.prompt('Thing Version', type=str)
        ctx.obj['td']['version'] = inp 
    # THING CREATION
    if(click.confirm('\nInsert Thing Creation Date?', default=False)):
        click.echo('\nHint: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('Thing Creation Date', type=click.DateTime(formats=['%m-%d-%Y %H:%M']), default=datetime.now().strftime('%m-%d-%Y %H:%M:%S'))
        inp = str(inp).replace(' ', 'T') 
        ctx.obj['td']['created'] = inp  
    # THING MODIFICATION
    if(click.confirm('\nInsert Thing Modification Date?', default=False)):
        click.echo('\nHint: Insert date (mm-dd-yyyy) and time (hh:mm) split by one space')
        inp = click.prompt('Thing Modification Date', type=click.DateTime(formats=['%m-%d-%Y %H:%M']), default=datetime.now().strftime('%m-%d-%Y %H:%M:%S'))
        inp = str(inp).replace(' ', 'T') 
        ctx.obj['td']['modified'] = inp   
    # THING SUPPORT
    if(click.confirm('\nInsert Thing Support URI?', default=False)):
        inp = click.prompt('Thing Support URI', type=SWN_STRING)
        ctx.obj['td']['support'] = inp
    # THING BASE
    if(click.confirm('\nInsert Thing Base URI?', default=False)):
        inp = click.prompt('Thing Base URI', type=SWN_STRING)
        ctx.obj['td']['base'] = inp   
    # THING LINKS
    if(click.confirm('\nInsert Thing Links?', default=False)):
        linksElements = click.prompt('Thing Links number of elements', type=NN_INT)
        if(linksElements != 0):
            ctx.obj['td'].setdefault('links', [])
            for i in range(1, linksElements+1):
                l = {}
                l['href'] = click.prompt('\nLink %d Href' % i, type=SWN_STRING)
                if(click.confirm('Insert Link %d Media Type' % i, default=False)):
                    l['type'] = click.prompt('Link %d Media Type' % i, type=SWN_STRING)
                if(click.confirm('Insert Link %d Relation Type' % i, default=False)):
                    l['rel'] = click.prompt('Link %d Relation Type' % i, type=SWN_STRING)    
                if(click.confirm('Insert Link %d Anchor' % i, default=False)):
                    l['anchor'] = click.prompt('Link %d Anchor' % i, type=SWN_STRING)      
                ctx.obj['td']['links'].append(l)
    # THING ADDITIONAL TERMS
    addTerm(ctx, False, 'Thing')       
    # THING PROPERTIES
    click.echo('\n\nTHING PROPERTIES')
    if(click.confirm('Insert Thing Properties?', default=True)):
        ctx.obj['td'].setdefault('properties', {})
        numProperties = click.prompt('Number of Properties', type=NN_INT)
        for p in range(1, numProperties+1):
            # PROPERTY NAME
            click.echo()
            propertyName = searchNameTD(thingProperties, 'Property', p)  
            thingProperties.append(propertyName)
            click.echo('\n%s' % propertyName.upper())
            ctx.obj['td']['properties'].setdefault(propertyName, {})
            # PROPERTY FORM
            opType = ['readproperty', 'writeproperty']
            addForm(ctx, opType, 'Property', 'properties', propertyName, p)
            # PROPERTY FORM ADDITIONAL TERMS 
            addTerm(ctx, True, 'Property', 'properties', propertyName) 
            # PROPERTY TYPE
            inpType = click.prompt('\nProperty %d Type' % p, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
            ctx.obj['td']['properties'][propertyName]['type'] = inpType
            handleThingTypes(ctx, inpType, 'properties', propertyName)       
            # PROPERTY FORMAT
            if(click.confirm('\nInsert Property Format?', default=False)):
                inp = click.prompt('Property Format', type=SWN_STRING) 
                ctx.obj['td']['properties'][propertyName]['format'] = inp
            # PROPERTY META-TYPE    
            addMetaType(ctx, 'Property', 'properties', propertyName)
            # PROPERTY READONLY/WRITEONLY
            ctx.obj['td']['properties'][propertyName]['observable'] = False  
            op = ctx.obj['td']['properties'][propertyName]['forms'][0]['op']
            if((len(op) == 2) and (op[0] == 'readproperty' and op[1] == 'writeproperty')):
                ctx.obj['td']['properties'][propertyName]['readOnly'] = True  
                ctx.obj['td']['properties'][propertyName]['writeOnly'] = True
            elif((len(op) == 1) and (op[0] == 'readproperty')):
                ctx.obj['td']['properties'][propertyName]['readOnly'] = True  
                ctx.obj['td']['properties'][propertyName]['writeOnly'] = False
            elif((len(op) == 1) and (op[0] == 'writeproperty')):
                ctx.obj['td']['properties'][propertyName]['readOnly'] = False  
                ctx.obj['td']['properties'][propertyName]['writeOnly'] = True
            # PROPERTY TITLE
            addTitle(ctx, 'Property', 'properties', propertyName, p)
            # PROPERTY DESCRIPTION 
            addDescription(ctx, 'Property', 'properties', propertyName, p)
            # PROPERTY ADDITIONAL TERMS
            addTerm(ctx, False, 'Property', 'properties', propertyName)          
    # THING ACTION
    click.echo('\n\nTHING ACTIONS')
    if(click.confirm('Insert Thing Actions?', default=True)):
        ctx.obj['td'].setdefault('actions', {})
        numActions = click.prompt('Number of Actions', type=NN_INT)
        for a in range(1, numActions+1):    
            # ACTION NAME
            click.echo()
            actionName = searchNameTD(thingActions, 'Action', a)    
            thingActions.append(actionName)
            actionFunctions.append({'name':actionName})        
            click.echo('\n%s' % actionName.upper())
            ctx.obj['td']['actions'].setdefault(actionName, {})
            # ACTION FORM          
            ctx.obj['td']['actions'][actionName].setdefault('forms', [])
            ctx.obj['td']['actions'][actionName]['forms'].append({'href': '', 'contentType': 'application/json', 'op': 'invokeaction'})
            # ACTION FORM ADDITIONAL TERMS 
            addTerm(ctx, True, 'Action', 'actions', actionName) 
            # ACTION INPUT
            actionFunctions[a-1].setdefault('input', [])
            actionFunctions[a-1].setdefault('output', {})
            if(click.confirm('\nAction %d has Inputs?' % a, default=True)):
                inputNumber = click.prompt('Number of Action Inputs', type=NN_INT)
                if(inputNumber != 0):
                    ctx.obj['td']['actions'][actionName].setdefault('input', {})
                    for i in range(1, inputNumber+1):
                        inpName = click.prompt('\nAction Input %s Name' % i, type=SWN_STRING)
                        inpType = click.prompt('Action Input %s Type' % i, type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                        ctx.obj['td']['actions'][actionName]['input'].setdefault(inpName, {})
                        ctx.obj['td']['actions'][actionName]['input'][inpName]['type'] = inpType
                        actionFunctions[a-1]['input'].append({'name':inpName})
                        actionFunctions[a-1]['input'][i-1]['type'] = inpType
                        handleThingTypes(ctx, inpType, 'actions', actionName, 'input', inpName, actionIndex=(a-1, i-1))              
            # ACTION OUTPUT
            if(click.confirm('\nAction %d has Output?' % a, default=True)):    
                ctx.obj['td']['actions'][actionName].setdefault('output', {})
                outName = click.prompt('\nAction Output Name', type=SWN_STRING)
                outType = click.prompt('Action Output Type', type=click.Choice(['boolean', 'integer', 'number', 'string', 'object', 'array', 'null']), show_default=True) 
                ctx.obj['td']['actions'][actionName]['output'].setdefault(outName, {})
                ctx.obj['td']['actions'][actionName]['output'][outName]['type'] = outType
                actionFunctions[a-1]['output']['name'] = outName
                actionFunctions[a-1]['output']['type'] = outType
                handleThingTypes(ctx, outType, 'actions', actionName, 'output', outName)
            # ACTION BODY 
            click.echo('\nHint: The Body of the function corresponding to the Thing Action MUST be written in Embedded-C directly executable in Embedded-Systems.')
            click.echo('You have to provide only the code enclosed by braces on one line, neither Function name, inputs, outputs (return).')
            click.echo('This elements are retrived from the information you gave before.')
            click.echo('In the Thing Event Section it is possibile to add the Thing Action where insert the if-condition and the relative code to handle the Event logic.')
            actionFunctions[a-1]['body'] = click.prompt('Function Body', type=str) 
            # ACTION SAFETY
            if(click.confirm('\nAction %d is safe?' % a, default=False)):
                ctx.obj['td']['actions'][actionName]['safe'] = True
            else:
                ctx.obj['td']['actions'][actionName]['safe'] = False
            # ACTION IDEMPOTENCY
            if(click.confirm('\nAction %d is idempotent?' % a, default=False)):
                ctx.obj['td']['actions'][actionName]['idempotent'] = True
            else:
                ctx.obj['td']['actions'][actionName]['idempotent'] = False            
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
        ctx.obj['td'].setdefault('events', {})
        numEvents = click.prompt('Number of Events', type=NN_INT)
        for e in range(1, numEvents+1):    
            # EVENT NAME
            click.echo()
            eventName = searchNameTD(thingEvents, 'Event', e)  
            thingEvents.append(eventName)   
            click.echo('\n%s' % eventName.upper())
            ctx.obj['td']['events'].setdefault(eventName, {})  
            # EVENT FORM
            opType = ["subscribeevent", "unsubscribeevent"]
            addForm(ctx, opType, 'Event', 'events', eventName, e)
            # EVENT FORM ADDITIONAL TERMS    
            addTerm(ctx, True, 'Event', 'events', eventName) 
            # EVENT CONDITION
            Hint = ('\nHint: The Event Condition that, when it is True, will trigger the asynchronous data pushing to Consumers,'
                    '\ncan include every relational and logic operator like standard conditions in programming languages.'
                    '\nTo send messages through WebSocket it is necessarly to use sendTXT() method of WebSocketServer library.')
            click.echo(Hint)
            click.echo('Example: if (property1_name <= 0) { if-body }')
            event = {}
            event['condition'] = click.prompt('Event %d Condition' % e, type=str)
            an = click.prompt('Number of Actions in which the Event Condition will triggered', type=click.IntRange(1, len(thingActions)))
            event.setdefault('actions', [])
            for i in range(1, an+1):
                inp = click.prompt('Event %d Action %d name' % (e, i), type=click.Choice(thingActions))
                event['actions'].append(inp)
            eventConditions.append(event)
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
    try:
        js.validate(ctx.obj['td'], schema)
    except Exception as e:
        click.echo(str(e))
    # click.echo('\n{}\n'.format(json.dumps(ctx.obj['td'], indent=4)))
    filePath = ctx.obj['td']['title'].lower() + '/' + ctx.obj['td']['title'].lower() + '.json'
    output = json.dumps(ctx.obj['td'], indent=4)
    writeFile(filePath, output)
    if(click.confirm('\nBuild Embedded-C File starting from this Thing Description?', default=True)):
        click.echo()
        ctx.invoke(build)
    else:
        pass    


@cli.command()
@click.pass_context
def build(ctx):
    '''Build executable Embedded-C File'''
    click.echo('Start building...\n')
    global thingProperties
    global thingActions
    global thingEvents
    if((ctx.obj is None) or ('td' not in ctx.obj)):
        correctJsonFile = False
        while(not(correctJsonFile)):
            fileName = click.prompt('Upload Thing Description as Json File', type=click.Path(exists=True, readable=True, resolve_path=True))
            if(fileName.rpartition('.')[-1] == 'json'):
                correctJsonFile = True
            else:
                click.echo("Error: Non-Json File provided")    
        ctx.ensure_object(dict)
        ctx.obj.setdefault('td', {})
        ctx.obj['td'] = json.load(open(fileName))
        if('properties' in ctx.obj['td']):
            thingProperties = list(ctx.obj['td']['properties'].keys())
        if('actions' in ctx.obj['td']):    
            thingActions = list(ctx.obj['td']['actions'].keys())
        if('events' in ctx.obj['td']):    
            thingEvents = list(ctx.obj['td']['events'].keys())
        for i in range(0, len(thingActions)):
            action = {}
            action['name'] = thingActions[i]
            action.setdefault('input', [])
            action.setdefault('output', {})
            if('input' in ctx.obj['td']['actions'][thingActions[i]]):
                actionInputs = list(ctx.obj['td']['actions'][thingActions[i]]['input'].keys())
                for j in range(0, len(actionInputs)):
                    inp = handleTemplateTypes(ctx, actionInputs[j], 'actions', thingActions[i], 'input')
                    action['input'].append(inp)
            if('output' in ctx.obj['td']['actions'][thingActions[i]]):
                actionOutput = list(ctx.obj['td']['actions'][thingActions[i]]['output'].keys())       
                out = handleTemplateTypes(ctx, actionOutput[0], 'actions', thingActions[i], 'output')
                action['output'] = out
            if(i == 0):
                click.echo('\nHint: The Body of the function corresponding to the Thing Action MUST be written in Embedded-C directly executable in Embedded-Systems')
                click.echo('You have to provide only the code enclosed by braces on one line, neither Function name, inputs, outputs (return)')
                click.echo('This elements are retrived from the information you gave before')    
            action['body'] = click.prompt('\nFunction %s Body' % thingActions[i], type=str) 
            actionFunctions.append(action)    
        for i in range(0, len(thingEvents)):
            if(i == 0):
                Hint = ('\nHint: The Event Condition that, when it is True, will trigger the asynchronous data pushing to Consumers,'
                    '\ncan include every relational and logic operator like standard conditions in programming languages.'
                    '\nTo send messages through WebSocket it is necessarly to use sendTXT() method of WebSocketServer library.')
                click.echo(Hint)
                click.echo('Example: if (property1_name <= 0) { if-body }')
            event = {}
            event['condition'] = click.prompt('\nEvent %s Condition' % thingEvents[i], type=str)
            an = click.prompt('Number of Actions in which the Event Condition will triggered', type=click.IntRange(1, len(thingActions)))
            event.setdefault('actions', [])
            for j in range(1, an+1):
                inp = click.prompt('Event %s Action %d name' % (thingEvents[i], j), type=click.Choice(thingActions))
                event['actions'].append(inp)
            eventConditions.append(event) 
        try:
            js.validate(ctx.obj['td'], schema)
        except Exception as e:
            click.echo(str(e))
        click.echo()        
    ctx.obj.setdefault('template', {})  
    # NETWORK SSID
    #inp = click.prompt('Network SSID to which the Embedded-System will connect', type=str)
    #ctx.obj['template']['ssid'] = inp
    ctx.obj['template']['ssid'] = 'Infostrada-2.4GHz-9454A5'
    # NETWORK PASSWORD
    '''if(click.confirm('\nNetwork has password?', default=True)):
        inp = click.prompt('Network Password (hide_input)', type=str, hide_input=True)
        ctx.obj['template']['password'] = inp
    else:
        ctx.obj['template']['password'] = ''   
    '''    
    ctx.obj['template']['password'] = '0525646993722559'
    # WEBSERVER PORT
    inp = click.prompt('\nWebServer Port', type=NN_INT, default=80, show_default=True)
    ctx.obj['template']['portserver'] = str(inp)
    # WEBSOCKET PORT
    if('events' in ctx.obj['td']):
        inp = click.prompt('\nWebSocket Port', type=NN_INT, default=81, show_default=True)
        ctx.obj['template']['portsocket'] = str(inp)    
    # ADDITIONAL LIBRARIES
    hint = 0
    nameList = []
    ctx.obj['template'].setdefault('libraries', [])
    while(click.confirm('\nAdd additional Arduino Library?', default=False)):
        if(hint == 0):
            click.echo('\nHint: Arduino Library Name example: library_name.h.\n.h extension MUST be included')
            hint = hint+1
        nameAlreadyExists = True
        while(nameAlreadyExists):
            inp = click.prompt('Arduino Library Name', type=ALIB_STRING) 
            nameAlreadyExists = searchName(nameList, inp)
            if(nameAlreadyExists):
                click.echo('Error: Arduino Library already included\n')
            else:
                nameList.append(inp)    
                ctx.obj['template']['libraries'].append(inp)
    # ADDITIONAL GLOBAL VARIABLES
    hint = 0
    nameList = []
    ctx.obj['template'].setdefault('globals', [])
    while(click.confirm('\nAdd additional Global Variables?', default=False)):
        if(hint == 0):
            click.echo('\nHint: For each global variable you have to insert its name and its type supported in Embedded-C. Value is optional')     
            click.echo('For array variables it is necessarly to insert their length and their elements one-by-one')
            hint = hint+1
        var = {}
        nameAlreadyExists = True
        while(nameAlreadyExists):
            inp = click.prompt('Variable Name', type=SWN_STRING)
            nameAlreadyExists = searchName(nameList, inp)
            if(nameAlreadyExists):
                click.echo('Error: Global Variable already exists\n')
            else:
                nameList.append(inp)
                var['name'] = inp
        var['type'] = click.prompt('Variable Type', type=str)
        var['isArray'] = click.confirm('Is an array?', default=False)
        if(var['isArray']):
            var['itemsNumber'] = click.prompt('Number of elements', type=NZ_INT)
            var.setdefault('items', [])
            if(click.confirm('\nInsert elements?', default=False)):
                for i in range(1, var['itemsNumber']+1):
                    inp = click.prompt('Element %d' % i, type=str)
                    var['items'].append(inp)
        else:
            if(click.confirm('Insert value?', default=False)):
                var['value'] = click.prompt('Value', type=str)
        ctx.obj['template']['globals'].append(var)            
    # ADDITIONAL FUNCTIONS
    hint = 0
    nameList = []
    nameListInp = []
    ctx.obj['template'].setdefault('functions', [])
    while(click.confirm('\nAdd additional function?', default=False)):
        if(hint == 0):
            click.echo('\nHint: Functions Return and Input Types MUST be available in Embedded-C')
        fun = {}
        nameAlreadyExists = True
        while(nameAlreadyExists):
            inp = click.prompt('Function Name', type=SWN_STRING)
            nameAlreadyExists = searchName(nameList, inp)
            if(nameAlreadyExists):
                click.echo('Error: Function already exists\n')
            else:
                nameList.append(inp)
                fun['name'] = inp    
        click.echo('\n%s' % fun['name'].upper())
        fun.setdefault('output', {})
        if(click.confirm('Insert Function Return Type?', default=False)):
            fun['output']['type'] = click.prompt('Function Return Type', type=SWN_STRING)
        if(click.confirm('\nInsert Function Inputs?', default=False)):
            inputsNumber = click.prompt('Insert Inputs number', type=NN_INT)
            if(inputsNumber > 0):
                fun.setdefault('inputs', [])
                for i in range(1, inputsNumber+1):
                    inp = {}
                    inp['type'] = click.prompt('Input %d Type' % i, type=SWN_STRING)
                    nameAlreadyExists = True
                    while(nameAlreadyExists):
                        inpName = click.prompt('Input %d Name' % i, type=SWN_STRING)
                        nameAlreadyExists = searchName(nameListInp, inpName)
                        if(nameAlreadyExists):
                            click.echo('Error: Input already declared\n')
                        else:
                            nameListInp.append(inpName)
                            inp['name'] = inpName
                    fun['inputs'].append(inp)
        if(hint == 0):            
            click.echo('\nHint: For the Function Body you have to provide only the code in Embedded-C enclosed by braces on one line')
            hint = hint+1
        fun['body'] = click.prompt('Function Body', type=str)
        ctx.obj['template']['functions'].append(fun)                 
    # ADDITIONAL SETUP CODE
    if(click.confirm('\nAdd additional code in setup() function?', default=False)):
        ctx.obj['template']['setup'] = click.prompt('Setup() code', type=str)    
    # ADDITIONAL LOOP CODE
    if(click.confirm('\nAdd additional code in loop() function?', default=False)):
        ctx.obj['template']['loop'] = click.prompt('Loop() code', type=str)   
    # THING INTERACTION AFFORDANCE
    ctx.obj['template']['numproperties'] = len(thingProperties)
    ctx.obj['template']['numactions'] = len(thingActions)
    ctx.obj['template']['numevents'] = len(thingEvents)
    # THING PROPERTIES
    ctx.obj['template'].setdefault('properties', [])
    for i in range(0, ctx.obj['template']['numproperties']):
        p = handleTemplateTypes(ctx, thingProperties[i], 'properties')
        ctx.obj['template']['properties'].append(p)         
    o = 0 
    for i in range(0, len(thingProperties)):    
        if(ctx.obj['td']['properties'][thingProperties[i]]['type'] == 'object'):
            o = o+1    
    ctx.obj['template']['numop'] = o
    # THING ACTIONS
    ctx.obj['template'].setdefault('actions', []) 
    ctx.obj['template']['actions'] = actionFunctions
    # THING EVENTS
    ctx.obj['template'].setdefault('events', [])    
    for i in range(0, ctx.obj['template']['numevents']):
        e = {}
        e['name'] = thingEvents[i]
        e['condition'] = eventConditions[i]['condition']
        e['actions'] = eventConditions[i]['actions']
        ctx.obj['template']['events'].append(e)  
        dataType = ['subscription', 'data', 'cancellation']
        for data in dataType:
            if(data in ctx.obj['td']['events'][e['name']]):
                dataTerm = list(ctx.obj['td']['events'][e['name']][data].keys())
                ctx.obj['template']['events'][i].setdefault(data, [])
                for key in dataTerm:
                    t = {}
                    t['name'] = key
                    t['value'] = ctx.obj['td']['events'][e['name']][data][key]['value']
                    ctx.obj['template']['events'][i][data].append(t)   
    # WEBSOCKET MESSAGE TYPES     
    if(len(thingEvents) > 0):           
        click.echo('\nHint: This application handle only three WebSocket Types in WebSocketEvent function for messages exchanged on the WebSocket channel: DISCONNECTED, CONNECTED and TEXT')
        click.echo('You have to insert only types that are allowed by WebSocket library and the relative logic on one line.')
        click.echo("The code will be insert inside the relative SWITCH-CASE section in WebSocketEvent function. It is not necessary to insert the latter 'break' in the SWITCH-CASE sections")
        nameList = []
        ctx.obj['template'].setdefault('websocket', [])
        while(click.confirm('Add additional WebSocket Message Type?', default=False)):
            t = {}
            nameAlreadyExists = True
            while(nameAlreadyExists):
                inp = click.prompt('WebSocket Message Type', type=str)
                nameAlreadyExists = searchName(nameList, inp)
                if(nameAlreadyExists):
                    click.echo('Error: WebSocket Message Type already included\n')
                else:
                    nameList.append(inp)
                    t['type'] = inp
            t['body'] = click.prompt('WebSocket Message Type logic', type=str)
            ctx.obj['template']['websocket'].append(t)
            click.echo('\n')
    output = template.render(td=ctx.obj['td'], template=ctx.obj['template'])    
    filePath = ctx.obj['td']['title'].lower() + '/' + ctx.obj['td']['title'].lower() + '.ino'
    writeFile(filePath, output)


if __name__ == "__main__":
    # la funzione che viene richiamata nel main è la sola ad essere esguita dalla cli,
    # per cui la cli vede solo le proprietà e gli argomenti da lei acceduti 
    # che poi verranno visualizzati nella documentazione (help) 
    # se si inserisce un'opzione prima del comando, allora viene gestita dalla cli, 
    # se viene inserita dopo, viene gestita dal comando stesso
    cli()

    # QUANDO SI INSERISCONO OGGETTI, VERIFICARE SE VALIDATE JSON DA ERRORE, SE SI L'UTENTE DEVE REINSERIRE I DATI
    # CONTROLLARE NOMI DOPPIONI IN LIBRERIE, VARIABILI GLOBALI, FUNZIONI E TIPI WEBSOCKET