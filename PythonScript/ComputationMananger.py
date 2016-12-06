#!/usr/bin/env python3

import argparse
import configparser
import os
import math

import numpy as np

import shutil

import subprocess as sub

class SolverConfiguration:
    def __init__(self, selectionPerSegment, nbQer, segmentDuration, minSurfaceBitrate, maxSurfaceBitrate, minSegQuality, bitrateRatio, nbTheta, nbPhi, nbHDim, nbVDim, dimMin, dimMax, nbHPixels, nbVPixels, viewportHAngle, viewportVAngle, pathToTraces, pathToTracesHash, pathToPrecomputedIntersection, pathToPrecomputedSegments):
        self.selectionPerSegment = selectionPerSegment
        self.nbQer = nbQer
        self.segmentDuration = segmentDuration
        self.minSurfaceBitrate = minSurfaceBitrate
        self.maxSurfaceBitrate = maxSurfaceBitrate
        self.minSegQuality = minSegQuality
        self.bitrateRatio = bitrateRatio
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
        s += 'selectionPerSegment={}\n'.format('true' if self.selectionPerSegment == '1' else 'false')
        s += 'selectionPerVideo={}\n'.format('true' if self.selectionPerSegment == '2' else 'false')
        s += 'nbQer={}\n'.format(self.nbQer)
        s += 'segmentDuration={}\n'.format(self.segmentDuration)
        s += 'minSurfaceBitrate={}\n'.format(self.minSurfaceBitrate)
        s += 'maxSurfaceBitrate={}\n'.format(self.maxSurfaceBitrate)
        s += 'minSegQuality={}\n'.format(self.minSegQuality)
        s += 'bitrateRatio={}\n'.format(self.bitrateRatio)
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
        return '{}_{:.6f}_{:.6f}_{:.6f}_{:.6f}_{:.6f}'.format(self.nbQer, float(self.segmentDuration), float(self.minSegQuality), float(self.minSurfaceBitrate), float(self.maxSurfaceBitrate), float(self.bitrateRatio))
    def OutputDirId(self):
        return '{:.6f}_{}_{}_{:.6f}_{:.6f}_{}_{}_{}_{}_{}_{:.6f}_{:.6f}_{}'.format(float(self.segmentDuration), self.nbTheta, self.nbPhi, float(self.dimMin), float(self.dimMax), self.nbHDim, self.nbVDim, self.nbHPixels, self.nbVPixels, self.nbHPixels, float(self.viewportHAngle)*math.pi/180, float(self.viewportVAngle)*math.pi/180, self.pathToTracesHash)
    def OutputDir(self):
        return 'output_{}'.format(self.OutputDirId()) + ('/individualSeg_{:.6f}'.format(float(self.segmentDuration)) if (self.selectionPerSegment == 1) else ('/individualVideo_{:.6f}'.format(float(self.segmentDuration)) if (self.selectionPerSegment == 2) else ''))
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
    def PathToPercentileName(self):
        return self.OutputDir() + '/'+self.PercentileName()
    def PathToRawQuality(self):
        return self.OutputDir() + '/'+self.RawQualityName()
    @staticmethod
    def PostprocessingOutputLineComment():
        return 'segmentDuration nbQer minSegQuality minSurfaceBitrate maxSurfaceBitrate'
    def PostprocessingOutputLineId(self):
        return '{} {} {} {} {}'.format(self.segmentDuration, self.nbQer, self.minSegQuality, self.minSurfaceBitrate, self.maxSurfaceBitrate)

def GetRawQualities(pathToRawQualities):
    opti = []
    heuri = []
    random = []
    with open(pathToRawQualities, 'r') as i:
        first = True
        for line in i:
            if first:
                first = False
            else:
                values = line.split(' ')
                opti.append(float(values[0]))
                heuri.append(float(values[1]))
                random.append(float(values[2]))
    return (opti, heuri, random)

