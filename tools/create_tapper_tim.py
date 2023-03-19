#!/usr/bin/env python
# coding: utf-8

from __future__ import print_function
from subprocess import call
import os

src_bmp = [
    {
        'img': 'bg',
        'org': {'x': 512, 'y': 0},
        'clut': {'x': 0, 'y': 481}
    },
    {
        'img': 'menu',
        'org': {'x': 512, 'y': 0},
        'clut': {'x': 0, 'y': 482}
    },
    {
        'img': 'barman',
        'org': {'x': 768, 'y': 0},
        'clut': {'x': 0, 'y': 483}
    },
    {
        'img': 'tip',
        'org': {'x': 960, 'y': 128},
        'clut': {'x': 0, 'y': 484}
    },
    {
        'img': 'beer_st',
        'org': {'x': 974, 'y': 128},
        'clut': {'x': 0, 'y': 485}
    },
    {
        'img': 'beer_sh',
        'org': {'x': 880, 'y': 0},
        'clut': {'x': 0, 'y': 486}
    },
	{
		'img': 'pad',
		'org': {"x": 960, "y": 144},
        'clut': {'x': 0, 'y': 487}
	},
	{
		'img': 'd_sheet',
		'org': {"x": 768, "y": 256},
        'clut': {'x': 0, 'y': 488}
	},
	{
		'img': 'd_drink',
		'org': {"x": 768, "y": 168},
        'clut': {'x': 0, 'y': 489}
	}
]

_PATH = "{}/{}".format(os.environ.get('HOME'), 'tmp/tapper')
CONVERT_BIN = "{}/{}".format(_PATH, 'bmp2tim')

def convert():
    for item in src_bmp:
        call([
            CONVERT_BIN,
            "{}/{}.{}".format(_PATH, item['img'], 'bmp'),
            "{}/{}.{}".format(_PATH, item['img'], 'TIM'),
            '8',
            '-mpink',
            "-org={},{}".format(item['org']['x'], item['org']['y']),
            "-clut={},{}".format(item['clut']['x'], item['clut']['y'])
            ])
        print ("converted...{}".format(item['img']))
    return 0

convert()
print ("DONE")
