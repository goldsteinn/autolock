#!/usr/bin/env python

import argparse
import csv
import sys
import re
import json
from pprint import pprint
import os

################################################################
# setup arguments
parser = argparse.ArgumentParser(description='process output of micro benchmark')
parser.add_argument("-v", "--verbosity", action="count", default=0, help="increase output verbosity")
parser.add_argument("-o", "--targetdir", default = ".", help="where to put all gnuplot files")
parser.add_argument("-w", "--warnIsError", action="store_true", help="don't die on warnings")
parser.add_argument("-b", "--skipBadUsage", action="store_true", help="if start usage is too high, skip data")
parser.add_argument("-a", "--againstBest", action="store_true", help="plot all against best")
parser.add_argument("inputs", nargs="+", help="file containing csv data")
flags = parser.parse_args()

verbose = flags.verbosity
warnIsError = not flags.warnIsError
inputs = flags.inputs
skipBadUsage = flags.skipBadUsage
againstBest = flags.againstBest
targetdir = flags.targetdir

################################################################
# known columns

known = {
    'lock': { 'name': 'lock', 'column': 'lock', 'help': 'name of lock' },
    'block': { 'name': 'lock', 'column': 'lock', 'help': 'name of lock second time in file' },
    'cpu_mask': { 'name': 'cpu_mask', 'column': 'cpu_mask', 'help': 'where threads are run' },
    'is_pinned': { 'name': 'is_pinned', 'column': 'is_pinned', 'help': 'whether threads are pinned or not' },
    'cs_iter': { 'name': 'cs_iter', 'column': 'cs_iter', 'help': 'cs_iter' },
    'extra_iter': { 'name': 'extra_iter', 'column': 'extra_iter', 'help': 'extra_iter' },
    'trials': { 'name': 'trials', 'column': 'trials', 'help': 'trials' },
    'outer_iter': { 'name': 'outer_iter', 'column': 'outer_iter', 'chkerr': True, 'help': 'outer_iter' },
    'nthreads': { 'name': 'nthreads', 'column': 'nthreads', 'chkerr': True, 'help': 'nthreads' },
    'ns_start': { 'name': 'ns_start', 'column': 'ns_start', 'chkerr': True, 'help': 'ns_start' },
    'ns_end': { 'name': 'ns_end', 'column': 'ns_end', 'chkerr': True, 'help': 'ns_end' },
    'cpu_usage_start': { 'name': 'cpu_usage_start', 'column': 'cpu_usage_start', 'chkerr': True, 'help': 'cpu_usage_start' },
    'cpu_usage_end': { 'name': 'cpu_usage_end', 'column': 'cpu_usage_end', 'chkerr': True, 'help': 'cpu_usage_end' },
    'min': { 'name': 'min', 'column': 'min', 'chkerr': True, 'help': 'min' },
    'max': { 'name': 'max', 'column': 'max', 'chkerr': True, 'help': 'max' },
    'geomean': { 'name': 'geomean', 'column': 'geomean', 'chkerr': True, 'help': 'geomean' },
    'stdev': { 'name': 'stdev', 'column': 'stdev', 'help': 'stdev' },
    'median': { 'name': 'median', 'column': 'median', 'help': 'median' },
    'p75': { 'name': 'p75', 'column': 'p75', 'help': 'p75' },
    'p90': { 'name': 'p90', 'column': 'p90', 'help': 'p90' },
    'p95': { 'name': 'p95', 'column': 'p95', 'help': 'p95' },
    'p99': { 'name': 'p99', 'column': 'p99', 'help': 'p99' }
}


################################################################
# help functions

def die(msg):
    sys.stderr.write(msg+"\n")
    exit(-1)

def vprint(msg, *args):
    if verbose:
        sys.stdout.write(msg+"\n")
        for arg in args:
            pprint(arg)

def wprint(inname, ln, msg, die=True):
    if inname != 0:
        sys.stderr.write("{}:{} {}\n".format(inname, ln, msg))
    else:
        sys.stderr.write("{}\n".format(msg))
    if warnIsError and die:
        exit(-1)

################################################################
# data about a run

class Run():
    def __init__(self, base, kind):
        self.base = base
        self.kind = kind
        self.runs = {}
        pass

    # add a run (data validated)
    def addrun(self, numProcs, cs_iter, extra_iter, outer_iter, cstart, cend, mincnt, maxcnt, geo):
        if numProcs not in self.runs:
            self.runs[numProcs] = {}
        key = "{}-{}".format(cs_iter, extra_iter)
        if key in self.runs[numProcs]:
            wprint(0, 0, "{} {} on {} procs with {} already exists".format(self.base, self.kind, numProcs, key))
        # add cnt scaled to number of outer_iter's
        self.runs[numProcs][key] = {'cstart': cstart, 'cend': cend, 'min': mincnt/outer_iter, 'max': maxcnt/outer_iter, 'geo': geo/outer_iter}

    # generate a list of keys to find the runs, sorted
    def genRunList(self):
        nprocs = sorted([int(x) for x in self.runs.keys()])
        return [(np, sorted(self.runs[np].keys())) for np in nprocs]
        
    # get the data for one run, None if doesn't exist
    def getRunData(self, nproc, oneiter):
        if nproc in self.runs and oneiter in self.runs[nproc]:
            return self.runs[nproc][oneiter]
        return None
        
    # get the cpu start usage for a run.  Already checked run exists
    def getCstart(self, nproc, oneiter):
        return self.runs[nproc][oneiter]['cstart']

    def __str__(self):
        return json.dumps({'lock base': self.base, 'lock kind': self.kind, 'runs': self.runs}, indent=2, sort_keys=True)
        
        
