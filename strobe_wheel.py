#!/usr/bin/env python
from __future__ import print_function
import argparse

PTS = [
    (1, 0, 0),
    (1, 1, 0),
    (0, 1, 0),
    (0, 1, 1),
    (0, 0, 1),
    (1, 0, 1),
]

def interpolate(pt_a, pt_b, wt_a):
    return tuple(a * wt_a + b * (1 - wt_a) for a, b in zip(pt_a, pt_b))

def scale(pt, scal):
    return tuple(int(a * scal) for a in pt)

def opposite(pt):
    return (1-pt[0], 1-pt[1], 1-pt[2])

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
    parser = argparse.ArgumentParser(description="Create a color wheel animation")
    parser.add_argument("--steps", type=int, default=64,
                        help="How many steps should each point in the color wheel get?")
    parser.add_argument("--step-length", type=int, default=1,
                        help="How many frames should each step be? (hint: use --steps instead)")
    parser.add_argument("--brightness", type=int, default=20, help="Maximum brightness per channel (0-255)")
    args = parser.parse_args()

    print(args)
    l1 = list(color_wheel(args.steps * len(PTS)))
    l2 = list(color_wheel(args.steps * len(PTS)))
    l2 = l2[len(l2)//2:] + l2[:len(l2)//2] 
    l = [x for pair in zip(l1, l2) for x in pair]
    for point in l:
        print(args.step_length, 0, ' '.join(str(x) for x in scale(point, args.brightness)))
