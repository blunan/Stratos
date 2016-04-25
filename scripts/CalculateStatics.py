import os
import math

if os.path.exists(os.path.expanduser("~/Desktop/ns-3")) :
	os.chdir(os.path.expanduser("~/Desktop/ns-3"))
else :
	os.chdir(os.path.expanduser("~/ns-3"))

def CalculateStandardDeviation(values, average) :
	deviation = 0
	for value in values :
		deviation += math.pow(value - average,  2)
	# We use x / (n - 1) as Bessel's correction suggests
	return math.sqrt(deviation / (len(values) - 1))

def CalculateStatics(resultsFile, nPackets = 10, nRequesters = 4) :
	nLines = 0
	nFound = 0
	nTimes = 0
	timesSum = 0 #
	nSuccess = 0
	avgTimes = []
	nTimesSum = 0
	totAvgTime = 0
	packetsSum = 0 #
	nSimulations = 0
	deviations = range(5)
	avgControlOverheads = []
	avgFoundPercentages = []
	totAvgControlOverhead = 0
	totAvgFoundPercentage = 0
	avgSuccessPercentages = []
	avgPacketsPercentages = []
	totAvgSuccessPercentage = 0
	totAvgPacketsPercentage = 0
	confidenceIntervals = range(5)

	nRequesters += 1 # Virtual requester to read total amount of data sent during simulation
	with open(resultsFile) as file :
		for line in file : # Read the file line by line
			nLines += 1
			if (nLines % nRequesters) == 0 : # We have read all the results for requesters on the same simulation
				aux = 0
				nSimulations += 1
					nTimesSum += 1
				if nTimes > 0 : # At least one requester received one data package
					aux = timesSum / nTimes
					totAvgTime += aux
					avgTimes.append(aux)
					try :
						aux = int(line) / (packetsSum * 256)
					except ZeroDivisionError:
						aux = int(line) #TODO should it be 0?
					totAvgControlOverhead += aux
					avgControlOverheads.append(aux)
					aux = (packetsSum * 100) / (nPackets * nTimes)
					totAvgPacketsPercentage += aux
					avgPacketsPercentages.append(aux)
				aux = (nFound * 100) / (nRequesters - 1)
				totAvgFoundPercentage += aux
				avgFoundPercentages.append(aux)
				aux = (nSuccess * 100) / (nRequesters - 1)
				totAvgSuccessPercentage += aux
				avgSuccessPercentages.append(aux)
				nTimes = 0
				nFound = 0
				timesSum = 0
				nSuccess = 0
				packetsSum = 0
			else : # We are reading the results of a requester on the same simulation
				values = line.split("|")
				if int(values[0]) >= 0 : # At least one data package was received
					nTimes += 1
					timesSum += int(values[0]) # Time elapsed since request was sent until first data package was received
					packetsSum += int(values[3]) # Amount of data packages received
				nFound += int(values[2]) # Was a node to provide us the requested service found?
				nSuccess += int(values[1]) # Was the node found the best one to provide us the requested service?
	totAvgTime = totAvgTime / nTimesSum;
	totAvgControlOverhead =  totAvgControlOverhead / nTimesSum
	totAvgFoundPercentage = totAvgFoundPercentage / nSimulations
	totAvgPacketsPercentage = totAvgPacketsPercentage / nTimesSum
	totAvgSuccessPercentage = totAvgSuccessPercentage / nSimulations
	# Calculate standard deviations
	deviations[0] = CalculateStandardDeviation(avgTimes, totAvgTime)
	deviations[1] = CalculateStandardDeviation(avgSuccessPercentages,totAvgSuccessPercentage)
	deviations[2] = CalculateStandardDeviation(avgFoundPercentages, totAvgFoundPercentage)
	deviations[3] = CalculateStandardDeviation(avgPacketsPercentages, totAvgPacketsPercentage)
	deviations[4] = CalculateStandardDeviation(avgControlOverheads, totAvgControlOverhead)
	# Calculate a 1 - alpha = 95% confidence interval, this means P(-1.96 < z < 1.96) = 0.95
	confidenceIntervals[0] = 1.96 * (deviations[0] / math.sqrt(nTimesSum))
	confidenceIntervals[1] = 1.96 * (deviations[1] / math.sqrt(nSimulations))
	confidenceIntervals[2] = 1.96 * (deviations[2] / math.sqrt(nSimulations))
	confidenceIntervals[3] = 1.96 * (deviations[3] / math.sqrt(nTimesSum))
	confidenceIntervals[4] = 1.96 * (deviations[4] / math.sqrt(nTimesSum))
	print "%.4f|%.4f|%.4f|%.4f|%.4f" % (confidenceIntervals[0], confidenceIntervals[1], confidenceIntervals[2], confidenceIntervals[3], confidenceIntervals[4])
	print "%.4f|%.4f|%.4f|%.4f|%.4f" % (totAvgTime, totAvgSuccessPercentage, totAvgFoundPercentage, totAvgPacketsPercentage, totAvgControlOverhead)

CalculateStatics("stratos/distributed_mobile_0.txt")
CalculateStatics("stratos/distributed_mobile_25.txt")
CalculateStatics("stratos/distributed_mobile_50.txt")
CalculateStatics("stratos/distributed_mobile_100.txt")
print ""
CalculateStatics("stratos/distributed_requesters_1.txt", 10, 1)
CalculateStatics("stratos/distributed_requesters_2.txt", 10, 2)
CalculateStatics("stratos/distributed_requesters_4.txt", 10, 4)
CalculateStatics("stratos/distributed_requesters_8.txt", 10, 8)
CalculateStatics("stratos/distributed_requesters_16.txt", 10, 16)
CalculateStatics("stratos/distributed_requesters_24.txt", 10, 24)
CalculateStatics("stratos/distributed_requesters_32.txt", 10, 32)
print ""
CalculateStatics("stratos/distributed_services_1.txt")
CalculateStatics("stratos/distributed_services_2.txt")
CalculateStatics("stratos/distributed_services_4.txt")
CalculateStatics("stratos/distributed_services_8.txt")
print ""
#CalculateStatics("stratos/distributed_packets_1.txt", 1)
CalculateStatics("stratos/distributed_packets_10.txt", 10)
CalculateStatics("stratos/distributed_packets_20.txt", 20)
CalculateStatics("stratos/distributed_packets_40.txt", 40)
CalculateStatics("stratos/distributed_packets_60.txt", 60)