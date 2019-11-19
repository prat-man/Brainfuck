class Brainfuck:
	def __init__(self, filePath):
		self.file = open(filePath, "r")
		self.tape = [0]
		self.pointer = 0
	
	def isOperator(self, ch):
		return ch in "><+-.,[]"
		
	def doOperation(self, ch):
		#print(ch)
		#print(self.pointer)
		#print(self.tape)
		#print()
	
		if ch == '>':
			self.pointer += 1
			if self.pointer == len(self.tape):
				self.tape.append(0)
		
		elif ch == '<':
			if self.pointer == 0:
				raise Error("ERROR: Tape underflow!")
			else:
				self.pointer -= 1
		
		elif ch == '+':
			self.tape[self.pointer] += 1
			self.tape[self.pointer] %= 256
		
		elif ch == '-':
			self.tape[self.pointer] -= 1
		
		elif ch == '.':
			print(chr(self.tape[self.pointer]), end='')
		
		elif ch == ',':
			self.tape[self.pointer] = int(input()[0])
		
		elif ch == '[':
			if self.tape[self.pointer] == 0:
				balance = 1
				ch2 = self.file.read(1)
				if ch2 == '[':
					balance += 1
				elif ch2 == ']':
					balance -= 1
				while ch2 != ']' or balance != 0:
					ch2 = self.file.read(1)
					if ch2 == '[':
						balance += 1
					elif ch2 == ']':
						balance -= 1
		
		elif ch == ']':
			if self.tape[self.pointer] != 0:
				balance = -1
				self.file.seek(self.file.tell() - 2)
				ch2 = self.file.read(1)
				if ch2 == '[':
					balance += 1
				elif ch2 == ']':
					balance -= 1
				while ch2 != '[' or balance != 0:
					if ch2 == '\n' or ch2 == '\r':
						self.file.seek(self.file.tell() - 3)
					else:
						self.file.seek(self.file.tell() - 2)
					ch2 = self.file.read(1)
					if ch2 == '[':
						balance += 1
					elif ch2 == ']':
						balance -= 1
		
		else:
			raise Error("ERROR: Invalid character '" + ch + "'")

	def execute(self):
		while True:
			ch = self.file.read(1)
			if not ch:
				break
			if self.isOperator(ch):
				self.doOperation(ch)

filePath = input("Enter file path: ")

bf = Brainfuck(filePath)

bf.execute()