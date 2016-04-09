#!/usr/bin/env python
from __future__ import print_function
from __future__ import division
import argparse
import fileinput
import sys

def scale(pt, scal):
    return tuple(int(a * scal) for a in pt)

def color_wheel(steps):
    steps_per_point = steps / len(PTS)

    cur_point = 0

    for idx in range(steps):
        incr = idx % steps_per_point
        if incr == 0:
            cur_point += 1
            if cur_point >= len(PTS):
                # wrap around
                pt_a = PTS[cur_point - 1]
                pt_b = PTS[0]
            else:
                pt_a = PTS[cur_point - 1]
                pt_b = PTS[cur_point]

        wt_a = float(steps_per_point - incr) / steps_per_point
        yield interpolate(pt_a, pt_b, wt_a)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Crop/stretch animation to X leds")
    parser.add_argument("--step-length", type=float, default=1,
                        help="How much should the frame count of each step be multiplied by?")
    parser.add_argument("--brightness", type=float, default=1, help="How much should brightness be multiplied by?")
    parser.add_argument("--leds", type=int, default=4, help="How many LEDs to generate channels for? (def 4)")
    parser.add_argument('infile', nargs='?', type=argparse.FileType('r'), default=sys.stdin)
    args = parser.parse_args()

    for line in args.infile:
        line = line.strip()
        if line == 'BEGIN':
            # Remove BEGIN from legacy animations
            continue
        elif line == 'END':
            # Pass END through normally
            print(line)
        elif line.startswith('#') or line.startswith(' ') or line.startswith('\t'):
            # Skip comment lines
            continue
        else:
            steps, zero, rest = line.split(' ', 2)
            channels = rest.split(' ')

            # Scale frame count
            steps = int(steps)
            steps *= args.step_length
            steps = int(steps)

            # Scale channels
            channels = scale((int(x) for x in channels), args.brightness)

            # We need 3 channels per LED
            num_channels_needed = args.leds * 3

            too_many = 0
            while too_many * len(channels) < num_channels_needed:
                too_many += 1

            channels = (channels * too_many)[:num_channels_needed]

            print(steps, zero, ' '.join(str(int(x)) for x in channels))
