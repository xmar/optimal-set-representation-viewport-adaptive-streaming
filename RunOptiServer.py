#!/usr/bin/env python3

import math
import subprocess as sub
import os
import queue
import atomic
import Pyro4
import Pyro4.naming
import argparse

def GetNiceTimeStr(seconds):
    hours, seconds =  seconds // 3600, seconds % 3600
    minutes, seconds = seconds // 60, seconds % 60
    ans = ''
    if hours > 0:
        ans += '{}h'.format(int(hours))
    if minutes > 0:
        if len(ans) > 0:
            ans += ' '
        ans += '{}m'.format(int(minutes))
    if seconds > 0 or len(ans) == 0:
        if len(ans) > 0:
            ans += ' '
        ans += '{:.02f}s'.format(seconds)
    return ans


def PrintStatus(testDone, nbTotalTest, durationList):
    with open('status.txt', 'w') as o:
        o.write('{}/{} -> {}%\n'.format(testDone, nbTotalTest, 100*testDone/nbTotalTest if nbTotalTest != 0 else 100))
        durationSeconds = sum(durationList)
        hours, seconds =  durationSeconds // 3600, durationSeconds % 3600
        minutes, seconds = seconds // 60, seconds % 60
        o.write('Spent time: {}\n'.format(GetNiceTimeStr(durationSeconds)))
        o.write('Estimated remaining time: {}\n'.format(GetNiceTimeStr((nbTotalTest-testDone)*durationSeconds/len(durationList) if len(durationList) != 0 else 0)))
        print('Spent time: {}'.format(GetNiceTimeStr(durationSeconds)))
        print('Estimated remaining time: {}\n'.format(GetNiceTimeStr((nbTotalTest-testDone)*durationSeconds/len(durationList) if len(durationList) != 0 else 0)))


