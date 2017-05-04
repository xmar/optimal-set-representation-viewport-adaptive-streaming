#!/usr/bin/env python3

import numpy as np
import struct
import argparse

if __name__ == '__main__':
        # get program arguments
        parser = argparse.ArgumentParser(
            description='read a binary statistics file and plot pictures')
        parser.add_argument('pathToBinFile', type=str,
                            help='path to the binary file',
                            )

        args = parser.parse_args()

        with open('{}/allowedVersion.bin'.format(args.pathToBinFile), 'rb') as iFile:
            nbVersion, = struct.unpack('L', iFile.read(struct.calcsize('L')))
            # for vId in range(nbVersion):
            #     versionId, theta, phi, hDim, vDim, sQer = struct.unpack('Lfffff', iFile.read(struct.calcsize('Lfffff')))
            #     print (versionId, theta, phi, hDim, vDim, sQer)

        with open('{}/generatedVersion.bin'.format(args.pathToBinFile), 'rb') as iFile:
            nbSeg, = struct.unpack('L', iFile.read(struct.calcsize('L')))
            print(nbSeg)
            for sId in range(nbSeg):
                segId, nbVersion = struct.unpack('LL', iFile.read(struct.calcsize('LL')))
                print(segId, nbVersion, end='')
                for v in range(nbVersion):
                    versionId, = struct.unpack('L', iFile.read(struct.calcsize('L')))
                    print('', versionId, end='')
                print('ll')


        with open('{}/userVersionQuality.bin'.format(args.pathToBinFile), 'rb') as iFile:
            nbUser, = struct.unpack('L', iFile.read(struct.calcsize('L')))
            print (nbUser)
            for u in range(nbUser):
                uid, nbSeg = struct.unpack('LL', iFile.read(struct.calcsize('LL')))
                print(nbSeg)
                for s in range(nbSeg):
                    segId, optiVers, nbVers = struct.unpack('LLL', iFile.read(struct.calcsize('LLL')))
                    print(uid, segId, optiVers, nbVers, end='')
                    for v in range(nbVers):
                        versionId, quality = struct.unpack('Ld', iFile.read(struct.calcsize('Ld')))
                        print('', versionId, quality, end='')
                    print('')
