import random
import csv
random.seed()


def loadCSV(fileName):
	list=[]
	with open(fileName, 'rb') as csvfile:
		next(csvfile)
		row = csv.reader(csvfile, delimiter=',', quotechar='"')
		list.extend(row)
	return list


def writeToCSV(list,fileName):
	file = open(fileName, "w")
	for li in list:
		count=1
		for thing in li:
			if count==len(li):
				file.write(str(thing) + "\n")
			else:
				file.write(str(thing) + ",")
			count=count+1


def genRandCols():
	numCols = random.randint(1,28)
	randCols = []
	for i in range(numCols):
		randCols.append(random.randint(1,28))
	return randCols

def genRandNumRows():

def genStartRow(numRows):


randCols = genRandCols()
for i in randCols
	print(i)