@Pyro4.behavior(instance_mode="single")
class MasterQueue:
    def __init__(self):
        self.queue = queue.Queue()
        self.nbTotalTest = atomic.AtomicLong(0)
        self.nbTestDone = atomic.AtomicLong(0)
        self.nbWorkingTest = atomic.AtomicLong(0)
        self.durationList = list()


    def Start(self):
        print('{} jobs are waiting to be executed'.format(self.nbTotalTest.value))

    def Add(self, *arg):
        self.nbTotalTest += 1
        self.queue.put((*arg,))

    @Pyro4.expose
    def AddBack(self, t):
        self.nbWorkingTest -= 1
        print('Add back a job, {} jobs are being executed'.format(self.nbWorkingTest.value))
        self.queue.put(t)

    @Pyro4.expose
    def Get(self):
        try:
            t = self.queue.get(block=False)
            if t is not None:
                self.nbWorkingTest += 1
                print('{} jobs are being executed'.format(self.nbWorkingTest.value))
        except queue.Empty:
            t = None
        return t

    @Pyro4.expose
    def Done(self, duration, outputFiles):
        self.nbWorkingTest -= 1
        self.nbTestDone += 1
        self.durationList.append(duration)
        PrintStatus(self.nbTestDone.value, self.nbTotalTest.value, self.durationList)
        print('{}/{} jobs done. {} jobs are still being executed'.format(self.nbTestDone.value,
                                                                         self.nbTotalTest.value,
                                                                         self.nbWorkingTest.value))
        for filePath in outputFiles:
            if not os.path.exists(os.path.dirname(filePath)):
                os.makedirs(os.path.dirname(filePath))
            with open(filePath, 'w') as o:
                o.write(outputFiles[filePath])
        return


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Run multiple optimal solver');

    parser.add_argument('serverHost', type=str, help='host name of the server')

    args = parser.parse_args()

    serverHost = args.serverHost

    pathToPreparedDataset = '../tracks_newDataset'

    defaultBitrate = 4*math.pi
    bitrateList = [defaultBitrate, 5.75, 7, 8, 10, 15, 20, 25]
    bmin = 0.45
    bmax = 2.1
    bRatio = 3.5
    defaultQerNb = 4
    # QerNbList = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    QerNbList = [11, 10, 9, 8, 7, 6, 5, 3, 2, 1]
    defaultSegSize = 2
    # SegSizeList = [0.5, 1, 1.5, 2.5, 3, 3.5, 4, 4.5, 5]
    SegSizeList = [5, 4.5, 4, 3.5, 3, 2.5, 1.5, 1, 0.5]
    # SegSizeList = [5, 4, 3, 1]
    nbThread = 4
    nbHArea = 25
    nbVArea = 25
    epGap = 0.03
    nbTheta = 17
    nbPhi = 17
    nbHDim = 12
    nbVDim = 12
    nbMaxUser = 60


    masterQueue = MasterQueue()

    #QER variation:
    defaults = {'epGap': epGap, 'nbThread':nbThread, 'nbQer':defaultQerNb,
                'segSize': defaultSegSize, 'Bmin':bmin, 'Bmax':bmax,
                'Bratio':bRatio, 'nbTheta':nbTheta, 'nbPhi':nbPhi, 'nbHDim':nbHDim,
                'nbVDim':nbVDim, 'nbHArea':nbHArea, 'nbVArea':nbVArea,
                'pathToPreparedDataset':pathToPreparedDataset, 'output':'output_allUsers/sim_B_12.56',
                'nbMaxUser':nbMaxUser}
    nbTotalTest = len(QerNbList) + len(SegSizeList) + len(bitrateList)
    testDone = 0
    PrintStatus(testDone, nbTotalTest, [])
    for bitrate in bitrateList:
        d = defaults.copy()
        d['Bmin'] = bmin/(bitrate/defaultBitrate)
        d['Bmax'] = bmax/(bitrate/defaultBitrate)
        d['output'] = 'output_allUsers/sim_B_{}'.format(bitrate)
        relOutputDir = os.path.join(d['output'], '{}_{}x{}_{:.6f}_{:.6f}_{:.6f}s'.format(d['nbQer'],
                                                                                       d['nbHArea'],
                                                                                       d['nbVArea'],
                                                                                       d['Bmin'],
                                                                                       d['Bmax'],
                                                                                       d['segSize']))
        absOutputDir = os.path.join(os.getcwd(), relOutputDir)
        if not os.path.exists(os.path.join(absOutputDir, 'results.txt')):
            masterQueue.Add(d, relOutputDir, absOutputDir)
    for segSize in SegSizeList:
        d = defaults.copy()
        d['segSize'] = segSize
        relOutputDir = os.path.join(d['output'], '{}_{}x{}_{:.6f}_{:.6f}_{:.6f}s'.format(d['nbQer'],
                                                                                       d['nbHArea'],
                                                                                       d['nbVArea'],
                                                                                       d['Bmin'],
                                                                                       d['Bmax'],
                                                                                       d['segSize']))
        absOutputDir = os.path.join(os.getcwd(), relOutputDir)
        if not os.path.exists(os.path.join(absOutputDir, 'results.txt')):
            masterQueue.Add(d, relOutputDir, absOutputDir)
    for nbQer in QerNbList:
        d = defaults.copy()
        d['nbQer'] = nbQer
        relOutputDir = os.path.join(d['output'], '{}_{}x{}_{:.6f}_{:.6f}_{:.6f}s'.format(d['nbQer'],
                                                                                       d['nbHArea'],
                                                                                       d['nbVArea'],
                                                                                       d['Bmin'],
                                                                                       d['Bmax'],
                                                                                       d['segSize']))
        absOutputDir = os.path.join(os.getcwd(), relOutputDir)
        if not os.path.exists(os.path.join(absOutputDir, 'results.txt')):
            masterQueue.Add(d, relOutputDir, absOutputDir)

    # Start pyros
    # with sub.Popen(['python3', '-m', 'Pyro4.naming', '-n', serverHost]) as nsProc:
    daemon = Pyro4.Daemon(host=serverHost)                # make a Pyro daemon
    ns = Pyro4.locateNS(host=serverHost)
    uri = daemon.register(masterQueue)   # register the masterQueue as a Pyro object
    ns.register('server.masterQueue', uri)

    masterQueue.Start()

    print("Ready. Object uri =", uri)      # print the uri so we can use it in the client later
    daemon.requestLoop()
