import sys
from getch import getchar

class Brainfuck:
	def __init__(self, filePath):
		self.file = open(filePath, "rb")
		self.tape = [0]
		self.pointer = 0
		
	def readChar(self):
		return self.file.read(1).decode("utf-8")
	
	def isOperator(self, ch):
		return ch in "><+-.,[]"
		
	def doOperation(self, ch):
		if ch == '>':
			self.pointer += 1
			if self.pointer == len(self.tape):
				self.tape.append(0)
		
		elif ch == '<':
			if self.pointer == 0:
				self.tape.insert(0, 0)
			else:
				self.pointer -= 1
		
		elif ch == '+':
			self.tape[self.pointer] += 1
			self.tape[self.pointer] %= 256
		
		elif ch == '-':
			if self.tape[self.pointer] == 0:
				self.tape[self.pointer] = 255
			else:
				self.tape[self.pointer] -= 1
		
		elif ch == '.':
			print(chr(self.tape[self.pointer]), end='', flush=True)
		
		elif ch == ',':
			self.tape[self.pointer] = getchar()
		
		elif ch == '[':
			if self.tape[self.pointer] == 0:
				balance = 1
				ch2 = self.readChar()
				if ch2 == '[':
					balance += 1
				elif ch2 == ']':
					balance -= 1
				while ch2 != ']' or balance != 0:
					ch2 = self.readChar()
					if ch2 == '[':
						balance += 1
					elif ch2 == ']':
						balance -= 1
		
		elif ch == ']':
			if self.tape[self.pointer] != 0:
				balance = -1
				self.file.seek(self.file.tell() - 2)
				ch2 = self.readChar()
				if ch2 == '[':
					balance += 1
				elif ch2 == ']':
					balance -= 1
				while ch2 != '[' or balance != 0:
					self.file.seek(-2, 1)
					ch2 = self.readChar()
					if ch2 == '[':
						balance += 1
					elif ch2 == ']':
						balance -= 1
		
		else:
			raise ValueError("Invalid character '" + ch + "'")

	def execute(self):
		while True:
			ch = self.readChar()
			if not ch:
				break
			if self.isOperator(ch):
				self.doOperation(ch)

def main():
	args = sys.argv[1:]
	if len(args) > 0:
		bf = Brainfuck(args[0])
		bf.execute()
		print()
	else:
		print("Usage: python Brainfuck.py [file path]")

if __name__ == "__main__":
	main()