################################################################
# process an input file

def processFile(inname, infile):
    locks = {}
    reader = csv.DictReader(infile, dialect='excel')
    headers = reader.fieldnames
    # check headers are known
    warning = 0
    for name in headers:
        if name not in known:
            wprint(0, 0, "{} is an unknown column from {}".format(name, inname), die=False)
            warning += 1
        else:
            known[name]['checked'] = 1
    for name in known.keys():
        if known[name].get('checked', 0) != 1:
            wprint(0, 0, "{} is not in csv file: {}".format(name, inname), die=False)
            warning += 1
    if (warning > 0) and warnIsError:
        die("known fields and csv file '{}' don't match".format(inname))

    ln = 1
    for line in reader:
        ln += 1
        lockname = line['lock']
        if lockname == 'ERROR':
            wprint(inname, ln, "is lockname is ERROR", die=False)
            pprint(line)
            continue
        m = re.search("(.*)_([^_]*)", lockname)
        if m is None:
            wprint(inname, ln, "unknown lock format {}".format(lockname))
        base = m.group(1)
        kind = m.group(2)
        if kind == "mutex":
            print("{}:{} skipping mutex for now".format(inname, ln))
            continue

        # check for error
        hasErrors = False
        for fld in known.keys():
            if known[fld].get('chkerr', False):
                if line[fld] == 'ERROR':
                    wprint(inname, ln, "skipping {} {} because has ERROR in some needed columns", die=False)
                    hasErrors = True
                    break

        if hasErrors:
            continue

        # run data is valid, so add to database
        
        if base not in locks:
            locks[base] = {}
        if kind not in locks[base]:
            locks[base][kind] = Run(base, kind)

        locks[base][kind].addrun(int(line['nthreads']), 
                                 int(line['cs_iter']), int(line['extra_iter']), 
                                 int(line['outer_iter']),
                                 float(line['cpu_usage_start']), float(line['cpu_usage_end']),
                                 float(line['min']),
                                 float(line['max']),
                                 float(line['geomean']))

        if line['is_pinned'] != '1':
            wprint(inname, ln, "threads in run are not pinned: '{}'".format(line['is_pinned']))

    # check for comparable cpu usage at start only
    allprocs = {}
    alliters = {}
    for lock in sorted(locks.keys()):
        norm = locks[lock]['normlock']
        auto = locks[lock]['autolock']
        runlist = norm.genRunList()
        byiters = {}
        # check thay we have auto for each normal
        for (nproc, iters) in runlist:
            allprocs[nproc] = 1
            for oneiter in iters:
                alliters[oneiter] = 1
                if oneiter not in byiters:
                    byiters[oneiter] = []
                byiters[oneiter].append(nproc)
                autorun = auto.getRunData(nproc, oneiter)
                if autorun is None:
                    wprint(0, 0, "normal lock present, autolock absent for {} {} {}".format(lock, nproc, oneiter))
                bound = max(1, autorun['cstart']*.3)
                if abs(autorun['cstart'] - norm.getCstart(nproc, oneiter)) > bound:
                    if skipBadUsage:
                        die("Not implemented yet")
                    else:
                        wprint(0, 0, 
                               "cpu usage at start is very differet for {} {} {}: Normal:{}, Auto:{}".format(lock, nproc, oneiter, 
                                                                                                             autorun['cstart'], 
                                                                                                             norm.getCstart(nproc, oneiter)),
                               die=False)
    Run.alldata = locks
    Run.allprocs = allprocs
    Run.alliters = alliters
                    

