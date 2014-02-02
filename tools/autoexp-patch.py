#!/usr/bin/env python

import os

AUTOEXP_FILES = [
  # Visual Studio 8.0 (2005).
  "C:\\Program Files\\Microsoft Visual Studio 8\\Common7\\Packages\\Debugger\\autoexp.dat",
  "C:\\Program Files (x86)\\Microsoft Visual Studio 8\\Common7\\Packages\\Debugger\\autoexp.dat",

  # Visual Studio 9.0 (2008).
  "C:\\Program Files\\Microsoft Visual Studio 9.0\\Common7\\Packages\\Debugger\\autoexp.dat",
  "C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\Common7\\Packages\\Debugger\\autoexp.dat",

  # Visual Studio 10.0 (2010).
  "C:\\Program Files\\Microsoft Visual Studio 10.0\\Common7\\Packages\\Debugger\\autoexp.dat",
  "C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\Packages\\Debugger\\autoexp.dat"
]

DIRECTIVE_SYMBOL = '@'

# =============================================================================
# [Log]
# =============================================================================

def log(str):
  print(str)

# =============================================================================
# [Is...]
# =============================================================================
  
def isDirective(c):
  return c == DIRECTIVE_SYMBOL

def isAlpha(c):
  c = ord(c)
  return (c >= ord('a') and c <= ord('z')) or (c >= ord('A') and c <= ord('Z'))

def isAlpha_(c):
  return isAlpha(c) or (c == '_')

def isNumber(c):
  c = ord(c)
  return (c >= ord('0')) and (c <= ord('9'))
  
def isAlnum(c):
  return isAlpha(c) or isNumber(c)

def isAlnum_(c):
  return isAlnum(c) or (c == '_')

def isSpace(c):
  return (c == ' ') or (c == '\t')

def isNewLine(c):
  return c == '\n'

# =============================================================================
# [SyntaxError]
# =============================================================================

class SyntaxError(Exception):
  def __init__(self, msg):
    self.msg = msg

  def __str__(self):
    return repr(self.msg)

# =============================================================================
# [Context]
# =============================================================================

class Context(object):
  def __init__(self, data):
    self.data = data
    self.index = 0
    self.size = len(data)

  def isNewLine(self):
    if self.index == 0:
      return True
    else:
      return self.data[self.index - 1] == '\n'

  def isEnd(self):
    return self.index >= self.size

  def getChar(self):
    if self.index >= self.size:
      return '\0'
    return self.data[self.index]

  def advance(self):
    if self.index < self.size:
      self.index += 1

  def parseUntilTrue(self, func, advance):
    while not self.isEnd():
      c = self.data[self.index]
      if func(c):
        self.index += 1
        continue
      if advance:
        self.index += 1
      return True
    return False

  def parseUntilFalse(self, func, advance):
    while not self.isEnd():
      c = self.data[self.index]
      if not func(c):
        self.index += 1
        continue
      if advance:
        self.index += 1
      return True
    return False

  def skipString(self):
    def func(c):
      return c == '"'
    return self.parseUntilFalse(func, True)

  def skipSpaces(self):
    return self.parseUntilTrue(isSpace, False)

  def skipLine(self):
    return self.parseUntilFalse(isNewLine, True)

  def parseDirective(self, index):
    start = index

    data = self.data
    size = self.size

    c = data[index]
    assert isAlpha_(c)

    while True:
      index += 1
      if index >= size:
        break
      c = data[index]
      if isAlnum_(c):
        continue
      break

    self.index = index
    return data[start:index]

  def parseSymbol(self, index):
    start = index

    data = self.data
    size = self.size

    c = data[index]
    assert isAlpha_(c)

    while True:
      index += 1
      if index >= size:
        return data[start:index]
      c = data[index]
      if isAlnum_(c):
        continue
      if c == ':' and index + 2 < size and data[index + 1] == ':' and isAlpha_(data[index + 2]):
        index += 2
        continue

      self.index = index
      return data[start:index]

  def parseMacro(self, index):
    start = index
    end = None

    data = self.data
    size = self.size
    
    if index >= size:
      return ""

    while True:
      c = data[index]
      index += 1

      if c == '\n' or index >= size:
        if end == None:
          end = index - 1
        break

      if c == ';':
        if end == None:
          end = index

    while start < end and isSpace(data[end - 1]):
      end -= 1

    self.index = index
    return data[start:end]

  def replaceRange(self, start, end, content):
    old = self.data

    self.data = old[0:start] + content + old[end:]
    self.size = len(self.data)
    
    assert(self.index >= end)

    self.index -= end - start
    self.index += len(content)

# =============================================================================
# [AutoExpDat]
# =============================================================================

