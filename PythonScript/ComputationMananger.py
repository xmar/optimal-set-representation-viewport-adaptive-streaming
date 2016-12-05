#!/usr/bin/env python3

import argparse
import configparser
import os
import math

import shutil

import subprocess as sub

class SolverConfiguration:
    def __init__(self, selectionPerSegment, nbQer, segmentDuration, minSurfaceBitrate, maxSurfaceBitrate, minSegQuality, nbTheta, nbPhi, nbHDim, nbVDim, dimMin, dimMax, nbHPixels, nbVPixels, viewportHAngle, viewportVAngle, pathToTraces, pathToTracesHash, pathToPrecomputedIntersection, pathToPrecomputedSegments):
        self.selectionPerSegment = selectionPerSegment
        self.nbQer = nbQer
        self.segmentDuration = segmentDuration
        self.minSurfaceBitrate = minSurfaceBitrate
        self.maxSurfaceBitrate = maxSurfaceBitrate
        self.minSegQuality = minSegQuality
        self.nbTheta = nbTheta
        self.nbPhi = nbPhi
        self.nbHDim = nbHDim
        self.nbVDim = nbVDim
        self.dimMin = dimMin
        self.dimMax = dimMax
        self.nbHPixels = nbHPixels
        self.nbVPixels = nbVPixels
        self.viewportHAngle = viewportHAngle
        self.viewportVAngle = viewportVAngle
        self.pathToTraces = pathToTraces
        self.pathToTracesHash = pathToTracesHash
        self.pathToPrecomputedIntersection = pathToPrecomputedIntersection
        self.pathToPrecomputedSegments = pathToPrecomputedSegments

    def GetConfigurationStr(self):
        s =  '[Global]\n'
        s += 'selectionPerSegment={}\n'.format(self.selectionPerSegment)
        s += 'nbQer={}\n'.format(self.nbQer)
        s += 'segmentDuration={}\n'.format(self.segmentDuration)
        s += 'minSurfaceBitrate={}\n'.format(self.minSurfaceBitrate)
        s += 'maxSurfaceBitrate={}\n'.format(self.maxSurfaceBitrate)
        s += 'minSegQuality={}\n'.format(self.minSegQuality)
        s += 'nbTheta={}\n'.format(self.nbTheta)
        s += 'nbPhi={}\n'.format(self.nbPhi)
        s += 'nbHDim={}\n'.format(self.nbHDim)
        s += 'nbVDim={}\n'.format(self.nbVDim)
        s += 'dimMin={}\n'.format(self.dimMin)
        s += 'dimMax={}\n'.format(self.dimMax)
        s += 'nbHPixels={}\n'.format(self.nbHPixels)
        s += 'nbVPixels={}\n'.format(self.nbVPixels)
        s += 'viewportHAngle={}\n'.format(self.viewportHAngle)
        s += 'viewportVAngle={}\n'.format(self.viewportVAngle)
        s += 'pathToTraces={}\n'.format(self.pathToTraces)
        s += 'pathToPrecomputedIntersection={}\n'.format(self.pathToPrecomputedIntersection)
        s += 'pathToPrecomputedSegments={}\n'.format(self.pathToPrecomputedSegments)
        return s

    def SolutionId(self):
        return '{}_{:.6f}_{:.6f}_{:.6f}_{:.6f}'.format(self.nbQer, float(self.segmentDuration), float(self.minSegQuality), float(self.minSurfaceBitrate), float(self.maxSurfaceBitrate))
    def OutputDirId(self):
        return '{:.6f}_{}_{}_{:.6f}_{:.6f}_{}_{}_{}_{}_{}_{:.6f}_{:.6f}_{}'.format(float(self.segmentDuration), self.nbTheta, self.nbPhi, float(self.dimMin), float(self.dimMax), self.nbHDim, self.nbVDim, self.nbHPixels, self.nbVPixels, self.nbHPixels, float(self.viewportHAngle)*math.pi/180, float(self.viewportVAngle)*math.pi/180, self.pathToTracesHash)
    def OutputDir(self):
        return 'output_{}'.format(self.OutputDirId()) + ('/individualSeg_{:.6f}'.format(float(self.segmentDuration)) if (self.selectionPerSegment == 'true') else '')
    def PosHeatmapName(self, fullId=False):
        return 'heatmap_pos_solution_'+(self.OutputDirId() + '_{:.6f}_'.format(float(self.segmentDuration)) if fullId else '')+self.SolutionId()+'.txt'
    def DimHeatmapName(self, fullId=False):
        return 'heatmap_dim_solution_'+(self.OutputDirId() + '_{:.6f}_'.format(float(self.segmentDuration)) if fullId else '')+self.SolutionId()+'.txt'
    def LatexPlotName(self, fullId=False):
        return 'plot_heatmap_'+(self.OutputDirId() + '_{:.6f}_'.format(float(self.segmentDuration)) if fullId else '')+self.SolutionId()+'.tex'
    def PostprocessingName(self, fullId=False):
        return 'postprocessing_'+(self.OutputDirId() + '_{:.6f}_'.format(float(self.segmentDuration)) if fullId else '')+self.SolutionId()+'.txt'
    def PercentileName(self, fullId=False):
        return 'percentile_'+(self.OutputDirId() + '_{:.6f}_'.format(float(self.segmentDuration)) if fullId else '')+self.SolutionId()+'.txt'
    def RawQualityName(self, fullId=False):
        return 'rawQuality_'+(self.OutputDirId() + '_{:.6f}_'.format(float(self.segmentDuration)) if fullId else '')+self.SolutionId()+'.txt'
    def PathToPosHeatmap(self):
        return self.OutputDir() + '/'+self.PosHeatmapName()
    def PathToDimHeatmap(self):
        return self.OutputDir() + '/'+self.DimHeatmapName()
    def PathToLatexPlot(self):
        return self.OutputDir() + '/'+self.LatexPlotName()
    def PathToPostprocessing(self):
        return self.OutputDir() + '/'+self.PostprocessingName()
    @staticmethod
    def PostprocessingOutputLineComment():
        return 'segmentDuration nbQer minSegQuality minSurfaceBitrate maxSurfaceBitrate'
    def PostprocessingOutputLineId(self):
        return '{} {} {} {} {}'.format(self.segmentDuration, self.nbQer, self.minSegQuality, self.minSurfaceBitrate, self.maxSurfaceBitrate)
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run multiple optimal solutions');

    parser.add_argument('configFile', type=str, help='path to the configuration file')

    parser.add_argument('outputDir', type=str, help='path to outputDir' )

    args = parser.parse_args()

    outputDir = args.outputDir

    #read the configuration file
    config = configparser.ConfigParser()
    config.read(args.configFile)

    listOfOutputPdf = []

    with open(outputDir+'/mergedPostprocessing.txt', 'w') as postprocessingO:
        postprocessingO.write(SolverConfiguration.PostprocessingOutputLineComment()+' opti heur random\n')
        for selectionPerSegment in config['ManagerConfig']['selectionPerSegment'].split(','):
            for nbQer in config['ManagerConfig']['nbQer'].split(','):
                for segmentDuration in config['ManagerConfig']['segmentDuration'].split(','):
                    for minSurfaceBitrate in config['ManagerConfig']['minSurfaceBitrate'].split(','):
                        for maxSurfaceBitrate in config['ManagerConfig']['maxSurfaceBitrate'].split(','):
                            for minSegQuality in config['ManagerConfig']['minSegQuality'].split(','):
                                nbTheta = config['ManagerConfig']['nbTheta']
                                nbPhi = config['ManagerConfig']['nbPhi']
                                nbHDim = config['ManagerConfig']['nbHDim']
                                nbVDim = config['ManagerConfig']['nbVDim']
                                dimMin = config['ManagerConfig']['dimMin']
                                dimMax = config['ManagerConfig']['dimMax']
                                nbHPixels = config['ManagerConfig']['nbHPixels']
                                nbVPixels = config['ManagerConfig']['nbVPixels']
                                viewportHAngle = config['ManagerConfig']['viewportHAngle']
                                viewportVAngle = config['ManagerConfig']['viewportVAngle']
                                pathToTraces = config['ManagerConfig']['pathToTraces']
                                pathToTracesHash = config['ManagerConfig']['pathToTracesHash']
                                pathToPrecomputedIntersection = config['ManagerConfig']['pathToPrecomputedIntersection']
                                pathToPrecomputedSegments =  config['ManagerConfig']['pathToPrecomputedSegments']
                                solverConfig = SolverConfiguration(selectionPerSegment, nbQer, segmentDuration, minSurfaceBitrate, maxSurfaceBitrate, minSegQuality, nbTheta, nbPhi, nbHDim, nbVDim, dimMin, dimMax, nbHPixels, nbVPixels, viewportHAngle, viewportVAngle, pathToTraces, pathToTracesHash, pathToPrecomputedIntersection, pathToPrecomputedSegments)
                                with open('/tmp/tmpSolverConfig.ini', 'w') as o:
                                    o.write(solverConfig.GetConfigurationStr())
                                sub.call([config['ManagerConfig']['pathToPreprocessing'], '-c', '/tmp/tmpSolverConfig.ini'])
                                sub.call([config['ManagerConfig']['pathToPostprocessing'], '-c', '/tmp/tmpSolverConfig.ini'])
                                shutil.copyfile(solverConfig.PathToPosHeatmap(), '{}/{}'.format(outputDir, solverConfig.PosHeatmapName(True)))
                                shutil.copyfile(solverConfig.PathToDimHeatmap(), '{}/{}'.format(outputDir, solverConfig.DimHeatmapName(True)))
                                shutil.copyfile(solverConfig.PathToLatexPlot(), '{}/{}'.format(outputDir, solverConfig.LatexPlotName(True)))
                                shutil.copyfile(solverConfig.PathToPostprocessing(), '{}/{}'.format(outputDir, solverConfig.PostprocessingName(True)))
                                shutil.copyfile(solverConfig.PercentileName(), '{}/{}'.format(outputDir, solverConfig.PercentileName(True)))
                                shutil.copyfile(solverConfig.RawQualityName(), '{}/{}'.format(outputDir, solverConfig.RawQualityName(True)))
                                sub.call(['sed', '-i.bak', 's/{}/{}/g'.format(solverConfig.PosHeatmapName(), solverConfig.PosHeatmapName(True)), solverConfig.LatexPlotName(True)], cwd=outputDir)
                                sub.call(['sed', '-i.bak', 's/{}/{}/g'.format(solverConfig.DimHeatmapName(), solverConfig.DimHeatmapName(True)), solverConfig.LatexPlotName(True)], cwd=outputDir)
                                sub.call(['latexrun', solverConfig.LatexPlotName(True)], cwd=outputDir)
                                listOfOutputPdf.append('{}/{}.pdf'.format(outputDir, solverConfig.LatexPlotName(True)[:-4]))
                                #copy postprocessing output into one file
                                with open('{}/{}'.format(outputDir, solverConfig.PostprocessingName(True)), 'r') as i:
                                    for line in i:
                                        postprocessingO.write(solverConfig.PostprocessingOutputLineId()+' '+line)
    sub.call(['pdfunite']+listOfOutputPdf+['{}/mergedPlots.pdf'.format(outputDir)])
