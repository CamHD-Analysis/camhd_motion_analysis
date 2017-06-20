#!/usr/bin/env python3

import json
import logging
import argparse
import glob

import pandas as pd
import numpy as np
import os
import os.path as path
from scipy import misc

import camhd_motion_analysis as ma
import pycamhd.lazycache as camhd

parser = argparse.ArgumentParser(description='')

parser.add_argument('input', metavar='N', nargs='+',
                    help='*_optical_flow_regions.json file to analyze')

# parser.add_argument('--base-dir', dest='basedir', metavar='o', nargs='?',
#                     help='Base directory')

parser.add_argument('--output-dir', dest='outdir', metavar='o', nargs='?', default="./",
                    help='File for output')

parser.add_argument('--log', metavar='log', nargs='?', default='INFO',
                    help='Logging level')

args = parser.parse_args()

logging.basicConfig( level=args.log.upper() )

lazycache_url = "https://camhd-app-dev-nocache.appspot.com/v1/org/oceanobservatories/rawdata/files/"
qt = camhd.lazycache( lazycache_url )

for input_path in args.input:

    # p = input_path
    # if args.basedir:
    #     p = args.basedir + p
    # else:
    #     p = input_path

    for infile in glob.iglob( input_path, recursive=True ):

        # if args.basedir:
        #     subpath = path.relpath(infile, args.basedir)
        # else:
        #     subpath = infile
        #
        # subpath = path.splitext( subpath )[0]

        logging.info( "Processing %s" % infile)

        regions_json = ma.load_regions( infile )
        mov_path = regions_json['movie']['URL']

        regions = pd.DataFrame( regions_json["regions"] ).drop('stats',1)

        static = regions[ regions.type == "static"]

        min_length = 30

        static["length"] = static.endFrame - static.startFrame
        static = static.loc[ static.length >= min_length ]

        avg_images = {}

        static = static.head(2)

        for idx,r in static.iterrows():

            logging.info("   Processing region from %d to %d" % (r.startFrame, r.endFrame) )

            samples = 5
            frames = range( r.startFrame, r.endFrame, round(r.length / (samples+1)) )
            frames = frames[1:-1]

            for f in frames:
                image_path = args.outdir + path.splitext(mov_path)[0] + ("/frame_%08d.png" % f)
                print(image_path)

                image = qt.get_frame( mov_path, f, timeout=30 )

                os.makedirs( path.dirname(image_path), exist_ok=True )

                misc.imsave( image_path, image )

            #images = [ qt.get_frame( mov_path, f, timeout=30 ) for f in frames ]



            # # Create a numpy array of floats to store the average (assume RGB images)
            # arr = np.zeros(images[0].shape,np.float)
            #
            # # Build up average pixel intensities, casting each image as an array of floats
            # for im in images:
            #      arr = arr+im
            # arr = arr / len(images)
            #
            # # Round values in array and cast as 8-bit integer
            # arr=np.array(np.round(arr),dtype=np.uint8)
            #
            # imshow(np.asarray(arr))
            #
            # avg_images[idx] = arr
