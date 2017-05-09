#!/usr/bin/env python

import argparse
from matplotlib import rc
import matplotlib.pyplot as plt
#import matplotlib.patches as mpatches
import numpy as np

rc('text', usetex=True)
rc('font',**{'family':'serif','serif':['Computer Modern Roman']})

dot_formats = {0: 'bs', 1: 'rd', 2: 'go'}
fit_formats = {0: '-b', 1: '--r', 2: '-.g'}

def plot(xs, ys, i, filename):
    fit = np.polyfit(xs,ys,1)
    fit_fn = np.poly1d(fit)
    plt.plot(xs, ys, dot_formats[i], label=filename)
    plt.plot(xs, fit_fn(xs), fit_formats[i])

    legend = plt.legend(loc='upper left', shadow=True, fontsize='medium')


def parse_file(i, filename):
    xs = []
    ys = []
    with open(filename) as f:
        for line in f:
            ws = line.split()
            x = int(ws[0])
            y = float(ws[1])
            xs.append(x)
            ys.append(y)
    plot(xs, ys, i, filename)


parser = argparse.ArgumentParser()
parser.add_argument('ps', nargs='*')
args = parser.parse_args()
i = 0
for arg in args.ps:
    print arg
    parse_file(i, arg)
    i = i + 1

#plt.title('Comparing Compile Times')
#plt.xlabel('\#member accesses')
plt.xlabel('\#friends')
plt.ylabel('Time (s)')
#plt.show()

import os
cwd = os.getcwd()
basename = os.path.basename(os.path.normpath(cwd))
plt.savefig(basename + ".eps", format='eps', dpi=1000)
