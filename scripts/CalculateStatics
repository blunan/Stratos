#!/bin/bash

if [ -d ~/Desktop/ns-3 ]
then
	cd ~/Desktop/ns-3
else
	cd ~/ns-3
fi

CalculateStandardDeviations() {
	file=$1
	nLines=0
	nFound=0
	nTimes=0
	timesSum=0
	nSuccess=0
	avgTimes=0
	nTimesSum=0
	nPackets=$2
	packetsSum=0
	simulations=0
	nRequesters=$3
	timeDeviation=0
	foundDeviation=0
	successDeviation=0
	packetsDeviation=0
	avgControlOverhead=0
	avgFoundPercentage=0
	avgSuccessPercentage=0
	avgPacketsPercentage=0
	controlOverheadDeviation=0
	while read line
	do
		let "nLines += 1"
		if [ $(($nLines%$nRequesters)) -eq 0 ]
		then
			let "simulations += 1"
			if [ $nTimes -gt 0 ]
			then
				avgTimes=$(echo "$timesSum / $nTimes" | bc)
				avgControlOverhead=$(echo "$line / ($packetsSum * 256)" | bc)
				avgPacketsPercentage=$(echo "($packetsSum * 100) / ($nPackets * $nTimes)" | bc)
				timeDeviation=$(echo "$timeDeviation + (($avgTimes - $4) ^ 2)" | bc)
				packetsDeviation=$(echo "$packetsDeviation + (($avgPacketsPercentage - $7) ^ 2)" | bc)
				controlOverheadDeviation=$(echo "$controlOverheadDeviation + (($avgControlOverhead - $8) ^ 2)" | bc)
			fi
			avgFoundPercentage=$(echo "($nFound * 100) / ($nRequesters - 1)" | bc)
			avgSuccessPercentage=$(echo "($nSuccess * 100) / ($nRequesters - 1)" | bc)
			foundDeviation=$(echo "$foundDeviation + (($avgFoundPercentage - $6) ^ 2)" | bc)
			successDeviation=$(echo "$successDeviation + (($avgSuccessPercentage - $5) ^ 2)" | bc)
			let "nTimes = 0"
			let "nFound = 0"
			let "timesSum = 0"
			let "nSuccess = 0"
			let "packetsSum = 0"
		else
			IFS='|' read -a values <<< "$line"
			if [ ${values[0]} -ge 0 ]
			then
				let "nTimes += 1"
				let "nTimesSum += 1"
				let "timesSum += values[0]"
				let "packetsSum += values[3]"
			fi
			let "nFound += values[2]"
			let "nSuccess += values[1]"
		fi
	done < "$file"
	# We use x / (n - 1) as Bessel's correction suggests
	timeDeviation=$(echo "sqrt($timeDeviation / ($nTimesSum - 1))" | bc)
	foundDeviation=$(echo "sqrt($foundDeviation / ($simulations - 1))" | bc)
	successDeviation=$(echo "sqrt($successDeviation / ($simulations - 1))" | bc)
	packetsDeviation=$(echo "sqrt($packetsDeviation / ($nTimesSum - 1))" | bc)
	controlOverheadDeviation=$(echo "sqrt($controlOverheadDeviation / ($nTimesSum - 1))" | bc)
	echo "$timeDeviation $successDeviation $foundDeviation $packetsDeviation $controlOverheadDeviation"
	echo " "
}

