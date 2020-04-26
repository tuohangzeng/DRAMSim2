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
    if (filename[-5:] == ".json") :
        listOfFiles.append(filename)

x_granularity = []
y_interval = []
z_writeratio = []
c_aggregate = []
c_average = []

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

    x_granularity.append(math.log2(granularity))
    y_interval.append(interval)
    z_writeratio.append(ratio)

    c_aggregate.append(aggregateBandwidth)
    c_average.append(averageBandwidth)

    print(granularity, interval, ratio, aggregateBandwidth, averageBandwidth)

fig = plt.figure()
ax_aggregate = fig.add_subplot(121, projection='3d')
ax_average = fig.add_subplot(122, projection='3d')

aggregateNorm = matplotlib.colors.Normalize(vmin=min(c_aggregate), vmax=max(c_aggregate), clip=True)
averageNorm = matplotlib.colors.Normalize(vmin=min(c_average), vmax=max(c_average), clip=True)

aggregateMapper = cm.ScalarMappable(aggregateNorm, 'viridis')
aggregateColor = aggregateMapper.to_rgba(c_aggregate)

averageMapper = cm.ScalarMappable(averageNorm, 'plasma')
averageColor = averageMapper.to_rgba(c_average)

ax_aggregate.scatter(x_granularity, y_interval, z_writeratio, c=aggregateColor)
ax_average.scatter(x_granularity, y_interval, z_writeratio, c=averageColor)


cbar_aggregate = fig.colorbar(aggregateMapper, ax=ax_aggregate)
cbar_aggregate.set_label("Bandwidth (GB/s)")

cbar_average = fig.colorbar(averageMapper, ax=ax_average)
cbar_average.set_label("Bandwidth (GB/s)")


ax_aggregate.set_xlabel("Granularity (log_2 Bytes)")
ax_aggregate.set_ylabel("Txn Arriving Interval (ns)")
ax_aggregate.set_zlabel("Write ratio")

ax_average.set_xlabel("Granularity (log_2 Bytes)")
ax_average.set_ylabel("Txn Arriving Interval (ns)")
ax_average.set_zlabel("Write ratio")

plt.title(os.path.relpath(__file__),pad=40)
plt.show()