def PrintCdfLatex(solverConfig):
    return """\\documentclass{article}
\\usepackage{pgfplots}
\\usepackage{xcolor}

\\pgfplotsset{compat=newest}

\\begin{document}
\\begin{figure}
 \\section{ Segment size: """+str(solverConfig.segmentDuration)+""" s; nbTheta = """+str(solverConfig.nbTheta)+""";
 nbPhi = """+str(solverConfig.nbPhi)+""" nbHdim = """+str(solverConfig.nbHDim)+""" ; nbVdim = """+str(solverConfig.nbVDim)+""" ; dimMin= """+str(solverConfig.dimMin)+""" ; dimMax = """+str(solverConfig.dimMax)+""" ; bMin = """+str(solverConfig.minSurfaceBitrate)+""" ; bMax = """+str(solverConfig.maxSurfaceBitrate)+""" ; Rb = """+str(solverConfig.bitrateRatio)+""" ; segmentQualityMin = """+str(solverConfig.segmentDuration)+""" ;}
\\begin{tikzpicture}
 \\pgfplotscreateplotcyclelist{My color list}{%
     {blue,solid, very thick},%
     {red,densely dashed, very thick},%
     {green,densely dotted, very thick},%
 }
 \\pgfplotsset{every axis legend/.append style={
         at={(0.05,0.97)},
 anchor=south west,
 draw=none,
 fill=none,
 legend columns=4,
 column sep=15pt,
 /tikz/every odd column/.append style={column sep=0cm},
 %font=\\tiny
 }}
\\begin{axis}[
  xlabel={visible bitrate $/$ $ S_{qer}$},
  ylabel={CDF},
  %xmin=0, xmax=180,
  ymin=0, ymax=1,
  y filter/.code={\\pgfmathparse{#1/100}\\pgfmathresult},
  cycle list name={My color list},
  legend cell align=left,
  ]
  \\addplot
  table [col sep=space, x expr=\\thisrow{optimalValues}, y expr=\\thisrow{cdf}] {"""+solverConfig.PercentileName(True)+"""};
  \\addlegendentry{Optimal}
  \\addplot
  table [col sep=space, x expr=\\thisrow{heuristicValues}, y expr=\\thisrow{cdf}] {"""+solverConfig.PercentileName(True)+"""};
  \\addlegendentry{QEC heuristic}
  \\addplot
  table [col sep=space, x expr=\\thisrow{heuristicRandomValues}, y expr=\\thisrow{cdf}] {"""+solverConfig.PercentileName(True)+"""};
  \\addlegendentry{Random}

  \\draw[dashed] (axis cs:1,0) -- (axis cs:1,1);
\\end{axis}
\\end{tikzpicture}
\\caption{Position generated}
\\end{figure}

\\end{document}"""