CalculateStatics() {
	file=$1
	nLines=0
	nFound=0
	nTimes=0
	timesSum=0
	nSuccess=0
	avgTimes=0
	nTimesSum=0
	packetsSum=0
	simulations=0
	nPackets=${2:-10}
	nRequesters=${3:-4}
	avgControlOverhead=0
	avgFoundPercentage=0
	avgSuccessPercentage=0
	avgPacketsPercentage=0
	let "nRequesters += 1"
	while read line
	do
		let "nLines += 1"
		if [ $(($nLines%$nRequesters)) -eq 0 ]
		then # We have read all the results for requesters on the same simulation
			let "simulations += 1"
			if [ $nTimes -gt 0 ]
			then # At least one requester received one data package
				avgTimes=$(echo "$avgTimes + ($timesSum / $nTimes)" | bc)
				avgControlOverhead=$(echo "$avgControlOverhead + ($line / ($packetsSum * 256))" | bc)
				avgPacketsPercentage=$(echo "$avgPacketsPercentage + (($packetsSum * 100) / ($nPackets * $nTimes))" | bc)
			fi
			avgFoundPercentage=$(echo "$avgFoundPercentage + (($nFound * 100) / ($nRequesters - 1))" | bc)
			avgSuccessPercentage=$(echo "$avgSuccessPercentage + (($nSuccess * 100) / ($nRequesters - 1))" | bc)
			let "nTimes = 0"
			let "nFound = 0"
			let "timesSum = 0"
			let "nSuccess = 0"
			let "packetsSum = 0"
		else # We are reading the results of a requester on the same simulation
			IFS='|' read -a values <<< "$line"
			if [ ${values[0]} -ge 0 ]
			then # At least one data package was received
				let "nTimes += 1"
				let "nTimesSum += 1"
				let "timesSum += values[0]" # Time elapsed since request was sent until first data package was received
				let "packetsSum += values[3]" # Amount of data packages received
			fi
			let "nFound += values[2]" # Was a node to provide us the requested service found?
			let "nSuccess += values[1]" # Was the node found the best one to provide us the requested service?
		fi
	done < "$file"
	avgTimes=$(echo "$avgTimes / $nTimesSum" | bc)
	avgControlOverhead=$(echo "$avgControlOverhead / $nTimesSum" | bc)
	avgFoundPercentage=$(echo "$avgFoundPercentage / $simulations" | bc)
	avgPacketsPercentage=$(echo "$avgPacketsPercentage / $nTimesSum" | bc)
	avgSuccessPercentage=$(echo "$avgSuccessPercentage / $simulations" | bc)
	declare -a deviations=($(CalculateStandardDeviations $file $nPackets $nRequesters $avgTimes $avgSuccessPercentage $avgFoundPercentage $avgPacketsPercentage $avgControlOverhead))
	# Calculate a 1 - α = 95% confidence interval, this means P(-1.96 < z < 1.96) = 0.95
	confidenceInterval[0]=$(echo "1.96 * (${deviations[0]} / sqrt($nTimesSum))" | bc)
	confidenceInterval[1]=$(echo "1.96 * (${deviations[1]} / sqrt($simulations))" | bc)
	confidenceInterval[2]=$(echo "1.96 * (${deviations[2]} / sqrt($simulations))" | bc)
	confidenceInterval[3]=$(echo "1.96 * (${deviations[3]} / sqrt($nTimesSum))" | bc)
	confidenceInterval[4]=$(echo "1.96 * (${deviations[4]} / sqrt($nTimesSum))" | bc)
	echo "${confidenceInterval[0]}|${confidenceInterval[1]}|${confidenceInterval[2]}|${confidenceInterval[3]}|${confidenceInterval[4]}"
	echo "$avgTimes|$avgSuccessPercentage|$avgFoundPercentage|$avgPacketsPercentage|$avgControlOverhead"
}

CalculateStatics stratos/distributed_mobile_0.txt > stratos/distributed_statics.txt
CalculateStatics stratos/distributed_mobile_25.txt >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_mobile_50.txt >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_mobile_100.txt >> stratos/distributed_statics.txt
echo " " >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_requesters_1.txt 10 1 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_requesters_2.txt 10 2 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_requesters_4.txt 10 4 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_requesters_8.txt 10 8 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_requesters_16.txt 10 16 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_requesters_24.txt 10 24 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_requesters_32.txt 10 32 >> stratos/distributed_statics.txt
echo " " >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_services_1.txt >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_services_2.txt >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_services_4.txt >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_services_8.txt >> stratos/distributed_statics.txt
echo " " >> stratos/distributed_statics.txt
#CalculateStatics stratos/distributed_packets_1.txt 1 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_packets_10.txt 10 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_packets_20.txt 20 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_packets_40.txt 40 >> stratos/distributed_statics.txt
CalculateStatics stratos/distributed_packets_60.txt 60 >> stratos/distributed_statics.txt