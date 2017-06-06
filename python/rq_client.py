#!/usr/bin/env python

import argparse

from redis import Redis
from rq import Queue

import os.path
import re

import pycamhd.lazycache as pycamhd

import camhd_motion_analysis as ma

DEFAULT_STRIDE=10

parser = argparse.ArgumentParser(description='Process a file using frame_stats.')

parser.add_argument('input', metavar='N', nargs='+',
                    help='Path to process')

parser.add_argument('--threads', metavar='j', type=int, nargs='?', default=1,
                    help='Number of threads to run with dask')

parser.add_argument('--stride', metavar='s', type=int, nargs='?', default=DEFAULT_STRIDE,
                    help='Stride for frame stats')

parser.add_argument('--output-file', dest='outfile', metavar='o', nargs='?', default="output.json",
                    help='File for output')

parser.add_argument('--output-dir', dest='outdir', metavar='o', nargs='?', default=".",
                    help='File for output')

parser.add_argument('--dry-run', dest='dryrun', action='count', help='Dry run')

parser.add_argument('--redis-url', dest='redis', default="redis://localhost:6379/1",
                    help='URL to Redis server')

args = parser.parse_args()


#mov_path = args.'/RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov'
## If output-file is specified, then input must be a single file
## otherwise output-dir must be set
#  and len(args.input) == 1:
#     filepairs = [ [args.input[0], args.outfile] ]
# else:
    ## First convert input args to list of files

def iterate_path( path ):
    repo = pycamhd.lazycache()
    dir_info = repo.get_dir( path )

    outfiles = []
    for f in dir_info['Files']:
        if re.search('\.mov$', f ):
            outfiles.append( path + f)

    for d in dir_info['Directories']:
        outfiles += iterate_path( path + d + "/" )

    return outfiles



filepairs = []

infiles = []
for f in args.input:
    if re.search('\.mov$', f ):
        infiles.append(f)
    else:
        print("Iterating on %s" % f)
        infiles += iterate_path( f )

## Generate infile->outfile pairs:
if len(infiles) == 0:
    exit
elif len(infiles) == 1:
    filepairs = [ [infiles[0], args.outfile] ]
else:
    filepairs = [[f,(args.outdir + f)] for f in infiles]



q = Queue(connection=Redis.from_url(args.redis))

for f in filepairs:
    print("Processing %s, Saving results to %s" % (f[0], f[1]) )

    if args.dryrun > 0:
        job = q.enqueue( ma.process_file,
                        f[0],
                    f[1],
                    num_threads=args.threads,
                    stride=args.stride,
                        timeout='2h',
                        ttl=3600*24 )