class AutoExpDat(object):
  def __init__(self, data):
    self.library = None
    self.symbols = {}
    self.data = self.process(data.replace('\r', ''))

  def process(self, data):
    ctx = Context(data)

    while not ctx.isEnd():
      c = ctx.getChar()

      # Skip comments.
      if c == ';':
        ctx.skipLine()
        continue

      # Skip strings.
      if c == '"':
        ctx.advance()
        ctx.skipString()
        continue

      # Skip numbers.
      if isNumber(c):
        ctx.parseUntilTrue(isAlnum_, True)
        continue

      # Parse directives.
      if isDirective(c) and ctx.isNewLine():
        start = ctx.index

        ctx.advance()
        c = ctx.getChar()

        # Remove lines that have '@' followed by space or newline.
        if isNewLine(c) or c == '\0':
          ctx.advance()
          ctx.replaceRange(start, ctx.index, "")
          continue
        if isSpace(c):
          ctx.skipLine()
          ctx.replaceRange(start, ctx.index, "")
          continue

        directive = ctx.parseDirective(ctx.index)

        c = ctx.getChar()
        if not isSpace(c):
          self.error("Directive Error: @" + directive + ".")
        ctx.skipSpaces()

        # Directive '@library'.
        if directive == "library":
          self.library = ctx.parseMacro(ctx.index)

        # Directive '@define'.
        elif directive == "define":
          c = ctx.getChar()
          if not isAlpha_(c):
            self.error("Define Directive has to start with alpha character or underscore")
          symbol = ctx.parseSymbol(ctx.index)

          c = ctx.getChar()

          # No Macro.
          if isNewLine(c):
            ctx.advance()
            self.addSymbol(symbol, "")
          # Has Macro.
          else:
            ctx.skipSpaces()

            macro = ctx.parseMacro(ctx.index)
            self.addSymbol(symbol, macro)
        
        # Unknown Directive.
        else:
          self.error("Unknown Directive: @" + directive + ".")

        ctx.replaceRange(start, ctx.index, "")
        continue

      # Parse/Replace symbol.
      if isAlpha_(c) and ctx.index > 0 and ctx.data[ctx.index - 1] != '#':
        start = ctx.index
        symbol = ctx.parseSymbol(start)

        if symbol in self.symbols:
          ctx.replaceRange(start, start + len(symbol), self.symbols[symbol])
        continue

      ctx.advance()

    return ctx.data

  def addSymbol(self, symbol, macro):
    if symbol in self.symbols:
      self.error("Symbol '" + symbol + "' redefinition.")
    else:
      # Recurse.
      macro = self.process(macro)

      log("-- @define " + symbol + " " + macro)
      self.symbols[symbol] = macro
     
  def error(self, msg):
    raise SyntaxError(msg)

# =============================================================================
# [LoadFile / SaveFile]
# =============================================================================

def loadFile(file):
  h = None
  data = None

  try:
    h = open(file, "rb")
    data = h.read()
  except:
    pass
  finally:
    if h:
      h.close()

  return data

def saveFile(file, data):
  h = None
  result = False

  try:
    h = open(file, "wb")
    h.truncate()
    h.write(data)
    result = True
  except:
    pass
  finally:
    if h:
      h.close()

  return result

# =============================================================================
# [PatchFile]
# =============================================================================

def patchFile(file, mark, data):
  input = loadFile(file)
  if not input:
    return

  beginMark = ";${" + mark + ":Begin}"
  endMark = ";${" + mark + ":End}"

  if beginMark in input:
    # Replace.
    if not endMark in input:
      log("-- Corrupted File:\n" + "   " + file)
      return

    beginMarkIndex = input.find(beginMark)
    endMarkIndex = input.find(endMark)

    beginMarkIndex = input.find('\n', beginMarkIndex) + 1
    endMarkIndex = input.rfind('\n', 0, endMarkIndex) + 1

    if beginMarkIndex == -1 or \
       endMarkIndex == -1 or \
       beginMarkIndex > endMarkIndex:
      log("-- Corrupted File:\n" + "   " + file)
      return

    output = input[:beginMarkIndex] + data + input[endMarkIndex:]

  else:
    # Add.
    output = input
    output += "\n"
    output += beginMark + "\n"
    output += data
    output += endMark + "\n"
  
  if input == output:
    log("-- Unaffected:\n" + "   " + file)
  else:
    log("-- Patching:\n" + "   " + file)
    if not saveFile(file, output):
      log("!! Can't write:\n" + "   " + file)

def main():
  src = loadFile("autoexp.dat")
  if src == None:
    log("!! Can't read autoexp.dat")
    return

  src = AutoExpDat(src)
  if not src.library:
    log("!! Library not defined, use @library directive.")
    return

  for file in AUTOEXP_FILES:
    patchFile(file, src.library, src.data)

main()