# generate gnuplot files, one per lock comparing normal versus auto
# gp files and data files in same directory
def createGnuplotByLock(outpath):
    allprocs = Run.allprocs
    alliters = Run.alliters
    locks = Run.alldata
    outdata = []                # for potentially creating csv file
    pprint(allprocs)
    everyproc = sorted([int(x) for x in allprocs.keys()])
    line = [ 'lock', 'iters' ]
    for np in everyproc:
        line.extend([np, np, np])
    outdata.append(line)
    for lock in sorted(locks.keys()):
        norm = locks[lock]['normlock']
        auto = locks[lock]['autolock']
        # generate plot file
        gp = open("{}/out-{}.gp".format(outpath, lock), "w")
        gp.write("set title \"{} (norm/auto)\"\n".format(lock))
        gp.write("set logscale x 2\n");
        gp.write("set logscale y\n");
        gp.write("set xrange [1:256]\n");
        gp.write("set yrange [.1:10]\n");
        plotline = "plot 1"
        sepr = ", "
        # generate csv for now
        for oneiter in sorted(alliters.keys()):
            name = "out-{}-{}.gdata".format(lock, oneiter)
            plotline += sepr
            plotline += "'{}' with errorlines title '{}'".format(name, oneiter)
            sepr = ", "
            outf = open(os.path.join(outpath, name), "w")
            line = [ lock, oneiter ]
            for nproc in everyproc:
                normdata = norm.getRunData(nproc, oneiter)
                autodata = auto.getRunData(nproc, oneiter)
                if normdata is None or autodata is None:
                    line.append('-')
                    line.append('-')
                    line.append('-')
                else:
                    line.append(normdata['min']/autodata['min'])
                    line.append(normdata['geo']/autodata['geo'])
                    line.append(normdata['max']/autodata['max'])
                    outf.write("{}, {}, {}, {}\n".format(nproc, normdata['geo']/autodata['geo'], normdata['min']/autodata['min'], normdata['max']/autodata['max']))
            outf.close()
            outdata.append(line)
        gp.write("set terminal pdf\nset output '{}.pdf'\n".format(lock))
        gp.write(plotline+"\n")
        gp.close()
        
    # print(outdata)
    if False:
        with open("out.csv", "w") as outf:
            writer = csv.writer(outf, dialect='excel')
            for line in outdata:
                writer.writerow(line)

    return

    for lock in sorted(locks.keys()):
        for kind in sorted(locks[lock].keys()):
            # print("{}\t{}\t{}".format(lock, kind, locks[lock][kind].runs))
            print(locks[lock][kind])
            

################################################################
# plot for each number of procs, the ratio of each lock against the
# best one for that number of procs

def createGPbyProc(outpath):
    allprocs = Run.allprocs
    alliters = Run.alliters
    locks = Run.alldata
    everyproc = sorted([int(x) for x in allprocs.keys()])
    everyiter = sorted(alliters.keys())
    for nproc in everyproc:
        print(nproc)
        # plot per number of procs
        denoms = {}
        for oneiter in everyiter:
            # find best geo time for this iter
            best = None
            for lock in sorted(locks.keys()):
                for kind in ['normlock', 'autolock']:
                    run = locks[lock][kind]
                    rundata = run.getRunData(nproc, oneiter)
                    if rundata is None:
                        continue
                    gettime = rundata['geo']
                    if best is None or gettime < best:
                        best = gettime
                        denoms[oneiter] = run
        for x in denoms.keys():
            print("best {} - {} {}".format(x, denoms[x].base, denoms[x].kind))

        # now plot relative to best in each case

        # generate plot file
        gp = open("{}/out-{}.gp".format(outpath, nproc), "w")
        gp.write("set title \"{} (all/best)\"\n".format(nproc))
        gp.write("set logscale y\n");
        xgroup = (len(locks.keys())*2)+1
        xtotal = xgroup*len(everyiter)
        gp.write("set xrange [0:{}]\n".format(xtotal+1));
        gp.write("set yrange [.1:20]\n");
        gp.write("set style fill solid\n");
        gp.write("set boxwidth .9 relative\n");
        gp.write("set xtics rotate\n");
        gp.write("set xtics font \", 9\"\n");
        plotline = "plot 1"
        # generate the data files
        xoffset = 1
        for oneiter in everyiter:
            name = "out-{}-{}.gdata".format(nproc, oneiter)
            plotline += ", '{}' using 1:2:xticlabel(3) with boxes title '{}'".format(name, oneiter)
            outf = open(os.path.join(outpath, name), "w")
            bestdata = denoms[oneiter].getRunData(nproc, oneiter)
            for lock in sorted(locks.keys()):
                for kind in ['normlock', 'autolock']:
                    thisdata = locks[lock][kind].getRunData(nproc, oneiter)
                    if thisdata is None:
                        outf.write("{}, {}, {}-{}\n".format(xoffset, '-', lock, kind))
                    else:
                        outf.write("{}, {}, {}-{}\n".format(xoffset, thisdata['geo']/bestdata['geo'], lock, kind))
                    xoffset += 1
            outf.close()
            xoffset += 1
        gp.write("set terminal pdf\nset output '{}.pdf'\n".format(nproc))
        gp.write(plotline+"\n")
        gp.close()
    



################################################################
# process input file names

for name in inputs:
    with open(name, "r") as infile:
        processFile(name, infile)
        if not os.path.isdir(targetdir):
            os.mkdir(targetdir)
        if againstBest:
            createGPbyProc(targetdir)
        else:
            createGnuplotByLock(targetdir)
