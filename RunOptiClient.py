#!/usr/bin/env python3

import math
import subprocess as sub
import os
import sys
import time
import Pyro4
import glob
import argparse

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Run multiple optimal solver');

    parser.add_argument('serverHost', type=str, help='host name of the server')

    args = parser.parse_args()

    serverHost = args.serverHost

    optimalSoftPath = './build/preprocessingV2'

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
nbMaxUser={nbMaxUser}
inputVideoList={inputVideoList}
pathToTraces={pathToPreparedDataset}
pathToOutputDir={output}
"""

    ns = Pyro4.locateNS(host=serverHost)
    uri = ns.lookup('server.masterQueue')
    masterQueue = Pyro4.Proxy(uri)

    t = masterQueue.Get()
    while t is not None:
        d, relOutputDir, absOutputDir = t
        done = False
        addBack = True
        try:
            startTime = time.time()
            # print(d, relOutputDir, absOutputDir, startTime)

            conf = template.format(**d)
            with open('tmpConfig.ini', 'w') as o:
                o.write(conf)
            if not os.path.exists(d['output']):
                os.makedirs(d['output'])
            sub.check_call([optimalSoftPath, '-c', 'tmpConfig.ini'])


            outputFiles = dict()
            for fileName in ['results.txt', 'results_avg.txt', 'pos_results.txt', 'dim_results.txt']:
                filePath = os.path.join(relOutputDir, fileName)
                if os.path.exists(filePath):
                    f = ''
                    with open(filePath, 'r') as i:
                        for line in i:
                            f += line
                    outputFiles[filePath] = f

            for f in glob.glob(os.path.join(relOutputDir, '*.sol')):
                os.remove(f)

            done = True
        except KeyboardInterrupt:
            addBack = False
            if not done:
                try:
                    masterQueue.AddBack(t)
                except:
                    ns = Pyro4.locateNS(host=serverHost)
                    uri = ns.lookup('server.masterQueue')
                    masterQueue = Pyro4.Proxy(uri)
                    masterQueue.AddBack(t)
            print('KeyboardInterrupt')
            sys.exit()
        finally:
            if not done:
                if addBack:
                    try:
                        masterQueue.AddBack(t)
                    except:
                        ns = Pyro4.locateNS(host=serverHost)
                        uri = ns.lookup('server.masterQueue')
                        masterQueue = Pyro4.Proxy(uri)
                        masterQueue.AddBack(t)
            else:
                try:
                    masterQueue.Done(time.time()-startTime, outputFiles)
                except:
                    ns = Pyro4.locateNS(host=serverHost)
                    uri = ns.lookup('server.masterQueue')
                    masterQueue = Pyro4.Proxy(uri)
                    masterQueue.Done(time.time()-startTime, outputFiles)

        try:
            t = masterQueue.Get()
        except:
            ns = Pyro4.locateNS(host=serverHost)
            uri = ns.lookup('server.masterQueue')
            masterQueue = Pyro4.Proxy(uri)
            t = masterQueue.Get()
