# MUST CREATE THESE FOLDERS IN ORDER FOR THIS CODE TO WORK: 
# 4files, 8files, 16files, ... 256files
import random
import csv
random.seed()

def add_quote(a):
    return '"{0}"'.format(a)

def loadCSV(fileName):
	list=[]
	with open(fileName, 'rb') as csvfile:
		# next(csvfile)
		row = csv.reader(csvfile, quotechar='"', delimiter=',')
		list.extend(row)
	return list

def writeToCSV(list,fileName):
	file = open(fileName, "w")
	for li in list:
		count=1
		for thing in li:
			if "," in thing:
				thing = add_quote(thing)
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
	randCols=removeDup(randCols)
	return randCols

def removeDup(duplicate): 
    final_list = [] 
    for num in duplicate: 
        if num not in final_list: 
            final_list.append(num) 
    return final_list 

def genRandNumRows(totalRows):
	return(random.randint(20,totalRows))

def genStartRow(numRows, numOutputRows):
	largestRow = numRows - numOutputRows
	return(random.randint(1,largestRow))

numtoGen = [4,8,16,32,64,128,256];
k=0;
csv = loadCSV("movie_metadata.csv")
numRows = len(csv)-1
# print numRows

for i in range(7):
	count = 0
	for i in range(numtoGen[k]):
		# print(numtoGen[k])
		randCols = genRandCols()
		randCols.sort()
		# print("output cols:")
		# print(randCols)

		numOutputRows = genRandNumRows(500)
		# print("num output rows")
		# print(numOutputRows)

		startRow = genStartRow(numRows, numOutputRows)
		# print("start row")
		# print(startRow)

		outputList = []
		list = csv[0]
		# print(list)

		headers = []
		for i in range(len(list)):
			if i in randCols:
				headers.append(list[i])

		outputList.append(headers)

		# print("output col names:")
		# print(outputList)


		for li in range(numOutputRows):
			# print(li)
			row = []
			for i in range(28):
				if i in randCols:
					# print("thing")
					# print(csv[startRow][i])
					row.append(csv[startRow][i])
			# print("printing row:")
			# print(row)
			outputList.append(row)
			startRow+=1


		# for i in range(numOutputRows):
		# 	print(str(outputList[i]) + "\n"),


		fileName = str(numtoGen[k]) + "files/movieMet" + str(count) + ".csv"
		print(fileName)
		count+=1
		writeToCSV(outputList,fileName)
	k+=1