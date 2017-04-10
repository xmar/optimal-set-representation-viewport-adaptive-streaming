#!/usr/bin/python3

import math
import subprocess as sub
import os
import time

if __name__ == '__main__':

    optimalSoftPath = './build/preprocessingV2'
    pathToPreparedDataset = '../tracks_newDataset'

    bitrateList = [5, 7, 8, 10, 15, 20, 25]
    defaultBitrate = 4*math.pi
    bmin = 0.45
    bmax = 2.1
    bRatio = 3.5
    defaultQerNb = 4
    QerNbList = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    defaultSegSize = 2
    SegSizeList = [0.5, 1, 1.5, 2.5, 3, 3.5, 4, 4.5, 5]
    nbThread = 1
    nbHArea = 20
    nbVArea = 20
    epGap = 0.03
    nbTheta = 17
    nbPhi = 17
    nbHDim = 12
    nbVDim = 12

    template = """[Global]
selectionPerSegment=true
selectionPerVideo=false
epGap={epGap}
nbThread={nbThread}
nbQer={nbQer}
useTile=false
segmentDuration={segSize}
minSurfaceBitrate={Bmin}
maxSurfaceBitrate={Bmax}
bitrateRatio={Bratio}
minSegQuality=0
nbTheta={nbTheta}
nbPhi={nbPhi}
nbHDim={nbHDim}
nbVDim={nbVDim}
dimMin=0.1
dimMax=0.6
nbHPixels={nbHArea}
nbVPixels={nbVArea}
viewportHAngle=110
viewportVAngle=90
pathToTraces={pathToPreparedDataset}
pathToOutputDir={output}
"""

    def PrintStatus(testDone, nbTotalTest, durationSeconds):
        with open('status.txt', 'w') as o:
            o.write('{}/{} -> {}%\n'.format(testDone, nbTotalTest, 100*testDone/nbTotalTest))
            o.write('Spent time: {}s\n'.format(durationSeconds))
            o.write('Estimated remaining time: {}s\n'.format((nbTotalTest-testDone)*durationSeconds/testDone if testDone != 0 else -1))

    #QER variation:
    defaults = {'epGap': epGap, 'nbThread':nbThread, 'nbQer':defaultQerNb,
                'segSize': defaultSegSize, 'Bmin':bmin, 'Bmax':bmax,
                'Bratio':bRatio, 'nbTheta':nbTheta, 'nbPhi':nbPhi, 'nbHDim':nbHDim,
                'nbVDim':nbVDim, 'nbHArea':nbHArea, 'nbVArea':nbVArea,
                'pathToPreparedDataset':pathToPreparedDataset, 'output':'output/sim_B_12.56'}
    nbTotalTest = len(QerNbList) + len(SegSizeList) + len(bitrateList)
    testDone = 0
    start = time.time()
    PrintStatus(testDone, nbTotalTest, time.time() - start)
    for nbQer in QerNbList:
        d = defaults.copy()
        d['nbQer'] = nbQer
        conf = template.format(**d)
        with open('tmpConfig.ini', 'w') as o:
            o.write(conf)
        if not os.path.exists(d['output']):
            os.makedirs(d['output'])
        sub.check_call([optimalSoftPath, '-c', 'tmpConfig.ini'])
        testDone += 1
        PrintStatus(testDone, nbTotalTest, time.time() - start)
    for segSize in SegSizeList:
        d = defaults.copy()
        d['segSize'] = segSize
        conf = template.format(**d)
        with open('tmpConfig.ini', 'w') as o:
            o.write(conf)
        if not os.path.exists(d['output']):
            os.makedirs(d['output'])
        sub.check_call([optimalSoftPath, '-c', 'tmpConfig.ini'])
        testDone += 1
        PrintStatus(testDone, nbTotalTest, time.time() - start)
    for bitrate in bitrateList:
        d = defaults.copy()
        d['Bmin'] = bmin/(bitrate/defaultBitrate)
        d['Bmax'] = bmax/(bitrate/defaultBitrate)
        d['output'] = 'output/sim_B_{}'.format(bitrate)
        conf = template.format(**d)
        with open('tmpConfig.ini', 'w') as o:
            o.write(conf)
        if not os.path.exists(d['output']):
            os.makedirs(d['output'])
        sub.check_call([optimalSoftPath, '-c', 'tmpConfig.ini'])
        testDone += 1
        PrintStatus(testDone, nbTotalTest, time.time() - start)
