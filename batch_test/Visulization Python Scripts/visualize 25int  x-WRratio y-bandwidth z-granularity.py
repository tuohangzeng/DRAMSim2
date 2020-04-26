import os
import sys
import math
import json
import matplotlib
import matplotlib.cm as cm
import matplotlib.pyplot as plt

relPath = os.path.dirname(os.path.realpath(__file__))

resultsDirPath = relPath + '/results'
files = os.listdir(resultsDirPath)

listOfFiles = []
for filename in files:
    if (filename[-5:] == ".json" and filename.find("I25.0")!=-1) :
        listOfFiles.append(filename)

averageData = {}
aggregateData = {}

fig = plt.figure()
ax_aggregate = fig.add_subplot(121)
ax_average = fig.add_subplot(122)

for filename in listOfFiles:
    jsonIn = open(resultsDirPath + '/' + filename,"r")
    obj = json.load(jsonIn)
   
    filenameList = filename.split('_')
    factor = int(filenameList[4][1:])
    interval = float(filenameList[5][1:])
    ratio = float(filenameList[7][1:][:-5])/100

    averageBandwidth = obj["ms"]["0.150000"]["Channels"][0]["Average_Bandwidth"]
    aggregateBandwidth = obj["ms"]["0.150000"]["Channels"][0]["Aggregate_Bandwidth"]

    # processing
    granularity = 64 * factor # bytes
    granularity_log2 = int(math.log2(granularity))

    if (averageData.get(granularity_log2) == None):
        averageData[granularity_log2] = []
    if (aggregateData.get(granularity_log2) == None):
        aggregateData[granularity_log2] = []

    averageData[granularity_log2].append(tuple((ratio,averageBandwidth)))      # locate in dictionary the granularity bin
    aggregateData[granularity_log2].append(tuple((ratio,aggregateBandwidth)))


    print(granularity, interval, aggregateBandwidth, averageBandwidth)


for key, value in aggregateData.items():
    x_ratio = []
    y_bandwidth = []
    granularity = str(pow(2,key)) + 'B'

    for tup in sorted(value):
        x_ratio.append(tup[0])
        y_bandwidth.append(tup[1])
    
    ax_aggregate.scatter(x_ratio, y_bandwidth, s=4, label=granularity)
    ax_aggregate.plot(x_ratio, y_bandwidth)


for key, value in averageData.items():
    x_ratio = []
    y_bandwidth = []
    granularity = str(pow(2,key)) + 'B' 

    for tup in sorted(value):
        x_ratio.append(tup[0])
        y_bandwidth.append(tup[1])
    
    ax_average.scatter(x_ratio, y_bandwidth, s=3, label=granularity)
    ax_average.plot(x_ratio, y_bandwidth)


ax_aggregate.set_xlabel("Write ratio")
ax_aggregate.set_ylabel("Bandwidth (GB/s)")
ax_aggregate.legend(loc='upper right')

ax_average.set_xlabel("Write ratio")
ax_average.set_ylabel("Bandwidth (GB/s)")
ax_average.legend(loc='upper right')

plt.title(os.path.relpath(__file__),pad=40)
plt.show()