def PrintCdfGlobalLatex():
    return """\\documentclass{article}
\\usepackage{pgfplots}
\\usepackage{xcolor}

\\pgfplotsset{compat=newest}

\\begin{document}
\\begin{figure}
 \\section{ Global }
\\begin{tikzpicture}
 \\pgfplotscreateplotcyclelist{My color list}{%
     {blue,solid, very thick},%
     {red,densely dashed, very thick},%
     {green,densely dotted, very thick},%
 }
 \\pgfplotsset{every axis legend/.append style={
         at={(0.05,0.97)},
 anchor=south west,
 draw=none,
 fill=none,
 legend columns=4,
 column sep=15pt,
 /tikz/every odd column/.append style={column sep=0cm},
 %font=\\tiny
 }}
\\begin{axis}[
  xlabel={visible bitrate $/$ $ S_{qer}$},
  ylabel={CDF},
  %xmin=0, xmax=180,
  ymin=0, ymax=1,
  y filter/.code={\\pgfmathparse{#1/100}\\pgfmathresult},
  cycle list name={My color list},
  legend cell align=left,
  ]
  \\addplot
  table [col sep=space, x expr=\\thisrow{opti}, y expr=\\thisrow{cdf}] {globalPercentile.txt};
  \\addlegendentry{Optimal}
  \\addplot
  table [col sep=space, x expr=\\thisrow{heuri}, y expr=\\thisrow{cdf}] {globalPercentile.txt};
  \\addlegendentry{QEC heuristic}
  \\addplot
  table [col sep=space, x expr=\\thisrow{random}, y expr=\\thisrow{cdf}] {globalPercentile.txt};
  \\addlegendentry{Random}

  \\draw[dashed] (axis cs:1,0) -- (axis cs:1,1);
\\end{axis}
\\end{tikzpicture}
\\caption{Position generated}
\\end{figure}

\\end{document}"""


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
    optiRawQualities = []
    heuriRawQualities = []
    randomRawQualities = []

    with open(outputDir+'/mergedPostprocessing.txt', 'w') as postprocessingO:
        postprocessingO.write(SolverConfiguration.PostprocessingOutputLineComment()+' opti heur random\n')
        for selectionPerSegment in config['ManagerConfig']['selectionPerSegment'].split(','):
            for nbQer in config['ManagerConfig']['nbQer'].split(','):
                for segmentDuration in config['ManagerConfig']['segmentDuration'].split(','):
                    for bitrateRatio in config['ManagerConfig']['bitrateRatio'].split(','):
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
                                    solverConfig = SolverConfiguration(selectionPerSegment, nbQer, segmentDuration, minSurfaceBitrate, maxSurfaceBitrate, minSegQuality, bitrateRatio, nbTheta, nbPhi, nbHDim, nbVDim, dimMin, dimMax, nbHPixels, nbVPixels, viewportHAngle, viewportVAngle, pathToTraces, pathToTracesHash, pathToPrecomputedIntersection, pathToPrecomputedSegments)
                                    with open('/tmp/tmpSolverConfig.ini', 'w') as o:
                                        o.write(solverConfig.GetConfigurationStr())
                                    sub.call([config['ManagerConfig']['pathToPreprocessing'], '-c', '/tmp/tmpSolverConfig.ini'])
                                    sub.call([config['ManagerConfig']['pathToPostprocessing'], '-c', '/tmp/tmpSolverConfig.ini'])
                                    shutil.copyfile(solverConfig.PathToPosHeatmap(), '{}/{}'.format(outputDir, solverConfig.PosHeatmapName(True)))
                                    shutil.copyfile(solverConfig.PathToDimHeatmap(), '{}/{}'.format(outputDir, solverConfig.DimHeatmapName(True)))
                                    shutil.copyfile(solverConfig.PathToLatexPlot(), '{}/{}'.format(outputDir, solverConfig.LatexPlotName(True)))
                                    shutil.copyfile(solverConfig.PathToPostprocessing(), '{}/{}'.format(outputDir, solverConfig.PostprocessingName(True)))
                                    shutil.copyfile(solverConfig.PathToPercentileName(), '{}/{}'.format(outputDir, solverConfig.PercentileName(True)))
                                    shutil.copyfile(solverConfig.PathToRawQuality(), '{}/{}'.format(outputDir, solverConfig.RawQualityName(True)))
                                    rawQualities = GetRawQualities(solverConfig.PathToRawQuality())
                                    optiRawQualities += rawQualities[0]
                                    heuriRawQualities += rawQualities[1]
                                    randomRawQualities += rawQualities[2]
                                    sub.call(['sed', '-i.bak', 's/{}/{}/g'.format(solverConfig.PosHeatmapName(), solverConfig.PosHeatmapName(True)), solverConfig.LatexPlotName(True)], cwd=outputDir)
                                    sub.call(['sed', '-i.bak', 's/{}/{}/g'.format(solverConfig.DimHeatmapName(), solverConfig.DimHeatmapName(True)), solverConfig.LatexPlotName(True)], cwd=outputDir)
                                    sub.call(['latexrun', solverConfig.LatexPlotName(True)], cwd=outputDir)
                                    listOfOutputPdf.append('{}/{}.pdf'.format(outputDir, solverConfig.LatexPlotName(True)[:-4]))
                                    #copy postprocessing output into one file
                                    with open('{}/{}'.format(outputDir, solverConfig.PostprocessingName(True)), 'r') as i:
                                        for line in i:
                                            postprocessingO.write(solverConfig.PostprocessingOutputLineId()+' '+line)
                                    with open('{}/{}.tex'.format(outputDir, solverConfig.PercentileName(True)[:-4]), 'w') as o:
                                        o.write(PrintCdfLatex(solverConfig))
                                    sub.call(['latexrun', '{}.tex'.format(solverConfig.PercentileName(True)[:-4])], cwd=outputDir)
                                    listOfOutputPdf.append('{}/{}.pdf'.format(outputDir, solverConfig.PercentileName(True)[:-4]))

    with open('{}/{}'.format(outputDir, 'globalPercentile.txt'), 'w') as o:
        o.write('cdf opti heuri random\n')
        for i in range(0,101):
            opti = np.percentile(optiRawQualities, i)
            heuri = np.percentile(heuriRawQualities, i)
            random = np.percentile(randomRawQualities, i)
            o.write('{} {} {} {}\n'.format(i, opti, heuri, random))

    with open('{}/globalPercentile.tex'.format(outputDir), 'w') as o:
        o.write(PrintCdfGlobalLatex())
    sub.call(['latexrun', 'globalPercentile.tex'], cwd=outputDir)
    listOfOutputPdf.append('{}/globalPercentile.pdf'.format(outputDir))

    sub.call(['pdfunite']+listOfOutputPdf+['{}/mergedPlots.pdf'.format(outputDir)])
